/*
	Empath - Mailer for KDE
	
	Copyright (C) 1998 Rik Hemsley rikkus@postmaster.co.uk
	
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

class RDateTime : public QDateTime, public RHeaderBody {

	public:

		RDateTime();
		RDateTime(const RDateTime & dt);
		RDateTime(const QCString & s) : RHeaderBody(s) { }
		const RDateTime & operator = (const RDateTime & dt);
		void set(const QCString & s) { RHeaderBody::set(s); }

		friend QDataStream & operator >> (
			QDataStream & s, RDateTime & dt);
		
		friend QDataStream & operator << (
			QDataStream & s, const RDateTime & dt);

		virtual ~RDateTime();

		void parse();
		void assemble();

		bool isValid() const;

		void			setTimeZone(const QCString &);
		void			set(Q_UINT32 unixTime);
		
		const QCString &	timeZone() const;
		
		const QCString &		asString() const { return RHeaderBody::asString(); }
		Q_UINT32	asUnixTime() const;

		void createDefault();
		
		const char * className() const { return "RDateTime"; }

	private:

		QCString			zone_;
};

#endif //RDATETIME_H
