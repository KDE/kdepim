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

// Qt includes
#include <qimage.h>
#include <qdir.h>

// KDE includes
#include <klocale.h>
#include <kconfig.h>
#include <kapp.h>

// Local includes
#include "EmpathUIUtils.h"
#include "EmpathDisplaySettingsDialog.h"
#include "EmpathConfig.h"
#include "Empath.h"
#include "RikGroupBox.h"
		
EmpathDisplaySettingsDialog::EmpathDisplaySettingsDialog(
		QWidget * parent,
		const char * name)
	:	QWidget(parent, name)
{
	empathDebug("ctor");

	QLineEdit	tempLineEdit((QWidget *)0);
	Q_UINT32 h	= tempLineEdit.sizeHint().height();
	
	fontDialog_	= new KFontDialog(this, "fontDialog");
	CHECK_PTR(fontDialog_);
	fontDialog_->hide();

	rgb_font_	= new RikGroupBox(i18n("Font"), 8, this, "rgb_font");
	CHECK_PTR(rgb_font_);
	
	rgb_colour_	= new RikGroupBox(i18n("Colours"), 8, this, "rgb_colour");
	CHECK_PTR(rgb_colour_);
	
	rgb_style_	= new RikGroupBox(i18n("Message font"), 8, this, "rgb_style");
	CHECK_PTR(rgb_style_);

	w_font_		= new QWidget(rgb_font_, "w_font");
	CHECK_PTR(w_font_);
	
	w_colour_	= new QWidget(rgb_colour_, "w_colour");
	CHECK_PTR(w_colour_);
	
	w_style_	= new QWidget(rgb_style_, "w_style");
	CHECK_PTR(w_style_);
	
	rgb_font_->setWidget(w_font_);
	rgb_style_->setWidget(w_style_);
	rgb_colour_->setWidget(w_colour_);

	rgb_font_->setMinimumHeight(h * 7);
	rgb_style_->setMinimumHeight(h * 5);
	rgb_colour_->setMinimumHeight(h * 8);
	
	setMinimumHeight(h * 24);
	
	// Fonts
	
	// Variable font
	
	l_variableFont_	=
		new QLabel(i18n("Variable width font"), w_font_, "l_variableFont");
	CHECK_PTR(l_variableFont_);

	l_variableFont_->setFixedHeight(h);
	
	l_sampleVariable_	=
		new QLabel(i18n("Sample"), w_font_, "l_sampleVariable");
	CHECK_PTR(l_sampleVariable_);
	
	pb_chooseVariableFont_	=
		new QPushButton(i18n("Choose..."), w_font_, "pb_chooseVariableFont");
	CHECK_PTR(pb_chooseVariableFont_);
	
	QObject::connect(pb_chooseVariableFont_, SIGNAL(clicked()),
			this, SLOT(s_chooseVariableFont()));
	
	// Fixed font
	
	l_fixedFont_	=
		new QLabel(i18n("Fixed width font"), w_font_, "l_fixedFont");
	CHECK_PTR(l_fixedFont_);
	
	l_fixedFont_->setFixedHeight(h);
	
	l_sampleFixed_		=
		new QLabel(i18n("Sample"), w_font_, "l_sampleFixed");
	CHECK_PTR(l_sampleFixed_);
	
	pb_chooseFixedFont_		=
		new QPushButton(i18n("Choose..."), w_font_, "pb_chooseFixedFont");
	CHECK_PTR(pb_chooseFixedFont_);
	
	QObject::connect(pb_chooseFixedFont_, SIGNAL(clicked()),
			this, SLOT(s_chooseFixedFont()));

	// Quoted font
	
	l_quotedFont_	=
		new QLabel(i18n("Quoted text font"), w_font_, "l_quotedFont");
	CHECK_PTR(l_quotedFont_);
	
	l_quotedFont_->setFixedHeight(h);
	
	l_sampleQuoted_		=
		new QLabel(i18n("Sample"), w_font_, "l_sampleQuoted");
	CHECK_PTR(l_sampleQuoted_);

	pb_chooseQuotedFont_	=
		new QPushButton(i18n("Choose..."), w_font_, "pb_chooseQuotedFont");
	CHECK_PTR(pb_chooseQuotedFont_);

	QObject::connect(pb_chooseQuotedFont_, SIGNAL(clicked()),
			this, SLOT(s_chooseQuotedFont()));

	// underline links
	
	cb_underlineLinks_	=
		new QCheckBox(i18n("&Underline Links"), w_font_, "cb_underlineLinks");
	CHECK_PTR(cb_underlineLinks_);
	
	cb_underlineLinks_->setFixedHeight(h);
	
	// use default
	cb_useDefaultFonts_		=
		new QCheckBox(i18n("Use Defaults"), w_font_, "cb_useDefaultFonts");
	CHECK_PTR(cb_useDefaultFonts_);
	
	QObject::connect(cb_useDefaultFonts_, SIGNAL(toggled(bool)),
			this, SLOT(s_useDefaultFonts(bool)));

	// Message font style
	
	buttonGroup_			=
		new QButtonGroup(this, "buttonGroup");
	CHECK_PTR(buttonGroup_);

	buttonGroup_->hide();
	buttonGroup_->setExclusive(true);
	
	rb_messageFontFixed_	=
		new QRadioButton(i18n("Use &fixed width font"),
				w_style_, "rb_messageFontFixed");
	CHECK_PTR(rb_messageFontFixed_);
	
	rb_messageFontFixed_->setFixedHeight(h);
	
	rb_messageFontVariable_	=
		new QRadioButton(i18n("Use &variable width font"),
				w_style_, "rb_messageFontVariable");
	CHECK_PTR(rb_messageFontVariable_);
	
	rb_messageFontVariable_->setFixedHeight(h);
	
	buttonGroup_->insert(rb_messageFontFixed_,		Fixed);
	buttonGroup_->insert(rb_messageFontVariable_,	Variable);
	
	// Colours
	
	// Background
	
	l_backgroundColour_	=
		new QLabel(i18n("Background Colour"), w_colour_, "l_backgroundColour");
	CHECK_PTR(l_backgroundColour_);
	
	l_backgroundColour_->setFixedHeight(h);
	
	kcb_backgroundColour_	=
		new KColorButton(w_colour_, "kcb_backgroundColour");
	CHECK_PTR(kcb_backgroundColour_);

	// Text
	
	l_textColour_	=
		new QLabel(i18n("Text Colour"), w_colour_, "l_textColour");
	CHECK_PTR(l_textColour_);
	
	l_textColour_->setFixedHeight(h);
	
	kcb_textColour_	=
		new KColorButton(w_colour_, "kcb_textColour");
	CHECK_PTR(kcb_textColour_);

	// Link
	
	l_linkColour_	=
		new QLabel(i18n("Link Colour"), w_colour_, "l_linkColour");
	CHECK_PTR(l_linkColour_);
	
	l_linkColour_->setFixedHeight(h);
	
	kcb_linkColour_	=
		new KColorButton(w_colour_, "kcb_linkColour");
	CHECK_PTR(kcb_linkColour_);

	// Visited link
	
	l_visitedLinkColour_	=
		new QLabel(i18n("Visited Link Colour"), w_colour_, "l_visitedColour");
	CHECK_PTR(l_visitedLinkColour_);
	
	l_visitedLinkColour_->setFixedHeight(h);
	
	kcb_visitedLinkColour_	=
		new KColorButton(w_colour_, "kcb_visitedColour");
	CHECK_PTR(kcb_visitedLinkColour_);
	
	// use defaults
	
	cb_useDefaultColours_	=
		new QCheckBox(i18n("Use Defaults"), w_colour_, "cb_useDefaultColours");
	CHECK_PTR(cb_useDefaultColours_);
	
	cb_useDefaultColours_->setFixedHeight(h);
	
	QObject::connect(cb_useDefaultColours_, SIGNAL(toggled(bool)),
			this, SLOT(s_useDefaultColours(bool)));
	
	l_iconSet_ = new QLabel(i18n("Icon set"), this, "l_iconSet");
	CHECK_PTR(l_iconSet_);
	
	l_iconSet_->setFixedHeight(h);
	
	cb_iconSet_ = new QComboBox(this, "cb_iconSet");
	CHECK_PTR(cb_iconSet_);
	
	cb_iconSet_->setFixedHeight(h);
	
	// Layouts
	
	topLevelLayout_				= new QGridLayout(this, 4, 2, 10, 10);
	CHECK_PTR(topLevelLayout_);

	topLevelLayout_->setRowStretch(0, 4);
	topLevelLayout_->setRowStretch(1, 2);
	topLevelLayout_->setRowStretch(2, 5);

	fontGroupLayout_			= new QGridLayout(w_font_, 4, 3, 0, 10);
	CHECK_PTR(fontGroupLayout_);

	messageFontsGroupLayout_	= new QGridLayout(w_style_, 2, 1, 0, 10);
	CHECK_PTR(messageFontsGroupLayout_);

	colourGroupLayout_			= new QGridLayout(w_colour_, 5, 2, 0, 10);
	CHECK_PTR(colourGroupLayout_);

	colourGroupLayout_->setColStretch(0, 4);
	colourGroupLayout_->setColStretch(1, 2);
	
	fontGroupLayout_->setColStretch(0, 5);
	fontGroupLayout_->setColStretch(1, 3);
	fontGroupLayout_->setColStretch(2, 4);
	
	topLevelLayout_->addMultiCellWidget(rgb_font_, 0, 0, 0, 1);
	topLevelLayout_->addMultiCellWidget(rgb_style_, 1, 1, 0, 1);
	topLevelLayout_->addMultiCellWidget(rgb_colour_, 2, 2, 0, 1);
	topLevelLayout_->addWidget(l_iconSet_, 3, 0);
	topLevelLayout_->addWidget(cb_iconSet_, 3, 1);

	fontGroupLayout_->addWidget(l_variableFont_,	0, 0);
	fontGroupLayout_->addWidget(l_fixedFont_,		1, 0);
	fontGroupLayout_->addWidget(l_quotedFont_,		2, 0);
	
	fontGroupLayout_->addWidget(l_sampleVariable_,	0, 1);
	fontGroupLayout_->addWidget(l_sampleFixed_,		1, 1);
	fontGroupLayout_->addWidget(l_sampleQuoted_,	2, 1);
	
	fontGroupLayout_->addWidget(pb_chooseVariableFont_,	0, 2);
	fontGroupLayout_->addWidget(pb_chooseFixedFont_,	1, 2);
	fontGroupLayout_->addWidget(pb_chooseQuotedFont_,	2, 2);
	
	fontGroupLayout_->addMultiCellWidget(cb_underlineLinks_,	3, 3, 0, 1);
	fontGroupLayout_->addWidget(cb_useDefaultFonts_,	3, 2);

	fontGroupLayout_->activate();
	
	messageFontsGroupLayout_->addWidget(rb_messageFontFixed_,		0, 0);
	messageFontsGroupLayout_->addWidget(rb_messageFontVariable_,	1, 0);
	
	messageFontsGroupLayout_->activate();
	
	colourGroupLayout_->addWidget(l_backgroundColour_,		0, 0);
	colourGroupLayout_->addWidget(l_textColour_,			1, 0);
	colourGroupLayout_->addWidget(l_linkColour_,			2, 0);
	colourGroupLayout_->addWidget(l_visitedLinkColour_,		3, 0);

	colourGroupLayout_->addWidget(kcb_backgroundColour_,	0, 1);
	colourGroupLayout_->addWidget(kcb_textColour_,			1, 1);
	colourGroupLayout_->addWidget(kcb_linkColour_,			2, 1);
	colourGroupLayout_->addWidget(kcb_visitedLinkColour_,	3, 1);
	
	colourGroupLayout_->addWidget(cb_useDefaultColours_,		4, 1);

	colourGroupLayout_->activate();

	topLevelLayout_->activate();
};

	void
