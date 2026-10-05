#include "Observer.h"

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>
#include <cmath>

namespace dogma
{
    namespace
    {
        using Clock = std::chrono::steady_clock;

        const char* UnavailableReason()
        {
            auto* ui = RE::UI::GetSingleton();
            if (!ui) { return "no_ui"; }
            if (ui->GameIsPaused() || ui->IsItemMenuOpen() || ui->IsApplicationMenuOpen() ||
                ui->IsModalMenuOpen() || ui->IsMenuOpen(RE::DialogueMenu::MENU_NAME) ||
                ui->IsMenuOpen(RE::MainMenu::MENU_NAME) || ui->IsMenuOpen(RE::LoadingMenu::MENU_NAME) ||
                ui->IsMenuOpen(RE::Console::MENU_NAME) || ui->IsMenuOpen(RE::RaceSexMenu::MENU_NAME)) {
                return "menu";
            }
            auto* camera = RE::PlayerCamera::GetSingleton();
            if (!camera || !camera->IsInThirdPerson()) { return "camera"; }
            return nullptr;
        }
    }

    Observer::Observer(Settings settings, const SKSE::TaskInterface* tasks) :
        settings_(settings), model_(settings.model), tasks_(tasks)
    {}

    void Observer::Start()
    {
        // This worker ONLY schedules work. No RE objects are accessed here.
        worker_ = std::thread([this] {
            for (;;) {
                std::this_thread::sleep_for(settings_.sample_interval);
                if (!active_.load() || pending_.exchange(true)) { continue; }
                const auto generation = generation_.load();
                try {
                    tasks_->AddTask([this, generation] {
                        struct PendingGuard
                        {
                            std::atomic<bool>& pending;
                            ~PendingGuard() { pending.store(false); }
                        } guard{ pending_ };
                        try {
                            Sample(generation);
                        } catch (const std::exception& error) {
                            active_.store(false);
                            spdlog::error("observer_disabled exception={}", error.what());
                        }
                    });
                } catch (...) {
                    pending_.store(false);
                    active_.store(false);
                }
            }
        });
    }

    void Observer::SetSession(bool active)
    {
        active_.store(false);
        generation_.fetch_add(1);
        active_.store(active);
    }

    void Observer::Suspend(const char* reason)
    {
        model_.Suspend();
        continuous_ = false;
        if (suspended_reason_ != reason) {
            spdlog::info("suspend reason={} shadow_state=unavailable", reason);
            suspended_reason_ = reason;
        }
    }

