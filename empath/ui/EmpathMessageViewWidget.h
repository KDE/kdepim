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

#ifndef EMPATHMESSAGEVIEWWIDGET_H
#define EMPATHMESSAGEVIEWWIDGET_H

// Qt includes
#include <qpopupmenu.h>
#include <qscrollbar.h>
#include <qlayout.h>

// KDE includes
#include <ktmainwindow.h>
#include <kapp.h>
#include <kstdaccel.h>
#include <khtml.h>

// Local includes
#include <RMM_Message.h>
#include "EmpathDefines.h"
#include "EmpathMessageHTMLView.h"

class EmpathMessageViewWidget : public QWidget
{
	Q_OBJECT

	public:
		
		EmpathMessageViewWidget(
				const EmpathURL & url,
				QWidget * parent = 0L,
				const char * name = 0L);
		
		~EmpathMessageViewWidget();
		
		/**
		 * Tell the internal widget to start parsing
		 */
		void go();
		void resizeEvent(QResizeEvent * e);
		
	public slots:

		void s_print();
		void s_setMessage(const EmpathURL &);

	protected slots:
		
		void s_docChanged();
		void s_hScrollbarSetValue(int);
		void s_vScrollbarSetValue(int);
		void s_URLSelected(const char *, int);

	private:
		
		EmpathMessageHTMLWidget	* messageWidget_;
		QGridLayout				* mainLayout_;
		QScrollBar				* verticalScrollBar_;
		QScrollBar				* horizontalScrollBar_;
		EmpathURL				url_;
		int						scrollbarSize_;
};

#endif
