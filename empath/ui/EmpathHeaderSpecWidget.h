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

#ifndef EMPATHHEADERSPECWIDGET_H
#define EMPATHHEADERSPECWIDGET_H

// Qt includes
#include <qwidget.h>
#include <qlist.h>
#include <qstring.h>
#include <qstrlist.h>
#include <qlayout.h>
#include <qpushbutton.h>

// Local includes
#include "EmpathDefines.h"
#include <RMM_Header.h>

class EmpathHeaderNameWidget;
class EmpathHeaderBodyWidget;

class EmpathHeaderSpecWidget : public QWidget
{
	Q_OBJECT

	public:
		
		EmpathHeaderSpecWidget(
			int headerIndex, QWidget * parent = 0, const char * name = 0);
		~EmpathHeaderSpecWidget();

		QCString	header();
		void		setHeaderList(const QStrList & headerList);
		void		setHeaderName(const QString & headerName);
		void		setHeaderBody(const QString & headerBody);

	protected slots:

		void s_headerNameActivated(const char * text);
		void s_headerNameAccepted();
		void s_headerBodyChanged();
		void s_headerBodyAccepted();
		void s_selectRecipients();
	
	signals:
	
		void nameActivated(const char * text);
		void textChanged();
		void returnPressed();
		void _empath_textChanged(int);
		void _empath_returnPressed(int);
		
	private:

		EmpathHeaderNameWidget	* headerNameWidget_;
		EmpathHeaderBodyWidget	* headerBodyWidget_;
		QPushButton				* pb_selectRecipients_;
};

#endif
