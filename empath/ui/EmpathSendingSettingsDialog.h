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

#ifndef EMPATHSENDINGSETTINGSDIALOG_H
#define EMPATHSENDINGSETTINGSDIALOG_H

// Qt includes
#include <qlayout.h>
#include <qwidget.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qbuttongroup.h>

#ifdef EMPATH_QT_BUILD
# include <qspinbox.h>
#else
# include <kspinbox.h>
#endif

// Local includes
#include "EmpathDefines.h"

class RikGroupBox;
class Empath;

class EmpathSendingSettingsDialog : public QWidget
{
	Q_OBJECT

	public:
		
		EmpathSendingSettingsDialog(QWidget * parent = 0, const char * name = 0);

		~EmpathSendingSettingsDialog() { empathDebug("dtor"); }

		void saveData();
		void loadData();

	protected:


	private:

		QButtonGroup		* serverButtonGroup_;
		
		QGridLayout			* topLevelLayout_;

		QGridLayout			* serverGroupLayout_;
		QGridLayout			* copiesGroupLayout_;

		RikGroupBox			* rgb_server_;
		RikGroupBox			* rgb_copies_;
	
		QWidget				* w_server_;
		QWidget				* w_copies_;

		QLineEdit			* le_copyOther_;
		QLineEdit			* le_sendmail_;
		QLineEdit			* le_qmail_;
		QLineEdit			* le_smtpServer_;
		QLineEdit			* le_qmtpServer_;

		QCheckBox			* cb_copySelf_;
		QCheckBox			* cb_copyOther_;
		QCheckBox			* cb_copyFolder_;

		QComboBox			* cmb_copyFolder_;
		
		QPushButton			* pb_browseCopyOther_;
		QPushButton			* pb_sendmailBrowse_;
		QPushButton			* pb_qmailBrowse_;

		QRadioButton		* rb_sendmail_;
		QRadioButton		* rb_qmail_;
		QRadioButton		* rb_smtp_;
		QRadioButton		* rb_qmtp_;

		QLabel				* l_smtpServerPort_;
		QLabel				* l_qmtpServerPort_;
		
#ifdef EMPATH_QT_BUILD
		QSpinBox			* sb_smtpPort_;
		QSpinBox			* sb_qmtpPort_;
#else
		KNumericSpinBox		* sb_smtpPort_;
		KNumericSpinBox		* sb_qmtpPort_;
#endif
};

#endif
