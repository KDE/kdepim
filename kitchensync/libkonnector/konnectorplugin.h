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

#ifndef konnectorplugin_h
#define konnectorplugin_h

#include <qcstring.h>
#include <qwidget.h>
#include <qstringlist.h>
#include <ksyncentry.h>
#include <qptrlist.h>

class Kapabilities;
class KSyncEntry;
class KonnectorPlugin : public QObject
{
Q_OBJECT
 public:
  KonnectorPlugin(QWidget *obj, const char *name, const QStringList &args=QStringList() );
  virtual ~KonnectorPlugin() = 0;
  virtual void setUDI(const QString & ) = 0;
  virtual QString udi()const = 0;
  virtual Kapabilities capabilities( ) const = 0 ;
  virtual void setCapabilities( const Kapabilities &kaps ) = 0;
  virtual bool startSync() = 0;
  virtual bool insertFile(const QString &fileName ) = 0;
 public:
  virtual void slotWrite(const QString &, const QByteArray & ) = 0;
  virtual void slotWrite(QPtrList<KSyncEntry> ) = 0;
 signals:
  void sync(QPtrList<KSyncEntry> ); 
};
#endif


