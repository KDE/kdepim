/* This file is part of the KDE libraries
    Copyright (C) 2002,2003 Holger Freyther <freyther@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

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

class KonnectorManager : public QObject
{
    Q_OBJECT
    friend class KStaticDeleter<KonnectorManager>;
  public:
    static KonnectorManager *self();

    Device::ValueList query();
    Konnector *load( const Device& device );
    Konnector *load( const QString& deviceName );
    bool unload( Konnector * );

    ConfigWidget *configWidget( Konnector *, QWidget *parent,
                                const char *name );
    ConfigWidget *configWidget( Konnector *,
                                const Kapabilities &,
                                QWidget *parent, const char *name );

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
    void write( Konnector *, const Syncee::PtrList& );

  signals:
    void sync( Konnector *, Syncee::PtrList );
    void progress( Konnector *, const Progress & );
    void error( Konnector *, const   Error & );
    void downloaded( Konnector *, Syncee::PtrList );

  private slots:
    void slotSync( Konnector *, Syncee::PtrList );
    void slotProgress( Konnector *, const Progress & );
    void slotError( Konnector *, const Error & );
    void slotDownloaded( Konnector *, Syncee::PtrList );

  private:
    void filter( Syncee::PtrList unknown, Syncee::PtrList &real );

    KonnectorManager();
    ~KonnectorManager();

    Device::ValueList allDevices();
    Device parseDevice( const QString& path );
    Device find( const QString& deviceName );
    Syncee::PtrList findUnknown( Syncee::PtrList& );

    static KonnectorManager *m_self;

    Filter::PtrList m_filter;
    Filter::PtrList m_filAdded;
    bool m_auto;

    Device::ValueList m_devices;

    QPtrList<Konnector> m_konnectors;

    class Private;
//    Private *d;
};

}

#endif
