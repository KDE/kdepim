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

#include <qcstring.h>
#include <qstring.h>
#include <qdatetime.h>

#include "kunknownsyncentry.h"



class KUnknownSyncEntry::KUnknownSyncEntryPrivate {
public:
  KUnknownSyncEntryPrivate( ){ 

  }
  QString name;
  QString id;
  QString oldId;
  QString fileName;
  QDateTime time;
  QByteArray byteArray;
};


KUnknownSyncEntry::KUnknownSyncEntry()
{
  d = new KUnknownSyncEntryPrivate;
  d->time = QDateTime::currentDateTime();
}
KUnknownSyncEntry::KUnknownSyncEntry(const QString &name, const QString &id, const QString &fileName)
{
  d = new KUnknownSyncEntryPrivate;
  d->name = name;
  d->id = id;
  d->fileName = fileName;
  d->time = QDateTime::currentDateTime();
}
KUnknownSyncEntry::~KUnknownSyncEntry()
{
  delete d;
}
QString KUnknownSyncEntry::name()
{
  return d->name;
}
void KUnknownSyncEntry::setName(const QString &name )
{
  d->name = name;
}
QString KUnknownSyncEntry::id()
{
  return d->id;
}
void KUnknownSyncEntry::setId(const QString &id)
{
  d->id = id;
}
QString KUnknownSyncEntry::oldId()
{
  return d->oldId;
}
void KUnknownSyncEntry::setOldId(const QString &oldId )
{
  d->oldId = oldId;
}
QDateTime KUnknownSyncEntry::timestamp()
{
  return d->time;
}
void KUnknownSyncEntry::setTimestamp(const QDateTime &time)
{
  d->time = time;

}
bool KUnknownSyncEntry::equals( KUnknownSyncEntry *entry )
{
  if( d->time == entry->timestamp() && d->name == entry->name() )
    return true;
  else
    return false;
}
QByteArray KUnknownSyncEntry::byteArray() const
{
  return d->byteArray;
}
void KUnknownSyncEntry::setByteArray(const QByteArray &array )
{
  d->byteArray = array;
}
void KUnknownSyncEntry::setSrcFileName(const QString &fileName )
{
  d->fileName = fileName;
}
QString KUnknownSyncEntry::fileName() const
{
  return d->fileName;
}
