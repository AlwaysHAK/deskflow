/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "client/ServerProxy1_9.h"

#include "base/Log.h"
#include "deskflow/ProtocolTypes.h"
#include "deskflow/ProtocolUtil.h"

ServerProxy1_9::ServerProxy1_9(Client *client, deskflow::IStream *stream, IEventQueue *events)
    : ServerProxy1_8(client, stream, events)
{
  // do nothing
}

bool ServerProxy1_9::isGameLockSupported() const
{
  return true;
}

void ServerProxy1_9::sendGameLock(bool locked)
{
  LOG_DEBUG("send game lock: %s", locked ? "on" : "off");
  ProtocolUtil::writef(getStream(), kMsgCGameLock, locked ? 1 : 0);
}