EmpathDisplaySettingsDialog::s_chooseVariableFont()
{
	QFont fnt = l_sampleVariable_->font();
	fontDialog_->setFont(fnt);
	if (fontDialog_->getFont(fnt) != 0)
		l_sampleVariable_->setFont(fnt);
}

	void
EmpathDisplaySettingsDialog::s_chooseFixedFont()
{
	QFont fnt = l_sampleFixed_->font();
	fontDialog_->setFont(fnt);
	if (fontDialog_->getFont(fnt) != 0)
		l_sampleFixed_->setFont(fnt);
}

	void
EmpathDisplaySettingsDialog::s_chooseQuotedFont()
{
	QFont fnt = l_sampleQuoted_->font();
	fontDialog_->setFont(fnt);
	if (fontDialog_->getFont(fnt) != 0)
		l_sampleQuoted_->setFont(fnt);
}

	void
EmpathDisplaySettingsDialog::s_useDefaultColours(bool yn)
{
	kcb_textColour_->setEnabled(!yn);
	kcb_backgroundColour_->setEnabled(!yn);
	kcb_linkColour_->setEnabled(!yn);
	kcb_visitedLinkColour_->setEnabled(!yn);
}

	void
EmpathDisplaySettingsDialog::s_useDefaultFonts(bool yn)
{
	l_sampleVariable_->setEnabled(!yn);
	l_sampleFixed_->setEnabled(!yn);
	l_sampleQuoted_->setEnabled(!yn);
	
	pb_chooseVariableFont_->setEnabled(!yn);
	pb_chooseFixedFont_->setEnabled(!yn);
	pb_chooseQuotedFont_->setEnabled(!yn);
}

	void
