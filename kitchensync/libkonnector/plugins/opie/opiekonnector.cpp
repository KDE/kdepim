
#include <qvaluelist.h>
#include <qpair.h>
#include <qhostaddress.h>
#include <kapabilities.h>
#include "opiekonnector.h"

OpiePlugin::OpiePluginPrivate{
 public:
  OpiePluginPrivate(){
    connection = false;
  }
  QString udi;
  bool connection;
  QStringList retrFiles;
};

OpiePlugin::OpiePlugin(QObject *obj, const char *name, const QStringList )
  : KonnectorPlugin(obj, name )
{
  d = new OpiePluginPrivate;

}
void OpiePlugin::setUDI(const QString &udi )
{
  d->udi = udi;
}

QString OpiePlugin::udi()const
{
  return d->udi;
}


Kapabilities OpiePlugin::capabilities( )
{
  // create the capabilities Apply
  Kapabilities caps;
  caps.setSupportsPushSync( true );
  caps.setNeedsConnection( true );
  caps.setSupportListDir( false );
  caps.setNeedsIPs( true );
  caps.setNeedsSrcIP( false );
  caps.setNeedsDestIP( true );
  caps.setAutoHandle( false );
  caps.setNeedAuthentication( true );
  QValueList<QPair<QString,QString> > user;
  user.append(qMakePair("root", "test123" ) );
  user.append(qMakePair("ich", "test321" ) );
  caps.setUserProposals( user );
  QValueList< QPair<QHostAddress, QHostAddress> > ips;
  QHostAddress src;
  QHostAddress dest;
  src.setAddress("192.168.1.100" );
  dest.setAddress("192.168.1.201");
  ips.append( qMakePair(src, dest ) );
  src.setAddress("127.0.0.1" );
  dest.setAddress("127.0.0.1");
  ips.append( qMakePair(src, dest ) );
}

void OpiePlugin::setCapabilities( const Kapabilities &kaps )
{
  // create 
  //->setSrcIP( kaps.srcIP() );
  //->setDestIP( kaps.destIP() );
  //->setUser( kaps.user() );
  //->setPassword( kaps.password() );
}
bool OpiePlugin::startSync()
{
  //return ->startSync();
}
bool OpiePlugin::isConnected()
{
  //return ->isConnected();
}
bool OpiePlugin::insertFile(const QString &fileName )
{

}
QByteArray OpiePlugin::retrFile(const QString &path )
{

}
void OpiePlugin::slotWrite(const QString &, const QByteArray & )
{

}
void OpiePlugin::slotWrite(QPtrList<KSyncEntry> )
{

}
void OpiePlugin::slotWrite(QValueList<KOperations> )
{

}

