#include <klocale.h>

#include "socket.h"

using namespace KSync;


AgendaSocket::AgendaSocket( QObject* obj )
    : QObject(obj, "AgendaSocket") {
    m_isConnected = false;
}
AgendaSocket::~AgendaSocket() {
}
void AgendaSocket::setIP(const QString& ip ) {
    m_ip = ip;
}
void AgendaSocket::setMetaName( const QString& name ) {
    m_meta = name;
}
QString AgendaSocket::metaName()const {
    return m_meta;
}
/* try to connect */
void AgendaSocket::startUP() {
    emit prog( StdProgress::connection() );
    emit prog( StdProgress::authenticated() );
    emit prog( StdProgress::connected() ); // finally connected
}
void AgendaSocket::hangUP() {
    emit prog( Progress(i18n("Disconnected from the device.") ) );
    m_isConnected = false;
}
bool AgendaSocket::isConnected()const {
    return m_isConnected;
}
void AgendaSocket::startSync() {
    emit prog( Progress( i18n("Starting to sync now") ) );

    /*
     * download and convert
     *
     * DO Collect MetaInformations
     */

    SynceeList lst;
    emit sync(lst);
}
void AgendaSocket::write( SynceeList lst) {
    // reconvert and write back!!!

    /**
     * convert and upload
     *
     * Do write MetaInformations!!
     */

    emit prog(StdProgress::done() );
}

#include "socket.moc"
