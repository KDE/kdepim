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

#ifndef EMPATHATTACHMENTLISTITEM_H
#define EMPATHATTACHMENTLISTITEM_H

// Qt includes
#include <qlistview.h>
#include <qlist.h>

// Local includes
#include <RMM_BodyPart.h>

class EmpathAttachmentListItem : public QListViewItem
{
	public:
		
		EmpathAttachmentListItem(QListView * parent, const RBodyPart &);
		EmpathAttachmentListItem(QListViewItem * parent, const RBodyPart &);
		virtual ~EmpathAttachmentListItem();
		
		virtual void setup();
		
		const RBodyPart & bodyPart() const { return bodyPart_; }

		QString key(int, bool) const;
		
		const char * className() const { return "EmpathAttachmentListItem"; }
		
	private:
	
		RBodyPart bodyPart_;
};

typedef QList<EmpathAttachmentListItem> EmpathAttachmentList;
typedef QListIterator<EmpathAttachmentListItem> EmpathAttachmentListIterator;

#endif
