#include "notify.h"

using namespace KSync;

Notify::Notify( int code,  const QString& text ){
    m_code = code;
    m_text = text;
}
Notify::Notify( const QString& text ) {
    m_code = -1;
    m_text = text;
}
Notify::~Notify() {
// delete d;
}
bool Notify::operator==( const Notify& rhs) {
    if ( m_code != rhs.m_code ) return false;
    if ( m_text != rhs.m_text ) return false;

    return true;
}
int Notify::code()const{
    return m_code;
}
QString Notify::text()const{
    return m_text;
}
