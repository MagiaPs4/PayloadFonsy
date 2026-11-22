/* Copyright (C) 2026 FonSyHen / LightningMods */

#pragma once

#include <cstdint>
#include <string_view>
#include <sys/types.h>

namespace onion::daemon::app_lifecycle {

/** Start the serial lifecycle dispatcher before the SceSysCore listener. */
bool start();
void stop();

/** Called by the SceSysCore kqueue listener after Big App identification. */
bool publish_big_app_started(pid_t pid, uint32_t app_id,
                             std::string_view title_id);
bool publish_big_app_exited(pid_t pid, uint32_t app_id,
                            std::string_view title_id);

} // namespace onion::daemon::app_lifecycle
