#include <kgenericfactory.h>

#include <konnectorinfo.h>

#include "configwidget.h"
#include "socket.h"
#include "agendaplugin.h"

typedef KGenericFactory<KSync::AgendaPlugin, QObject>  AgendaKonnectorPlugin;
K_EXPORT_COMPONENT_FACTORY( libagendakonnector,  AgendaKonnectorPlugin )

using namespace KSync;


AgendaPlugin::AgendaPlugin( QObject* obj, const char* name,const QStringList )
    : Konnector( obj, name ) {
    m_socket = new AgendaSocket(this);

    /* connect the bridge */
    connect(m_socket, SIGNAL(sync(SynceeList) ),
            this, SLOT(slotSync(SynceeList) ) );
    connect(m_socket, SIGNAL(error(const Error& ) ),
            this, SLOT(slotError(const Error& ) ) );
    connect(m_socket, SIGNAL(prog(const Progress&) ),
            this, SLOT(slotProg(const Progress& ) ) );
}
AgendaPlugin::~AgendaPlugin() {

}
KSync::Kapabilities AgendaPlugin::capabilities() {
    KSync::Kapabilities caps;

    caps.setSupportMetaSyncing( true ); // we can meta sync
    caps.setSupportsPushSync( true ); // we can initialize the sync from here
    caps.setNeedsConnection( true ); // we need to have pppd running
    caps.setSupportsListDir( true ); // we will support that once there is API for it...
    caps.setNeedsIPs( true ); // we need the IP
    caps.setNeedsSrcIP( false ); // we do not bind to any address...
    caps.setNeedsDestIP( true ); // we need to know where to connect
    caps.setAutoHandle( false ); // we currently do not support auto handling
    caps.setNeedAuthentication( false ); // HennevL says we do not need that
    caps.setNeedsModelName( true ); // we need a name for our meta path!

    return caps;
}
void AgendaPlugin::setCapabilities( const KSync::Kapabilities& caps ) {
    m_socket->setIP( caps.destIP() );
    m_socket->setMetaName( caps.modelName() );
    m_socket->startUP(); // connect now
}
bool AgendaPlugin::readSyncees() {
    m_socket->startSync();
    return true;
}
bool AgendaPlugin::connectDevice() {
    m_socket->startUP();
    return true;
}
bool AgendaPlugin::disconnectDevice() {
    m_socket->hangUP();
    return true;
}
KSync::KonnectorInfo AgendaPlugin::info()const {
    return KonnectorInfo( i18n("Agenda Vr3"),
                          QIconSet(),
                          QString::fromLatin1("AgendaVr3"),  // same as the .desktop file
                          m_socket->metaName(),
                          "agenda", // icon name
                          m_socket->isConnected() );
}
void AgendaPlugin::download( const QString& ) {
    error( StdError::downloadNotSupported() );
}
KSync::ConfigWidget* AgendaPlugin::configWidget( const KSync::Kapabilities& cap, QWidget* parent,
                                                 const char* name ) {
    return new Vr3::ConfigWidget( cap, parent, name );
}
KSync::ConfigWidget* AgendaPlugin::configWidget( QWidget* parent, const char* name ) {
    return new Vr3::ConfigWidget( parent, name );
}

// bridging!! below
bool AgendaPlugin::writeSyncees() {
    m_socket->write( SynceeList() );
    return true;
}
void AgendaPlugin::slotSync( SynceeList lst) {
    emit synceesRead( this, lst );
}
void AgendaPlugin::slotError( const Error& err ) {
    error( err );
}
void AgendaPlugin::slotProg( const Progress& prog ) {
    progress( prog );
}


#include "konnector.moc"
