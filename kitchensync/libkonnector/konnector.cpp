
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

class KOperations{
public:
  KOperations(){ };

};

class Konnector::KonnectorPrivate{
public:
  KonnectorPrivate(){

  }
  QMap<QString, KonnectorPlugin*> m_konnectors;
  QValueList<KDevice> m_devices;
};

Konnector::Konnector( QObject *object, const char *name ) : QObject( object, name )
{
  // initialize
}
Konnector::Konnector()
{

}
Konnector::~Konnector()
{

};
QValueList<KDevice> Konnector::query(const QString &category )
{
  // lets find 
  allDevices();
  if(category.isEmpty() ){
    return d->m_devices;
  }
  QValueList<KDevice> dev;
  for(QValueList<KDevice>::Iterator it = d->m_devices.begin(); it != d->m_devices.end(); ++it ){
    if( (*it).group() == category){
      dev.append( (*it) );
    }
  }
  return dev;
}
QString Konnector::registerKonnector(const QString &Device )
{
  for(QValueList<KDevice>::Iterator it = d->m_devices.begin(); it != d->m_devices.end(); ++it ){
    if((*it).identify() == Device ){ // ok found
      // now load the lib
      QString randStr;
      do{
	randStr = kapp->randomString(8);
      }while( d->m_konnectors.contains( randStr ) );
      KonnectorPlugin* plugin =  KParts::ComponentFactory::
	createInstanceFromLibrary<KonnectorPlugin>( (*it).library(), this );
      if(!plugin){
	return QString::null;
      }
      plugin->setUDI( randStr );
      connect(plugin, SIGNAL(sync(QString, int, QPtrLsit<KSyncEntry> ) ), this, SLOT(slotSync(QString, int, QPtrList<KSyncEntry> ) ) );
      d->m_konnectors.insert(randStr, plugin  );
      return randStr;
    }
  }
  return QString::null;
}
QString Konnector::registerKonnector(const KDevice &Device )
{
  QString randStr;
  do{
    randStr = kapp->randomString(8);
  }while( d->m_konnectors.contains( randStr ) );
  KonnectorPlugin* plugin =  KParts::ComponentFactory::
    createInstanceFromLibrary<KonnectorPlugin>( Device.library(), this );
  if(!plugin){
    return QString::null;
  }
  plugin->setUDI(randStr);
  connect(plugin, SIGNAL(sync(QString, int, QPtrLsit<KSyncEntry> ) ), this, SLOT(slotSync(QString, int, QPtrList<KSyncEntry> ) ) );
  d->m_konnectors.insert(randStr, plugin  );
  return randStr;
}
Kapabilities Konnector::capabilities (const QString&  udi ) 
{
  KonnectorPlugin *plugin = pluginByUDI( udi );
  if( plugin == 0)
    return Kapabilities();

  return plugin->capabilities();
}
void Konnector::setCapabilities(const QString &udi, const Kapabilities& cap )
{
  KonnectorPlugin *plugin = pluginByUDI( udi );
  if( plugin == 0)
    return;

  return plugin->setCapabilities( cap);
}
QByteArray Konnector::file(const QString &udi, const QString &path )
{
  KonnectorPlugin *plugin = pluginByUDI( udi );
  if( plugin == 0)
    return QByteArray();

  return plugin->retrFile( path );
}
void Konnector::retrieveFile(const QString &udi, const QString &file )
{
  KonnectorPlugin *plugin = pluginByUDI( udi );
  if( plugin == 0)
    return;

  plugin->insertFile(file );
}
void Konnector::write( const QString &udi, QPtrList<KSyncEntry> entry)
{
  KonnectorPlugin *plugin = pluginByUDI( udi );
  if( plugin == 0)
    return;

  plugin->slotWrite( entry );
}
void Konnector::write( const QString &udi, QValueList<KOperations> operations)
{
  KonnectorPlugin *plugin = pluginByUDI( udi );
  if( plugin == 0)
    return;

  plugin->slotWrite(operations );
}
void Konnector::write( const QString &udi, const QString &dest, const QByteArray &array )
{
  KonnectorPlugin *plugin = pluginByUDI( udi );
  if( plugin == 0)
    return;

  return plugin->slotWrite(dest, array );
}
bool Konnector::isConnected(const QString &udi ){
  KonnectorPlugin *plugin = pluginByUDI( udi );
  if( plugin == 0)
    return false;

  return plugin->isConnected();
}
bool Konnector::startSync(const QString &udi )
{
  KonnectorPlugin *plugin = pluginByUDI( udi );
  if( plugin == 0)
    return false;

  return plugin->startSync();
}
void Konnector::allDevices()
{
  d->m_devices.clear();
  QStringList list = KGlobal::dirs()->findDirs("apps", "kitchensync" );

  for(QStringList::Iterator it = list.begin(); it != list.end(); ++it ){
    QDir dir( (*it), "*.desktop" );

    QStringList list2 = dir.entryList();
    QStringList::Iterator it2;
    for(it2 = list2.begin(); it2 != list2.end(); ++it2 ){
      kdDebug(100200) << (*it) << " " << (*it2);
      KService service( (*it) + "/" + (*it2) );
      QString name = service.name();
      QString library = service.library();
      QString group = (service.property("group" )).toString();
      QString vendor = (service.property("vendor" )).toString();
      KDevice device( name, group, vendor, library );
      d->m_devices.append(device );

    } 
  }
}
KonnectorPlugin* Konnector::pluginByUDI(const QString &udi )
{
  KonnectorPlugin* plugin=0l;
  if( d->m_konnectors.contains(udi ) ){
    QMap<QString, KonnectorPlugin*>::Iterator it;
    it = d->m_konnectors.find( udi );
    plugin = it.data();
  }
  return plugin;
}
void Konnector::slotSync(QString udi, int mode, QPtrList<KSyncEntry> entry)
{
  emit wantsToSync(udi, mode, entry );
}

void Konnector::slotError(QString, int, QString )
{


}





