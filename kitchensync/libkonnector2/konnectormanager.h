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

#include "konnector.h"
#include "kdevice.h"
#include "filter.h"
#include "error.h"
#include "progress.h"

namespace KSync
{

class Kapabilities;
class ConfigWidget;
class Konnector;
class KonnectorInfo;

class KonnectorManager : public QObject, public KRES::Manager<Konnector>
{
    Q_OBJECT
    friend class KStaticDeleter<KonnectorManager>;
  public:
    static KonnectorManager *self();

    Device::ValueList query();
    Konnector *load( const Device& device );
    Konnector *load( const QString& deviceName );
    bool unload( Konnector * );

    bool autoLoadFilter() const;

    /**
     * Set whether or not to load
     * Filters automatically
     */
    void setAutoLoadFilter( bool = true );

    /**
     * adds a custom filter
     * a custom filter overwrites
     * automatic loaded filters
     */
    void add( Filter* filter );
    void deleteFilter( Filter* filter );
    const Filter::PtrList filters();

  public slots:
    void write( Konnector *, const SynceeList & );

  signals:
    /**
      Emitted when Syncee list becomes available as response to
      requestSyncees().
    */
    void synceesRead( Konnector * );

    /**
      Emitted when an error occurs during read.
    */
    void synceeReadError( Konnector * );

    /**
      Emitted when Syncee list was successfully written back to connected
      entity.
    */
    void synceesWritten( Konnector * );

    /**
      Emitted when an error occurs during write.
    */
    void synceeWriteError( Konnector * );

  signals:
    void sync( Konnector *, const SynceeList & );
    void progress( Konnector *, const Progress & );
    void error( Konnector *, const Error & );
    void downloaded( Konnector *, const SynceeList & );

  private slots:
    void slotSync( Konnector *, const SynceeList & );
    void slotProgress( Konnector *, const Progress & );
    void slotError( Konnector *, const Error & );
    void slotDownloaded( Konnector *, const SynceeList & );

  protected:
    void connectSignals();

  private:
    void filter( const SynceeList &unknown, const SynceeList &real );

    KonnectorManager();
    ~KonnectorManager();

    Device::ValueList allDevices();
    Device parseDevice( const QString& path );
    Device find( const QString& deviceName );
    SynceeList findUnknown( const SynceeList & );

    static KonnectorManager *m_self;

    Filter::PtrList m_filter;
    Filter::PtrList m_filAdded;
    bool m_auto;

    Device::ValueList m_devices;

    QPtrList<Konnector> m_konnectors;

    class Private;
    Private *d;
};

}

#endif
