/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Holger Freyther <zecke@handhelds.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <kapplication.h>
#include <kconfig.h>

#include "konnectorprofile.h"

using namespace KSync;

KonnectorProfile::KonnectorProfile( const QString& name,
                                    const QString& icon,
                                    const Device& dev )
    : m_name( name), m_icon( icon ),  m_dev( dev ), m_konnector( 0 )
{
    m_uid = kapp->randomString(8 );
    m_wasLoaded = false;
}

KonnectorProfile::KonnectorProfile()
  : m_konnector( 0 )
{
    m_uid = kapp->randomString( 8 );
    m_wasLoaded = false;
}

KonnectorProfile::KonnectorProfile( const KonnectorProfile& prof )
{
    (*this) = prof;
}

KonnectorProfile::~KonnectorProfile()
{
}

bool KonnectorProfile::operator==( const KonnectorProfile& other )
{
    if ( m_uid != other.m_uid ) return false;
    if ( m_konnector != other.m_konnector ) return false;
    if ( m_name != other.m_name ) return false;
    if ( !(m_dev == other.m_dev) ) return false; // fixme

    return true;
}

bool KonnectorProfile::operator==( const KonnectorProfile& other ) const
{
    if ( m_uid != other.m_uid ) return false;
    if ( m_konnector != other.m_konnector ) return false;
    if ( m_name != other.m_name ) return false;
    //if (!(m_dev == other.m_dev) ) return false; fixme const

    return true;
}

KonnectorProfile &KonnectorProfile::operator=( const KonnectorProfile& other )
{
    m_name = other.m_name;
    m_icon = other.m_icon;
    m_dev = other.m_dev;
    m_uid = other.m_uid;
    m_konnector = other.m_konnector;
    m_caps = other.m_caps;
    m_wasLoaded = other.m_wasLoaded;

    return *this;
}

QString KonnectorProfile::uid() const
{
    return m_uid;
}

QString KonnectorProfile::name() const
{
    return m_name;
}

QString KonnectorProfile::icon() const
{
    return m_icon;
}

Device KonnectorProfile::device() const
{
    return m_dev;
}

Konnector *KonnectorProfile::konnector() const
{
    return m_konnector;
}

Kapabilities KonnectorProfile::kapabilities() const
{
    return m_caps;
}

bool KonnectorProfile::wasLoaded() const
{
    return m_wasLoaded;
}

void KonnectorProfile::setUid( const QString& uid )
{
    m_uid = uid;
}

void KonnectorProfile::setName( const QString& name )
{
    m_name = name;
}

void KonnectorProfile::setIcon( const QString& icon )
{
    m_icon = icon;
}

void KonnectorProfile::setDevice( const Device& dev )
{
    m_dev = dev;
}

void KonnectorProfile::setKonnector( Konnector *k )
{
    m_konnector = k;
}

void KonnectorProfile::setKapabilities( const Kapabilities& caps )
{
    m_caps = caps;
}

bool KonnectorProfile::isValid() const
{
    return !m_dev.identify().stripWhiteSpace().isEmpty();
}

void KonnectorProfile::saveToConfig( KConfig* config ) const
{
    config->setGroup(m_uid );
    config->writeEntry("Name", m_name );
    config->writeEntry("Icon", m_icon );
    // store the device
    config->writeEntry("Ident", m_dev.identify() );
    config->writeEntry("Group", m_dev.group() );
    config->writeEntry("Vendor", m_dev.vendor() );
    config->writeEntry("DevName", m_dev.name() );
    config->writeEntry("Lib", m_dev.library() );
#ifdef fixsaveconfig
    config->writeEntry("UDI", udi() );
#endif

    saveKaps( config );
}

