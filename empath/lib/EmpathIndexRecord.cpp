/*
	Empath - Mailer for KDE
	
	Copyright (C) 1998 Rik Hemsley rik@kde.org
	
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

// Qt includes
// Something's broken in Qt 1.40 and I can't find it.
// This include solves the problem.
#include <qdir.h>
#include <qdatetime.h>

// Local includes
#include "Empath.h"
#include "EmpathIndexRecord.h"
#include "EmpathMessageDataCache.h"

EmpathIndexRecord::EmpathIndexRecord()
	:	id_(""),
		subject_(""),
		sender_(""),
		date_(""),
		status_(0),
		size_(0),
		messageId_(""),
		parentMessageId_("")
{
	empathDebug("default ctor");
}


EmpathIndexRecord::EmpathIndexRecord(
		const QString & id,
		const QString & subject,
		const RMailbox & sender,
		const RDateTime & date,
		int status,
		Q_UINT32 size,
		const RMessageID & messageId,
		const RMessageID & parentMessageId)
	:	id_(id),
		subject_(subject),
		sender_(sender),
		date_(date),
		status_(status),
		size_(size),
		messageId_(messageId),
		parentMessageId_(parentMessageId)
{
	empathDebug("ctor - my id == " + id_);
	empathDebug("I was given sender == " + QString(sender.asString()));
	empathDebug("My sender == " + QString(sender_.asString()));
}

EmpathIndexRecord::~EmpathIndexRecord()
{
	empathDebug("dtor");
}

	const QString &
EmpathIndexRecord::id() const
{
	return id_;
}

	const QString &
EmpathIndexRecord::subject() const
{
	return subject_;
}

	const RMailbox &
EmpathIndexRecord::sender() const
{
	return sender_;
}

	const RDateTime &
EmpathIndexRecord::date() const
{
	return date_;
}

	MessageStatus
EmpathIndexRecord::status() const
{
	return (MessageStatus)status_;
}

	Q_UINT32
EmpathIndexRecord::size() const
{
	return size_;
}

	const RMessageID &
EmpathIndexRecord::messageID() const
{
	return messageId_;
}

	const RMessageID &
EmpathIndexRecord::parentID() const
{
	return parentMessageId_;
}

	bool
EmpathIndexRecord::hasParent() const
{
	return !parentMessageId_.asString().isEmpty();
}

	void
EmpathIndexRecord::setStatus(int status)
{
	status_ = status;
}

	QString
EmpathIndexRecord::niceDate(bool twelveHour) const
{
	QDateTime now, then;
	now = QDateTime::currentDateTime();
	then = date_;
	
	// Use difference between times to work out how old a message is, and see if we
	// can represent it in a more concise fashion.

	QString dts;
	
	// If the dates differ, then print the day of week..
	if (then.daysTo(now) != 0) {
		dts += then.date().dayName(then.date().dayOfWeek());
		dts += " ";
	}

	
	// If the months differ, print day of month and month name
	if (then.date().month() != now.date().month()) {
		
		dts += QString().setNum(then.date().day());
		
		char endDigit = *(dts.right(1));
		
		switch (endDigit) {
			case 1:
				dts += "st";
				break;
			case 2:
				dts += "nd";
				break;
			case 3:
				dts += "rd";
				break;
			default:
				dts += "th";
				break;
		}
		
		dts += " ";
		
		dts += then.date().monthName(then.date().month());
		
		// If the message is from a different year, add that too.
		if (then.date().year() != now.date().year()) {
			
			dts += " ";
			dts += QString().setNum(then.date().year());
		}

	} else {
	
		// We're in the same month, so print the time of the message. 
		
		int hour = then.time().hour();
		
		if (twelveHour) {

			if (hour > 12)
				dts += QString().setNum(hour - 12);
			else
				dts += QString().setNum(hour);

		} else {
		
			// Add leading nought to hour for twatty 24hr time.
			if (hour < 10) dts += "0";
			dts += QString().setNum(hour);
		}

		dts += ":";
		int m = then.time().minute();
		
		// Add leading nought to minute.
		if (m < 10) dts += "0";
		dts += QString().setNum(m);
		
		if (twelveHour) {
			if (hour > 12) dts += " PM";
			else dts += " AM";
		}
	}

	return dts;
}

