#pragma once

#include <cstdint>

namespace dogma
{
    // Copied observations only; never contains engine pointers.
    struct ContinuitySample
    {
        std::uint32_t cell{ 0 };
        std::uint32_t worldspace{ 0 };
        bool interior{ false };
        double x{ 0 }, y{ 0 }, z{ 0 };
        double capacity{ 0 };
    };

    struct ContinuityResult
    {
        // nullptr means continuous; a stable label explains any reset.
        const char* reset_reason{ nullptr };
        bool cell_crossing{ false };
        double horizontal_speed{ 0 };
        double spatial_speed{ 0 };
    };

    [[nodiscard]] ContinuityResult ClassifyContinuity(const ContinuitySample& previous,
        const ContinuitySample& current, double seconds, double max_seconds) noexcept;
}
