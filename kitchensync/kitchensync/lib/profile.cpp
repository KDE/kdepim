
#include <kapplication.h>

#include "ksync_profile.h"

using namespace KSync;

Profile::Profile()
{
// generate new uid
    m_uid =kapp->randomString( 8 );
}

Profile::Profile( const Profile& prof )
{
    (*this) = prof;
}
Profile::~Profile()
{

}
QString Profile::name() const
{
    return m_name;
}
QString Profile::uid() const {
    return m_uid;
}
QString Profile::pixmap() const {
    return m_pixmap;
}
bool Profile::confirmDelete()const {
    return m_confirmDelete;
}
bool Profile::confirmSync()const {
    return m_confirmSync;
}
void Profile::setName( const QString& name ) {
    m_name = name;
}
void Profile::setPixmap( const QString& pixmap ) {
    m_pixmap = pixmap;
}
void Profile::setUid(const QString& uid) {
    m_uid = uid;
}
void Profile::setManParts( const ManPartService::ValueList& list ) {
    m_list = list;
}
ManPartService::ValueList Profile::manParts() const {
    return m_list;
}
QMap<QString,QString> Profile::paths() const {
    return m_map;
}
void Profile::setPaths(const QMap<QString, QString>& map ) {
    m_map = map;
}
void Profile::setPath( const QString& partName,  const QString& path ) {
    m_map.replace( partName, path );
}
void Profile::setConfirmSync( bool b ) {
    m_confirmSync = b;
}
void Profile::setConfirmDelete( bool b ) {
    m_confirmDelete = b;
}
QString Profile::path( const QString& partName) const {
    QMap<QString, QString>::ConstIterator it;
    QString path;
    it = m_map.find( partName );
    if ( it != m_map.end() )
        path = it.data();

    return path;
}
Profile &Profile::operator=( const Profile &prof )
{
    if (&prof == this ) return *this;

    m_name = prof.m_name;
    m_uid = prof.m_uid;
    m_pixmap = prof.m_pixmap;
    m_list = prof.m_list;
    m_map = prof.m_map;
    m_confirmSync = prof.m_confirmSync;
    m_confirmDelete = prof.m_confirmDelete;
    return *this;
}

bool Profile::operator==( const Profile& prof2 ) {
    if ( uid() == prof2.uid() &&
         name() == prof2.name() &&
         pixmap() == prof2.pixmap() )
        return true;
    else return false;

}
/*bool operator!=( const Profile& prof1, const Profile& prof2 ) {
    return !( prof1 == prof2 );
}
*/
