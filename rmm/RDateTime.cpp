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

#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <iostream>
#include <ctype.h>
#include <stdlib.h>

#include <qdatetime.h>

#include <RMM_DateTime.h>
#include <RMM_Enum.h>
#include <RMM_Token.h>

RDateTime::RDateTime()
	:	QDateTime(),
		RHeaderBody(),
		zone_("")
{
	rmmDebug("ctor");
}

RDateTime::~RDateTime()
{ 
	rmmDebug("dtor");
}

RDateTime::RDateTime(const RDateTime & t)
	:	QDateTime(t),
		RHeaderBody(t),
		zone_(t.zone_.data())
{
	rmmDebug("ctor");
	assembled_ = false;
}

	RDateTime &
RDateTime::operator = (const RDateTime & dt)
{
	rmmDebug("operator =");
    if (this == &dt) return *this; // Don't do a = a.

	QDateTime::operator = (dt);
	zone_ = dt.zone_.data();
	
	RHeaderBody::operator = (dt);
	
	return *this;
}

	QDataStream &
operator >> (QDataStream & s, RDateTime & dt)
{
	s >> (QDateTime &)dt;
	s >> dt.zone_;
	//cerr << " >> gave me : " << dt.toString() << endl;
	dt.parsed_		= true;
	dt.assembled_	= false;
	return s;
}

	QDataStream &
operator << (QDataStream & s, RDateTime & dt)
{
	//cerr << " << is getting : " << dt.toString() << endl;
	dt.parse();
	s << (QDateTime)dt;
	s << dt.zone_;
	return s;
}

	void
RDateTime::parse()
{
	if (parsed_) return;

	rmmDebug("parse() called");
	
	Q_UINT32	dayOfMonth_;
	Q_UINT32	month_;
	Q_UINT32	year_;
	Q_UINT32	hour_;
	Q_UINT32	min_;
	Q_UINT32	sec_;

	QStrList tokens;
	RTokenise(strRep_, " :", tokens);

	// date-time = [day ","] date time

	if (tokens.count() < 6 || tokens.count() > 9) {
		// Invalid date-time !
		rmmDebug("Invalid date-time");
		parsed_		= true;
		assembled_	= false;
		return;
	}
	
	int i = 0;
	
	// If the first token can be identified as a day name, then ignore it.
	
	bool haveDay = false;
	if (isalpha(tokens.at(i)[0])) { haveDay = true; i++; }

	rmmDebug("0 = \"" + QCString(tokens.at(i)) + "\"");
	if (tokens.at(i)[0] == '0')
		dayOfMonth_ = tokens.at(i++)[1] - '0';
	else
		dayOfMonth_ = atoi(tokens.at(i++));

	rmmDebug("Day of month = " + QCString().setNum(dayOfMonth_));
	month_ = RMM::strToMonth(tokens.at(i++)) + 1;
	rmmDebug("Month = " + QCString().setNum(month_));

	if (strlen(tokens.at(i)) == 2)
		year_ = atoi(tokens.at(i++)) + 1900;
	else
		year_ = atoi(tokens.at(i++));
	
	rmmDebug("Doing hour");
	hour_ = atoi(tokens.at(i++));
	rmmDebug("Hour:" + QCString().setNum(hour_));
	rmmDebug("Doing min");
	min_ = atoi(tokens.at(i++));
	rmmDebug("Min:" + QCString().setNum(min_));
	
	// If the earlier token for day of week was there, and the total token
	// count is 8, then we must also have a seconds field next
	// OR if the earlier token for day was NOT there, and the total token count
	// is 7, then again, we must have a seconds field.
	rmmDebug("Doing secs");
	if (( haveDay && (tokens.count() == 7)) ||
		(!haveDay && (tokens.count() == 6)))
		sec_ = strtol(tokens.at(i++), NULL, 10);
	else
		sec_ = 0;

	rmmDebug("Sec:" + QCString().setNum(sec_));

	rmmDebug("Doing tz");
	rmmDebug("Token count = " + QCString().setNum(tokens.count()));
	if (tokens.count() - 1 == (unsigned)i)
		zone_ = tokens.at(i);
	
	rmmDebug("setYMD(" +
		QCString().setNum(year_) +
		", " +
		QCString().setNum(month_) +
		", " +
		QCString().setNum(dayOfMonth_) +
		")");

	QDate d;
	d.setYMD(year_, month_, dayOfMonth_);
	setDate(d);
	
	rmmDebug("setHMS(" +
		QCString().setNum(hour_) +
		", " +
		QCString().setNum(min_) +
		", " +
		QCString().setNum(sec_) +
		")");
	
	QTime t;
	t.setHMS(hour_, min_, sec_);
	setTime(t);
	
	rmmDebug("XXX " + toString());
	
	parsed_		= true;
	assembled_	= false;
}

	void
RDateTime::assemble()
{
	if (assembled_) return;
	
	rmmDebug("assemble() called");
	if (!QDateTime::isValid()) {
		rmmDebug("I'm not VALID !");
		return;
	}

	QDate d = date();
	
	strRep_ = d.dayName(date().dayOfWeek());
	strRep_ += ',';
	strRep_ += ' ';
	strRep_ += QCString().setNum(d.day());
	strRep_ += ' ';
	strRep_ += d.monthName(d.month());
	strRep_ += ' ';
	strRep_ += QCString().setNum(d.year());
	strRep_ += ' ';
	strRep_ += time().toString();
	strRep_ += ' ';
	strRep_ += zone_;
	
	rmmDebug("assembled to \"" + strRep_ + "\"");
	
	assembled_ = true;
}

	void
RDateTime::createDefault()
{
}

	Q_UINT32
RDateTime::asUnixTime()
{
	parse();
	struct tm timeStruct;
	
	QDate d = date();
	QTime t = time();
	
	timeStruct.tm_sec	= t.second();
	timeStruct.tm_min	= t.minute();
	timeStruct.tm_hour	= t.hour();
	timeStruct.tm_mday	= d.day();
	timeStruct.tm_mon	= d.month() - 1;
	timeStruct.tm_year	= d.year() - 1900;
	timeStruct.tm_isdst	= -1; // Unknown
	
	time_t timeT = mktime(&timeStruct);	

	return (Q_UINT32)timeT;	
}


	const QCString &
RDateTime::asString()
{
	assemble();
	return strRep_;
}

