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

#include <qptrlist.h>
#include <libkcal/todo.h>
#include <libkcal/event.h>
#include <libkcal/journal.h>

#include "kalendarsyncentry.h"

namespace {

    void cloneEntry( KCal::CalendarLocal *dest,
                KCal::CalendarLocal *src )
    {
        QPtrList<KCal::Event> events  = src->getAllEvents();
        KCal::Event *e;
        for(e = events.first(); e != 0; e = events.next() ){
            dest->addEvent((KCal::Event*)e->clone() );
        };
        QPtrList<KCal::Todo> todos = src->getTodoList();
        KCal::Todo *t;
        for(t = todos.first(); t != 0; t = todos.next() ){
            dest->addTodo( (KCal::Todo*)t->clone() );
        };

        QPtrList<KCal::Journal> journal = src->journalList();
        KCal::Journal *j;
        for(j = journal.first(); j !=0; j = journal.next() ){
            dest->addJournal((KCal::Journal*)j->clone()  );
        }

    }
};

KAlendarSyncEntry::KAlendarSyncEntry()
{
  m_calendar = 0;
  m_modified = 0;
  m_added = 0;
  m_removed = 0;
  setSyncMode(SYNC_NORMAL );
}
KAlendarSyncEntry::~KAlendarSyncEntry()
{
  delete m_calendar;
  delete m_modified;
  delete m_added;
  delete m_removed;
}
KAlendarSyncEntry::KAlendarSyncEntry(KCal::CalendarLocal *cal,const QString &name)
{
  this->m_calendar = cal;
  this->m_name = name;
  m_added = 0;
  m_modified = 0;
  m_removed = 0;
  setSyncMode(SYNC_NORMAL );
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
void KAlendarSyncEntry::setId(const QString & )
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
KCal::CalendarLocal* KAlendarSyncEntry::modified()
{
    return m_modified;
}
void KAlendarSyncEntry::setModified( KCal::CalendarLocal* mod )
{
    m_modified = mod;
}
KCal::CalendarLocal* KAlendarSyncEntry::added()
{
    return m_added;
}
void KAlendarSyncEntry::setAdded( KCal::CalendarLocal* add )
{
    m_added = add;
}
KCal::CalendarLocal* KAlendarSyncEntry::removed()
{
    return m_removed;
}
void KAlendarSyncEntry::setRemoved( KCal::CalendarLocal *rem )
{
    m_removed = rem;
}
bool KAlendarSyncEntry::equals(KSyncEntry * )
{
  return false;
}
KSyncEntry* KAlendarSyncEntry::clone()
{
  KAlendarSyncEntry *entry = new KAlendarSyncEntry();
  KCal::CalendarLocal *cal = new KCal::CalendarLocal();
  entry->setCalendar( cal );
  entry->m_name = m_name;
  entry->m_oldId = m_oldId;
  entry->m_time = m_time;

  if ( m_calendar != 0 )
      cloneEntry ( cal,  m_calendar );

  if ( m_modified != 0 ) {
      KCal::CalendarLocal *mod = new KCal::CalendarLocal();
      entry->setModified( mod );
      cloneEntry( mod,  m_modified );
  }
  if ( m_added != 0 ) {
      KCal::CalendarLocal *add = new KCal::CalendarLocal();
      entry->setAdded( add );
      cloneEntry( add,  m_added );
  }
  if ( m_removed != 0 ) {
      KCal::CalendarLocal *rem = new KCal::CalendarLocal();
      entry->setRemoved( rem );
      cloneEntry( rem,  m_removed );
  }
  return entry;
}
