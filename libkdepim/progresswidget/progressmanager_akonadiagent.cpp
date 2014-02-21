/*
  progressmanager.cpp

  This file is part of libkdepim.

  Copyright (c) 2004 Till Adam <adam@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include "progressmanager.h"

#include "agentprogressmonitor.h"

#include <KDebug>

#include <Akonadi/AgentInstance>

using namespace Akonadi;

using namespace KPIM;

ProgressItem *ProgressManager::createProgressItemForAgent(ProgressItem *parent,
                                                           const Akonadi::AgentInstance &instance,
                                                           const QString &id,
                                                           const QString &label,
                                                           const QString &status,
                                                           bool cancellable,
                                                           ProgressItem::CryptoStatus cryptoStatus,
                                                          unsigned int progressType )
{
    const bool itemAlreadyExists = ( mTransactions.value( id ) != 0 );
    ProgressItem *t = createProgressItemImpl( parent, id, label, status, cancellable, cryptoStatus );
    t->setTypeProgressItem(progressType);
    // TODO ^ emits progressItemAdded() before I'm done connecting the signals.
    // Should I block that and emit it when I'm done?

    if (!itemAlreadyExists) {
        //    kDebug() << "Created ProgressItem for agent" << instance.name();
        new AgentProgressMonitor( instance, t );
    }
    return t;
}

