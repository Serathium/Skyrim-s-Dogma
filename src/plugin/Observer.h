#pragma once

#include "Settings.h"
#include "core/Continuity.h"
#include <atomic>
#include <chrono>
#include <cstdint>
#include <string>
#include <thread>

namespace SKSE { class TaskInterface; }

namespace dogma
{
    // One process-lifetime instance. SKSE does not hot-unload this DLL. Deliberately
    // not destroyed during CRT/loader-lock teardown; Windows reclaims it at exit.
    class Observer
    {
    public:
        Observer(Settings settings, const SKSE::TaskInterface* tasks);
        void Start();
        void SetSession(bool active);

    private:
        void Sample(std::uint64_t generation);
        void Suspend(const char* reason);

        Settings settings_;
        ShadowModel model_;
        const SKSE::TaskInterface* tasks_;
        std::thread worker_;
        std::atomic<bool> active_{ false };
        std::atomic<bool> pending_{ false };
        std::atomic<std::uint64_t> generation_{ 0 };

        // Below: accessed on the game task thread only.
        std::uint64_t sampled_generation_{ 0 };
        std::uint64_t sequence_{ 0 };
        ContinuitySample previous_sample_{};
        bool continuous_{ false };
        std::chrono::steady_clock::time_point previous_time_{};
        std::chrono::steady_clock::time_point last_log_{};
        std::string suspended_reason_;
    };
}
