#include <kconfig.h>

#include "md5map.h"

using namespace OpieHelper;

MD5Map::MD5Map( const QString& file)
    : m_conf(0l)
{
    load( file );
}
MD5Map::~MD5Map() {
    delete m_conf;
}
void MD5Map::load( const QString& file ) {
    m_file = file;
    /* return here cause reading is not possible */
    if (m_file.isEmpty() ) return;

    KConfig* conf = config();
    QStringList list = conf->groupList();
    for (QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
        conf->setGroup( (*it) );
        insert( (*it), conf->readEntry("sum") );
    }
}
void MD5Map::save() {
    KConfig* conf = config();

    QStringList groups = conf->groupList();
    for (QStringList::Iterator it = groups.begin(); it != groups.end(); ++it ) {
        conf->deleteGroup( (*it) );
    }

    Iterator it;
    for ( it = m_map.begin(); it != m_map.end(); ++it ) {
        conf->setGroup( it.key() );
        conf->writeEntry( "sum", it.data() );
    }
    conf->sync();
    qWarning("save %s",  m_file.latin1() );
}
void MD5Map::setFileName( const QString& file ) {
    m_file = file;
}
QString MD5Map::md5sum(const QString& uid )const {
    return m_map[uid];
}
bool MD5Map::contains( const QString& uid )const {
    return m_map.contains( uid );
}
void MD5Map::insert( const QString& uid, const QString& str) {
    m_map.insert( uid, str );
}
void MD5Map::set( const Map& map) {
    m_map = map;
}
MD5Map::Map MD5Map::map()const{
    return m_map;
}
void MD5Map::clear() {
    m_map.clear();
    KConfig* conf = config();
    QStringList groups = conf->groupList();
    for (QStringList::Iterator it = groups.begin(); it != groups.end(); ++it ) {
        conf->deleteGroup( (*it) );
    }
}
KConfig* MD5Map::config() {
    if (!m_conf ) {
        m_conf = new KConfig( m_file, false, false );
    }
    return m_conf;
}
