#include "filter.h"
#include "konnectorinfo.h"
#include "kapabilities.h"

#include "konnectormanager.h"

using namespace KSync;

KonnectorManager::KonnectorManager() {

}
KonnectorManager::~KonnectorManager() {

}
KonnectorManager* KonnectorManager::self() {

}
Device::ValueList KonnectorManager::query( const QString& category ) {

}
UDI KonnectorManager::load( const Device& dev ) {

}
UDI KonnectorManager::load( const QString& dev ) {

}
bool KonnectorManager::unload( const UDI& ) {

}
Kapabilities KonnectorManager::capabilities( const UDI& udi )const {

}
void KonnectorManager::setCapabilities( const UDI&, const Kapabilities& ) {

}
ConfigWidget* KonnectorManager::configWidget( const UDI& uid,
                                              QWidget* parent,
                                              const char* name ) {

}
ConfigWidget* KonnectorManager::configWidget( const UDI& uid,
                                              const Kapabilities&,
                                              QWidget* parent,
                                              const char* name ) {

}
void KonnectorManager::add( const UDI&,  const QString& resource ) {

}
void KonnectorManager::download( const UDI&, const QString& resource ) {

}
bool KonnectorManager::isConnected( const UDI& ) {

}
bool KonnectorManager::connect( const UDI& ) {

}
bool KonnectorManager::disconnect( const UDI& ) {

}
bool KonnectorManager::startSync( const UDI& ) {

}
bool KonnectorManager::startBackup( const UDI&, const QString& path ) {

}
bool KonnectorManager::startRestore( const UDI&, const QString& path ) {

}
KonnectorInfo KonnectorManager::info( const UDI& )const {

}
bool KonnectorManager::autoLoadFilter()const{

}
void KonnectorManager::setAutoLoadFilter( bool ) {

}
void KonnectorManager::add( Filter* ) {

}
void KonnectorManager::deleteFilter( Filter* ) {

}
const Filter::PtrList KonnectorManager::filters() {

}
void KonnectorManager::write( const QString&, Syncee::PtrList ) {

}

#include "konnectormanager.moc"
