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

#ifndef RMM_RADDRESS_H
#define RMM_RADDRESS_H

#include <qstring.h>
#include <RMM_HeaderBody.h>
#include <RMM_Defines.h>

class RGroup;
class RMailbox;

/**
 * An RAddress is schizophrenic. It's either an RGroup or an RMailbox. Don't
 * worry about it.
 */
class RAddress : public RHeaderBody {

	public:

		RAddress();
		RAddress(const RAddress &);
		RAddress(const QCString & s);
		virtual RAddress & operator = (const RAddress & a);
		virtual RAddress & operator = (const QCString & s);

		virtual ~RAddress();

		void parse();
		void assemble();

		RGroup * group();
		RMailbox * mailbox();

		void createDefault();

		const char * className() const { return "RAddress"; }
		
	private:
		
		RMailbox	* mailbox_;
		RGroup		* group_;
};

#endif //RADDRESS_H
