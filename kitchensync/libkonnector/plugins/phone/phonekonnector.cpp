
#include <qvaluelist.h>
#include <qpair.h>
#include <qptrlist.h>
#include <qhostaddress.h>
#include <kapabilities.h>
#include <koperations.h>
#include <kgenericfactory.h>
#include <kdebug.h>

#include "phonekonnector.h"
#include "modemhandler.h"

typedef KGenericFactory<PhonePlugin, QObject>  PhoneKonnectorPlugin;
K_EXPORT_COMPONENT_FACTORY( libphonekonnector,  PhoneKonnectorPlugin )

class PhonePlugin::PhonePluginPrivate
{
 public:
  PhonePluginPrivate(){
    isConnected = false;
    modemHandler = 0;
  }
  QString udi;
  QStringList retrFiles;
  ModemHandler *modemHandler;
  int isConnected;
};

PhonePlugin::PhonePlugin(QObject *obj, const char *name, const QStringList )
  : KonnectorPlugin(obj, name )
{
    kdDebug(5203) << "PhonePlugin " << endl;
    d = new PhonePluginPrivate;
    d->modemHandler = new ModemHandler;
/*
	connect(d->socket, SIGNAL(sync(KSyncEntry::List ) ),
	    this, SLOT(slotSync(KSyncEntry::List ) ) );

    connect(d->socket, SIGNAL(errorKonnector(int, QString ) ),
	    this, SLOT(slotErrorKonnector(int, QString) ) );
    connect(d->socket, SIGNAL(stateChanged( bool ) ),
            this, SLOT(slotChanged( bool ) ) );
*/

}
PhonePlugin::~PhonePlugin()
{
  delete d;
}
void PhonePlugin::setUDI(const QString &udi )
{
  d->udi = udi;
}

QString PhonePlugin::udi()const
{
  return d->udi;
}


Kapabilities PhonePlugin::capabilities( )
{
  // create the capabilities Apply
  kdDebug(5203) << "PhonePlugin capabilities" << endl;
  Kapabilities caps;
  caps.setSupportMetaSyncing( true );
  caps.setSupportsPushSync( true );
  caps.setNeedsConnection( true );
  caps.setSupportsListDir( false );
  caps.setNeedsIPs( false );
  caps.setNeedsSrcIP( false );
  caps.setNeedsDestIP( false );
  caps.setAutoHandle( false );
  caps.setNeedAuthentication( false );
  QValueList<QPair<QString,QString> > user;
  return caps;
}

bool PhonePlugin::connectDevice()
{
	kdDebug(5203) << "PhonePlugin::connectDevice()" << endl;
	if ( !d->isConnected )
	{ 
		// TODO: make configurable
		return d->modemHandler->openConnection("/dev/modem", "11500");
	}
	return true;
}

void PhonePlugin::setCapabilities( const Kapabilities &kaps )
{
  // create
}
bool PhonePlugin::startSync()
{
//  return d->socket->startSync();
	return true;
}
bool PhonePlugin::isConnected()
{
  return d->isConnected;
}
bool PhonePlugin::insertFile(const QString &fileName )
{
    //return d->socket->insertFile(fileName );
	return true;
}
QByteArray PhonePlugin::retrFile(const QString &path )
{
    //return d->socket->retrFile(path );
	QByteArray qb;
	return qb;
}
void PhonePlugin::slotWrite(const QString &path, const QByteArray &array )
{
    //d->socket->write(path, array );
}
void PhonePlugin::slotWrite(KSyncEntry::List entry)
{
    //d->socket->write(entry );
};
void PhonePlugin::slotSync(KSyncEntry::List entry )
{
    emit sync( d->udi, entry );
}
void PhonePlugin::slotErrorKonnector( int mode, QString error )
{
    //emit errorKonnector(d->udi, mode, error );
}
void PhonePlugin::slotWrite(KOperations::List operations )
{
    //d->socket->write(operations );
}
void PhonePlugin::slotChanged( bool b)
{
    kdDebug(5203) << "State changed Phonekonnector" << endl;
    //emit  stateChanged( d->udi,  b );
}
KSyncEntry* PhonePlugin::retrEntry( const QString& path )
{
    //return d->socket->retrEntry( path );
	return new KSyncEntry;
}
QString PhonePlugin::metaId()const
{
    //return d->socket->metaId();
	QString metaId;
	return metaId;
}

#include "phonekonnector.moc"
