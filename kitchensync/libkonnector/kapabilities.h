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
#include <qmap.h>
#include <qvariant.h>
#include <qglobal.h>

class Kapabilities {
 public:
  Kapabilities();
  Kapabilities(const Kapabilities & );
  ~Kapabilities();
  
  /**
   * If the device is capable of initializing the sync itself
   * @return bool
   */
  bool supportsPushSync() const;

  /**
   * Set if the device is capable to initialize the sync itself
   * @param bool
   */
  void setSupportsPushSync(bool push);

  /**
   * If the device is _not_ capable to establish a connection itself.
   * The user needs to take care of making an connection.
   * @return bool
   */
  bool needsConnection() const;

  /**
   * Set if the device is _not_ capable to establish a connection itself
   * @param is or is not capable
   */
  void setNeedsConnection(bool connection );

  bool supportsListDir() const;
  void setSupportsListDir(bool );

  /**
   * Which ports are possible.
   * @return Array with possible ports 
   */
  QStringList ports()const;
  
  /**
   * Set the possible ports
   * @param the possible ports
   */
  void setPorts(const QStringList & );

  /**
   * Which port is actually used.
   * @return the port in use
   */
  int currentPort() const;
  
  /**
   * Set the port which will be used.
   * @param the port to be used
   */
  void setCurrentPort( int );


  bool needsIPs()const;
  bool needsSrcIP()const;
  bool needsDestIP()const;
  void setNeedsIPs(bool ip);
  void setNeedsSrcIP( bool srcIp );
  void setNeedsDestIP(bool srcIp );

  /**
   * @param source ip
   */
  void setSrcIP( const QHostAddress & );

  /**
   * @return source ip
   */
  QHostAddress srcIP()const;

  /**
   * @param set destination ip
   */
  void setDestIP(const QHostAddress &);
  
  /**
   * @return destination ip
   */
  QHostAddress destIP()const;

  /**
   * The device can act on its own to establish an connection
   * @return bool
   */
  bool canAutoHandle() const;
  
  /**
   * Set if the device can take care of establishing the connection on its own.
   * For example if the device is plugged in, it brings up the connection on its own.
   * The device can also start syncing.
   * @param bool.
   */
  void setAutoHandle(bool);
  
  
  QValueList< QPair<QHostAddress, QHostAddress > > ipProposals() const;
  void setIpProposals( QValueList< QPair<QHostAddress, QHostAddress> >);

  /**
   * If the device need some kind of authentification first.
   * @return bool
   */
  bool needAuthentication();
  
  /**
   * Set if the device needs authentification or not.
   * @param if it need auth or not.
   */
  void setNeedAuthentication(bool need);

  /**
   * Sets the username for authentification
   * @param username for auth
   */
  void setUser(const QString &);
  
  /**
   * The username used for authentification.
   * @return the username.
   */
  QString user() const;

  /**
   * Sets the password for auth.
   * @param the password for auth.
   */
  void setPassword(const QString & );
  
  /**
   * The password used in auth.
   * @return the password used in auth.
   */
  QString password() const;

  /**
   * @return the pair username , password
   */
  QValueList< QPair<QString, QString> > userProposals();
  
  /**
   * If the device has a "shipping" username and password, the connector can
   * propose it.
   * @param the proposed username and password.
   */
  void setUserProposals( QValueList< QPair<QString, QString> > );
 
 Kapabilities &operator=(const Kapabilities & );

  void setExtraOption( const QString &, const QVariant & );
  QMap<QString, QVariant> extras()const { return m_extras; };
 
 private:
  class KapabilitiesPrivate;
  KapabilitiesPrivate *d;
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
  QValueList< QPair<QString, QString> > m_propAuth;
  bool m_canHandle;
  QStringList m_ports;
  int m_current;
  QString m_user;
  QString m_pass;
  QMap<QString,QVariant> m_extras;
};
#endif


