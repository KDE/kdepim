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

#ifndef RMM_RMAILBOXLIST_H
#define RMM_RMAILBOXLIST_H

#include <qstring.h>
#include <qlist.h>

#include <RMM_Defines.h>

typedef QListIterator<RMailbox> RMailboxListIterator;

/**
 * @short Simple encapsulation of a list of RMailbox
 * Simple encapsulation of a list of RMailbox, which is also an RHeaderBody.
 */
class RMailboxList : public QList<RMailbox>, public RHeaderBody {

	public:

		RMailboxList();
		RMailboxList(const RMailboxList & rMailboxList);
		RMailboxList(const QCString & s);
		RMailboxList & operator = (const RMailboxList &);

		virtual ~RMailboxList();

		void parse();
		void assemble();
		
		void createDefault();
		
		const char * className() const { return "RMailboxList"; }
};

#endif //RMAILBOXLIST_H