EmpathDisplaySettingsDialog::saveData()
{
	KConfig * c = kapp->getConfig();
	c->setGroup(GROUP_DISPLAY);
#define CWE c->writeEntry
	CWE( KEY_VARIABLE_FONT,			l_sampleVariable_->font()			);
	CWE( KEY_FIXED_FONT,			l_sampleFixed_->font()				);
	CWE( KEY_QUOTED_FONT,			l_sampleQuoted_->font()				);
	CWE( KEY_UNDERLINE_LINKS,		cb_underlineLinks_->isChecked()		);
	CWE( KEY_FONT_STYLE,
		(unsigned long)(rb_messageFontFixed_->isChecked() ? Fixed : Variable));
	CWE( KEY_USE_DEFAULT_FONTS,		cb_useDefaultFonts_->isChecked()	);
	CWE( KEY_USE_DEFAULT_COLOURS,	cb_useDefaultColours_->isChecked()	);
	CWE( KEY_BACKGROUND_COLOUR,		kcb_backgroundColour_->color()		);
	CWE( KEY_TEXT_COLOUR,			kcb_textColour_->color()			);
	CWE( KEY_LINK_COLOUR,			kcb_linkColour_->color()			);
	CWE( KEY_VISITED_LINK_COLOUR,	kcb_visitedLinkColour_->color()		);
	CWE( KEY_ICON_SET,				cb_iconSet_->currentText()			);
#undef CWE
}

	void
