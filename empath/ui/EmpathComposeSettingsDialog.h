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

#ifndef EMPATHCOMPOSESETTINGSDIALOG_H
#define EMPATHCOMPOSESETTINGSDIALOG_H

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
#include <qspinbox.h>
#include <qdialog.h>

// KDE includes
#include <kbuttonbox.h>

// Local includes
#include "EmpathDefines.h"

class RikGroupBox;
class Empath;

/**
 * Used to configure the settings for composing messages.
 */
class EmpathComposeSettingsDialog : public QDialog
{
	Q_OBJECT

	public:
		
		static void create();

		~EmpathComposeSettingsDialog() { empathDebug("dtor"); exists_ = false; }

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

		EmpathComposeSettingsDialog(QWidget * parent = 0, const char * name = 0);

		QButtonGroup		* buttonGroup_;
		
		QGridLayout			* topLevelLayout_;

		QGridLayout			* phrasesGroupLayout_;
		QGridLayout			* messageGroupLayout_;
		QGridLayout			* whenGroupLayout_;
		QGridLayout			* externalLayout_;

		RikGroupBox			* rgb_phrases_;
		RikGroupBox			* rgb_msg_;
		RikGroupBox			* rgb_when_;
	
		QWidget				* w_phrases_;
		QWidget				* w_msg_;
		QWidget				* w_when_;

		QLabel				* l_reply_;
		QLabel				* l_replyAll_;
		QLabel				* l_forward_;

		QLineEdit			* le_reply_;
		QLineEdit			* le_replyAll_;
		QLineEdit			* le_forward_;
	
		QSpinBox			* sb_wrap_;

		QCheckBox			* cb_addSig_;
		QCheckBox			* cb_digSign_;
		QCheckBox			* cb_wrap_;
		QCheckBox			* cb_quote_;
	
		QRadioButton		* rb_sendNow_;
		QRadioButton		* rb_sendLater_;
		
		QCheckBox			* cb_externalEditor_;
		QLineEdit			* le_externalEditor_;

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
