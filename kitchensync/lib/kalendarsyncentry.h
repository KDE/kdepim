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
    /**
     * c'tor
     */
    KAlendarSyncEntry();
    /**
     * c'tor
     * @param cal the Calendar
     * @param name set the Name
     */
    KAlendarSyncEntry(KCal::CalendarLocal *cal,const QString &name );
    virtual ~KAlendarSyncEntry();

    /**
     * @return the calendar
     */
    KCal::CalendarLocal *calendar();
    /**
     * @param cal Sets the Calendar for this KSyncEntry
     */
    void setCalendar(KCal::CalendarLocal *cal);
  // META Information
    /** These calendars are only != 0 if in SYNC_META
     *  @return the modifed() calendar entries
     */
    KCal::CalendarLocal *modified();
    /** set the modified calendar entries
     *  @param cal the Calendar holding modified entries
     */
    void setModified( KCal::CalendarLocal *cal);

    /**
     * @return the added() Calendar
     */
    KCal::CalendarLocal *added();
    /**
     * set the added calendar
     */
    void setAdded( KCal::CalendarLocal * );

    KCal::CalendarLocal *removed();
    void setRemoved(KCal::CalendarLocal *);
  //

    virtual QString type() {return QString::fromLatin1("KAlendarSyncEntry"); }
    virtual QString name();
    virtual void setName(const QString &name );
    virtual QString id();
    virtual void setId(const QString &id);
    virtual QString oldId();
    virtual void setOldId(const QString &oldId);
    virtual QString timestamp();
    virtual void setTimestamp(const QString & );
    virtual bool equals(KSyncEntry *entr );
    virtual KSyncEntry* clone();

private:
    QString m_name;
    QString m_oldId;
    QString m_time;
    KCal::CalendarLocal *m_calendar;
    KCal::CalendarLocal *m_added;
    KCal::CalendarLocal *m_removed;
    KCal::CalendarLocal *m_modified;
    class KAlendarSyncEntryPrivate;
    KAlendarSyncEntryPrivate *d;
};

#endif
