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
# pragma interface "EmpathDisplaySettingsDialog.h"
#endif

#ifndef EMPATHDISPLAYSETTINGSDIALOG_H
#define EMPATHDISPLAYSETTINGSDIALOG_H

// Qt includes
#include <qwidget.h>
#include <qlabel.h>
#include <qbuttongroup.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qdialog.h>

// KDE includes
#include <kcolorbtn.h>
#include <kbuttonbox.h>

// Local includes
#include "EmpathDefines.h"

class RikGroupBox;

/**
 * Configure the appearance of various stuff.
 */
class EmpathDisplaySettingsDialog : public QDialog
{
	Q_OBJECT

	public:
		
		static void create();
		~EmpathDisplaySettingsDialog() { empathDebug("dtor"); exists_ = false; }

		void saveData();
		void loadData();
		void closeEvent(QCloseEvent *);

	protected slots:
		
		void s_chooseFixedFont();

		void s_OK();
		void s_cancel();
		void s_help();
		void s_default();
		void s_apply();

	private:

		EmpathDisplaySettingsDialog(QWidget * parent = 0, const char * name = 0);

		QButtonGroup	* buttonGroup_;

		QGridLayout		* topLevelLayout_;
		QGridLayout		* viewGroupLayout_;
		QGridLayout		* listGroupLayout_;

		RikGroupBox		* rgb_list_;
		RikGroupBox		* rgb_view_;

		QWidget			* w_list_;
		QWidget			* w_view_;
		
		QLabel			* l_fixedFont_;
		QLabel			* l_sampleFixed_;
		QPushButton		* pb_chooseFixedFont_;
		
		QLabel			* l_quoteColourTwo_;
		QLabel			* l_quoteColourOne_;
		QLabel			* l_linkColour_;
		QLabel			* l_visitedLinkColour_;
		
		KColorButton	* kcb_quoteColourTwo_;
		KColorButton	* kcb_quoteColourOne_;
		KColorButton	* kcb_linkColour_;
		KColorButton	* kcb_visitedLinkColour_;
		
		QCheckBox		* cb_underlineLinks_;
		
		QCheckBox		* cb_threadMessages_;
		
		QLabel			* l_displayHeaders_;
		QLineEdit		* le_displayHeaders_;

		QLabel			* l_sortColumn_;
		QComboBox		* cb_sortColumn_;

		QCheckBox		* cb_sortAscending_;
		
		QLabel			* l_iconSet_;
		QComboBox		* cb_iconSet_;
		
		KButtonBox		* buttonBox_;
		QPushButton		* pb_help_;
		QPushButton		* pb_default_;
		QPushButton		* pb_apply_;
		QPushButton		* pb_OK_;
		QPushButton		* pb_cancel_;
		
		QCheckBox		* cb_timer_;
		QSpinBox		* sb_timer_;
		
		static bool		exists_;
		bool			applied_;
};

#endif
