#include <qiconset.h>
#include <qpair.h>
#include <qvaluelist.h>

#include <kdebug.h>
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <kgenericfactory.h>

#include <kapabilities.h>

#include "socket.h"
#include "konnector.h"


typedef KGenericFactory<KSync::QtopiaPlugin, QObject>  QtopiaKonnectorPlugin;
K_EXPORT_COMPONENT_FACTORY( libqtopiakonnector,  QtopiaKonnectorPlugin );


using namespace KSync;


class QtopiaPlugin::Private {
public:
    Private() : socket(0l), connection(false){
    }
    QtopiaSocket* socket;
    QString udi;
    bool connection : 1;
    QStringList retrFiles;
};

QtopiaPlugin::QtopiaPlugin( QObject* obj, const char* name, const QStringList )
    : KonnectorPlugin(obj, name ) {
    d = new Private;
    d->socket = new QtopiaSocket(this, "Opie Socket" );

    /* now do some signal and slot connection */
    connect(d->socket, SIGNAL(sync(Syncee::PtrList) ),
            this, SLOT(slotSync(Syncee::PtrList) ) );
    connect(d->socket, SIGNAL(errorKonnector(int, QString ) ),
            this, SLOT(slotError(int, QString) ) );
    connect(d->socket, SIGNAL(stateChanged(bool) ),
            this, SLOT(slotChanged(bool) ) );
}
QtopiaPlugin::~QtopiaPlugin() {
    delete d;
}
void QtopiaPlugin::setUDI( const QString& udi ) {
    d->udi = udi;
}
QString QtopiaPlugin::udi()const {
    return d->udi;
}
Kapabilities QtopiaPlugin::capabilities() {
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

    return caps;
}
void QtopiaPlugin::setCapabilities( const KSync::Kapabilities& caps) {
    d->socket->setDestIP( caps.destIP() );
    d->socket->setUser( caps.user() );
    d->socket->setPassword( caps.password() );
    d->socket->startUp();
}
bool QtopiaPlugin::startSync() {
    return d->socket->startSync();
}
bool QtopiaPlugin::startBackup( const QString&  ) {
    return true;
}
bool QtopiaPlugin::startRestore( const QString& ) {
    return true;
}
bool QtopiaPlugin::connectDevice() {
    return true;
}
void QtopiaPlugin::disconnectDevice() {

}
bool QtopiaPlugin::isConnected() {
    return d->socket->isConnected();
}
bool QtopiaPlugin::insertFile( const QString& file ) {
    return d->socket->insertFile( file );
}
QByteArray QtopiaPlugin::retrFile( const QString& file ) {
    return d->socket->retrFile( file );
}
Syncee* QtopiaPlugin::retrEntry( const QString& path ) {
    return d->socket->retrEntry( path );
}
QString QtopiaPlugin::metaId()const {
    return d->socket->metaId();
}
QIconSet QtopiaPlugin::iconSet()const {
    kdDebug(5224) << "iconSet" << endl;
    QPixmap logo;
    logo.load( locate("appdata",  "pics/opie_logo.png" ) );
    return QIconSet( logo );
}
QString QtopiaPlugin::iconName()const {
    return QString::fromLatin1("pics/opie_logo.png");
}
QString QtopiaPlugin::id()const {
    return QString::fromLatin1("Qtopia1.5");
}
ConfigWidget* QtopiaPlugin::configWidget( const Kapabilities&, QWidget*, const char* ) {
    return 0l;
}
ConfigWidget* QtopiaPlugin::configWidget( QWidget*, const char* ) {
    return 0l;
}
void QtopiaPlugin::slotWrite( const QString& str, const QByteArray& ar) {
    d->socket->write( str, ar );
}
void QtopiaPlugin::slotWrite( Syncee::PtrList lst) {
    d->socket->write( lst );
}
void QtopiaPlugin::slotWrite( KOperations::ValueList ) {

}

/* private slots for communication here */
void QtopiaPlugin::slotSync(Syncee::PtrList lst ) {
    emit sync(d->udi, lst );
}
void QtopiaPlugin::slotError(int mode, QString error) {
    emit errorKonnector(d->udi, mode, error );
}
void QtopiaPlugin::slotChanged(bool b) {
    emit stateChanged( d->udi, b );
}

#include "konnector.moc"