void KonnectorProfile::saveKaps( KConfig* conf ) const
{
    //let's save the Kapabilities object
    conf->writeEntry("Meta", m_caps.isMetaSyncingEnabled() );
    conf->writeEntry("Push", m_caps.supportsPushSync() );
    conf->writeEntry("Con", m_caps.needsConnection() );
    conf->writeEntry("listDir", m_caps.supportsListDir() );
    conf->writeEntry("Port", m_caps.currentPort() );
    conf->writeEntry("NeedsNetwork", m_caps.needsNetworkConnection() );
    if ( m_caps.needsNetworkConnection() ) {
        if (m_caps.needsIPs() && m_caps.needsSrcIP() )
            conf->writeEntry("SrcIP", m_caps.srcIP() );
        if (m_caps.needsIPs() && m_caps.needsDestIP() )
            conf->writeEntry("DestIP", m_caps.destIP() );
    }
    conf->writeEntry("AutoHandle",  m_caps.canAutoHandle() );

    // auth
    conf->writeEntry("Auth", m_caps.needAuthentication() );
    if (m_caps.needAuthentication() ) {
        conf->writeEntry("User", m_caps.user() );
        conf->writeEntry("Pass", m_caps.password() );
    }
    conf->writeEntry("Model", m_caps.currentModel() );
    if ( m_caps.needsModelName() ) {
        conf->writeEntry("ModelName", m_caps.modelName() );
    }
    conf->writeEntry("ConMode", m_caps.currentConnectionMode() );
    QMap<QString, QString> map = m_caps.extras();
    QMap<QString, QString>::Iterator it;
    QStringList Extras;
    for (it = map.begin(); it != map.end() ; ++it ) {
        Extras << it.key();
        conf->writeEntry("ExtraKey"+it.key(), it.data() );
    }
    conf->writeEntry("Extras", Extras );
}

void KonnectorProfile::loadFromConfig( KConfig* conf )
{
    m_uid = conf->group();
    m_icon = conf->readEntry("Icon");
    m_name = conf->readEntry("Name");

    QString ident, grp, vend, lib, name;
    ident = conf->readEntry("Ident");
    grp = conf->readEntry("Group");
    vend = conf->readEntry("Vendor");
    name = conf->readEntry("DevName");
    lib = conf->readEntry("Lib");
    m_dev = Device( name, grp,  vend,  lib,  ident );

#ifdef fixloadconfig
    // get the udi
    QString udi = conf->readEntry("UDI");
    if (!udi.isEmpty() )
        m_wasLoaded = true;
#endif

    m_caps = readKaps( conf );
}

Kapabilities KonnectorProfile::readKaps( KConfig* conf )
{
    Kapabilities caps;
    bool dummy;
    caps.setMetaSyncingEnabled( conf->readBoolEntry("Meta") );
    caps.setSupportsPushSync( conf->readBoolEntry("Push") );
    caps.setNeedsConnection( conf->readBoolEntry("Con") );
    caps.setSupportsListDir( conf->readBoolEntry("listDir") );
    caps.setCurrentPort( conf->readNumEntry("Port") );

    dummy = conf->readBoolEntry("NeedsNetwork", false );
    caps.setNeedsConnection(dummy );
    if ( dummy ) {
        caps.setSrcIP( conf->readEntry("SrcIP") );
        caps.setDestIP( conf->readEntry("DestIP") );
    }
    caps.setAutoHandle( conf->readBoolEntry("AutoHandle") );

    dummy = conf->readBoolEntry("Auth",  false);
    caps.setNeedAuthentication( dummy );
    if ( dummy ) {
        caps.setUser( conf->readEntry("User") );
        caps.setPassword( conf->readEntry("Pass") );
    }
    caps.setCurrentModel( conf->readEntry("Model")  );
    QString  str = conf->readEntry("ModelName");
    if (!str.isEmpty() ) {
        caps.setNeedsModelName( true );
        caps.setModelName( str );
    }

    caps.setCurrentConnectionMode( conf->readEntry("ConMode") );

    // extras
    QStringList list = conf->readListEntry("Extras");
    QStringList::Iterator it;

    for (it = list.begin(); it != list.end(); ++it ) {
        caps.setExtraOption( (*it),  conf->readEntry("ExtraKey"+(*it) ) );
    }

    return caps;
}
