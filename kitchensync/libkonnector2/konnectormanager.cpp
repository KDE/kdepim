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

class KonnectorManager::Private
{
  public:
    typedef QMap<QString, KonnectorPlugin*> PluginMap;
    PluginMap konnectors;
    Device::ValueList devices;
};

KonnectorManager::KonnectorManager()
{
    m_auto = false;
    m_filter.setAutoDelete( true );
    d = new Private;
}

KonnectorManager::~KonnectorManager()
{
    for ( Private::PluginMap::Iterator it = d->konnectors.begin();
          it != d->konnectors.end();
          ++it )
        delete it.data();

    delete d;
}

KonnectorManager* KonnectorManager::self()
{
    if ( !m_self )
        deleter.setObject( m_self,  new KonnectorManager() );

    return m_self;
}

Device::ValueList KonnectorManager::query()
{
    return allDevices();
}

UDI KonnectorManager::load( const Device& dev )
{
    KonnectorPlugin* plugin = KParts::ComponentFactory::
                              createInstanceFromLibrary<KonnectorPlugin>( dev.library().local8Bit(),
                                                                          this );
    if ( !plugin ) return QString::null;

    QString udi = newUDI();
    plugin->setUDI(udi);

    connect( plugin, SIGNAL( sync( const UDI&, Syncee::PtrList ) ),
             SLOT( slotSync(const UDI&, Syncee::PtrList) ) );
    connect( plugin, SIGNAL( sig_progress( const UDI&, const Progress & ) ),
             SLOT( slotProgress( const UDI &, const Progress & ) ) );
    connect( plugin, SIGNAL( sig_error( const UDI &, const Error & ) ),
             SLOT( slotError( const UDI &, const Error& ) ) );
    connect( plugin, SIGNAL( sig_downloaded( const UDI &, Syncee::PtrList ) ),
             SLOT( slotDownloaded( const UDI &, Syncee::PtrList ) ) );

    d->konnectors.insert( udi, plugin );
    return udi;
}

UDI KonnectorManager::load( const QString& deviceName )
{
    return load( find( deviceName ) );
}

bool KonnectorManager::unload( const UDI& udi )
{
    if ( !d->konnectors.contains( udi ) )
        return false;

    KonnectorPlugin* plugin = d->konnectors[udi];
    d->konnectors.remove( udi );
    delete plugin;

    return true;
}

Kapabilities KonnectorManager::capabilities( const UDI& udi ) const
{
    KonnectorPlugin* plugin = pluginByUDI( udi );
    if (!plugin ) return Kapabilities();

    return plugin->capabilities();
}

void KonnectorManager::setCapabilities( const UDI& udi, const Kapabilities& cap )
{
    KonnectorPlugin* plugin = pluginByUDI( udi );
    if (!plugin) return;

    plugin->setCapabilities( cap );
}

ConfigWidget* KonnectorManager::configWidget( const UDI& uid,
                                              QWidget* parent,
                                              const char* name )
{
    if ( kapp->type() == QApplication::Tty ) return 0;

    KonnectorPlugin* plugin = pluginByUDI( uid );
    if ( !plugin ) return 0;

    ConfigWidget* wid = plugin->configWidget( parent, name );
    if (!wid) wid = new ConfigPart( plugin->capabilities(), parent, name );

    return wid;
}

ConfigWidget* KonnectorManager::configWidget( const UDI& uid,
                                              const Kapabilities& caps,
                                              QWidget* parent,
                                              const char* name )
{
    if ( kapp->type() == QApplication::Tty ) return 0;

    KonnectorPlugin* plugin = pluginByUDI( uid );
    if ( !plugin ) return 0;

    ConfigWidget* wid = plugin->configWidget( caps, parent, name );
    if ( !wid ) wid = new ConfigPart( plugin->capabilities(), caps, parent, name );

    return wid;
}

void KonnectorManager::add( const UDI& udi,  const QString& resource )
{
    KonnectorPlugin* plugin = pluginByUDI( udi );
    if ( !plugin ) return;

    plugin->add( resource );
}

void KonnectorManager::remove( const UDI& udi, const QString& resource )
{
    KonnectorPlugin* plugin = pluginByUDI( udi );
    if (!plugin) return;

    plugin->remove( resource );
}

QStringList KonnectorManager::resources( const UDI& udi) const
{
    KonnectorPlugin* plugin = pluginByUDI( udi );
    if (!plugin) return QStringList();

    return plugin->resources();
}

void KonnectorManager::download( const UDI& udi, const QString& resource )
{
    KonnectorPlugin* plugin = pluginByUDI( udi );
    if (!plugin) return;

    plugin->download( resource );
}

bool KonnectorManager::isConnected( const UDI& udi ) const
{
    KonnectorPlugin* plugin = pluginByUDI( udi );
    if (!plugin) return false;

    return plugin->isConnected();
}

