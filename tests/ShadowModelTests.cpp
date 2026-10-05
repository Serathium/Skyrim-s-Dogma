#include "core/ShadowModel.h"

#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string_view>

namespace
{
    int failures = 0;
    int checks = 0;

    void Check(bool result, std::string_view message)
    {
        ++checks;
        if (!result) {
            ++failures;
            std::cerr << "FAIL: " << message << '\n';
        }
    }

    void Near(double actual, double expected, std::string_view message, double tolerance = 1e-8)
    {
        Check(std::isfinite(actual) && std::abs(actual - expected) <= tolerance, message);
    }

    dogma::ModelConfig Config()
    {
        return { 20.0, 10.0, 0.25, 60.0 };
    }

    void InitializationAndSuspension()
    {
        dogma::ShadowModel model(Config());
        Check(model.GetSnapshot().state == dogma::MovementState::Unavailable, "new model unavailable");
        Check(model.Step({ true, true, 0.1 }).state == dogma::MovementState::Unavailable, "step cannot initialize model");
        Check(model.Reset(80.0, 100.0), "valid reset succeeds");
        auto sample = model.Step({ true, true, 1.0 });
        Check(sample.state == dogma::MovementState::Sprinting, "observed sprint drives predicted state");
        Near(sample.stamina, 60.0, "stamina drains from initial observed value");
        Near(sample.simulated_seconds, 1.0, "elapsed shadow time advances");
        sample = model.Step({ false, true, 1.0 });
        Check(sample.state == dogma::MovementState::Unavailable, "unavailable sample suspends model");
        Near(sample.stamina, 0.0, "suspension discards stale stamina");
        Near(sample.max_stamina, 0.0, "suspension discards stale capacity");
        Near(sample.simulated_seconds, 0.0, "suspension resets elapsed time");
        Check(model.Step({ true, true, 1.0 }).state == dogma::MovementState::Unavailable, "resume needs new baseline");
        Check(model.Reset(40.0, 200.0), "new observation resets capacity and baseline");
        sample = model.Step({ true, false, 1.0 });
        Near(sample.stamina, 50.0, "recovery uses new baseline");
        model.Suspend();
        Check(model.GetSnapshot().state == dogma::MovementState::Unavailable, "explicit suspend clears model");
    }

    void HysteresisAndBoundaryTransitions()
    {
        dogma::ShadowModel model(Config());
        Check(model.Reset(20.0, 100.0), "positive initial stamina is usable below threshold");
        auto sample = model.Step({ true, true, 1.0 });
        Check(sample.state == dogma::MovementState::Exhausted, "exact depletion enters exhausted");
        Near(sample.stamina, 0.0, "depletion clamps at zero");
        sample = model.Step({ true, true, 2.0 });
        Check(sample.state == dogma::MovementState::Exhausted, "held sprint cannot exit exhaustion below threshold");
        Near(sample.stamina, 20.0, "exhaustion recovers despite held sprint");
        sample = model.Step({ true, true, 0.5 });
        Check(sample.state == dogma::MovementState::Sprinting, "exact threshold resumes held sprint");
        Near(sample.stamina, 25.0, "threshold crossing does not lose a frame of stamina");
        sample = model.Step({ true, true, 0.5 });
        Check(sample.state == dogma::MovementState::Sprinting, "running below recovery threshold does not re-exhaust");
        Near(sample.stamina, 15.0, "hysteresis drains while sprinting below threshold");
        sample = model.Step({ true, false, 0.5 });
        Check(sample.state == dogma::MovementState::Ready, "released sprint becomes ready");
        Near(sample.stamina, 20.0, "ready recovers stamina");

        Check(model.Reset(0.0, 100.0), "zero initial stamina accepted");
        sample = model.Step({ true, false, 2.4 });
        Check(sample.state == dogma::MovementState::Exhausted, "release does not bypass recovery threshold");
        sample = model.Step({ true, false, 0.2 });
        Check(sample.state == dogma::MovementState::Ready, "released sprint recovers to ready");
        Near(sample.stamina, 26.0, "released sprint recovers across threshold for whole timestep");
        sample = model.Step({ true, false, 60.0 });
        Near(sample.stamina, 100.0, "recovery saturates at observed maximum");

        Check(model.Reset(10.0, 100.0), "reset for transition inside sample");
        sample = model.Step({ true, true, 1.0 });
        Check(sample.state == dogma::MovementState::Exhausted, "depletion partway through sample enters exhausted");
        Near(sample.stamina, 5.0, "remaining time after depletion contributes recovery");
        Check(model.Reset(0.0, 100.0), "reset for recovery and drain in same sample");
        sample = model.Step({ true, true, 3.0 });
        Check(sample.state == dogma::MovementState::Sprinting, "recovery partway through sample resumes sprint");
        Near(sample.stamina, 15.0, "remaining time after recovery contributes drain");
        Check(model.Reset(0.0, 100.0), "reset for exact complete cycle");
        sample = model.Step({ true, true, 3.75 });
        Check(sample.state == dogma::MovementState::Exhausted, "exact full cycle ends exhausted");
        Near(sample.stamina, 0.0, "exact full cycle ends at zero");
    }

