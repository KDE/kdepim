/* kldapclient.h - LDAP access
 *      Copyright (C) 2002 Klarälvdalens Datakonsult AB
 *
 *      Author: Steffen Hansen <hansen@kde.org>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#ifndef __KLDAPCLIENT_H__
#define __KLDAPCLIENT_H__


#include <qobject.h>
#include <qstring.h>
#include <qcstring.h>
#include <qstringlist.h>
#include <qmemarray.h>
#include <qguardedptr.h>

#include <kio/job.h>

typedef QValueList<QByteArray> KLdapAttrValue;
typedef QMap<QString,KLdapAttrValue > KLdapAttrMap;

class KLdapObject {
public:
  KLdapObject() : dn( QString::null ) {};  
  explicit KLdapObject( QString _dn) : dn(_dn) {};
  KLdapObject( const KLdapObject& that ) { assign( that ); }
  
  KLdapObject& operator=( const KLdapObject& that ) {
    assign( that );
    return *this;
  }

  QString toString();

  void clear();

  QString dn;
  KLdapAttrMap attrs;

protected:
  void assign( const KLdapObject& that );
};

class KLdapClient : public QObject {
  Q_OBJECT
public:
  KLdapClient( QObject* parent = 0, const char* name = 0 );
  virtual ~KLdapClient();

  /*! returns true if there is a query running */
  bool isActive() const { return _active; }
signals:

  /*! Emitted when the query is done */
  void done();

  /*! Emitted in case of error */
  void error( const QString& );

  /*! Emitted once for each object returned
   * from the query
   */
  void result( const KLdapObject& );
public slots:

  /*!
   * Set the name or IP of the LDAP server
   */
  void setHost( const QString& host );
  QString host() const { return _host; }

  /*!
   * Set the port of the LDAP server
   * if using a nonstandard port
   */
  void setPort( const QString& port );
  QString port() const { return _port; }

  /*!
   * Set the base DN 
   */
  void setBase( const QString& base );
  QString base() const { return _base; }

  /*! Set the attributes that should be
   * returned, or an empty list if
   * all attributes are wanted
   */
  void setAttrs( const QStringList& attrs );
  QStringList attrs() const { return _attrs; }

  void setScope( const QString scope ) {
    _scope = scope;
  }

  /*!
   * Start the query with filter filter
   */
  void startQuery( const QString& filter );

  /*!
   * Abort a running query
   */
  void cancelQuery();

protected slots:
  void slotData( KIO::Job*, const QByteArray &data );
  void slotInfoMessage( KIO::Job*, const QString &info );
  void slotDone();
protected:
  void startParseLDIF();
  void parseLDIF( const QByteArray& data );
  void endParseLDIF();

  QString     _host;
  QString     _port;
  QString     _base;
  QString     _scope;
  QStringList _attrs;
  
  QGuardedPtr<KIO::SimpleJob> _job;
  bool _active;
  
  KLdapObject             _currentObject;
  //QValueList<KLdapObject> _objects;
  QCString                _buf;
  QCString                _lastAttrName;
  QCString                _lastAttrValue;
  bool                    _isBase64;
};

#endif // __KLDAPCLIENT_H__