    void Observer::Sample(std::uint64_t generation)
    {
        if (!active_.load() || generation != generation_.load()) { return; }
        const auto started = Clock::now();
        if (sampled_generation_ != generation) {
            sampled_generation_ = generation;
            Suspend("session_reset");
        }
        if (const auto* reason = UnavailableReason()) { Suspend(reason); return; }
        auto* player = RE::PlayerCharacter::GetSingleton();
        if (!player || !player->Is3DLoaded()) { Suspend("player_unavailable"); return; }
        auto* state = player->AsActorState();
        if (state->GetLifeState() != RE::ACTOR_LIFE_STATE::kAlive) { Suspend("not_alive"); return; }
        if (player->IsOnMount()) { Suspend("mounted"); return; }
        if (state->GetSitSleepState() != RE::SIT_SLEEP_STATE::kNormal) { Suspend("furniture"); return; }
        if (state->GetKnockState() != RE::KNOCK_STATE_ENUM::kNormal || state->IsStaggered()) {
            Suspend("hit_reaction"); return;
        }
        if (state->IsSwimming()) { Suspend("swimming"); return; }
        if (player->IsInMidair()) { Suspend("airborne"); return; }
        auto* cell = player->GetParentCell();
        if (!cell) { Suspend("no_cell"); return; }

        const auto cell_id = cell->GetFormID();
        const bool interior = cell->IsInteriorCell();
        // Read only on the game thread; retain IDs, never a worldspace pointer.
        const auto* worldspace = interior ? nullptr : cell->GetRuntimeData().worldSpace;
        const auto worldspace_id = worldspace ? worldspace->GetFormID() : 0;
        const auto position = player->GetPosition();
        const double stamina = player->AsActorValueOwner()->GetActorValue(RE::ActorValue::kStamina);
        const double maximum = player->GetActorValueMax(RE::ActorValue::kStamina);
        if (!std::isfinite(position.x) || !std::isfinite(position.y) || !std::isfinite(position.z) ||
            !std::isfinite(stamina) || !std::isfinite(maximum) || maximum <= 0) {
            Suspend("invalid_sample"); return;
        }
        const auto sprinting = state->IsSprinting();
        const auto moving = player->IsMoving();
        const auto dt = std::chrono::duration<double>(started - previous_time_).count();
        double horizontal_speed = 0;
        bool speed_valid = false;
        const ContinuitySample current_sample{ cell_id, worldspace_id, interior,
            position.x, position.y, position.z, maximum };
        ContinuityResult continuity;
        if (continuous_) {
            auto baseline_sample = previous_sample_;
            // Compare capacity with the model baseline, not just the last
            // sample, so gradual changes cannot accumulate without a reset.
            baseline_sample.capacity = model_.GetSnapshot().max_stamina;
            continuity = ClassifyContinuity(baseline_sample, current_sample, dt, settings_.model.max_step_seconds);
            if (continuity.reset_reason) { Suspend(continuity.reset_reason); }
            else {
                horizontal_speed = continuity.horizontal_speed;
                speed_valid = true;
            }
        }

        const auto old_state = model_.GetSnapshot().state;
        if (!continuous_) {
            if (!model_.Reset(stamina, maximum)) { Suspend("invalid_baseline"); return; }
            spdlog::info("resume cell=0x{:08X} worldspace=0x{:08X} interior={} actual_stamina={:.3f} max_stamina={:.3f} mode=observation_only", cell_id, worldspace_id, interior, stamina, maximum);
            suspended_reason_.clear();
        }
        const auto before = model_.GetSnapshot();
        const auto shadow = model_.Step({ true, sprinting, continuous_ ? dt : 0.0 });
        if (continuous_ && continuity.cell_crossing) {
            spdlog::info("cell_crossing continuity=preserved from=0x{:08X} to=0x{:08X} worldspace=0x{:08X} dt={:.6f} spatial_speed={:.3f} shadow_seconds_before={:.6f} shadow_seconds_after={:.6f} shadow_stamina_before={:.3f} shadow_stamina_after={:.3f} actual_stamina={:.3f}",
                previous_sample_.cell, cell_id, worldspace_id, dt, continuity.spatial_speed,
                before.simulated_seconds, shadow.simulated_seconds, before.stamina, shadow.stamina, stamina);
        }
        previous_sample_ = current_sample;
        previous_time_ = started;
        continuous_ = true;
        ++sequence_;
        const auto read_us = std::chrono::duration<double, std::micro>(Clock::now() - started).count();
        if (shadow.state != old_state || started - last_log_ >= settings_.log_interval) {
            spdlog::info("sample seq={} cell=0x{:08X} worldspace=0x{:08X} interior={} pos=({:.2f},{:.2f},{:.2f}) actual_stamina={:.3f} max_stamina={:.3f} observed_sprint={} observed_move={} horizontal_speed={:.3f} speed_valid={} shadow_state={} shadow_stamina={:.3f} shadow_seconds={:.3f} read_us={:.1f}",
                sequence_, cell_id, worldspace_id, interior, position.x, position.y, position.z, stamina, maximum, sprinting, moving,
                horizontal_speed, speed_valid, StateName(shadow.state), shadow.stamina, shadow.simulated_seconds, read_us);
            last_log_ = started;
        }
    }
}
