#include "socket.h"

using namespace KSync;

/**
 * QtopiaSocket is somehow a state machine
 * during authentication
 * then it takes care of fethcing and converting
 */
QtopiaSocket::QtopiaSocket( QObject* obj, const char* name ) {

}
QtopiaSocket::~QtopiaSocket() {

}
void QtopiaSocket::setUser( const QString& user ) {

}
void QtopiaSocket::setPassword( const QString& pass ) {

}
void QtopiaSocket::setSrcIP( const QString& ) {

}
void QtopiaSocket::setDestIP( const QString& ) {

}
void QtopiaSocket::startUp() {

}
bool QtopiaSocket::startSync() {

}
bool QtopiaSocket::isConnected() {

}
QByteArray QtopiaSocket::retrFile( const QString& file ) {

}
Syncee* QtopiaSocket::retrEntry( const QString& file ) {

}
bool QtopiaSocket::insertFile( const QString& ) {

}
void QtopiaSocket::write( const QString&, const QByteArray& ) {

}
void QtopiaSocket::write( Syncee::PtrList ) {

}
void QtopiaSocket::write( KOperations::ValueList ) {

}
QString QtopiaSocket::metaId()const {

};

#include "socket.moc"
