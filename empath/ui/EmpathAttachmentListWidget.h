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

#ifndef EMPATHATTACHMENTLISTWIDGET_H
#define EMPATHATTACHMENTLISTWIDGET_H

// Qt includes
#include <qwidget.h>
#include <qlistview.h>
#include <qlist.h>
#include <qstring.h>
#include <qpushbutton.h>

// Local includes
#include "EmpathDefines.h"

/**
 * This widget is used to get the attachments to be used in a message.
 * It shows a list with the filename, type, encoding and size of each
 * attachment. You can press the buttons to add a new attachment, edit an
 * existing, or delete one. You can drag and drop files from e.g. kfm into
 * the listview bit and they will be looked at and added.
 */
class EmpathAttachmentListWidget : public QWidget
{
	Q_OBJECT
	
	public:
		
		EmpathAttachmentListWidget(QWidget * parent = 0, const char * name = 0);
		~EmpathAttachmentListWidget();

		QSize sizeHint() const;

		/**
		 * Check if there are any attachments in the list
		 */
		bool hasAttachments() const;
		
		/**
		 * Get the list of attachments as EmpathAttachmentSpec(s).
		 */
		QList<EmpathAttachmentSpec> attachmentList() const;

		/**
		 * Add an attachment to the internal list and the widget
		 */
		void addAttachment(const EmpathAttachmentSpec & att);
		
		/**
		 * Add lots of attachments to the internal list and the widget.
		 * Useful when forwarding w/ attachments.
		 */
		void addAttachmentList(const QList<EmpathAttachmentSpec> & att);

	protected:


	private:

		void setupToolbar();
		
		QPushButton						* pb_add_;
		QPushButton						* pb_edit_;
		QPushButton						* pb_remove_;
		QListView						* lv_attachments_;
		QList<EmpathAttachmentSpec>		attachmentList_;
};

#endif
