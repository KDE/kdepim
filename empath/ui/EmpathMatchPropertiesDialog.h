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
# pragma interface "EmpathMatchPropertiesDialog.h"
#endif

#ifndef EMPATHMATCHPROPERTIESDIALOG_H
#define EMPATHMATCHPROPERTIESDIALOG_H

// Qt includes
#include <qwidget.h>
#include <qdialog.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qspinbox.h>

// KDE includes
#include <kbuttonbox.h>

class RikGroupBox;
class EmpathMatcher;

class EmpathMatchPropertiesDialog : public QDialog
{
	Q_OBJECT

	public:

		EmpathMatchPropertiesDialog(QWidget * parent, EmpathMatcher * matcher);

		~EmpathMatchPropertiesDialog();
		EmpathMatcher * matcher();

	protected slots:

		void s_OK();
		void s_cancel();
		void s_help();

	private:
		
		EmpathMatcher	* matcher_;

		QButtonGroup	* bg_choices_;

		QGridLayout		* mainLayout_;
		QGridLayout		* layout_;
		QGridLayout		* size_subLayout_;
		QGridLayout		* exprBody_subLayout_;
		QGridLayout		* exprHeader_subLayout_;

		KButtonBox		* buttonBox_;

		QPushButton		* pb_OK_;
		QPushButton		* pb_cancel_;
		QPushButton		* pb_help_;

		RikGroupBox		* rgb_choices_;

		QWidget			* w_choices_;

		QRadioButton	* rb_size_;
		QRadioButton	* rb_exprBody_;
		QRadioButton	* rb_exprHeader_;
		QRadioButton	* rb_attached_;
		QRadioButton	* rb_all_;

		QLineEdit		* le_exprBody_;
		QLineEdit		* le_exprHeader_;

		QComboBox		* cb_header_;

		QSpinBox		* sb_size_;
		
		int idx_size_, idx_exprBody_, idx_exprHeader_, idx_attached_, idx_all_;
};

#endif

