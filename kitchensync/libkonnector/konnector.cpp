
#include <qapplication.h>
#include <qdir.h>

#include <kglobal.h>
#include <kstandarddirs.h>
#include <kservice.h>
#include <kdebug.h>
#include <kapplication.h>
#include <kgenericfactory.h>
#include <kparts/componentfactory.h>


#include "kdevice.h"
#include "konnectorplugin.h"
#include "konnector.h"
#include "kapabilities.h"
#include "koperations.h"
#include "configwidget.h"
#include "configpart.h"

using namespace KSync;

class KonnectorManager::KonnectorPrivate{
public:
  KonnectorPrivate(){

  }
  QMap<QString, KonnectorPlugin*> m_konnectors;
  Device::ValueList m_devices;
};

KonnectorManager::KonnectorManager( QObject *object, const char *name ) : QObject( object, name )
{
  // initialize
  d = new KonnectorPrivate;
  kdDebug(5201) << "c'tor" << endl;
}
KonnectorManager::~KonnectorManager()
{
    QMap<QString,  KonnectorPlugin*>::Iterator it;
    for ( it = d->m_konnectors.begin(); it != d->m_konnectors.end(); ++it )
        delete it.data();

    delete d;
};
Device::ValueList KonnectorManager::query(const QString &category )
{
    // lets find it
    kdDebug(5201) << "query " << category << endl;
    allDevices();
    if(category.isEmpty() ){
        kdDebug(5201) << "no devices found" << endl;
        return d->m_devices;
    }
    // filter it
    Device::ValueList dev;
    for(Device::ValueList::Iterator it = d->m_devices.begin(); it != d->m_devices.end(); ++it ){
        if( (*it).group() == category){ // applies to the category filter
            dev.append( (*it) );
        }
    }
    return dev;
}
Device KonnectorManager::find( const QString& device ) {
    Device dev;
    if (d->m_devices.isEmpty() )
        return dev;
    Device::ValueList::Iterator it;
    for (it = d->m_devices.begin(); it != d->m_devices.end(); ++it ) {
        if ( (*it).identify() == device ) {
            dev = (*it);
            break;
        }
    };
    return dev;
}
QString KonnectorManager::generateUID() {
    QString uid;
    do{
	uid = kapp->randomString(8);
	kdDebug(5201) << "randStr :" << uid << endl;
    }while( d->m_konnectors.contains( uid ) );

    return uid;
}
QString KonnectorManager::registerKonnector(const QString &device )
{
    kdDebug(5201) << "registerKonnector " << device << endl;

    Device dev = find( device );
    if (dev.library().isEmpty() ) return QString::null; // no lib associtaed

    QString randStr = generateUID();


    KonnectorPlugin* plugin =  KParts::ComponentFactory::
                               createInstanceFromLibrary<KonnectorPlugin>(dev.library().latin1(),
                                                                          this );
    if(!plugin) // could not load
	return QString::null;

    plugin->setUDI( randStr );
    connect(plugin, SIGNAL(sync(const QString&, Syncee::PtrList ) ),
            this, SLOT(slotSync(const QString&, Syncee::PtrList ) ) );

    connect(plugin, SIGNAL(errorKonnector(const QString&, int, const QString&) ),
            this, SLOT(slotError(const QString&,int, const QString&)) );

    connect(plugin, SIGNAL(stateChanged(const QString&,  bool ) ),
            this, SLOT(slotChanged( const QString&,  bool ) ) );

    d->m_konnectors.insert(randStr, plugin  );
    return randStr;
}
QString KonnectorManager::registerKonnector(const Device &device )
{
    kdDebug(5201) << "registerKonnector lib:" << device.library() << endl;

    QString randStr = generateUID();

    KonnectorPlugin* plugin =  KParts::ComponentFactory::
                               createInstanceFromLibrary<KonnectorPlugin>( device.library().latin1(),
                                                                           this );
  if(!plugin)
    return QString::null;

  plugin->setUDI(randStr);
  connect(plugin, SIGNAL(sync(const QString&, Syncee::PtrList ) ),
          this, SLOT(slotSync(const QString&,  Syncee::PtrList ) ) );

  connect(plugin, SIGNAL(errorKonnector(const QString&, int, const QString&) ),
          this, SLOT(slotError(const QString&,int, const QString&)) );
  connect(plugin, SIGNAL(stateChanged(const QString&,  bool ) ),
          this, SLOT(slotChanged( const QString&,  bool ) ) );
  d->m_konnectors.insert(randStr, plugin  );

  return randStr;
}
Kapabilities KonnectorManager::capabilities (const QString&  udi )
{
  KonnectorPlugin *plugin = pluginByUDI( udi );
  if( plugin == 0)
    return Kapabilities();

  return plugin->capabilities();
}
void KonnectorManager::setCapabilities(const QString &udi, const Kapabilities& cap )
{
  KonnectorPlugin *plugin = pluginByUDI( udi );
  if( plugin == 0)
    return;

  return plugin->setCapabilities( cap);
}
QByteArray KonnectorManager::file(const QString &udi, const QString &path )
{
  KonnectorPlugin *plugin = pluginByUDI( udi );
  if( plugin == 0)
    return QByteArray();

  return plugin->retrFile( path );
}
Syncee* KonnectorManager::fileAsEntry(const QString& udi,  const QString &path )
{
    KonnectorPlugin *plugin = pluginByUDI( udi );
    if( plugin == 0l)
        return 0l;

    return plugin->retrEntry( path );
}
void KonnectorManager::retrieveFile(const QString &udi, const QString &file )
{
  KonnectorPlugin *plugin = pluginByUDI( udi );
  if( plugin == 0)
    return;

  plugin->insertFile(file );
}
void KonnectorManager::write( const QString &udi, Syncee::PtrList entry)
{
    kdDebug() << "write " << endl;
    KonnectorPlugin *plugin = pluginByUDI( udi );
    if( plugin == 0)
        return;

    plugin->slotWrite( entry );
}
void KonnectorManager::write( const QString &udi, KOperations::ValueList operations)
{
    KonnectorPlugin *plugin = pluginByUDI( udi );
    if( plugin == 0)
        return;

    plugin->slotWrite(operations );
}
void KonnectorManager::write( const QString &udi, const QString &dest, const QByteArray &array )
{
    KonnectorPlugin *plugin = pluginByUDI( udi );
    if( plugin == 0)
        return;

    return plugin->slotWrite(dest, array );
}
bool KonnectorManager::isConnected(const QString &udi ){
    KonnectorPlugin *plugin = pluginByUDI( udi );
    if( plugin == 0)
        return false;

    return plugin->isConnected();
}
bool KonnectorManager::startSync(const QString &udi )
{
    KonnectorPlugin *plugin = pluginByUDI( udi );
    if( plugin == 0)
        return false;

    return plugin->startSync();
}
bool KonnectorManager::startBackup(const QString &udi, const QString& path)
{
    KonnectorPlugin *plugin = pluginByUDI( udi );
    if( plugin == 0)
        return false;

    return plugin->startBackup(path);
}
bool KonnectorManager::startRestore(const QString &udi, const QString& path)
{
    KonnectorPlugin *plugin = pluginByUDI( udi );
    if( plugin == 0)
        return false;

    return plugin->startRestore(path);
}
/**
 * convert the Service to a Device
 */
