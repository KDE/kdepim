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

#ifndef RMM_RMAILBOX_H
#define RMM_RMAILBOX_H

#include <qstring.h>

#include <RMM_Address.h>
#include <RMM_Defines.h>

class RMailbox : public RAddress {

	public:

		RMailbox();
		RMailbox(const RMailbox & m);
		RMailbox(const QString & s) : RAddress(s) { }
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

		void setPhrase(const QString & phrase);
		void setRoute(const QString & route);
		void setLocalPart(const QString & localPart);
		void setDomain(const QString & domain);
		const QString & phrase()const;
		const QString & route()const;
		const QString & localPart()const;
		const QString & domain()const;
		
		const char * className() const { return "RMailbox"; }

	private:

		QString phrase_;
		QString route_;
		QString localPart_;
		QString domain_;
		bool isValid_;
};

#endif //RMAILBOX_H
