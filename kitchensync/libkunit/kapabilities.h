/* This file is part of the KDE libraries
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

#ifndef kapabilities_h
#define kapabilities_h

#include <qstring.h>
#include <qstringlist.h>
#include <qvaluelist.h>
#include <qhostaddress.h>
#include <qpair.h>

class Kapabilities {
 public:
  Kapabilities();
  Kapabilities(const Kapabilities & );
  QStringList supportedProtocols( ) const;
  void setSupportedProtocols(const QString & );

  int useProtocol( const QString & );

  bool supportsPushSync() const;
  void setSupportsPushSync(bool push);

  bool needsConnection() const;
  void setNeedsConnection(bool connection );

  bool supportsListDir() const;
  void setSupportsListDir(bool ) const;

  bool needsIPs()const;
  bool needsSrcIP()const;
  bool needDestIP()const;
  void setNeedsIPs(bool ip);
  void setNeedsSrcIP( bool srcIp );
  void setNeedsDestIp(bool srcIp );
  void setSrcIP( const QHostAddress & );
  void setDestIp();
  bool canAutoHandle();
  void setBoolAutoHandle(bool);
  QPair<QValueList<QHostAddress>, QValueList<QHostAddress> > ipProposals();

  Kapabilities &operator=(const Kapabilities & );

 private:
  class KapabilitiesPrivate;
  KapabilitiesPrivate *d;

};
#endif

