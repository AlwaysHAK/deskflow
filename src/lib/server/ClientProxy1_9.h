/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#pragma once

#include "base/Event.h"
#include "server/ClientProxy1_8.h"

//! Proxy for client implementing protocol version 1.9
class ClientProxy1_9 : public ClientProxy1_8
{
public:
  //! Game lock reported by the client
  class GameLockInfo : public EventData
  {
  public:
    explicit GameLockInfo(bool locked) : m_locked(locked)
    {
      // do nothing
    }
    ~GameLockInfo() override = default;

  public:
    bool m_locked;
  };

  ClientProxy1_9(const std::string &name, deskflow::IStream *adoptedStream, Server *server, IEventQueue *events);
  ~ClientProxy1_9() override = default;

protected:
  bool parseMessage(const uint8_t *code) override;

private:
  bool recvGameLock();

  IEventQueue *m_events;
};
