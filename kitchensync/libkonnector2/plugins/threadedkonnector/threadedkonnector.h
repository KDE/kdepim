/*
    This file is part of KitchenSync.

    Copyright (c) 2003 Mathias Froehlich <Mathias.Froehlich@web.de>

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

#ifndef THREADED_KONNECTOR_H
#define THREADED_KONNECTOR_H

#include <qiconset.h>
#include <qptrlist.h>

#include <konnector.h>

#include "clientmanager.h"

namespace Threaded {

  /**
   * This plugin gets loaded by the KonnectorManager
   * this is the key to the KonnectorWorld
   * we need to implement the interface to fully support it...
   */
  class ThreadedPlugin
    : public KSync::Konnector {
    Q_OBJECT;
    public:
    ThreadedPlugin( const KConfig *config );
    ~ThreadedPlugin();

    /** return our capabilities() */
    KSync::Kapabilities capabilities();

    /**
     * the user configured this konnector
     * apply his preferecnes
     */
    void setCapabilities( const KSync::Kapabilities& );

    SynceeList syncees();

    bool readSyncees();
    bool writeSyncees();

    bool connectDevice();
    bool disconnectDevice();

    /** the state and some informations */
    KSync::KonnectorInfo info() const;

    /** download a resource/url/foobar */
    void download( const QString& );

  protected:
/*     QString metaId() const; */
/*     QIconSet iconSet() const; */
/*     QString iconName() const; */

  private slots:
    void slotFinished();
    void slotError( const KSync::Error& );
    void slotProgress( const KSync::Progress& );

  private:

    ClientManager mClientManager;
  };

} // namespace Threaded

#endif
