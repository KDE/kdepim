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
# pragma interface "EmpathSendingSettingsDialog.h"
#endif

#ifndef EMPATHSENDINGSETTINGSDIALOG_H
#define EMPATHSENDINGSETTINGSDIALOG_H

// Qt includes
#include <qlayout.h>
#include <qwidget.h>
#include <qdialog.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qbuttongroup.h>
#include <qspinbox.h>

// KDE includes
#include <kbuttonbox.h>

// Local includes
#include "EmpathDefines.h"

class RikGroupBox;
class EmpathAddressSelectionWidget;
class EmpathFolderChooserWidget;

class EmpathSendingSettingsDialog : public QDialog
{
	Q_OBJECT

	public:
		
		static void create();

		~EmpathSendingSettingsDialog() { empathDebug("dtor"); exists_ = false; }

		void saveData();
		void loadData();
		void closeEvent(QCloseEvent *);
		
	protected slots:

		void s_OK();
		void s_cancel();
		void s_help();
		void s_default();
		void s_apply();

	private:

		EmpathSendingSettingsDialog(QWidget * parent = 0, const char * name = 0);

		QButtonGroup		* serverButtonGroup_;
		
		QGridLayout			* topLevelLayout_;

		QGridLayout			* serverGroupLayout_;
		QGridLayout			* copiesGroupLayout_;
		QGridLayout			* queuingGroupLayout_;

		RikGroupBox			* rgb_queuing_;
		RikGroupBox			* rgb_server_;
		RikGroupBox			* rgb_copies_;
		
		QLabel				* l_queueFolder_;
		QLabel				* l_sentFolder_;
		EmpathFolderChooserWidget	* fcw_queueFolder_;
		EmpathFolderChooserWidget	* fcw_sentFolder_;
	
		QWidget				* w_queuing_;
		QWidget				* w_server_;
		QWidget				* w_copies_;

		QLineEdit			* le_sendmail_;
		QLineEdit			* le_qmail_;
		QLineEdit			* le_smtpServer_;

		QCheckBox			* cb_copyOther_;
		
		QPushButton			* pb_sendmailBrowse_;
		QPushButton			* pb_qmailBrowse_;

		QRadioButton		* rb_sendmail_;
		QRadioButton		* rb_qmail_;
		QRadioButton		* rb_smtp_;

		QLabel				* l_smtpServerPort_;

		QSpinBox			* sb_smtpPort_;
		
		EmpathAddressSelectionWidget	* asw_copyOther_;

		KButtonBox		* buttonBox_;
		QPushButton		* pb_help_;
		QPushButton		* pb_default_;
		QPushButton		* pb_apply_;
		QPushButton		* pb_OK_;
		QPushButton		* pb_cancel_;
		
		static bool		exists_;
		bool			applied_;

};

#endif
