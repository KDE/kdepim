#include "konnectorinfo.h"

using namespace KSync;

KonnectorInfo::KonnectorInfo( const QString& name,
                              const QIconSet& set,
                              const QString& id,
                              const QString& meta,
                              const QString& iconName,
                              const QString& udi,
                              bool isCon ) {
    m_na = name;
    m_icon = set;
    m_id = id;
    m_meta = meta;
    m_name = iconName;
    m_udi = udi;
    m_con = isCon;
}
KonnectorInfo::~KonnectorInfo() {
}
bool KonnectorInfo::operator==( const KonnectorInfo& rhs) {
    if ( m_na   != rhs.m_na   ) return false;
    if ( m_id   != rhs.m_id   ) return false;
    if ( m_meta != rhs.m_meta ) return false;
    if ( m_name != rhs.m_name ) return false;
    if ( m_udi  != rhs.m_udi  ) return false;

    return true;
}
QString KonnectorInfo::name()const{
    return m_na;
}
QIconSet KonnectorInfo::iconSet()const {
    return m_icon;
}
QString KonnectorInfo::id()const {
    return m_id;
}
QString KonnectorInfo::metaId()const {
    return m_meta;
}
QString KonnectorInfo::iconName()const {
    return m_name;
}
QString KonnectorInfo::udi()const {
    return m_udi;
}
bool KonnectorInfo::isConnected()const {
    return m_con;
}
