/* This file is part of the KDE librariesand KitchenSync Projects
    Copyright (C) 2002 Holger Freyther <freyher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/

#include <kdebug.h>
#include <qglobal.h>
#include "kapabilities.h"

using namespace KSync;

class Kapabilities::KapabilitiesPrivate
{
public:
  KapabilitiesPrivate(){
  };
};

Kapabilities::Kapabilities()
{
    //d = 0;
    //d = new KapabilitiesPrivate;
    m_push = false;
    m_needConnection = false;
    m_listdir = false;
    m_needsIp = false;
    m_needsSrcIp = false;
    m_needsDestIp = false;
    m_canHandle = false;
    m_needsAuthent=false;
    m_meta = false;
    m_supMeta = false;
    m_needsNet = true;
    m_current = -1;
}
Kapabilities::Kapabilities(const Kapabilities &kap )
    //: //d( 0 )
{
    //d = new KapabilitiesPrivate;
    //d = kap.d;
    (*this) = kap;
}
Kapabilities::~Kapabilities()
{
    //delete d;
}
bool Kapabilities::supportsMetaSyncing() const
{
    return m_supMeta;
}
void Kapabilities::setSupportMetaSyncing( bool meta )
{
    m_supMeta = meta;
}
void Kapabilities::setMetaSyncingEnabled( bool enable )
{
    m_meta = enable;
}
bool Kapabilities::isMetaSyncingEnabled() const
{
    return m_meta;
}
bool Kapabilities::supportsPushSync() const
{
  return m_push;
}
void Kapabilities::setSupportsPushSync(bool push)
{
  m_push = push;
}
bool Kapabilities::needsConnection() const
{
  return m_needConnection;
}
void Kapabilities::setNeedsConnection(bool connection)
{
  m_needConnection = connection;
}
bool Kapabilities::supportsListDir() const
{
  return m_listdir;
}

void Kapabilities::setSupportsListDir(bool listDir)
{
  m_listdir = listDir;
}
QMemArray<int> Kapabilities::ports()const
{
  return m_ports;
}
void Kapabilities::setPorts(const QMemArray<int> & ports)
{
  m_ports = ports;
}
int Kapabilities::currentPort() const
{
  return m_current;
}
void Kapabilities::setCurrentPort(int port )
{
  m_current = port;
}
bool Kapabilities::needsIPs() const
{
  return m_needsIp;
}
bool Kapabilities::needsSrcIP() const
{
  return m_needsSrcIp;
}
bool Kapabilities::needsDestIP() const
{
  return m_needsDestIp;
}
void Kapabilities::setNeedsIPs(bool needs)
{
  m_needsIp = needs;
}
void Kapabilities::setNeedsSrcIP(bool needs)
{
  m_needsSrcIp = needs;
}
void Kapabilities::setNeedsDestIP(bool needs)
{
  m_needsDestIp = needs;
}
void Kapabilities::setSrcIP(const QString &addr)
{
  m_src = addr;
}
void Kapabilities::setDestIP(const QString &addr)
{
  m_dest = addr;
}
QString Kapabilities::srcIP() const
{
  return m_src;
}
QString Kapabilities::destIP() const
{
  return m_dest;
}
bool Kapabilities::canAutoHandle()const
{
  return m_canHandle;
}
void Kapabilities::setAutoHandle(bool handle)
{
  m_canHandle = handle;
}
QStringList  Kapabilities::ipProposals() const
{
  return m_propsIPs;
}
void Kapabilities::setIpProposals( const QStringList &ips )
{
  m_propsIPs = ips;
}
bool Kapabilities::needAuthentication() const
{
  return m_needsAuthent;
}
void Kapabilities::setNeedAuthentication(bool authent)
{
  m_needsAuthent = authent;
}
void Kapabilities::setUser(const QString &user )
{
  m_user = user;
}
void Kapabilities::setPassword(const QString &pass )
{
  m_pass = pass;
}
QString Kapabilities::password() const
{
  return m_pass;
}
QString Kapabilities::user() const
{
  return m_user;
}
QValueList<QPair<QString,QString> > Kapabilities::userProposals() const
{
  return m_propAuth;
}
void Kapabilities::setUserProposals( QValueList< QPair<QString, QString> > auth )
{
  m_propAuth = auth;
}
void Kapabilities::setExtraOption( const QString &extra, const QString &variant )
{
    m_extras.replace( extra, variant );
}
bool Kapabilities::needsNetworkConnection()const
{
    return m_needsNet;
}
void Kapabilities::setNeedsNetworkConnection( bool net )
{
    m_needsNet = net;
}
QStringList Kapabilities::models() const
{
    return m_models;
}
void Kapabilities::setModels( const QStringList& model )
{
    m_models = model;
}
QString Kapabilities::currentModel() const
{
    return m_currModell;
}
void Kapabilities::setCurrentModel( const QString &mod )
{
    m_currModell = mod;
}
void Kapabilities::setConnectionMode( const QStringList &mode )
{
    m_modes= mode;
}
QStringList Kapabilities::connectionModes() const
{
    return m_modes;
}
QString Kapabilities::currentConnectionMode()const
{
    return m_currMode;
}
void Kapabilities::setCurrentConnectionMode( const QString &mode )
{
    m_currMode = mode;
}
Kapabilities &Kapabilities::operator=(const Kapabilities &rhs )
{
    m_push = rhs.m_push;
    m_needConnection = rhs.m_needConnection;
    m_listdir = rhs.m_listdir;
    m_needsIp = rhs.m_needsIp;
    m_needsSrcIp = rhs.m_needsSrcIp;
    m_needsDestIp = rhs.m_needsDestIp;
    m_needsAuthent = rhs.m_needsAuthent;
    m_src = rhs.m_src;
    m_dest = rhs.m_dest;
    m_propsIPs = rhs.m_propsIPs;
    m_propAuth = rhs.m_propAuth;
    m_canHandle = rhs.m_canHandle;
    m_ports = rhs.m_ports;
    m_current = rhs.m_current;
    m_user = rhs.m_user;
    m_pass = rhs.m_pass;
    m_extras = rhs.m_extras;
    m_meta = rhs.m_meta;
    m_supMeta = rhs.m_supMeta;
    m_needsNet = rhs.m_needsNet;

    m_currModell = rhs.m_currModell;
    m_models = rhs.m_models;
    m_currMode = rhs.m_currMode;
    m_modes = rhs.m_modes;

    return (*this );
}
/*
void Kapabilities::dump()const
{
    kdDebug(5201) << "Needs Net " << m_needsNet << endl;
    kdDebug(5201) << "Can Push " << m_push << endl;
    kdDebug(5201) << "Needs Conn " << m_needConnection << endl;
    kdDebug(5201) << "ListDir" <<  m_listdir << endl;
    kdDebug(5201) << "Needs IP" << m_needsIp << endl;
    kdDebug(5201) << "Needs SRC " << m_needsSrcIp << endl;

}
*/


