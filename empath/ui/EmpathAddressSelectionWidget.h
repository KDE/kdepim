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
# pragma interface "EmpathAddressSelectionWidget.h"
#endif

#ifndef EMPATH_ADDRESS_SELECTION_WIDGET_H
#define EMPATH_ADDRESS_SELECTION_WIDGET_H

// Qt includes
#include <qwidget.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlayout.h>

/**
 * Megawidget used to get an address from the user.
 * Not yet implemented: Allows user to browse through address book.
 */
class EmpathAddressSelectionWidget : public QWidget
{
	Q_OBJECT
	
	public:
	
		EmpathAddressSelectionWidget(QWidget * parent = 0, const char * name = 0);
		virtual ~EmpathAddressSelectionWidget();

		QString selectedAddress() const;
		void setAddress(const QString &);
		QString text() const { return selectedAddress(); }
		void setText(const QString & s) { setAddress(s); }

	protected slots:

		void s_textChanged(const QString&);
		void s_lostFocus();
		void s_browseClicked();
		
	private:

		QGridLayout		* layout_;

		QLineEdit		* le_address_;
		QPushButton		* pb_browse_;
};

#endif

