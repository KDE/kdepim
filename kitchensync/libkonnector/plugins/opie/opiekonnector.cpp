
#include <qvaluelist.h>
#include <qpair.h>
#include <qptrlist.h>
#include <qhostaddress.h>
#include <kapabilities.h>
#include <koperations.h>
#include <kgenericfactory.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <kglobal.h>
#include <klocale.h>

#include "opiesocket.h"
#include "opiekonnector.h"

typedef KGenericFactory<KSync::OpiePlugin, QObject>  OpieKonnectorPlugin;
K_EXPORT_COMPONENT_FACTORY( libopiekonnector,  OpieKonnectorPlugin );

using namespace KSync;

class OpiePlugin::OpiePluginPrivate{
 public:
  OpiePluginPrivate(){
    connection = false;
    socket = 0;
  }
  QString udi;
  bool connection;
  QStringList retrFiles;
  OpieSocket *socket;
};

OpiePlugin::OpiePlugin(QObject *obj, const char *name, const QStringList )
  : KonnectorPlugin(obj, name )
{
    kdDebug(5202) << "OpiePlugin ";
    d = new OpiePluginPrivate;
    d->socket = new OpieSocket(this, "opiesocket");

    connect(d->socket, SIGNAL(sync(Syncee::PtrList ) ),
	    this, SLOT(slotSync(Syncee::PtrList ) ) );
    connect(d->socket, SIGNAL(errorKonnector(int, QString ) ),
	    this, SLOT(slotErrorKonnector(int, QString) ) );

    connect(d->socket, SIGNAL(stateChanged( bool ) ),
            this, SLOT(slotChanged( bool ) ) );


}
OpiePlugin::~OpiePlugin()
{
  delete d;
}

void OpiePlugin::setUDI(const QString &udi )
{
  d->udi = udi;
}

QString OpiePlugin::udi()const
{
  return d->udi;
}

QIconSet OpiePlugin::iconSet()const
{
    kdDebug() << "iconSet" << endl;
    QPixmap logo;
    logo.load( locate("appdata",  "pics/opie_logo.png" ) );
    return QIconSet( logo );
}
QString OpiePlugin::iconName()const
{
    kdDebug() << "icon Name from Opie" << endl;
    return QString::fromLatin1("pics/opie_logo.png");
};
Kapabilities OpiePlugin::capabilities( )
{
  // create the capabilities Apply
  kdDebug(5202) << "OpiePlugin capabilities" << endl;
  Kapabilities caps;
  caps.setSupportMetaSyncing( true );
  caps.setSupportsPushSync( true );
  caps.setNeedsConnection( true );
  caps.setSupportsListDir( false );
  caps.setNeedsIPs( true );
  caps.setNeedsSrcIP( false );
  caps.setNeedsDestIP( true );
  caps.setAutoHandle( false );
  caps.setNeedAuthentication( true );
  QValueList<QPair<QString,QString> > user;
  user.append(qMakePair(QString::fromLatin1("root"),  QString::fromLatin1("test123") ) );
  user.append(qMakePair(QString::fromLatin1("ich"), QString::fromLatin1("test321") ) );
  caps.setUserProposals( user );
  QStringList ips;
  QString  src = "192.168.1.100";
  ips.append( src );
  caps.setIpProposals( ips );
  return caps;
}

void OpiePlugin::setCapabilities( const Kapabilities &kaps )
{
  // create
  d->socket->setSrcIP( kaps.srcIP() );
  d->socket->setDestIP( kaps.destIP() );
  d->socket->setUser( kaps.user() );
  d->socket->setPassword( kaps.password() );
  d->socket->setMeta( kaps.isMetaSyncingEnabled() );
  d->socket->startUp();
}
bool OpiePlugin::startSync()
{
  return d->socket->startSync();
}
bool OpiePlugin::startBackup(const QString& )
{
  return true;
}
bool OpiePlugin::startRestore(const QString& )
{
  return true;
}
bool OpiePlugin::isConnected()
{
  return d->socket->isConnected();
}
bool OpiePlugin::insertFile(const QString &fileName )
{
    return d->socket->insertFile(fileName );
}
QByteArray OpiePlugin::retrFile(const QString &path )
{
    return d->socket->retrFile(path );
}
void OpiePlugin::slotWrite(const QString &path, const QByteArray &array )
{
    d->socket->write(path, array );
}
void OpiePlugin::slotWrite(Syncee::PtrList entry)
{
    d->socket->write(entry );
};
void OpiePlugin::slotSync(Syncee::PtrList entry )
{
    emit sync( d->udi, entry );
}
void OpiePlugin::slotErrorKonnector( int mode, QString error )
{
    emit errorKonnector(d->udi, mode, error );
}
void OpiePlugin::slotWrite(KOperations::ValueList operations )
{
    d->socket->write(operations );
}
void OpiePlugin::slotChanged( bool b)
{
    kdDebug(5202) << "State changed Opiekonnector" << endl;
    emit  stateChanged( d->udi,  b );
}
Syncee* OpiePlugin::retrEntry( const QString& path )
{
    return d->socket->retrEntry( path );
}
QString OpiePlugin::metaId()const
{
    return d->socket->metaId();
}

#include "opiekonnector.moc"
