/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "server/ClientProxy1_9.h"

#include "base/IEventQueue.h"
#include "base/Log.h"
#include "deskflow/ProtocolUtil.h"

#include <cstring>

ClientProxy1_9::ClientProxy1_9(const std::string &name, deskflow::IStream *stream, Server *server, IEventQueue *events)
    : ClientProxy1_8(name, stream, server, events),
      m_events(events)
{
  // do nothing
}

bool ClientProxy1_9::parseMessage(const uint8_t *code)
{
  if (memcmp(code, kMsgCGameLock, 4) == 0) {
    return recvGameLock();
  }
  return ClientProxy1_8::parseMessage(code);
}

bool ClientProxy1_9::recvGameLock()
{
  uint8_t locked = 0;
  if (!ProtocolUtil::readf(getStream(), kMsgCGameLock + 4, &locked)) {
    return false;
  }

  LOG_DEBUG("recv game lock from \"%s\": %s", getName().c_str(), locked != 0 ? "on" : "off");
  m_events->addEvent(Event(EventTypes::ClientProxyGameLockChanged, getEventTarget(), new GameLockInfo(locked != 0)));
  return true;
}
