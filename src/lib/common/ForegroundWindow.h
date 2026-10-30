/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#pragma once

#include <cstdint>
#include <string>

namespace deskflow {

/**
 * @brief What the platform reports about the window in the foreground.
 */
struct ForegroundWindow
{
  /// false when nothing that could be a game is in front: no window, the desktop or shell,
  /// a minimized or hidden window, or one of our own windows
  bool present = false;

  /// true when the window covers the whole monitor it is on
  bool fullscreen = false;

  /// executable file name, e.g. "eldenring.exe", empty when the process could not be
  /// queried (for example an elevated or anti-cheat protected game)
  std::string executable;

  /// platform window handle, only used to tell windows apart
  std::uintptr_t window = 0;
};

} // namespace deskflow
