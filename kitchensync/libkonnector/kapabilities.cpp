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
    m_push = false;
    m_needConnection = false;
    m_listdir = false;
    m_needsIp = false;
    m_needsSrcIp = false;
    m_needsDestIp = false;
    m_canHandle = false;
    m_needsAuthent=false;
  };
  bool m_push:1;
  bool m_needConnection:1;
  bool m_listdir:1;
  bool m_needsIp:1;
  bool m_needsSrcIp:1;
  bool m_needsDestIp:1;
  bool m_needsAuthent:1;
  QHostAddress m_src;
  QHostAddress m_dest;
  QValueList< QPair<QHostAddress,QHostAddress> > m_propsIPs; 
  QPair<QString, QString> m_propAuth;
  bool m_canHandle;
  QStringList m_ports;
  int m_current;
  QString m_user;
  QString m_pass;
};

Kapabilities::Kapabilities()
{
  d = new KapabilitiesPrivate();
}
Kapabilities::Kapabilities(const Kapabilities &kap )
{
  d = new KapabilitiesPrivate();
  copy(kap );
}
void Kapabilities::copy(const Kapabilities &kap )
{
  d->m_push = kap.d->m_push;

}
Kapabilities::~Kapabilities()
{
  delete d;
}
bool Kapabilities::supportsPushSync() const
{
  return d->m_push;
}
void Kapabilities::setSupportsPushSync(bool push)
{
  d->m_push = push;
}
bool Kapabilities::needsConnection() const
{
  return d->m_needConnection;
}
void Kapabilities::setNeedsConnection(bool connection)
{
  d->m_needConnection = connection;
}
bool Kapabilities::supportsListDir() const
{
  return d->m_listdir;
}
void Kapabilities::setSupportsListDir(bool listDir)
{
  d->m_listdir = listDir;
}
QStringList Kapabilities::ports()const
{
  return d->m_ports;
}
void Kapabilities::setPorts(const QStringList& ports)
{
  d->m_ports = ports;
}
int Kapabilities::currentPort() const
{
  return d->m_current;
}
void Kapabilities::setCurrentPort(int port )
{
  d->m_current = port;
}
bool Kapabilities::needsIPs() const
{
  return d->m_needsIp;
}
bool Kapabilities::needsSrcIP() const
{
  return d->m_needsSrcIp;
}
bool Kapabilities::needsDestIP() const
{
  return d->m_needsDestIp;
}
void Kapabilities::setNeedsIPs(bool needs)
{
  d->m_needsIp = needs;
}
void Kapabilities::setNeedsSrcIP(bool needs)
{
  d->m_needsSrcIp = needs;
}
void Kapabilities::setNeedsDestIp(bool needs)
{
  d->m_needsDestIp = needs;
}
void Kapabilities::setSrcIP(const QHostAddress &addr)
{
  d->m_src = addr;
}
void Kapabilities::setDestIP(const QHostAddress &addr)
{
  d->m_dest = addr;
}
QHostAddress Kapabilities::srcIP() const
{
  return d->m_src;
}
QHostAddress Kapabilities::destIP() const
{
  return d->m_dest;
}
bool Kapabilities::canAutoHandle()const
{
  return d->m_canHandle;
}
void Kapabilities::setAutoHandle(bool handle)
{
  d->m_canHandle = handle;
}
QValueList< QPair<QHostAddress, QHostAddress > >  Kapabilities::ipProposals() const
{
  return d->m_propsIPs;
}
void Kapabilities::setIpProposals( QValueList<QPair<QHostAddress,QHostAddress> > ips )
{
  d->m_propsIPs = ips;
}
bool Kapabilities::needAuthentication()
{
  return d->m_needsAuthent;
}
void Kapabilities::setUser(const QString &user )
{
  d->m_user = user;
}
void Kapabilities::setPassword(const QString &pass )
{
  d->m_pass = pass;
}
QString Kapabilities::password()
{
  return d->m_pass;
}
QString Kapabilities::user()
{
  return d->m_user;
}
QPair<QString,QString> Kapabilities::userProposals()
{
  return d->m_propAuth;
}
void Kapabilities::setUserProposals(const QPair<QString, QString> &auth )
{
  d->m_propAuth = auth;
}




