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

#ifndef RMM_RDATETIME_H
#define RMM_RDATETIME_H

#include <qstring.h>
#include <qdatastream.h>
#include <qdatetime.h>

#include <RMM_Defines.h>
#include <RMM_HeaderBody.h>
#include <RMM_Enum.h>

/**
 * An RDateTime encapsulates a time value. It is basically a QDateTime with an
 * added 'asUnixTime()' method and knowledge of time zones.
 */ 
class RDateTime : public RHeaderBody {

	public:

		RDateTime();
		RDateTime(const RDateTime & dt);
		RDateTime(const QCString & s);
		RDateTime & operator = (RDateTime & dt);
		
		QDateTime qdt() { parse(); return qdate_; }

		friend QDataStream & operator >> (QDataStream & s, RDateTime & dt);
		friend QDataStream & operator << (QDataStream & s, RDateTime & dt);

		virtual ~RDateTime();

		void parse();
		void assemble();

		void	setTimeZone	(const QCString &);
		
		QCString 	timeZone();
		Q_UINT32	asUnixTime();

		void		createDefault();
		const char * className() const { return "RDateTime"; }

	private:

		QCString	zone_;
		QDateTime	qdate_;
		
		bool		parsed_;
		bool		assembled_;
};

#endif //RDATETIME_H
