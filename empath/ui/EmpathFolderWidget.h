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

#ifndef EMPATHFOLDERWIDGET_H
#define EMPATHFOLDERWIDGET_H

// Qt includes
#include <qpixmap.h>
#include <qlistview.h>
#include <qlist.h>
#include <qpopupmenu.h>

// Local includes
#include "EmpathDefines.h"
#include "EmpathFolder.h"
#include "EmpathMailbox.h"
#include "EmpathFolderListItem.h"
#include "Empath.h"

class EmpathFolderWidget : public QListView
{
	Q_OBJECT

	public:
		
		EmpathFolderWidget(QWidget * parent = 0, const char * name = 0);

		~EmpathFolderWidget() { empathDebug("dtor"); }
		
		void update();

		EmpathFolder * selectedFolder() const;

	protected slots:

		void s_currentChanged(QListViewItem *);
		void s_rightButtonPressed(QListViewItem *, const QPoint &, int);
		void s_folderProperties();
		void s_mailboxCheck();
		void s_mailboxProperties();
		void s_update();
	
	signals:

		void showFolder(EmpathFolder *);

	private:

		enum OverType { Folder, Mailbox };
		EmpathFolderListItem * _parentFolderListFolder(const EmpathFolder & folder);
		void _addMailbox(const EmpathMailbox & mailbox);
		void _addChildren(const EmpathFolder & item);
		
		QList<EmpathFolderListItem> itemList_;
		
		QPopupMenu folderPopup_;
		QPopupMenu mailboxPopup_;
		
		OverType popupMenuIsOverType;
		EmpathMailbox * popupMenuIsOverMailbox;
		EmpathFolder * popupMenuIsOverFolder;
};

#endif

