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

#ifndef RMM_RADDRESSLIST_H
#define RMM_RADDRESSLIST_H

#include <qstring.h>
#include <qlist.h>
#include <RMM_Address.h>
#include <RMM_Defines.h>

typedef QListIterator<RAddress> RAddressListIterator;

/**
 * @short Simple encapsulation of a list of RAddress, which is also an
 * RHeaderBody.
 */
class RAddressList : public QList<RAddress>, public RHeaderBody {

	public:

		RAddressList();
		RAddressList(const RAddressList &);
		RAddressList(const QCString & s) : RHeaderBody(s) { }
		RAddressList & operator = (const RAddressList &);

		virtual ~RAddressList();

		virtual void parse();
		virtual void assemble();

		void createDefault();

		const char * className() const { return "RAddressList"; }
};

#endif //RADDRESSLIST_H
