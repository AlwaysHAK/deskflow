/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#pragma once

#include <set>
#include <string>

namespace deskflow::server {

/**
 * @brief Keeps track of why the cursor must stay on the active computer.
 *
 * The manual lock (scroll lock, the lock cursor action or the default lock state) and game
 * lock are kept apart, so neither can clear the other. A game lock belongs to the computer
 * that reported it and only holds the cursor while that computer is the active one.
 */
class CursorLock
{
public:
  void setManual(bool locked);
  bool isManual() const;

  /**
   * @brief Set the game lock reported by \p computer.
   * @return true if this changed the lock
   */
  bool setGameLock(const std::string &computer, bool locked);
  bool isGameLocked(const std::string &computer) const;

  /**
   * @brief true iff the cursor must stay on \p activeComputer
   */
  bool isLocked(const std::string &activeComputer) const;

private:
  bool m_manual = false;
  std::set<std::string, std::less<>> m_gameLocked;
};

} // namespace deskflow::server
