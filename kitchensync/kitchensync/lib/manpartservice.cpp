
#include "manpartservice.h"

using namespace KSync;


ManPartService::ManPartService() {

}
ManPartService::~ManPartService() {

}
ManPartService::ManPartService( const KService::Ptr& service )
    : m_name( service->name() ), m_comment( service->comment() ) ,m_icon( service->icon() ),  m_lib( service->library() ) {


}
ManPartService::ManPartService( const ManPartService& man ) {
    *this = man;
};
QString ManPartService::name() const {
    return m_name;
}
QString ManPartService::comment() const {
    return m_comment;
}
QString ManPartService::libname() const {
    return m_lib;
}
QString ManPartService::icon() const {
    return m_icon;
}
void ManPartService::setName( const QString& name ) {
    m_name = name;
}
void ManPartService::setComment( const QString& comment ) {
    m_comment = comment;
}
void ManPartService::setLibname( const QString& libName) {
    m_lib = libName;
}
void ManPartService::setIcon( const QString& icon ) {
    m_icon = icon;
}
ManPartService &ManPartService::operator=( const ManPartService& man1 ) {

    m_name = man1.m_name;
    m_comment = man1.m_comment;
    m_icon = man1.m_icon;
    m_lib = man1.m_lib;
    return *this;
}
bool ManPartService::operator== ( const ManPartService& par2 ) {
    if ( name() == par2.name() &&
         comment() == par2.comment() &&
         icon() == par2.icon() &&
         libname() == par2.libname() )
        return true;
    else return false;
}
bool ManPartService::operator== ( const ManPartService& par2 )const {
    if ( name() == par2.name() &&
         comment() == par2.comment() &&
         icon() == par2.icon() &&
         libname() == par2.libname() )
        return true;
    else return false;
}
