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
# pragma interface "EmpathSubjectSpecWidget.h"
#endif

#ifndef EMPATHSUBJECTSPECWIDGET_H
#define EMPATHSUBJECTSPECWIDGET_H

// Qt includes
#include <qwidget.h>
#include <qstring.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qlineedit.h>

// Local includes
#include "EmpathDefines.h"

/**
 * This is a simple megawidget that's just a label and a lineedit in
 * a layout - it's used to get the subject for a message.
 */
class EmpathSubjectSpecWidget : public QWidget
{
	Q_OBJECT

	public:
		
		EmpathSubjectSpecWidget(QWidget * parent = 0, const char * name = 0);
		~EmpathSubjectSpecWidget();
		
		/**
		 * Get what's currently in the subject field
		 */
		QString	getSubject() const;

		/**
		 * Set the subject field. Use this immediately after creation
		 * to set the field to 'Re: your mail' or whatever.
		 */
		void	setSubject(const QString & subject);
		void	setFocus();
		
	private:

		QLabel		* l_subject_;
		QLineEdit	* le_subject_;
};

#endif
