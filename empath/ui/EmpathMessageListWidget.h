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

#ifndef EMPATHMESSAGELISTWIDGET_H
#define EMPATHMESSAGELISTWIDGET_H

// Qt includes
#include <qpixmap.h>
#include <qlist.h>
#include <qlistview.h>
#include <qstring.h>
#include <qpopupmenu.h>

// Local includes
#include "EmpathDefines.h"
#include "EmpathIndexRecord.h"
#include "EmpathMessageListItem.h"
#include "EmpathURL.h"

class EmpathFolder;
class EmpathMessageListWidget;
class EmpathMainWindow;

/**
 * This is the widget that shows the threaded mail list.
 */
class EmpathMessageListWidget : public QListView
{
	Q_OBJECT

	public:
	
		EmpathMessageListWidget(QWidget * parent = 0, const char * name = 0);
		
		~EmpathMessageListWidget();
		
		EmpathMessageListItem * find(const RMessageID & msgId);
		EmpathMessageListItem * findRecursive(
				EmpathMessageListItem * initialItem, const RMessageID & msgId);
		
		void addItem(const EmpathIndexRecord & item);
		EmpathURL firstSelectedMessage() const;
		
		void setSignalUpdates(bool yn);
		const EmpathURL & currentFolder() { return url_; }
		
	protected slots:
	
		void s_messageMark();
		void s_messageMarkRead();
		void s_messageMarkReplied();
		void s_messageView();
		void s_messageReply();
		void s_messageReplyAll();
		void s_messageForward();
		void s_messageBounce();
		void s_messageDelete();
		void s_messageSaveAs();
		void s_messageCopyTo();
		void s_messagePrint();
		void s_messageFilter();
		void s_messageViewSource();
		void s_rightButtonClicked(QListViewItem *, const QPoint &, int);
		void s_doubleClicked(QListViewItem *);
		void s_currentChanged(QListViewItem *);
		void s_showFolder(const EmpathURL &);
		void s_headerClicked(int);
	
	signals:
		
		void changeView(const EmpathURL &);
		
	private:
		
		void	_setupMessageMenu();

		void getDescendants(
			EmpathMessageListItem * initialItem,
			QList<EmpathMessageListItem> * itemList);

		void append(EmpathMessageListItem * item);

		EmpathMainWindow	* parent_;
		QPopupMenu			messageMenu_;
		QPopupMenu			messageMarkMenu_;
		
		QList<EmpathIndexRecord> masterList_;
		QList<EmpathMessageListItem> threadItemList_;

		QPixmap px_read_marked;
		QPixmap px_unread_marked;
		QPixmap px_read_unmarked;
		QPixmap px_unread_unmarked;
		bool wantScreenUpdates_;
		
		void setStatus(EmpathMessageListItem * item, RMM::MessageStatus status);

		EmpathURL url_;

		int lastHeaderClicked_;
		bool sortType_; // Ascending, Descending
		
		int messageMenuItemMark;
		int messageMenuItemMarkRead;
		int messageMenuItemMarkReplied;
};

#endif