void KonnectorManager::addDevice( const QString& path ) {
    KService service( path );
    QString name = service.name();
    QString library = service.library();

    QString group = (service.property( QString::fromLatin1("Group")  )).toString();
    QString vendor = (service.property("Vendor" )).toString();

    QString id = (service.property("Id")).toString();

    Device device( name, group, vendor, library, id );
    d->m_devices.append(device );
}
void KonnectorManager::allDevices()
{
    kdDebug(5201) << "searching for devices" << endl;
    d->m_devices.clear();
    QStringList list = KGlobal::dirs()->findDirs("data", "kitchensync" );

    if(list.isEmpty() )
        kdDebug(5201) << "no dirs found" << endl;

    for(QStringList::Iterator it = list.begin(); it != list.end(); ++it ){

        QDir dir( (*it), "*.desktop" );
        kdDebug(5201) << "searching for devices in " << (*it) << endl;
        QStringList list2 = dir.entryList();
        QStringList::Iterator it2;
        for(it2 = list2.begin(); it2 != list2.end(); ++it2 )
            addDevice( (*it) + (*it2) );
    }
}
KonnectorPlugin* KonnectorManager::pluginByUDI(const QString &udi )
{
  KonnectorPlugin* plugin=0l;

  if( d->m_konnectors.contains(udi ) ){
    QMap<QString, KonnectorPlugin*>::Iterator it;
    it = d->m_konnectors.find( udi );
    plugin = it.data();
   }
  return plugin;
}
KonnectorPlugin* KonnectorManager::pluginByUDI(const QString &udi )const
{
  KonnectorPlugin* plugin=0l;

  if( d->m_konnectors.contains(udi ) ){
    QMap<QString, KonnectorPlugin*>::ConstIterator it;
    it = d->m_konnectors.find( udi );
    plugin = it.data();
   }
  return plugin;
}
void KonnectorManager::slotSync(const QString &udi,  Syncee::PtrList entry)
{
  emit wantsToSync(udi, entry );
}

