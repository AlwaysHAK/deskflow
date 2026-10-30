/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "server/CursorLock.h"

namespace deskflow::server {

void CursorLock::setManual(bool locked)
{
  m_manual = locked;
}

bool CursorLock::isManual() const
{
  return m_manual;
}

bool CursorLock::setGameLock(const std::string &computer, bool locked)
{
  if (locked) {
    return m_gameLocked.insert(computer).second;
  }
  return m_gameLocked.erase(computer) != 0;
}

bool CursorLock::isGameLocked(const std::string &computer) const
{
  return m_gameLocked.contains(computer);
}

bool CursorLock::isLocked(const std::string &activeComputer) const
{
  return m_manual || isGameLocked(activeComputer);
}

} // namespace deskflow::server
