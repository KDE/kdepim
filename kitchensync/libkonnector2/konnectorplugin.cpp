#include "error.h"
#include "progress.h"

#include "konnectorplugin.h"

using namespace KSync;

KonnectorPlugin::KonnectorPlugin( QObject* obj, const char* name, const QStringList& )
    : QObject( obj,  name ) {

}
void KonnectorPlugin::setUDI( const QString& ) {

}
QString KonnectorPlugin::udi()const{

}

void KonnectorPlugin::add( const QString& res ) {

}
bool KonnectorPlugin::isConnected()const{

}
void KonnectorPlugin::progress( const Progress& prog ) {
    emit sig_progress( m_udi, prog );
}
void KonnectorPlugin::error( const Error& err ) {
    emit sig_error( m_udi, err );
}

#include "konnectorplugin.moc"
