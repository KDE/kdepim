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
#include <RMM_Message.h>
#include "EmpathDefines.h"
#include "EmpathAttachmentListItem.h"

/**
 * This widget shows the structure of a message.
 */
class EmpathAttachmentListWidget : public QWidget
{
	Q_OBJECT
	
	public:
		
		EmpathAttachmentListWidget(QWidget * parent = 0, const char * name = 0);
		~EmpathAttachmentListWidget();

		void use(const RMessage &);
		
	private:

		QListView	* lv_attachments_;
		RMessage	message_;
};

#endif
