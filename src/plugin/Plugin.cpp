#include "Observer.h"
#include "Settings.h"

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>
#include <spdlog/sinks/rotating_file_sink.h>

// PluginDeclaration's unused VersionNumber entries default to 1.0.0 in the
// pinned CommonLib. Use the zero-initialized SKSE metadata form so the binary
// advertises exactly one runtime, including a zero terminator.
SKSE_PLUGIN_VERSION = []() constexpr {
    SKSE::PluginVersionData data;
    data.PluginVersion({ 0, 1, 1, 0 });
    data.PluginName("SkyrimsDogma");
    data.AuthorName("Serathium");
    data.CompatibleVersions({ SKSE::RUNTIME_SSE_1_7_104 });
    data.MinimumRequiredXSEVersion({ 2, 3, 1, 0 });
    return data;
}();

SKSE_PLUGIN_QUERY(const SKSE::QueryInterface* skse, SKSE::PluginInfo* info)
{
    if (!skse || !info) { return false; }
    info->infoVersion = SKSE::PluginInfo::kVersion;
    info->name = "SkyrimsDogma";
    info->version = REL::Version{ 0, 1, 1, 0 }.pack();
    return !skse->IsEditor() && skse->RuntimeVersion() == SKSE::RUNTIME_SSE_1_7_104;
}

namespace
{
    dogma::Observer* observer = nullptr;

    void OnMessage(SKSE::MessagingInterface::Message* message)
    {
        if (!message) { return; }
        using Messages = SKSE::MessagingInterface;
        switch (message->type) {
        case Messages::kDataLoaded:
            spdlog::info("lifecycle=data_loaded");
            break;
        case Messages::kPreLoadGame:
            if (observer) { observer->SetSession(false); }
            spdlog::info("lifecycle=pre_load_game observation=paused");
            break;
        case Messages::kPostLoadGame: {
            // SKSE casts the success bool to void*. It is NOT a bool pointer.
            const bool success = message->data != nullptr;
            if (observer) { observer->SetSession(success); }
            spdlog::info("lifecycle=post_load_game success={}", success);
            break;
        }
        case Messages::kNewGame:
            if (observer) { observer->SetSession(true); }
            spdlog::info("lifecycle=new_game");
            break;
        case Messages::kSaveGame:
            spdlog::info("lifecycle=save_game plugin_save_data=false");
            break;
        default:
            break;
        }
    }
}

SKSE_PLUGIN_LOAD(const SKSE::LoadInterface* skse)
{
    // Gate before initializing CommonLib or accessing any engine singleton.
    if (!skse || skse->IsEditor() || skse->RuntimeVersion() != SKSE::RUNTIME_SSE_1_7_104 ||
        skse->SKSEVersion() < REL::Version{ 2, 3, 1, 0 }.pack()) {
        return false;
    }
    bool listener_registered = false;
    try {
        SKSE::Init(skse, SKSE::InitInfo{ .log = false });
        const auto logs = SKSE::log::log_directory();
        if (!logs) { return false; }
        std::filesystem::create_directories(*logs);
        auto sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            (*logs / "SkyrimsDogma.log").string(), 5 * 1024 * 1024, 2, true);
        auto logger = std::make_shared<spdlog::logger>("SkyrimsDogma", std::move(sink));
        logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
        logger->set_level(spdlog::level::info);
        logger->flush_on(spdlog::level::info);
        spdlog::set_default_logger(std::move(logger));
        spdlog::info("plugin_loaded name=SkyrimsDogma version={} runtime=1.7.104.0 skse_packed=0x{:08X} mode=observation_only commonlib=94faaed0c60eddd8347767f2d4d29a97c93bde8c", DOGMA_VERSION, skse->SKSEVersion());

        const auto game_root = std::filesystem::path(REL::Module::get().filePath().data()).parent_path();
        const auto settings = dogma::ReadSettings(game_root / "Data/SKSE/Plugins/SkyrimsDogma.ini");
        spdlog::info("config enabled={} sample_ms={} log_ms={} shadow_drain={} shadow_recovery={} shadow_threshold={} timebase=wall_seconds",
            settings.enabled, settings.sample_interval.count(), settings.log_interval.count(),
            settings.model.sprint_drain_per_second, settings.model.recovery_per_second, settings.model.recovery_threshold_fraction);
        const auto* messaging = SKSE::GetMessagingInterface();
        const auto* tasks = SKSE::GetTaskInterface();
        if (!messaging || !tasks || !messaging->RegisterListener(OnMessage)) {
            spdlog::error("plugin_failed reason=required_skse_interface");
            return false;
        }
        // SKSE retains registered listeners even if Load returns false and
        // unloads the DLL. From this point onward this DLL must stay loaded.
        listener_registered = true;
        if (settings.enabled) {
            observer = new dogma::Observer(settings, tasks);
            observer->Start();
        }
        spdlog::info("plugin_ready gameplay_writes=false hot_unload_supported=false");
        return true;
    } catch (const std::exception& error) {
        if (observer) { observer->SetSession(false); }
        spdlog::error("plugin_failed exception={}", error.what());
        return listener_registered;
    }
}
