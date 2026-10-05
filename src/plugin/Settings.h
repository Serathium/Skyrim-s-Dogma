#pragma once

#include "core/ShadowModel.h"
#include <chrono>
#include <filesystem>

namespace dogma
{
    struct Settings
    {
        bool enabled{ true };
        std::chrono::milliseconds sample_interval{ 100 };
        std::chrono::milliseconds log_interval{ 1000 };
        ModelConfig model;
    };

    [[nodiscard]] Settings ReadSettings(const std::filesystem::path& path);
}