void KonnectorManager::slotError(const QString &udi, int mode, const QString &
                          info)
{
  kdDebug() << "KonnectorManager::Error received" << endl;
  emit konnectorError(udi, mode, info );

}
bool KonnectorManager::connectDevice( const QString &udi )
{
    KonnectorPlugin *plugin = pluginByUDI( udi );
    if( plugin == 0l)
        return false;

    return plugin->connectDevice();
}
void KonnectorManager::disconnectDevice( const QString &udi )
{
    KonnectorPlugin *plugin = pluginByUDI( udi );
    if( plugin == 0l)
        return;

    return plugin->disconnectDevice();
}
QIconSet KonnectorManager::iconSet(const QString& udi )const
{
    KonnectorPlugin *plugin = pluginByUDI( udi );
    if( plugin == 0l)
        return QIconSet();

    return plugin->iconSet();
}
QString KonnectorManager::iconName( const QString& udi)const
{
    KonnectorPlugin *plugin = pluginByUDI( udi );
    if (plugin == 0l )
        return QString::null;
    return plugin->iconName();
}
QString KonnectorManager::id(const QString& udi )const
{
    KonnectorPlugin *plugin = pluginByUDI( udi );
    if( plugin == 0l)
        return QString::null;

    return plugin->id();
}
void KonnectorManager::slotChanged(const QString& i,  bool b)
{
    kdDebug(5201) << "Konnector state changed" << endl;
    emit stateChanged( i,  b );
}
QString KonnectorManager::metaId( const QString& udi ) const
{
    KonnectorPlugin *plugin = pluginByUDI( udi );
    if( plugin == 0l)
        return QString::null;

    return plugin->metaId();
}
ConfigWidget* KonnectorManager::configWidget( const QString& udi, QWidget* parent, const char* name ) {
    if ( kapp->type() == QApplication::Tty )
        return 0l;
    KonnectorPlugin * plugin = pluginByUDI( udi );
    if ( plugin == 0l )
        return 0l;
    ConfigWidget* wid = plugin->configWidget( parent, name );

    if (wid ==0 ) {
        wid = new ConfigPart( plugin->capabilities(), parent, name );
    }

    return wid;
}
ConfigWidget* KonnectorManager::configWidget( const QString& udi,
                                              const Kapabilities& caps,  QWidget* parent,
                                              const char* name ) {
    if ( kapp->type() == QApplication::Tty )
        return 0l;
    KonnectorPlugin * plugin = pluginByUDI( udi );
    if ( plugin == 0l )
        return 0l;

    ConfigWidget* wid = plugin->configWidget( caps, parent,  name );
    if (wid ==0 ) {
        wid = new ConfigPart( plugin->capabilities(), parent, name );
    }

    return wid;
}
bool KonnectorManager::unregisterKonnector( const QString& udi ) {
    bool ok = false;
    if( d->m_konnectors.contains(udi ) ){
        KonnectorPlugin *plugin;
        QMap<QString, KonnectorPlugin*>::Iterator it;
        it = d->m_konnectors.find( udi );
        plugin = it.data();
        d->m_konnectors.remove( it );
        delete plugin;
        ok = true;
    }
    return ok;
}

#include "konnector.moc"
