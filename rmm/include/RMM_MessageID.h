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

#ifndef RMM_RMESSAGEID_H
#define RMM_RMESSAGEID_H

#include <qstring.h>
#include <RMM_HeaderBody.h>
#include <RMM_Defines.h>

/**
 * An RMessageID encapsulates the body of a Message-Id header field as defined
 * by RFC822. This means it has two strings, a local-part and a domain.
 */
class RMessageID : public RHeaderBody {

	public:

		RMessageID();
		RMessageID(const RMessageID &);
		RMessageID(const QCString & s) : RHeaderBody(s) { }
		bool operator == (RMessageID &);
		bool operator != (RMessageID &);
		RMessageID & operator = (const RMessageID &);

		friend QDataStream & operator >> (
			QDataStream & s, RMessageID & mid);
		
		friend QDataStream & operator << (
			QDataStream & s, RMessageID & mid);
		
		virtual ~RMessageID();

		void parse();
		void assemble();
		
		void createDefault();

		const QCString & localPart();
		const QCString & domain();
		void setLocalPart(const QCString & localPart);
		void setDomain(const QCString & domain);
		
		const char * className() const { return "RMessageID"; }
		

	private:

		QCString localPart_;
		QCString domain_;
};

#endif //RMESSAGEID_H
