#include "core/Continuity.h"
#include "core/ShadowModel.h"

#include <cmath>
#include <iostream>
#include <limits>
#include <string_view>

namespace
{
    int checks = 0, failures = 0;
    void Check(bool result, std::string_view message)
    {
        ++checks;
        if (!result) { ++failures; std::cerr << "FAIL: " << message << '\n'; }
    }
    void Reason(const dogma::ContinuitySample& previous, const dogma::ContinuitySample& current,
        double dt, std::string_view expected, double max_dt = 0.5)
    {
        const auto result = dogma::ClassifyContinuity(previous, current, dt, max_dt);
        Check(result.reset_reason && result.reset_reason == expected, expected);
        Check(!result.cell_crossing && result.horizontal_speed == 0 && result.spatial_speed == 0,
            "rejected observations have no valid movement output");
    }
}

int main()
{
    const dogma::ContinuitySample original{ 10, 20, false, 0, 0, 0, 100 };
    auto next = original;
    next.cell = 11;
    next.x = 30;
    next.z = 40;
    auto result = dogma::ClassifyContinuity(original, next, 0.1, 0.5);
    Check(!result.reset_reason && result.cell_crossing, "same-worldspace exterior crossing continues");
    Check(std::abs(result.horizontal_speed - 300) < 1e-8, "horizontal speed uses shared coordinates");
    Check(std::abs(result.spatial_speed - 500) < 1e-8, "vertical displacement contributes to safety guard");
    next.cell = original.cell;
    Check(!dogma::ClassifyContinuity(original, next, 0.1, 0.5).cell_crossing, "ordinary sample is not a crossing");
    next.worldspace = 21;
    Reason(original, next, 0.1, "space_change");
    next = original;
    next.interior = true;
    Reason(original, next, 0.1, "space_change");
    auto interior = original;
    interior.interior = true;
    interior.worldspace = 0;
    next = interior;
    next.cell = 11;
    Reason(interior, next, 0.1, "cell_change");
    Check(!dogma::ClassifyContinuity(interior, interior, 0.1, 0.5).reset_reason,
        "walking inside the same interior continues");
    auto unknown = original;
    unknown.worldspace = 0;
    next = unknown;
    next.cell = 11;
    Reason(unknown, next, 0.1, "cell_change");
    next = original;
    next.cell = 11;
    next.x = 201;
    Reason(original, next, 0.1, "position_discontinuity");
    next.x = 0;
    next.z = 201;
    Reason(original, next, 0.1, "position_discontinuity");
    next.z = 0;
    next.x = 200;
    Check(!dogma::ClassifyContinuity(original, next, 0.1, 0.5).reset_reason,
        "exact speed bound is permitted");
    next = original;
    for (double dt : { 0.0, -0.1, 0.500001, std::numeric_limits<double>::infinity(),
        std::numeric_limits<double>::quiet_NaN() }) {
        Reason(original, next, dt, "sample_gap");
    }
    Check(!dogma::ClassifyContinuity(original, next, 0.5, 0.5).reset_reason,
        "exact interval limit is permitted");
    Reason(original, next, 0.1, "sample_gap", 0);
    next.capacity = 101;
    Reason(original, next, 0.1, "capacity_change");
    next = original;
    next.cell = 0;
    Reason(original, next, 0.1, "invalid_sample");
    next = original;
    next.z = std::numeric_limits<double>::quiet_NaN();
    Reason(original, next, 0.1, "invalid_sample");
    next = original;
    next.capacity = 0;
    Reason(original, next, 0.1, "invalid_sample");
    next = original;
    next.x = std::numeric_limits<double>::max();
    Reason(original, next, 0.1, "position_discontinuity");
    Reason(next, original, 0.1, "position_discontinuity");

    // Regression: repeated city crossings must not rebase the independent
    // model to the native stamina value, even if that value remains 100.
    dogma::ShadowModel model;
    Check(model.Reset(100, 100), "integration baseline initialized");
    auto previous = original;
    for (int i = 0; i < 20; ++i) {
        next = previous;
        next.cell = previous.cell == 10 ? 11 : 10;
        next.x += 30;
        result = dogma::ClassifyContinuity(previous, next, 0.1, 0.5);
        Check(!result.reset_reason && result.cell_crossing, "repeated crossing retains continuity");
        const auto snapshot = model.Step({ true, true, 0.1 });
        Check(std::abs(snapshot.simulated_seconds - (i + 1) * 0.1) < 1e-8,
            "model time increases across boundaries");
        Check(std::abs(snapshot.stamina - (100 - (i + 1) * 1.5)) < 1e-8,
            "model drains without rebasing to native stamina");
        previous = next;
    }
    next = previous;
    next.worldspace = 99;
    result = dogma::ClassifyContinuity(previous, next, 0.1, 0.5);
    if (result.reset_reason) { model.Suspend(); }
    Check(model.GetSnapshot().state == dogma::MovementState::Unavailable,
        "unsafe crossing suspends prediction");
    Check(model.Reset(83, 100), "unsafe crossing takes fresh native baseline");
    Check(model.GetSnapshot().simulated_seconds == 0 && model.GetSnapshot().stamina == 83,
        "fresh baseline discards pre-transition model state");

    std::cout << checks << " checks, " << failures << " failures\n";
    return failures == 0 ? 0 : 1;
}
