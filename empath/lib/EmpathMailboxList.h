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
# pragma interface "EmpathMailboxList.h"
#endif

#ifndef EMPATHMAILBOXLIST_H
#define EMPATHMAILBOXLIST_H

// Qt includes
#include <qlist.h>
#include <qstring.h>

// Local includes

#include "EmpathMailbox.h"
#include "EmpathURL.h"

typedef QListIterator<EmpathMailbox> EmpathMailboxListIterator;

class EmpathMailboxList : public QObject, public QList<EmpathMailbox>
{
	Q_OBJECT

	public:
	
		EmpathMailboxList();
		~EmpathMailboxList();
		
		void init();
		
		void readConfig();
		void saveConfig() const;
		
		EmpathMailbox & operator [] (Q_UINT32 i);

		void append(EmpathMailbox * mailbox);
	
		bool remove(EmpathMailbox * mailbox);

		EmpathMailbox * find(const QString & name) const;
		EmpathFolder * folder(const EmpathURL & folderURL) const;

		void getNewMail();

	signals:
		
		void updateFolderLists();

};

#endif

