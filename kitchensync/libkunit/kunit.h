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

#ifndef kunit_h
#define kunit_h

#include <qobject.h>
#include <qvaluelist.h>
#include <qstring.h>
#include <qcstring.h>

class Kapabilities;
class KDevice;
class KUnit : public QObject{
Q_OBJECT
 public:
  KUnit(QObject *, const char*);
  QValueList<KDevice> query(const QString &category= QString::null );
  int /*unique-dev-id*/ register(const QString &DeviceIdentification );
  Kapabilities capabilities( int udi ) const;
  void setConfig( int udi, const Kapabilities& );
  QByteArray file( int udi );

 public slots:
  void write(int udi, QValueList<KSyncEntries> );
  void write(int udi, QValueList<KOperations> );
  void write(int udi, const QString &dest, const QByteArray& );
 signals:
  void wantsToSync(int udi, int way, QValueList<SyncEntries> );

 private:
  class KUnitPrivate;
  KUnitPrivate *d; 

};

#endif
