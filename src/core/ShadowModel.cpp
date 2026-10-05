#include "ShadowModel.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace dogma
{
    namespace
    {
        [[nodiscard]] bool PositiveFinite(double value) noexcept
        {
            return std::isfinite(value) && value > 0.0;
        }
    }

    bool ValidateConfig(const ModelConfig& config) noexcept
    {
        return PositiveFinite(config.sprint_drain_per_second) &&
               PositiveFinite(config.recovery_per_second) &&
               PositiveFinite(config.recovery_threshold_fraction) &&
               config.recovery_threshold_fraction <= 1.0 &&
               PositiveFinite(config.max_step_seconds);
    }

    std::string_view StateName(MovementState state) noexcept
    {
        switch (state) {
        case MovementState::Unavailable:
            return "unavailable";
        case MovementState::Ready:
            return "ready";
        case MovementState::Sprinting:
            return "sprinting";
        case MovementState::Exhausted:
            return "exhausted";
        }
        return "unknown";
    }

    ShadowModel::ShadowModel(ModelConfig config) : config_(config)
    {
        if (!ValidateConfig(config_)) {
            throw std::invalid_argument("Invalid shadow movement/stamina configuration");
        }
    }

    bool ShadowModel::Reset(double observed_stamina, double observed_max_stamina) noexcept
    {
        Suspend();
        if (!std::isfinite(observed_stamina) || !PositiveFinite(observed_max_stamina)) {
            return false;
        }

        const auto threshold = observed_max_stamina * config_.recovery_threshold_fraction;
        const auto recover_time = threshold / config_.recovery_per_second;
        const auto drain_time = threshold / config_.sprint_drain_per_second;
        if (!PositiveFinite(threshold) || !PositiveFinite(recover_time) ||
            !PositiveFinite(drain_time) || !PositiveFinite(recover_time + drain_time)) {
            return false;
        }

        snapshot_.stamina = std::clamp(observed_stamina, 0.0, observed_max_stamina);
        snapshot_.max_stamina = observed_max_stamina;
        snapshot_.state = snapshot_.stamina > 0.0 ? MovementState::Ready : MovementState::Exhausted;
        return true;
    }

    void ShadowModel::Suspend() noexcept
    {
        snapshot_ = {};
    }

    ModelSnapshot ShadowModel::Step(const Observation& observation) noexcept
    {
        if (!observation.available || !std::isfinite(observation.dt_seconds) ||
            observation.dt_seconds < 0.0 || observation.dt_seconds > config_.max_step_seconds) {
            Suspend();
            return snapshot_;
        }
        if (snapshot_.state == MovementState::Unavailable) {
            return snapshot_;
        }

        snapshot_.simulated_seconds += observation.dt_seconds;
        if (!std::isfinite(snapshot_.simulated_seconds)) {
            Suspend();
            return snapshot_;
        }

        const auto threshold = snapshot_.max_stamina * config_.recovery_threshold_fraction;
        if (!observation.sprinting) {
            // Division before multiplication prevents overflow with a large
            // valid rate while saturating exactly at capacity.
            const auto time_to_full = (snapshot_.max_stamina - snapshot_.stamina) / config_.recovery_per_second;
            snapshot_.stamina = observation.dt_seconds >= time_to_full ? snapshot_.max_stamina :
                snapshot_.stamina + config_.recovery_per_second * observation.dt_seconds;
            if (snapshot_.state != MovementState::Exhausted || snapshot_.stamina >= threshold) {
                snapshot_.state = MovementState::Ready;
            }
            return snapshot_;
        }

        if (snapshot_.state != MovementState::Exhausted) {
            snapshot_.state = MovementState::Sprinting;
        }

        auto remaining = observation.dt_seconds;
        // Resolve transitions inside a sample instead of charging a whole
        // frame to whichever state happened to be active at its start.
        while (remaining > 0.0) {
            if (snapshot_.state == MovementState::Exhausted) {
                if (snapshot_.stamina == 0.0) {
                    // Skip complete recovery/drain cycles in constant time.
                    // This also bounds work for extreme but valid rates.
                    const auto cycle = threshold / config_.recovery_per_second +
                                       threshold / config_.sprint_drain_per_second;
                    if (remaining >= cycle) {
                        remaining = std::fmod(remaining, cycle);
                        if (remaining == 0.0) {
                            break;
                        }
                    }
                }
                const auto time_to_ready = std::max(0.0, threshold - snapshot_.stamina) / config_.recovery_per_second;
                if (remaining < time_to_ready) {
                    snapshot_.stamina += config_.recovery_per_second * remaining;
                    break;
                }
                snapshot_.stamina = threshold;
                snapshot_.state = MovementState::Sprinting;
                remaining -= time_to_ready;
            } else {
                const auto time_to_empty = snapshot_.stamina / config_.sprint_drain_per_second;
                if (remaining < time_to_empty) {
                    snapshot_.stamina -= config_.sprint_drain_per_second * remaining;
                    break;
                }
                snapshot_.stamina = 0.0;
                snapshot_.state = MovementState::Exhausted;
                remaining -= time_to_empty;
            }
        }
        return snapshot_;
    }

    const ModelSnapshot& ShadowModel::GetSnapshot() const noexcept
    {
        return snapshot_;
    }

    const ModelConfig& ShadowModel::GetConfig() const noexcept
    {
        return config_;
    }
}
