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

#ifndef EMPATHFOLDERLISTITEM_H
#define EMPATHFOLDERLISTITEM_H

// Qt includes
#include <qlistview.h>
#include <qlist.h>

class EmpathFolder;
class EmpathMailbox;

class EmpathFolderListItem : public QObject, public QListViewItem
{
	Q_OBJECT

	public:
		
		EmpathFolderListItem(QListViewItem * parent, const EmpathFolder & folder);
		EmpathFolderListItem(QListView * parent, const EmpathMailbox & mailbox);
		virtual ~EmpathFolderListItem();
		
		virtual void setup();
		
		EmpathFolder & folder() const;
		EmpathMailbox & mailbox() const;

		QString key(int, bool) const;

		bool isFolderItem() const { return (type_ == Folder); }
		
	protected slots:

		void s_setCount(int, int);
		
	private:
	
		enum FolderListItemType { Folder, Mailbox };
		
		FolderListItemType type_;
		
		EmpathFolder * folder_;
		EmpathMailbox * mailbox_;
};

typedef QListIterator<EmpathFolderListItem> EmpathFolderListItemIterator;

#endif
