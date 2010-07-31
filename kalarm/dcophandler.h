/*
 *  dcophandler.h  -  handler for DCOP calls by other applications
 *  Program:  kalarm
 *  Copyright © 2001,2002,2004-2006,2008 by David Jarvie <djarvie@kde.org>
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
 *  You should have received a copy of the GNU General Public License
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef DCOPHANDLER_H
#define DCOPHANDLER_H

#include <tqwidget.h>
#include <dcopobject.h>

#include "datetime.h"
#include "kalarmiface.h"


class DcopHandler : public TQWidget, virtual public KAlarmIface
{
    public:
	DcopHandler();
	virtual bool cancelEvent(const TQString& url,const TQString& eventId);
	virtual bool triggerEvent(const TQString& url,const TQString& eventId);

	virtual bool scheduleMessage(const TQString& message, const TQString& startDateTime, int lateCancel, unsigned flags,
	                             const TQString& bgColor, const TQString& fgColor, const TQString& font,
	                             const KURL& audioFile, int reminderMins, const TQString& recurrence,
	                             int subRepeatInterval, int subRepeatCount);
	virtual bool scheduleMessage(const TQString& message, const TQString& startDateTime, int lateCancel, unsigned flags,
	                             const TQString& bgColor, const TQString& fgColor, const TQString& font,
	                             const KURL& audioFile, int reminderMins, int recurType, int recurInterval, int recurCount);
	virtual bool scheduleMessage(const TQString& message, const TQString& startDateTime, int lateCancel, unsigned flags,
	                             const TQString& bgColor, const TQString& fgColor, const TQString& font,
	                             const KURL& audioFile, int reminderMins, int recurType, int recurInterval, const TQString& endDateTime);
	virtual bool scheduleFile(const KURL& file, const TQString& startDateTime, int lateCancel, unsigned flags, const TQString& bgColor,
	                          const KURL& audioFile, int reminderMins, const TQString& recurrence,
	                          int subRepeatInterval, int subRepeatCount);
	virtual bool scheduleFile(const KURL& file, const TQString& startDateTime, int lateCancel, unsigned flags, const TQString& bgColor,
	                          const KURL& audioFile, int reminderMins, int recurType, int recurInterval, int recurCount);
	virtual bool scheduleFile(const KURL& file, const TQString& startDateTime, int lateCancel, unsigned flags, const TQString& bgColor,
	                          const KURL& audioFile, int reminderMins, int recurType, int recurInterval, const TQString& endDateTime);
	virtual bool scheduleCommand(const TQString& commandLine, const TQString& startDateTime, int lateCancel, unsigned flags,
	                             const TQString& recurrence, int subRepeatInterval, int subRepeatCount);
	virtual bool scheduleCommand(const TQString& commandLine, const TQString& startDateTime, int lateCancel, unsigned flags,
	                             int recurType, int recurInterval, int recurCount);
	virtual bool scheduleCommand(const TQString& commandLine, const TQString& startDateTime, int lateCancel, unsigned flags,
	                             int recurType, int recurInterval, const TQString& endDateTime);
	virtual bool scheduleEmail(const TQString& fromID, const TQString& addresses, const TQString& subject, const TQString& message,
	                           const TQString& attachments, const TQString& startDateTime, int lateCancel, unsigned flags,
	                           const TQString& recurrence, int recurInterval, int recurCount);
	virtual bool scheduleEmail(const TQString& fromID, const TQString& addresses, const TQString& subject, const TQString& message,
	                           const TQString& attachments, const TQString& startDateTime, int lateCancel, unsigned flags,
	                           int recurType, int recurInterval, int recurCount);
	virtual bool scheduleEmail(const TQString& fromID, const TQString& addresses, const TQString& subject, const TQString& message,
	                           const TQString& attachments, const TQString& startDateTime, int lateCancel, unsigned flags,
	                           int recurType, int recurInterval, const TQString& endDateTime);
	virtual bool edit(const TQString& eventID);
	virtual bool editNew(const TQString& templateName);

    private:
	static bool scheduleMessage(const TQString& message, const DateTime& start, int lateCancel, unsigned flags,
                                const TQString& bgColor, const TQString& fgColor, const TQString& fontStr,
                                const KURL& audioFile, int reminderMins, const KARecurrence&,
	                            int subRepeatInterval = 0, int subRepeatCount = 0);
	static bool scheduleFile(const KURL& file, const DateTime& start, int lateCancel, unsigned flags, const TQString& bgColor,
                             const KURL& audioFile, int reminderMins, const KARecurrence&,
	                         int subRepeatInterval = 0, int subRepeatCount = 0);
	static bool scheduleCommand(const TQString& commandLine, const DateTime& start, int lateCancel, unsigned flags,
                                const KARecurrence&, int subRepeatInterval = 0, int subRepeatCount = 0);
	static bool scheduleEmail(const TQString& fromID, const TQString& addresses, const TQString& subject, const TQString& message,
                              const TQString& attachments, const DateTime& start, int lateCancel, unsigned flags,
                              const KARecurrence&, int subRepeatInterval = 0, int subRepeatCount = 0);
	static DateTime  convertStartDateTime(const TQString& startDateTime);
	static unsigned  convertStartFlags(const DateTime& start, unsigned flags);
	static TQColor    convertBgColour(const TQString& bgColor);
	static bool      convertRecurrence(DateTime& start, KARecurrence&, const TQString& startDateTime, const TQString& icalRecurrence, int& subRepeatInterval);
	static bool      convertRecurrence(DateTime& start, KARecurrence&, const TQString& startDateTime, int recurType, int recurInterval, int recurCount);
	static bool      convertRecurrence(DateTime& start, KARecurrence&, const TQString& startDateTime, int recurType, int recurInterval, const TQString& endDateTime);
	static bool      convertRecurrence(KARecurrence&, const DateTime& start, int recurType, int recurInterval, int recurCount, const TQDateTime& end);
};


#ifdef OLD_DCOP
class DcopHandlerOld : public TQWidget, public DCOPObject
{
		Q_OBJECT
	public:
		DcopHandlerOld();
		~DcopHandlerOld()  { }
		virtual bool process(const TQCString& func, const TQByteArray& data, TQCString& replyType, TQByteArray& replyData);
};
#endif

#endif // DCOPHANDLER_H
