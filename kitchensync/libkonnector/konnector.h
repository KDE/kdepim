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

#ifndef konnector_h
#define konnector_h

#include <qobject.h>
#include <qvaluelist.h>
#include <qptrlist.h>
#include <qstring.h>
#include <qcstring.h>
#include <ksyncentry.h>


class Kapabilities;
class KDevice;
class KOperations;
class Konnector : public QObject{
Q_OBJECT
 public:
  Konnector(QObject *, const char*);
  ~Konnector();
  QValueList<KDevice> query(const QString &category= QString::null );
  QString /*runtime unique-dev-id*/ registerKonnector(const QString &DeviceIdentification );
  QString /*runtime unique-dev-id*/ registerKonnector(const KDevice &Device );
  Kapabilities capabilities( const QString &udi ) const;
  void setCapabilities( const QString &udi, const Kapabilities& );
  QByteArray file( const QString &udi, const QString &path ); // this would allow some post processing
  void retrieveFile(const QString &udi, const QString &);
  bool isConnected(const QString &udi );

 public slots:
  void write(const QString &udi, QPtrList<KSyncEntry> );
  void write(const QString &udi, QValueList<KOperations> );
  void write(const QString &udi, const QString &dest, const QByteArray& ); // post processing
 signals:
  void wantsToSync(int, int, QPtrList<KSyncEntry>);
  void stateChanged(QString, bool ); // udi + state

 private:
  class KonnectorPrivate;
  KonnectorPrivate *d; 

};

#endif

