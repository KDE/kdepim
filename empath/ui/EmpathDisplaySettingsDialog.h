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

// KDE includes
#include <kfontdialog.h>
#include <kcolorbtn.h>

// Local includes
#include "EmpathDefines.h"

class RikGroupBox;

class EmpathDisplaySettingsDialog : public QWidget
{
	Q_OBJECT

	public:
		
		EmpathDisplaySettingsDialog(QWidget * parent = 0, const char * name = 0);

		~EmpathDisplaySettingsDialog() { empathDebug("dtor"); }

		void saveData();
		void loadData();

	protected slots:
		
		void s_chooseFixedFont();
		void s_chooseVariableFont();
		void s_chooseQuotedFont();

		void s_useDefaultFonts(bool yn);
		void s_useDefaultColours(bool yn);

	private:

		KFontDialog		* fontDialog_;

		QButtonGroup	* buttonGroup_;

		QGridLayout		* topLevelLayout_;
		QGridLayout		* fontGroupLayout_;
		QGridLayout		* messageFontsGroupLayout_;
		QGridLayout		* colourGroupLayout_;

		RikGroupBox		* rgb_font_;
		RikGroupBox		* rgb_colour_;
		RikGroupBox		* rgb_style_;

		QWidget			* w_font_;
		QWidget			* w_colour_;
		QWidget			* w_style_;
		
		QLabel			* l_variableFont_;
		QLabel			* l_fixedFont_;
		QLabel			* l_quotedFont_;
		
		QLabel			* l_sampleVariable_;
		QLabel			* l_sampleFixed_;
		QLabel			* l_sampleQuoted_;
		
		QPushButton		* pb_chooseVariableFont_;
		QPushButton		* pb_chooseFixedFont_;
		QPushButton		* pb_chooseQuotedFont_;
		
		QCheckBox		* cb_useDefaultFonts_;
		QCheckBox		* cb_useDefaultColours_;

		QLabel			* l_textColour_;
		QLabel			* l_backgroundColour_;
		QLabel			* l_linkColour_;
		QLabel			* l_visitedLinkColour_;
		
		KColorButton	* kcb_textColour_;
		KColorButton	* kcb_backgroundColour_;
		KColorButton	* kcb_linkColour_;
		KColorButton	* kcb_visitedLinkColour_;
		
		QCheckBox		* cb_underlineLinks_;

		QRadioButton	* rb_messageFontFixed_;
		QRadioButton	* rb_messageFontVariable_;
		
		QLabel			* l_iconSet_;
		QComboBox		* cb_iconSet_;
};

#endif
