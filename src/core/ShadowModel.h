#pragma once

#include <string_view>

namespace dogma
{
    enum class MovementState
    {
        Unavailable,
        Ready,
        Sprinting,
        Exhausted
    };

    // Experimental values, not measured Dragon's Dogma constants.
    struct ModelConfig
    {
        double sprint_drain_per_second{ 15.0 };
        double recovery_per_second{ 10.0 };
        double recovery_threshold_fraction{ 0.25 };
        double max_step_seconds{ 0.5 };
    };

    [[nodiscard]] bool ValidateConfig(const ModelConfig& config) noexcept;
    [[nodiscard]] std::string_view StateName(MovementState state) noexcept;

    struct Observation
    {
        bool available{ false };
        bool sprinting{ false };
        double dt_seconds{ 0.0 };
    };

    struct ModelSnapshot
    {
        MovementState state{ MovementState::Unavailable };
        double stamina{ 0.0 };
        double max_stamina{ 0.0 };
        double simulated_seconds{ 0.0 };
    };

    // Owns only a prediction. It has no game objects or game-writing callbacks.
    // Call Reset when a new continuous observation begins; later samples do not
    // resynchronize stamina with Skyrim. Suspended/invalid samples require Reset.
    class ShadowModel
    {
    public:
        // Invalid configuration throws std::invalid_argument.
        explicit ShadowModel(ModelConfig config = {});

        // Non-finite values, non-positive capacity, or unrepresentable cycle
        // durations are rejected. Finite stamina is clamped to [0, capacity].
        [[nodiscard]] bool Reset(double observed_stamina, double observed_max_stamina) noexcept;
        void Suspend() noexcept;

        // dt == 0 updates the requested state without consuming/recovering
        // stamina. Negative/non-finite/excessive dt invalidates continuity.
        [[nodiscard]] ModelSnapshot Step(const Observation& observation) noexcept;
        [[nodiscard]] const ModelSnapshot& GetSnapshot() const noexcept;
        [[nodiscard]] const ModelConfig& GetConfig() const noexcept;

    private:
        ModelConfig config_;
        ModelSnapshot snapshot_;
    };
}
