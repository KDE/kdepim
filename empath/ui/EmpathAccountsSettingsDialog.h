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

#ifndef EMPATHACCOUNTSSETTINGSDIALOG_H
#define EMPATHACCOUNTSSETTINGSDIALOG_H

// Qt includes
#include <qlayout.h>
#include <qwidget.h>
#include <qlabel.h>
#include <qlistview.h>
#include <qbuttongroup.h>
#include <qstring.h>
#include <qpushbutton.h>

// Local includes
#include "EmpathDefines.h"

class EmpathMailboxList;
class EmpathMailboxLocal;
class EmpathMailboxPOP3;
class EmpathMailboxIMAP4;
class RikGroupBox;

/**
 * Dialog used to configure settings for each account.
 * User can add, remove and edit accounts and see some vital info.
 */
class EmpathAccountsSettingsDialog : public QWidget
{
	Q_OBJECT

	public:
		
		EmpathAccountsSettingsDialog(QWidget * parent, const char * name);

		~EmpathAccountsSettingsDialog() { empathDebug("dtor"); }

		void saveData();

	protected slots:

		void s_newAccount();
		void s_editAccount();
		void s_removeAccount();

	private:

		void updateMailboxList();

		QButtonGroup			* buttonGroup_;
		
		RikGroupBox				* rgb_account_;
		QGridLayout				* accountGroupLayout_;
	
		QWidget					* w_account_;
	
		QListView				* lv_accts_;
		
		QPushButton				* pb_newAccount_;
		QPushButton				* pb_editAccount_;
		QPushButton				* pb_removeAccount_;
	
		QGridLayout				* topLevelLayout_;

};

#endif
