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

#include "kapabilities.h"

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
QStringList Kapabilities::ports()const
{
  return m_ports;
}
void Kapabilities::setPorts(const QStringList& ports)
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
void Kapabilities::setSrcIP(const QHostAddress &addr)
{
  m_src = addr;
}
void Kapabilities::setDestIP(const QHostAddress &addr)
{
  m_dest = addr;
}
QHostAddress Kapabilities::srcIP() const
{
  return m_src;
}
QHostAddress Kapabilities::destIP() const
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
QValueList< QPair<QHostAddress, QHostAddress > >  Kapabilities::ipProposals() const
{
  return m_propsIPs;
}
void Kapabilities::setIpProposals( QValueList<QPair<QHostAddress,QHostAddress> > ips )
{
  m_propsIPs = ips;
}
bool Kapabilities::needAuthentication()
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
QValueList<QPair<QString,QString> > Kapabilities::userProposals()
{
  return m_propAuth;
}
void Kapabilities::setUserProposals( QValueList< QPair<QString, QString> > auth )
{
  m_propAuth = auth;
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

    return (*this );   
}




