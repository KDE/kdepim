/*
    This file is part of KitchenSync.

    Copyright (c) 2002,2003 Holger Freyther <freyther@kde.org>

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
#ifndef KSYNC_KONNECTORMANAGER_H
#define KSYNC_KONNECTORMANAGER_H

#include <qobject.h>
#include <qstring.h>

#include <kstaticdeleter.h>
#include <kresources/manager.h>

#include <syncee.h>
#include <synceelist.h>

#include "filter.h"
#include "konnector.h"

namespace KSync {
class Konnector;
class KonnectorInfo;
}

using KSync::Konnector;

class KonnectorManager : public QObject, public KRES::Manager<Konnector>
{
  Q_OBJECT

  public:
    KonnectorManager();
    ~KonnectorManager();

    void connectSignals();

    void readConfig( KConfig* );
    void writeConfig( KConfig* );

  public slots:
    void emitFinished();

  signals:
    /**
      Emitted when Syncee list becomes available as response to
      requestSyncees().
     */
    void synceesRead( KSync::Konnector* );

    /**
      Emitted when an error occurs during read.
     */
    void synceeReadError( KSync::Konnector* );

    /**
      Emitted when Syncee list was successfully written back to connected
      entity.
     */
    void synceesWritten( KSync::Konnector* );

    /**
      Emitted when an error occurs during write.
     */
    void synceeWriteError( KSync::Konnector* );

    /**
      Emitted when the synchronization has finished.
     */
    void syncFinished();

  private:
    class Private;
    Private *d;
};

#endif
