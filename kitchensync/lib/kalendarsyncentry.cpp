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

#include "kalendarsyncentry.h"

KAlendarSyncEntry::KAlendarSyncEntry()
{
  m_calendar = 0;
}
KAlendarSyncEntry::~KAlendarSyncEntry()
{
  delete m_calendar;
}
KAlendarSyncEntry::KAlendarSyncEntry(KCal::CalendarLocal *cal,const QString &name)
{
  this->m_calendar = cal;
  this->m_name = name;
}
QString KAlendarSyncEntry::name()
{
  if(m_calendar==0 )
    return QString();

  return m_name;
}
void KAlendarSyncEntry::setName(const QString &name )
{
  this->m_name = name;
}
QString KAlendarSyncEntry::id()
{
  QString myId;
  myId.append(m_calendar->getOwner());
  myId.append( "_###_" );
  myId.append(m_calendar->getEmail()  );
  return myId;
}
void KAlendarSyncEntry::setId(const QString &id )
{
  // nothing here though
}
QString KAlendarSyncEntry::oldId()
{
  return m_oldId;
}
void KAlendarSyncEntry::setOldId(const QString &oldId )
{
  this->m_oldId = oldId;
}
QString KAlendarSyncEntry::timestamp()
{
  return m_time;
}
void KAlendarSyncEntry::setTimestamp(const QString &time)
{
  m_time = time;
}
KCal::CalendarLocal* KAlendarSyncEntry::calendar()
{
  return m_calendar;
}
void KAlendarSyncEntry::setCalendar(KCal::CalendarLocal *cal )
{
  m_calendar = cal;
}
bool KAlendarSyncEntry::equals(KSyncEntry *sync )
{
  return false;
}
