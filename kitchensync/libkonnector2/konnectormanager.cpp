/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Holger Freyther <zecke@handhelds.org>

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

#include <qdir.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kparts/componentfactory.h>
#include <kstandarddirs.h>

#include "configpart.h"
#include "konnectorinfo.h"


#include "konnectorplugin.h"
#include "konnectormanager.h"

using namespace KSync;

static KStaticDeleter<KonnectorManager> deleter;
KonnectorManager* KonnectorManager::m_self = 0;

KonnectorManager::KonnectorManager()
  : KRES::Manager<Konnector>( "konnector" )
{
    m_auto = false;
    m_filter.setAutoDelete( true );
    m_konnectors.setAutoDelete( true );

  readConfig();

  connectSignals();
}

KonnectorManager::~KonnectorManager()
{
}

KonnectorManager* KonnectorManager::self()
{
  if ( !m_self ) deleter.setObject( m_self, new KonnectorManager() );

  return m_self;
}

Device::ValueList KonnectorManager::query()
{
    return allDevices();
}

Konnector *KonnectorManager::load( const Device& dev )
{
    Konnector *plugin = KParts::ComponentFactory::
        createInstanceFromLibrary<Konnector>( dev.library().local8Bit(), this );
    if ( !plugin ) return 0;

    connect( plugin, SIGNAL( synceesRead( Konnector *, const SynceeList & ) ),
             SLOT( slotSync( Konnector *, const SynceeList & ) ) );
    connect( plugin, SIGNAL( sig_progress( Konnector *, const Progress & ) ),
             SLOT( slotProgress( Konnector *, const Progress & ) ) );
    connect( plugin, SIGNAL( sig_error( Konnector *, const Error & ) ),
             SLOT( slotError( Konnector *, const Error& ) ) );
    connect( plugin, SIGNAL( sig_downloaded( Konnector *, const SynceeList & ) ),
             SLOT( slotDownloaded( Konnector *, const SynceeList & ) ) );

    m_konnectors.append( plugin );

    return plugin;
}

Konnector *KonnectorManager::load( const QString& deviceName )
{
    return load( find( deviceName ) );
}

bool KonnectorManager::unload( Konnector *k )
{
    return m_konnectors.remove( k );
}

ConfigWidget *KonnectorManager::configWidget( Konnector *konnector,
                                              QWidget *parent,
                                              const char *name )
{
    if ( kapp->type() == QApplication::Tty ) return 0;

    if ( !konnector ) return 0;

    ConfigWidget *wid = konnector->configWidget( parent, name );
    if ( !wid ) wid = new ConfigPart( konnector->capabilities(), parent, name );

    return wid;
}

ConfigWidget *KonnectorManager::configWidget( Konnector *konnector,
                                              const Kapabilities &caps,
                                              QWidget *parent,
                                              const char *name )
{
    if ( kapp->type() == QApplication::Tty ) return 0;

    if ( !konnector ) return 0;

    ConfigWidget *wid = konnector->configWidget( caps, parent, name );
    if ( !wid ) wid = new ConfigPart( konnector->capabilities(), caps, parent, name );

    return wid;
}


bool KonnectorManager::autoLoadFilter() const
{
    return m_auto;
}

void KonnectorManager::setAutoLoadFilter( bool aut )
{
    m_auto = aut;
}

void KonnectorManager::add( Filter* filter)
{
    m_filAdded.append( filter );
}

void KonnectorManager::deleteFilter( Filter* filter)
{
    m_filAdded.remove( filter ); // autoDelete is on!
}

const Filter::PtrList KonnectorManager::filters()
{
    return m_filAdded;
}

void KonnectorManager::write( Konnector * /*plugin*/, const SynceeList & )
{
// Konnectors should be directly called.
#if 0
    kdDebug(5201) << "KonnectorManager::write" << endl;
    if ( !plugin ) {
        kdDebug(5201) << " Did not contain the plugin " << endl;
        emit error( plugin, StdError::konnectorDoesNotExist() );
        emit progress( plugin, StdProgress::done() );
        return;
    }
    kdDebug(5201) << "Konnector: " << plugin->info().name() << endl;
    plugin->writeSyncees();
#endif
}

