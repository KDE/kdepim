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
  };
  bool m_push:1;
  bool m_needConnection:1;
  bool m_listdir:1;
  bool m_needsIp:1;
  bool m_needsSrcIp:1;
  bool m_needsDestIp:1;
  QHostAddress m_src;
  QHostAddress m_dest;
  QPair<QValueList<QHostAddress>, QValueList<QHostAddress> > m_propsIPs;
  bool m_canHandle;
  QStringList ports;
  QString current;
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

}
void Kapabilities::setSupportsPushSync(bool push)
{

}
bool Kapabilities::needsConnection() const
{

}
void Kapabilities::setNeedsConnection(bool connection)
{

}
bool Kapabilities::supportsListDir() const
{

}
void Kapabilities::setSupportsListDir(bool listDir)
{

}
QStringList Kapabilities::ports()const
{

}
void Kapabilities::setPorts(const QStringList& )
{

}
QString Kapabilities::currentPort() const
{

}
void setCurrentPort(const QString & )
{

}
bool Kapabilities::needsIPs() const
{

}
bool Kapabilities::needsSrcIP() const
{

}
bool Kapabilities::needsDestIP() const
{

}
void Kapabilities::setNeedsIPs(bool needs)
{

}
void Kapabilities::setNeedsSrcIP(bool needs)
{

}
void Kapabilities::setNeedsDestIp(bool needs)
{

}
void Kapabilities::setSrcIP(const QHostAddress &addr)
{

}
void Kapabilities::setDestIP(const QHostAddress &addr)
{

}
QHostAddress Kapabilities::srcIP() const
{

}
QHostAddress Kapabilities::destIP() const
{

}
bool Kapabilities::canAutoHandle()const
{

}
void Kapabilities::setAutoHandle(bool handle)
{

}
QValueList< QPair<QHostAddress, QHostAddress > >  Kapabilities::ipProposals() const
{

}
void Kapabilities::setIpProposals( QValueList<QPair<QHostAddress,QHostAddress> > )
{

}
