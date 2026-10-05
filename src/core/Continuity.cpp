#include "Continuity.h"

#include <cmath>

namespace dogma
{
    ContinuityResult ClassifyContinuity(const ContinuitySample& previous,
        const ContinuitySample& current, double seconds, double max_seconds) noexcept
    {
        const auto valid = [](const ContinuitySample& sample) {
            return sample.cell != 0 && std::isfinite(sample.x) && std::isfinite(sample.y) &&
                std::isfinite(sample.z) && std::isfinite(sample.capacity) && sample.capacity > 0;
        };
        if (!valid(previous) || !valid(current)) { return { "invalid_sample" }; }
        if (!std::isfinite(seconds) || !std::isfinite(max_seconds) || max_seconds <= 0 ||
            seconds <= 0 || seconds > max_seconds) { return { "sample_gap" }; }
        if (previous.interior != current.interior || previous.worldspace != current.worldspace) {
            return { "space_change" };
        }
        const bool crossing = previous.cell != current.cell;
        if (crossing && (current.interior || current.worldspace == 0)) { return { "cell_change" }; }
        if (std::abs(previous.capacity - current.capacity) > 0.01) { return { "capacity_change" }; }
        const double dx = current.x - previous.x;
        const double dy = current.y - previous.y;
        const double dz = current.z - previous.z;
        const double horizontal_speed = std::hypot(dx, dy) / seconds;
        const double spatial_speed = std::hypot(dx, dy, dz) / seconds;
        if (!std::isfinite(spatial_speed) || spatial_speed > 2000) {
            return { "position_discontinuity" };
        }
        return { nullptr, crossing, horizontal_speed, spatial_speed };
    }
}