bool KonnectorManager::connectDevice( const UDI& udi )
{
    KonnectorPlugin* plugin = pluginByUDI( udi );
    if (!plugin) return false;

    return plugin->connectDevice();
}

bool KonnectorManager::disconnectDevice( const UDI& udi )
{
    KonnectorPlugin* plugin = pluginByUDI( udi );
    if (!plugin) return false;

    return plugin->disconnectDevice();
}

bool KonnectorManager::startSync( const UDI& udi)
{
    KonnectorPlugin* plugin = pluginByUDI( udi );
    if (!plugin) return false;

    return plugin->startSync();
}

bool KonnectorManager::startBackup( const UDI& udi, const QString& path )
{
    KonnectorPlugin* plugin = pluginByUDI( udi );
    if (!plugin) return false;

    return plugin->startBackup(path);
}

bool KonnectorManager::startRestore( const UDI& udi, const QString& path )
{
    KonnectorPlugin* plugin = pluginByUDI( udi );
    if (!plugin) return false;

    return plugin->startRestore(path);
}

KonnectorInfo KonnectorManager::info( const UDI& udi) const
{
    KonnectorPlugin* plugin = pluginByUDI( udi );
    if (!plugin) return KonnectorInfo();

    return plugin->info();
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

void KonnectorManager::write( const QString& udi, const Syncee::PtrList &lst )
{
    kdDebug(5201) << "Write now " <<  udi << endl;
    KonnectorPlugin* plugin =pluginByUDI(udi);
    if (!plugin ) {
        kdDebug(5201) << " Did not contain the plugin " << endl;
        emit error( udi, StdError::konnectorDoesNotExist(udi) );
        emit progress( udi, StdProgress::done() );
        return;
    }
    kdDebug(5201) << "Plugin is " << plugin << " " << plugin->info().name() << endl;
    plugin->doWrite( lst );
}

/*
 * find all available desktop files
 * we'll find the kitchensync dir
 * and then parse each .desktop file
 */
Device::ValueList KonnectorManager::allDevices()
{
    d->devices.clear(); // clean up first

    QStringList list = KGlobal::dirs()->findDirs("data", "kitchensync" );

    /* for each dir */
    for (QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
        QDir dir( (*it), "*.desktop" ); // data dir of kitchensync + .desktop as a filter
        QStringList files = dir.entryList();

        QStringList::Iterator fileIt;
        /* for each file */
        for (fileIt = files.begin(); fileIt != files.end(); ++fileIt )
            d->devices.append( parseDevice( (*it) + (*fileIt  ) ) );
    }
    return d->devices;
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
    if (d->devices.isEmpty() )
        return dev;

    Device::ValueList::Iterator it;
    for ( it = d->devices.begin(); it != d->devices.end(); ++it ) {
        if ( (*it).identify() == device ) {
            dev = (*it);
            break;
        }
    }
    return dev;
}

UDI KonnectorManager::newUDI() const
{
    QString uid;
    do {
        uid = kapp->randomString(8);
    } while ( d->konnectors.contains( uid ) );
    return uid;
}

KonnectorPlugin* KonnectorManager::pluginByUDI( const UDI& udi ) const
{
    KonnectorPlugin* plugin = 0;

    if ( d->konnectors.contains( udi ) )
        plugin =  d->konnectors[udi];

    return plugin;
}

void KonnectorManager::slotSync( const UDI& udi, Syncee::PtrList list )
{
    Syncee::PtrList unknown = findUnknown( list );
    filter( unknown, list );
    emit sync( udi, list );
}

void KonnectorManager::slotProgress( const UDI& udi, const Progress& pro )
{
    emit progress( udi, pro );
}

void KonnectorManager::slotError( const UDI& udi, const Error& err)
{
    emit error( udi, err );
}

void KonnectorManager::slotDownloaded( const UDI& udi, Syncee::PtrList list)
{
    Syncee::PtrList unknown = findUnknown( list );
    filter( unknown, list );
    emit downloaded( udi, list );
}

/*
 * FIXME Cornelius take a look here when you want to implement
 * a generic KIO <-> KonnectorPlugin FileBridge
 * The KIO KonnectorPlugin only retrieves data and the Filter
 * filters for example the AddressBook or any other data...
 *
 * FIXME use filters!!!!
 */
void KonnectorManager::filter( Syncee::PtrList lst , Syncee::PtrList& real)
{
    Syncee* syncee= 0;
    for ( syncee = lst.first(); syncee; syncee = lst.next() ) {
        real.append( syncee );
    }
}

Syncee::PtrList KonnectorManager::findUnknown( Syncee::PtrList& lst )
{
    lst.setAutoDelete( false );
    Syncee::PtrList list;
    Syncee* syn;
    for ( syn = lst.first(); syn; syn = lst.next() ) {
        if ( syn->type() == QString::fromLatin1("UnknownSyncEntry") ) {
            lst.remove( syn ); // setAutoDelete should be false
            list.append( syn );
        }
    }
    return list;
}

#include "konnectormanager.moc"
