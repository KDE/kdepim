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

#ifdef __GNUG__
# pragma interface "RMM_AddressList.h"
#endif

#ifndef RMM_RADDRESSLIST_H
#define RMM_RADDRESSLIST_H

#include <qstring.h>
#include <qlist.h>
#include <RMM_Address.h>
#include <RMM_Defines.h>

namespace RMM {

typedef QListIterator<RAddress> RAddressListIterator;

/**
 * @short Simple encapsulation of a list of RAddress, which is also an
 * RHeaderBody.
 */
class RAddressList : public RHeaderBody {

	public:

#include "generated/RAddressList_generated.h"
		
		RAddress * at(int);
		unsigned int count();

	private:
		
		QList<RAddress> list_;
};

};

#endif //RADDRESSLIST_H
