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

#ifndef RMM_RMAILBOX_H
#define RMM_RMAILBOX_H

#include <qstring.h>

#include <RMM_Address.h>
#include <RMM_Defines.h>

/**
 * An RMailbox holds either a (phrase route-addr) or (localpart domain).
 * (localpart domain) is called an addr-spec by RFC822.
 * You can see which type this is by calling phrase().isEmpty(). If it's empty,
 * you have an addr-spec.
 */
class RMailbox : public RAddress {

	public:

		RMailbox();
		RMailbox(const RMailbox & m);
		RMailbox(const QCString & s) : RAddress(s) { }
		const RMailbox & operator = (const RMailbox & m);
		
		friend QDataStream & operator >> (
			QDataStream & s, RMailbox & mailbox);
		
		friend QDataStream & operator << (
			QDataStream & s, const RMailbox & mailbox);

		virtual ~RMailbox();

		void parse();
		void assemble();

		bool isValid() const;

		void createDefault();

		void setPhrase(const QCString & phrase);
		void setRoute(const QCString & route);
		void setLocalPart(const QCString & localPart);
		void setDomain(const QCString & domain);
		const QCString & phrase()const;
		const QCString & route()const;
		const QCString & localPart()const;
		const QCString & domain()const;
		
		const char * className() const { return "RMailbox"; }

	private:

		QCString phrase_;
		QCString route_;
		QCString localPart_;
		QCString domain_;
		bool isValid_;
};

#endif //RMAILBOX_H
