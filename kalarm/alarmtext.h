/*
 *  alarmtext.h  -  text/email alarm text conversion
 *  Program:  kalarm
 *  Copyright (C) 2004, 2005 by David Jarvie <software@astrojar.org.uk>
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

#ifndef ALARMTEXT_H
#define ALARMTEXT_H

#include <tqstring.h>
class TQStringList;
class KAEvent;


class AlarmText
{
	public:
		AlarmText(const TQString& text = TQString::null)  { setText(text); }
		void           setText(const TQString&);
		void           setScript(const TQString& text)   { setText(text);  mIsScript = true; }
		void           setEmail(const TQString& to, const TQString& from, const TQString& cc, const TQString& time,
		                        const TQString& subject, const TQString& body, unsigned long kmailSerialNumber = 0);
		TQString        displayText() const;
		TQString        calendarText() const;
		TQString        to() const                 { return mTo; }
		TQString        from() const               { return mFrom; }
		TQString        cc() const                 { return mCc; }
		TQString        time() const               { return mTime; }
		TQString        subject() const            { return mSubject; }
		TQString        body() const               { return mIsEmail ? mBody : TQString::null; }
		bool           isEmpty() const;
		bool           isEmail() const            { return mIsEmail; }
		bool           isScript() const           { return mIsScript; }
		unsigned long  kmailSerialNumber() const  { return mKMailSerialNum; }
		static TQString summary(const KAEvent&, int maxLines = 1, bool* truncated = 0);
		static bool    checkIfEmail(const TQString&);
		static TQString emailHeaders(const TQString&, bool subjectOnly);
		static TQString fromCalendarText(const TQString&, bool& email);
		static TQString toCalendarText(const TQString&);

	private:
		static void    setUpTranslations();
		static int     emailHeaderCount(const TQStringList&);

		static TQString mFromPrefix;       // translated header prefixes
		static TQString mToPrefix;
		static TQString mCcPrefix;
		static TQString mDatePrefix;
		static TQString mSubjectPrefix;
		static TQString mFromPrefixEn;     // untranslated header prefixes
		static TQString mToPrefixEn;
		static TQString mCcPrefixEn;
		static TQString mDatePrefixEn;
		static TQString mSubjectPrefixEn;
		TQString        mBody, mFrom, mTo, mCc, mTime, mSubject;
		unsigned long  mKMailSerialNum;   // if email, message's KMail serial number, else 0
		bool           mIsEmail;
		bool           mIsScript;
};

#endif // ALARMTEXT_H
