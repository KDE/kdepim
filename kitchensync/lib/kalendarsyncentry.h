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

#include <ksyncentry.h>
#include <calendarlocal.h>

#ifndef kalendarsyncentry_h
#define kalendarsyncentry_h


class KAlendarSyncEntry : public KSyncEntry
{
 public:
  KAlendarSyncEntry();
  KAlendarSyncEntry(KCal::CalendarLocal *cal,const QString &name );
  virtual ~KAlendarSyncEntry();

  KCal::CalendarLocal *calendar();
  void setCalendar(KCal::CalendarLocal *);
  virtual QString type() {return QString::fromLatin1("KAlendarSyncEntry"); }
  virtual QString name();
  virtual void setName(const QString &name );
  virtual QString id();
  virtual void setId(const QString &id);
  virtual QString oldId();
  virtual void setOldId(const QString &oldId);
  virtual QDateTime timestamp();
  virtual void setTimestamp(const QDateTime & );
  virtual bool equals(KAlendarSyncEntry * );

 private:
  QString m_name;
  QString m_oldId;
  QDateTime m_time;
  KCal::CalendarLocal *m_calendar;
  class KAlendarSyncEntryPrivate;
  KAlendarSyncEntryPrivate *d;
};

#endif
