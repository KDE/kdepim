
#include <kapplication.h>
#include <kconfig.h>

#include "konnectorprofile.h"

using namespace KSync;

KonnectorProfile::KonnectorProfile( const QString& name,
                                    const QString& icon,
                                    const Device& dev )
    : m_name( name), m_icon( icon ),  m_dev( dev )
{
    m_uid = kapp->randomString(8 );
}
KonnectorProfile::KonnectorProfile() {
    m_uid = kapp->randomString( 8 );
}
KonnectorProfile::KonnectorProfile( const KonnectorProfile& prof ) {
    (*this) = prof;
}
KonnectorProfile::~KonnectorProfile() {

}
bool KonnectorProfile::operator==( const KonnectorProfile& other ) {
    if ( m_uid != other.m_uid ) return false;
    if ( m_udi != other.m_udi ) return false;
    if ( m_name != other.m_name ) return false;
    if ( !(m_dev == other.m_dev) ) return false; // fixme

    return true;
}
KonnectorProfile &KonnectorProfile::operator=( const KonnectorProfile& other ) {
    m_name = other.m_name;
    m_icon = other.m_icon;
    m_dev = other.m_dev;
    m_uid = other.m_uid;
    m_udi = other.m_udi;

    return *this;
}
QString KonnectorProfile::uid() const {
    return m_uid;
}
QString KonnectorProfile::name() const {
    return m_name;
}
QString KonnectorProfile::icon() const {
    return m_icon;
}
Device KonnectorProfile::device() const {
    return m_dev;
}
QString KonnectorProfile::udi() const {
    return m_udi;
}

void KonnectorProfile::setUid( const QString& uid ) {
    m_uid = uid;
}
void KonnectorProfile::setName( const QString& name ) {
    m_name = name;
}
void KonnectorProfile::setIcon( const QString& icon ) {
    m_icon = icon;
}
void KonnectorProfile::setDevice( const Device& dev ) {
    m_dev = dev;
}
void KonnectorProfile::setUdi( const QString& udi ) {
    m_udi = udi;
}
void KonnectorProfile::saveToConfig( KConfig* config ) const{
    config->setGroup(m_uid );
    config->writeEntry("Name", m_name );
    config->writeEntry("Icon", m_icon );
    // store the device
    config->writeEntry("Ident", m_dev.identify() );
    config->writeEntry("Group", m_dev.group() );
    config->writeEntry("Vendor", m_dev.vendor() );
    config->writeEntry("Id", m_dev.id() );
    config->writeEntry("Lib", m_dev.library() );

}
void KonnectorProfile::loadFromConfig( KConfig* conf ) {
    m_uid = conf->group();
    m_icon = conf->readEntry("Icon");
    m_name = conf->readEntry("Name");

    QString ident, grp, vend, lib, id;
    ident = conf->readEntry("Ident");
    grp = conf->readEntry("Group");
    vend = conf->readEntry("Vendor");
    id = conf->readEntry("Id");
    lib = conf->readEntry("Lib");
    m_dev = Device( ident, grp,  vend,  lib,  id );

}