    dogma::ModelSnapshot Simulate(int frequency)
    {
        dogma::ShadowModel model(Config());
        Check(model.Reset(100.0, 100.0), "framerate model reset succeeds");
        for (int i = 0; i < frequency * 23; ++i) {
            const auto sample = model.Step({ true, true, 1.0 / frequency });
            Check(sample.stamina >= 0.0 && sample.stamina <= sample.max_stamina, "stamina remains in bounds throughout simulation");
        }
        return model.GetSnapshot();
    }

    void TimestepIndependence()
    {
        dogma::ShadowModel model(Config());
        Check(model.Reset(100.0, 100.0), "large-step model reset succeeds");
        const auto reference = model.Step({ true, true, 23.0 });
        for (const int frequency : { 1, 30, 60, 144 }) {
            const auto sample = Simulate(frequency);
            Check(sample.state == reference.state, "state agrees across sample frequencies");
            Near(sample.stamina, reference.stamina, "stamina agrees across exhaustion/recovery cycles");
            Near(sample.simulated_seconds, 23.0, "elapsed time agrees across sample frequencies");
        }

        dogma::ModelConfig extreme = Config();
        extreme.sprint_drain_per_second = 1e12;
        extreme.recovery_per_second = 1e12;
        dogma::ShadowModel fast(extreme);
        Check(fast.Reset(0.0, 100.0), "large finite rates accepted");
        const auto sample = fast.Step({ true, true, 1.0 });
        Check(sample.state != dogma::MovementState::Unavailable && std::isfinite(sample.stamina), "many cycles complete with bounded work");
        Check(sample.stamina >= 0.0 && sample.stamina <= 25.0, "fast-cycle stamina remains inside cycle bounds");
    }

    void InvalidInputAndConfiguration()
    {
        const auto nan = std::numeric_limits<double>::quiet_NaN();
        const auto inf = std::numeric_limits<double>::infinity();
        dogma::ShadowModel model;
        for (const auto dt : { -0.1, nan, inf, 0.50001 }) {
            Check(model.Reset(50.0, 100.0), "reset before invalid timestep");
            Check(model.Step({ true, true, dt }).state == dogma::MovementState::Unavailable, "invalid or delayed sample invalidates model");
        }
        Check(model.Reset(50.0, 100.0), "reset before exact allowed timestep");
        Check(model.Step({ true, true, 0.5 }).state == dogma::MovementState::Sprinting, "maximum timestep is inclusive");
        const auto previous = model.GetSnapshot().stamina;
        Near(model.Step({ true, false, 0.0 }).stamina, previous, "zero dt does not alter stamina");
        Check(model.GetSnapshot().state == dogma::MovementState::Ready, "zero dt updates observed state");

        Check(!model.Reset(nan, 100.0), "NaN initial stamina rejected");
        Check(!model.Reset(inf, 100.0), "infinite initial stamina rejected");
        for (const auto capacity : { 0.0, -1.0, nan, inf }) {
            Check(!model.Reset(50.0, capacity), "invalid capacity rejected");
            Check(model.GetSnapshot().state == dogma::MovementState::Unavailable, "failed reset leaves no old prediction");
        }
        Check(model.Reset(-10.0, 100.0), "negative finite stamina clamps");
        Near(model.GetSnapshot().stamina, 0.0, "negative stamina clamped to zero");
        Check(model.GetSnapshot().state == dogma::MovementState::Exhausted, "clamped zero starts exhausted");
        Check(model.Reset(200.0, 100.0), "over-capacity finite stamina clamps");
        Near(model.GetSnapshot().stamina, 100.0, "over-capacity stamina clamped to maximum");

        Check(dogma::ValidateConfig({}), "default configuration valid");
        for (const auto invalid : { 0.0, -1.0, nan, inf }) {
            auto config = Config();
            config.sprint_drain_per_second = invalid;
            Check(!dogma::ValidateConfig(config), "invalid drain rejected");
            config = Config();
            config.recovery_per_second = invalid;
            Check(!dogma::ValidateConfig(config), "invalid recovery rejected");
            config = Config();
            config.recovery_threshold_fraction = invalid;
            Check(!dogma::ValidateConfig(config), "invalid threshold rejected");
            config = Config();
            config.max_step_seconds = invalid;
            Check(!dogma::ValidateConfig(config), "invalid max timestep rejected");
        }
        auto config = Config();
        config.recovery_threshold_fraction = 1.1;
        Check(!dogma::ValidateConfig(config), "threshold beyond capacity rejected");
        bool threw = false;
        try {
            const dogma::ShadowModel invalid(config);
        } catch (const std::invalid_argument&) {
            threw = true;
        }
        Check(threw, "invalid configuration cannot construct a model");
        Check(dogma::StateName(dogma::MovementState::Unavailable) == "unavailable", "stable unavailable log label");
        Check(dogma::StateName(dogma::MovementState::Exhausted) == "exhausted", "stable exhausted log label");
    }
}

int main()
{
    InitializationAndSuspension();
    HysteresisAndBoundaryTransitions();
    TimestepIndependence();
    InvalidInputAndConfiguration();
    std::cout << checks << " checks, " << failures << " failures\n";
    return failures == 0 ? 0 : 1;
}
