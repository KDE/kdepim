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

#ifndef RMM_RADDRESS_H
#define RMM_RADDRESS_H

#include <qstring.h>
#include <RMM_HeaderBody.h>
#include <RMM_Defines.h>

class RGroup;
class RMailbox;

class RAddress : public RHeaderBody {

	public:

		RAddress();
		RAddress(const RAddress & rAddress);
		RAddress(const QString & s);
		const RAddress & operator = (const RAddress & rAddress);

		virtual ~RAddress();

		void parse();
		void assemble();
//		void set(const char *s) { strRep_ = s; }
//		const char * asString() { return strRep_; }

		bool isValid() const;
		
		RGroup * group();
		RMailbox * mailbox();

		void createDefault();

		const char * className() const { return "RAddress"; }
		
	protected:
		
//		QString strRep_;
		
	private:
		
		bool isValid_;

		RMailbox * mailbox_;
		RGroup * group_;
};

#endif //RADDRESS_H
