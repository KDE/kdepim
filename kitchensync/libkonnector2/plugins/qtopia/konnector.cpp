
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kgenericfactory.h>

#include <konnectorinfo.h>

#include "qtopiaconfig.h"
#include "socket.h"
#include "konnector.h"

typedef KGenericFactory<KSync::QtopiaPlugin, QObject>  QtopiaKonnectorPlugin;
K_EXPORT_COMPONENT_FACTORY( libqtopiakonnector,  QtopiaKonnectorPlugin );

using namespace KSync;

class QtopiaPlugin::Private
{
  public:
    Private() : socket( 0 ) {}

    QtopiaSocket *socket;
};

QtopiaPlugin::QtopiaPlugin( QObject* obj, const char* name, const QStringList )
    : Konnector(obj, name )
{
    d = new Private;
    d->socket = new QtopiaSocket(this, "Opie Socket" );

    /* now do some signal and slot connection */
    connect(d->socket, SIGNAL(sync(Syncee::PtrList) ),
            this, SLOT(slotSync(Syncee::PtrList) ) );
    connect(d->socket, SIGNAL(error(const Error&) ),
            this, SLOT(slotError(const Error& ) ) );
    connect(d->socket, SIGNAL(prog(const Progress& ) ),
            this, SLOT(slotProg(const Progress& ) ) );
}

QtopiaPlugin::~QtopiaPlugin()
{
    delete d;
}

Kapabilities QtopiaPlugin::capabilities()
{
    Kapabilities caps;
    caps.setSupportMetaSyncing( true );
    caps.setSupportsPushSync( true );
    caps.setNeedsConnection( true );
    caps.setSupportsListDir( true );
    caps.setNeedsIPs( true );
    caps.setNeedsSrcIP( false );
    caps.setNeedsDestIP( true );
    caps.setAutoHandle( false );
    caps.setNeedAuthentication( true );

    QValueList<QPair<QString, QString> > user;
    user.append(qMakePair(QString::fromLatin1("root"), QString::fromLatin1("rootme") ) );
    caps.setUserProposals( user );

    QStringList ips;
    ips << "1.1.1.1";
    caps.setIpProposals( ips );

    // Model Stuff
    QStringList models;
    models << "Opie and Qtopia 1.6" << "Sharp Zaurus ROM";
    caps.setModels( models );
    caps.setNeedsModelName( true );

    return caps;
}

void QtopiaPlugin::setCapabilities( const KSync::Kapabilities& caps )
{
    d->socket->setDestIP( caps.destIP() );
    d->socket->setUser( caps.user() );
    d->socket->setPassword( caps.password() );
    d->socket->setModel( caps.currentModel(), caps.modelName() );
    d->socket->startUp();
}

bool QtopiaPlugin::readSyncees()
{
    d->socket->setResources( resources() );
    return d->socket->startSync();
}

bool QtopiaPlugin::startBackup( const QString & )
{
    return false;
}

bool QtopiaPlugin::startRestore( const QString& )
{
    return false;
}

bool QtopiaPlugin::connectDevice()
{
    d->socket->startUp();
    return true;
}

bool QtopiaPlugin::disconnectDevice()
{
    d->socket->hangUP();
    return true;
}

QString QtopiaPlugin::metaId() const
{
    return d->socket->metaId();
}

QIconSet QtopiaPlugin::iconSet() const
{
    kdDebug(5225) << "iconSet" << endl;
    QPixmap logo;
    logo.load( locate( "appdata", "pics/opie.png" ) );
    return QIconSet( logo );
}

QString QtopiaPlugin::iconName() const
{
    return QString::fromLatin1("opie.png");
}

bool QtopiaPlugin::writeSyncees()
{
    kdDebug(5201) << " writing it now " << endl;
    d->socket->write( SynceeList() );
    return true;
}

/* private slots for communication here */
void QtopiaPlugin::slotSync(SynceeList lst )
{
    emit synceesRead( this , lst );
}

void QtopiaPlugin::slotError( const Error& err )
{
    error( err );
}

void QtopiaPlugin::slotProg( const Progress& prog )
{
    progress( prog );
}

KonnectorInfo QtopiaPlugin::info() const
{
    return KonnectorInfo(QString::fromLatin1("Qtopia Konnector"),
                         iconSet(),
                         QString::fromLatin1("Qtopia1.5"),
                         metaId(),
                         iconName(),
                         d->socket->isConnected() );
}

void QtopiaPlugin::download( const QString& res)
{
    d->socket->download( res );
}

ConfigWidget* QtopiaPlugin::configWidget( const Kapabilities& cap, QWidget* parent, const char* name )
{
    return new OpieHelper::QtopiaConfig( cap, parent, name );
}

ConfigWidget* QtopiaPlugin::configWidget( QWidget* parent, const char* name )
{
    return new OpieHelper::QtopiaConfig( parent, name );
}

#include "konnector.moc"
