/*
    This file is part of KitchenSync.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef KSYNC_ENGINE_H
#define KSYNC_ENGINE_H

#include <konnector.h>
#include <syncer.h>

#include <qobject.h>
#include <qptrlist.h>

class KonnectorManager;
class KonnectorPair;

namespace KSync {

class SyncUiKde;

/**
  This class provides the engine for the syncing process. It's responsible for
  control of the action flow through Konnectors and ActionParts. It handles
  reading and writing of Syncees by the Konnectors and triggers the actions of
  the ActionParts in the correct sequence.
*/
class Engine : public QObject
{
  Q_OBJECT

  public:
    Engine();
    ~Engine();

    static QString progressId();

    void go( KonnectorPair *pair );
    void setResolveStrategy( int strategy );

  protected:
    void logMessage( const QString& );
    void logError( const QString& );

    void tryExecuteActions();
    void executeActions();

    void tryFinish();
    void finish();

    void disconnectDevice( Konnector *k );

  protected slots:
    void slotSynceesRead( KSync::Konnector * );
    void slotSynceeReadError( KSync::Konnector * );
    void slotSynceesWritten( KSync::Konnector * );
    void slotSynceeWriteError( KSync::Konnector * );

  signals:
    void error( const QString& );
    void doneSync();

  private:
    void doSync();

    Konnector::List mOpenedKonnectors;
    Konnector::List mProcessedKonnectors;
    uint mKonnectorCount;

    Konnector::List mKonnectors;
    KonnectorManager *mManager;

    Syncer mCalendarSyncer;
    Syncer mAddressBookSyncer;

    SyncUi *mSyncUi;
};

}

#endif
