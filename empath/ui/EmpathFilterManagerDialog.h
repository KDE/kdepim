
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

#ifndef EMPATHFILTERMANAGERDIALOG_H
#define EMPATHFILTERMANAGERDIALOG_H

// Qt includes
#include <qwidget.h>
#include <qlistview.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>

// KDE includes
#include <kbuttonbox.h>

// Local includes
#include "EmpathFilterListItem.h"

class RikGroupBox;

class EmpathFilterManagerDialog : public QWidget
{
	Q_OBJECT

	public:
	
		EmpathFilterManagerDialog(QWidget * parent = 0, const char * name = 0);
		virtual ~EmpathFilterManagerDialog();
		void saveData();
		
	protected slots:
		
		void s_addFilter();
		void s_editFilter();
		void s_removeFilter();
		void s_moveUp();
		void s_moveDown();
		
	private:

		void			update();

		RikGroupBox		* rgb_filters_;

		QWidget			* w_filters_;

		QGridLayout		* mainLayout_;
		QGridLayout		* filtersLayout_;

		QListView		* lv_filters_;

		QLabel			* l_about_;
		
		QPushButton		* pb_addFilter_;
		QPushButton		* pb_editFilter_;
		QPushButton		* pb_removeFilter_;
		QPushButton		* pb_moveUp_;
		QPushButton		* pb_moveDown_;
		QPushButton		* pb_editAction_;

		KButtonBox		* filtersButtonBox_;

		QList<EmpathFilterListItem> filterList_;
};

#endif