/*
 * find all available desktop files
 * we'll find the kitchensync dir
 * and then parse each .desktop file
 */
Device::ValueList KonnectorManager::allDevices()
{
    m_devices.clear(); // clean up first

    QStringList list = KGlobal::dirs()->findDirs("data", "kitchensync" );

    /* for each dir */
    for (QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
        QDir dir( (*it), "*.desktop" ); // data dir of kitchensync + .desktop as a filter
        QStringList files = dir.entryList();

        QStringList::Iterator fileIt;
        /* for each file */
        for (fileIt = files.begin(); fileIt != files.end(); ++fileIt )
            m_devices.append( parseDevice( (*it) + (*fileIt  ) ) );
    }
    return m_devices;
}

Device KonnectorManager::parseDevice( const QString &path )
{
    KService service( path );

    QString name  = service.name();
    QString lib   = service.library();
    QString group = service.property( QString::fromLatin1("Group" ) ).toString();
    QString vendo = service.property( QString::fromLatin1("Vendor") ).toString();
    QString id    = service.property( QString::fromLatin1("Id"    ) ).toString();

    kdDebug(5201) << "Id " << id << " " << name << endl;

    return Device(name, group, vendo, lib, id );
}

Device KonnectorManager::find( const QString& device )
{
    Device dev;
    if ( m_devices.isEmpty() ) return dev;

    Device::ValueList::Iterator it;
    for ( it = m_devices.begin(); it != m_devices.end(); ++it ) {
        if ( (*it).identify() == device ) {
            dev = (*it);
            break;
        }
    }
    return dev;
}

void KonnectorManager::slotSync( Konnector *k, const SynceeList & list )
{
    const SynceeList & unknown = findUnknown( list );
    filter( unknown, list );
    emit sync( k, list );
}

void KonnectorManager::slotProgress( Konnector *k, const Progress &pro )
{
    emit progress( k, pro );
}

void KonnectorManager::slotError( Konnector *k, const Error &err )
{
    emit error( k, err );
}

void KonnectorManager::slotDownloaded( Konnector *k, const SynceeList & list)
{
    const SynceeList & unknown = findUnknown( list );
    filter( unknown, list );
    emit downloaded( k, list );
}

/*
 * FIXME Cornelius take a look here when you want to implement
 * a generic KIO <-> Konnector FileBridge
 * The KIO Konnector only retrieves data and the Filter
 * filters for example the AddressBook or any other data...
 *
 * FIXME use filters!!!!
 */
void KonnectorManager::filter( const SynceeList & /*lst*/,
                               const SynceeList & /*real*/ )
{
    kdError() << "KonnectorManager::filter() not implemented" << endl;
}

SynceeList KonnectorManager::findUnknown( const SynceeList & )
{
#if 0
    lst.setAutoDelete( false );
    const SynceeList & list;
    Syncee* syn;
    for ( syn = lst.first(); syn; syn = lst.next() ) {
        if ( syn->type() == QString::fromLatin1("UnknownSyncEntry") ) {
            lst.remove( syn ); // setAutoDelete should be false
            list.append( syn );
        }
    }
    return list;
#endif
  return SynceeList();
}

void KonnectorManager::connectSignals()
{
  Iterator it;
  for( it = begin(); it != end(); ++it ) {
    connect( *it, SIGNAL( synceesRead( Konnector *, const SynceeList & ) ),
             SIGNAL( synceesRead( Konnector *, const SynceeList & ) ) );
    connect( *it, SIGNAL( synceeReadError( Konnector * ) ),
             SIGNAL( synceeReadError( Konnector * ) ) );
    connect( *it, SIGNAL( synceesWritten( Konnector * ) ),
             SIGNAL( synceesWritten( Konnector * ) ) );
    connect( *it, SIGNAL( synceeWriteError( Konnector * ) ),
             SIGNAL( synceeWriteError( Konnector * ) ) );
  }
}

#include "konnectormanager.moc"