EmpathDisplaySettingsDialog::loadData()
{
	KConfig * c = kapp->getConfig();
	c->setGroup(GROUP_DISPLAY);
	
	QFont font; QColor col;

	font = empathGeneralFont();
	
	l_sampleVariable_->setFont(
		c->readFontEntry(KEY_VARIABLE_FONT, &font));
	
	font = empathFixedFont();
	
	l_sampleFixed_->setFont(
		c->readFontEntry(KEY_FIXED_FONT, &font));
	
	l_sampleQuoted_->setFont(
		c->readFontEntry(KEY_QUOTED_FONT, &font));
	
	cb_underlineLinks_->setChecked(c->readBoolEntry(KEY_UNDERLINE_LINKS, true));
	
	rb_messageFontVariable_->setChecked(
		((FontStyle)c->readNumEntry(KEY_FONT_STYLE, Fixed)) == Variable);

	rb_messageFontFixed_->setChecked(!rb_messageFontVariable_->isChecked());
	
	cb_useDefaultFonts_->setChecked(
		c->readBoolEntry(KEY_USE_DEFAULT_FONTS, true));
	
	cb_useDefaultColours_->setChecked(
		c->readBoolEntry(KEY_USE_DEFAULT_COLOURS, true));

	s_useDefaultFonts(		cb_useDefaultFonts_->isChecked());
	s_useDefaultColours(	cb_useDefaultColours_->isChecked());

	col = empathWindowColour();
	
	kcb_backgroundColour_->setColor(
		c->readColorEntry(KEY_BACKGROUND_COLOUR, &col));
	
	col = empathTextColour();
	
	kcb_textColour_->setColor(
		c->readColorEntry(KEY_TEXT_COLOUR, &col));
	
#if QT_VERSION >= 200
		col = Qt::darkBlue;
#else
		col = darkBlue;
#endif

	kcb_linkColour_->setColor(
		c->readColorEntry(KEY_LINK_COLOUR, &col));

#if QT_VERSION >= 200
		col = Qt::darkCyan;
#else
		col = darkCyan;
#endif
	
	kcb_visitedLinkColour_->setColor(
		c->readColorEntry(KEY_VISITED_LINK_COLOUR, &col));
	
	// Fill in the icon set combo.
	QDir d(empathDir() + "/pics/");
	
	d.setFilter(QDir::Dirs | QDir::Readable);
	cb_iconSet_->clear();
	
	QString s = c->readEntry(KEY_ICON_SET, "standard");
	empathDebug("Saved icon set was \"" + s + "\"");

	
	bool found = false;
	int index = 0;
	
	QStringList el = d.entryList();

	QValueListConstIterator<QString> it = el.begin();

	for (; it != el.end() ; ++it) {
		
		if (it->at(0) == ".")
			continue;
		
		if (*it == s)
			found = true;
		else
			++index;
		
		cb_iconSet_->insertItem(*it);
	}
	
	if (found) cb_iconSet_->setCurrentItem(index);
}	
