#include "Settings.h"

#include <REX/W32/KERNEL32.h>
#include <spdlog/spdlog.h>
#include <array>
#include <charconv>
#include <cmath>
#include <string>

namespace dogma
{
    namespace
    {
        double ReadNumber(const std::filesystem::path& path, const wchar_t* section,
            const wchar_t* key, const char* label, double fallback, double minimum, double maximum, bool integer = false)
        {
            std::array<wchar_t, 128> buffer{};
            const auto count = REX::W32::GetPrivateProfileStringW(section, key, L"", buffer.data(),
                static_cast<std::uint32_t>(buffer.size()), path.c_str());
            if (count == 0) {
                return fallback;
            }
            std::string value;
            for (std::uint32_t i = 0; i < count; ++i) {
                value.push_back(buffer[i] <= 127 ? static_cast<char>(buffer[i]) : '?');
            }
            double parsed{};
            const auto result = std::from_chars(value.data(), value.data() + value.size(), parsed);
            if (result.ec != std::errc{} || result.ptr != value.data() + value.size() ||
                !std::isfinite(parsed) || parsed < minimum || parsed > maximum ||
                (integer && std::floor(parsed) != parsed) || count >= buffer.size() - 1) {
                spdlog::warn("config_invalid key={} fallback={}", label, fallback);
                return fallback;
            }
            return parsed;
        }
    }

    Settings ReadSettings(const std::filesystem::path& path)
    {
        Settings settings;
        if (!std::filesystem::exists(path)) {
            spdlog::warn("config_missing using_defaults=true");
        }
        settings.enabled = ReadNumber(path, L"Observer", L"Enabled", "Enabled", 1, 0, 1, true) == 1;
        settings.sample_interval = std::chrono::milliseconds(static_cast<int>(ReadNumber(path,
            L"Observer", L"SampleIntervalMs", "SampleIntervalMs", 100, 50, 250, true)));
        settings.log_interval = std::chrono::milliseconds(static_cast<int>(ReadNumber(path,
            L"Observer", L"LogIntervalMs", "LogIntervalMs", 1000, 250, 10000, true)));
        settings.model.sprint_drain_per_second = ReadNumber(path, L"ShadowModel", L"SprintDrainPerSecond",
            "SprintDrainPerSecond", 15, 0.1, 1000);
        settings.model.recovery_per_second = ReadNumber(path, L"ShadowModel", L"RecoveryPerSecond",
            "RecoveryPerSecond", 10, 0.1, 1000);
        settings.model.recovery_threshold_fraction = ReadNumber(path, L"ShadowModel", L"RecoveryThresholdFraction",
            "RecoveryThresholdFraction", 0.25, 0.01, 1);
        return settings;
    }
}
