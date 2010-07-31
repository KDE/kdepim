/*
 *  adcalendar.h  -  calendar file access
 *  Program:  KAlarm's alarm daemon (kalarmd)
 *  Copyright (c) 2001, 2004-2006 by David Jarvie <software@astrojar.org.uk>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef ADCALENDAR_H
#define ADCALENDAR_H

#include <libkcal/calendarlocal.h>
namespace KIO { class Job; }
class ADCalendar;


// Alarm Daemon calendar access
class ADCalendar : public KCal::CalendarLocal
{
		Q_OBJECT
	public:
		typedef TQValueList<ADCalendar*>::ConstIterator ConstIterator;

		~ADCalendar();

		const TQString&  urlString() const       { return mUrlString; }
		const TQCString& appName() const         { return mAppName; }

		void            setEnabled(bool enabled) { mEnabled = enabled; }
		bool            enabled() const         { return mEnabled && !unregistered(); }
		bool            available() const       { return loaded() && !unregistered(); }

		// Client has registered since calendar was constructed, but
		// has not since added the calendar. Monitoring is disabled.
		void            setUnregistered(bool u) { mUnregistered = u; }
		bool            unregistered() const    { return mUnregistered; }
  
		void            setEventPending(const KCal::Event*, const TQValueList<TQDateTime>&);
		void            setEventHandled(const TQString& eventID);
		void            clearEventsHandled(bool nonexistentOnly = false);
		bool            eventHandled(const KCal::Event*, const TQValueList<TQDateTime>&);

		bool            loadFile(bool reset);
		bool            setLoadedConnected();     // check status of mLoadedConnected and set it to true
		bool            downloading() const     { return !mTempFileName.isNull(); }
		bool            loaded() const          { return mLoaded; }


		static ConstIterator begin()            { return mCalendars.begin(); }
		static ConstIterator end()              { return mCalendars.end(); }
		static ADCalendar*   getCalendar(const TQString& calendarURL);

	signals:
		void            loaded(ADCalendar*, bool success);

	protected:
		// Only ClientInfo can construct ADCalendar objects
		friend class ClientInfo;
		ADCalendar(const TQString& url, const TQCString& appname);

	private slots:
		void            slotDownloadJobResult(KIO::Job*);

	private:
		struct EventKey
		{
			EventKey() : calendarIndex(-1) { }
			EventKey(const TQString& id, int cal) : eventID(id), calendarIndex(cal) { }
			bool    operator<(const EventKey& k) const
			            { return (calendarIndex == k.calendarIndex)
			                   ? (eventID < k.eventID) : (calendarIndex < k.calendarIndex);
			            }
			TQString eventID;
			int     calendarIndex;
		};
		struct EventItem
		{
			EventItem() : eventSequence(0) { }
			EventItem(int seqno, const TQValueList<TQDateTime>& alarmtimes)
			        : eventSequence(seqno), alarmTimes(alarmtimes) {}
			int                   eventSequence;
			TQValueList<TQDateTime> alarmTimes;
		};

		typedef TQMap<EventKey, EventItem>  EventsMap;   // calendar/event ID, event sequence num
		static EventsMap               mEventsHandled;  // IDs of already triggered events which have been processed by KAlarm
		static EventsMap               mEventsPending;  // IDs of already triggered events not yet processed by KAlarm
		static TQStringList             mCalendarUrls;   // URLs of all calendars ever opened
		static TQValueList<ADCalendar*> mCalendars;      // list of all constructed calendars

		ADCalendar(const ADCalendar&);             // prohibit copying
		ADCalendar& operator=(const ADCalendar&);  // prohibit copying

		void      loadLocalFile(const TQString& filename);
		void      clearEventMap(EventsMap&, bool nonexistentOnly);
		void      setEventInMap(EventsMap&, const EventKey&, const TQValueList<TQDateTime>& alarmtimes, int sequence);

		TQString   mUrlString;       // calendar file URL
		TQCString  mAppName;         // name of application owning this calendar
		TQString   mTempFileName;    // temporary file used if currently downloading, else null
		int       mUrlIndex;        // unique index to URL in mCalendarUrls
		bool      mLoaded;          // true if calendar file is currently loaded
		bool      mLoadedConnected; // true if the loaded() signal has been connected to AlarmDaemon
		bool      mUnregistered;    // client has registered, but has not since added the calendar
		bool      mEnabled;         // events are currently manually enabled
};

#endif // ADCALENDAR_H
