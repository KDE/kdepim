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
#include <kquickhelp.h>

// Local includes
#include "EmpathUIUtils.h"
#include "EmpathDisplaySettingsDialog.h"
#include "EmpathConfig.h"
#include "Empath.h"
#include "RikGroupBox.h"

bool EmpathDisplaySettingsDialog::exists_ = false;

	void
EmpathDisplaySettingsDialog::create()
{
	if (exists_) return;
	exists_ = true;
	EmpathDisplaySettingsDialog * d = new EmpathDisplaySettingsDialog(0, 0);
	CHECK_PTR(d);
	d->show();
	kapp->processEvents();
	d->loadData();
}
		
EmpathDisplaySettingsDialog::EmpathDisplaySettingsDialog(
		QWidget * parent,
		const char * name)
	:	QDialog(parent, name, false),
		applied_(false)
{
	empathDebug("ctor");
	setCaption(i18n("Display Settings - ") + kapp->getCaption());
	
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

	rgb_font_->setMinimumHeight(h * 6);
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
	
	KQuickHelp::add(pb_chooseVariableFont_, i18n(
			"Empath uses two main fonts. The fixed font is\n"
			"used for displaying and composing messages.\n"
			"The variable font is used everywhere else that\n"
			"the system font is unsuitable, i.e. when displaying\n"
			"some parts of a message that don't need to be in\n"
			"the fixed width font."));
			
	
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
	
	KQuickHelp::add(pb_chooseFixedFont_, i18n(
			"Empath uses two main fonts. The fixed font is\n"
			"used for displaying and composing messages.\n"
			"The variable font is used everywhere else that\n"
			"the system font is unsuitable, i.e. when displaying\n"
			"some parts of a message that don't need to be in\n"
			"the fixed width font."));
	
	// underline links
	
	cb_underlineLinks_	=
		new QCheckBox(i18n("&Underline Links"), w_font_, "cb_underlineLinks");
	CHECK_PTR(cb_underlineLinks_);
	
	cb_underlineLinks_->setFixedHeight(h);
	
	KQuickHelp::add(cb_underlineLinks_, i18n(
			"Choose whether to have links underlined.\n"
			"Links are email addresses, http:// type\n"
			"addresses, etc. If you're colour blind,\n"
			"this is a smart move."));
		
	
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
	
	KQuickHelp::add(rb_messageFontFixed_, i18n(
			"Use a fixed width font for showing and\n"
			"composing messages. This is a really good\n"
			"idea as messages can look dreadful in a\n"
			"variable width font."));
	
	rb_messageFontVariable_	=
		new QRadioButton(i18n("Use &variable width font"),
				w_style_, "rb_messageFontVariable");
	CHECK_PTR(rb_messageFontVariable_);
	
	rb_messageFontVariable_->setFixedHeight(h);
	
	KQuickHelp::add(rb_messageFontFixed_, i18n(
			"Use a variable width font for showing and\n"
			"composing messages. Not a good plan. The\n"
			"rest of the world uses fixed width fonts\n"
			"for email, so you'll be the odd one out.\n"));
	
	buttonGroup_->insert(rb_messageFontFixed_,		Fixed);
	buttonGroup_->insert(rb_messageFontVariable_,	Variable);
	
	// Colours
	
	// Background
	
	l_backgroundColour_	=
		new QLabel(i18n("Background Colour"), w_colour_, "l_backgroundColour");
	CHECK_PTR(l_backgroundColour_);
	
	l_backgroundColour_->setFixedHeight(h);
	
	KQuickHelp::add(l_backgroundColour_, i18n(
			"Choose the background colour for reading\n"
			"messages. If you don't like the standard,\n"
			"you can use this. You could instead change\n"
			"the template for displaying messages, if you\n"
			"know HTML. This allows for greater power,\n"
			"but is a little trickier."));
	
	kcb_backgroundColour_	=
		new KColorButton(w_colour_, "kcb_backgroundColour");
	CHECK_PTR(kcb_backgroundColour_);

	// Text
	
	l_textColour_	=
		new QLabel(i18n("Text Colour"), w_colour_, "l_textColour");
	CHECK_PTR(l_textColour_);
	
	l_textColour_->setFixedHeight(h);
		
	KQuickHelp::add(l_textColour_, i18n(
			"Choose the text colour for reading\n"
			"messages. If you don't like the standard,\n"
			"you can use this. You could instead change\n"
			"the template for displaying messages, if you\n"
			"know HTML. This allows for greater power,\n"
			"but is a little trickier."));
	
	kcb_textColour_	=
		new KColorButton(w_colour_, "kcb_textColour");
	CHECK_PTR(kcb_textColour_);

	// Link
	
	l_linkColour_	=
		new QLabel(i18n("Link Colour"), w_colour_, "l_linkColour");
	CHECK_PTR(l_linkColour_);
	
	l_linkColour_->setFixedHeight(h);
			
	KQuickHelp::add(l_linkColour_, i18n(
			"Choose the link colour for reading\n"
			"messages. If you don't like the standard,\n"
			"you can use this. You could instead change\n"
			"the template for displaying messages, if you\n"
			"know HTML. This allows for greater power,\n"
			"but is a little trickier."));
	
	kcb_linkColour_	=
		new KColorButton(w_colour_, "kcb_linkColour");
	CHECK_PTR(kcb_linkColour_);

	// Visited link
	
	l_visitedLinkColour_	=
		new QLabel(i18n("Visited Link Colour"), w_colour_, "l_visitedColour");
	CHECK_PTR(l_visitedLinkColour_);
	
	l_visitedLinkColour_->setFixedHeight(h);
				
	KQuickHelp::add(l_visitedLinkColour_, i18n(
			"Choose the visited link colour for reading\n"
			"messages. If you don't like the standard,\n"
			"you can use this. You could instead change\n"
			"the template for displaying messages, if you\n"
			"know HTML. This allows for greater power,\n"
			"but is a little trickier."));
	
	kcb_visitedLinkColour_	=
		new KColorButton(w_colour_, "kcb_visitedColour");
	CHECK_PTR(kcb_visitedLinkColour_);
	
	// use defaults
	
	l_iconSet_ = new QLabel(i18n("Icon set"), this, "l_iconSet");
	CHECK_PTR(l_iconSet_);
	
	l_iconSet_->setFixedHeight(h);
	
	KQuickHelp::add(l_visitedLinkColour_, i18n(
			"Here you get to choose the icon set\n"
			"that will be used by Empath. This covers\n"
			"the toolbars, the icons on menus, and\n"
			"those in the message list and folder\n"
			"tree. You can create your own icon sets\n"
			"by simply copying one and changing it.\n"
			"See the help for details.\n"
			"<b>Note</b> that this change does not\n"
			"take effect until Empath is restarted."));
	
	cb_iconSet_ = new QComboBox(this, "cb_iconSet");
	CHECK_PTR(cb_iconSet_);
	
	cb_iconSet_->setFixedHeight(h);
	
///////////////////////////////////////////////////////////////////////////////
// Button box

	buttonBox_	= new KButtonBox(this);
	CHECK_PTR(buttonBox_);

	buttonBox_->setFixedHeight(h);
	
	pb_help_	= buttonBox_->addButton(i18n("&Help"));	
	CHECK_PTR(pb_help_);
	
	pb_default_	= buttonBox_->addButton(i18n("&Default"));	
	CHECK_PTR(pb_default_);
	
	buttonBox_->addStretch();
	
	pb_OK_		= buttonBox_->addButton(i18n("&OK"));
	CHECK_PTR(pb_OK_);
	
	pb_OK_->setDefault(true);
	
	pb_apply_	= buttonBox_->addButton(i18n("&Apply"));
	CHECK_PTR(pb_apply_);
	
	pb_cancel_	= buttonBox_->addButton(i18n("&Cancel"));
	CHECK_PTR(pb_cancel_);
	
	buttonBox_->layout();
	
	QObject::connect(pb_OK_,		SIGNAL(clicked()),	SLOT(s_OK()));
	QObject::connect(pb_default_,	SIGNAL(clicked()),	SLOT(s_default()));
	QObject::connect(pb_apply_,		SIGNAL(clicked()),	SLOT(s_apply()));
	QObject::connect(pb_cancel_,	SIGNAL(clicked()),	SLOT(s_cancel()));
	QObject::connect(pb_help_,		SIGNAL(clicked()),	SLOT(s_help()));
/////////////////////////////////////////////////////////////////////////////

	// Layouts
	
	topLevelLayout_				= new QGridLayout(this, 5, 2, 10, 10);
	CHECK_PTR(topLevelLayout_);

	topLevelLayout_->setRowStretch(0, 4);
	topLevelLayout_->setRowStretch(1, 2);
	topLevelLayout_->setRowStretch(2, 5);

	fontGroupLayout_			= new QGridLayout(w_font_, 3, 3, 0, 10);
	CHECK_PTR(fontGroupLayout_);

	messageFontsGroupLayout_	= new QGridLayout(w_style_, 2, 1, 0, 10);
	CHECK_PTR(messageFontsGroupLayout_);

	colourGroupLayout_			= new QGridLayout(w_colour_, 4, 2, 0, 10);
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
	topLevelLayout_->addMultiCellWidget(buttonBox_, 4, 4, 0, 1);
	
	fontGroupLayout_->addWidget(l_variableFont_,	0, 0);
	fontGroupLayout_->addWidget(l_fixedFont_,		1, 0);
	
	fontGroupLayout_->addWidget(l_sampleVariable_,	0, 1);
	fontGroupLayout_->addWidget(l_sampleFixed_,		1, 1);
	
	fontGroupLayout_->addWidget(pb_chooseVariableFont_,	0, 2);
	fontGroupLayout_->addWidget(pb_chooseFixedFont_,	1, 2);
	
	fontGroupLayout_->addMultiCellWidget(cb_underlineLinks_,	3, 3, 0, 1);

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
	
	colourGroupLayout_->activate();

	topLevelLayout_->activate();
	
	setMinimumSize(minimumSizeHint());
	resize(minimumSizeHint());
};

	void
EmpathDisplaySettingsDialog::s_chooseVariableFont()
{
	QFont fnt = l_sampleVariable_->font();
	fontDialog_->setFont(fnt);
	if (fontDialog_->getFont(fnt) == QDialog::Accepted)
		l_sampleVariable_->setFont(fnt);
}

	void
EmpathDisplaySettingsDialog::s_chooseFixedFont()
{
	QFont fnt = l_sampleFixed_->font();
	fontDialog_->setFont(fnt);
	if (fontDialog_->getFont(fnt) == QDialog::Accepted)
		l_sampleFixed_->setFont(fnt);
}

	void
EmpathDisplaySettingsDialog::saveData()
{
	KConfig * c(kapp->getConfig());
	c->setGroup(EmpathConfig::GROUP_DISPLAY);
#define CWE c->writeEntry
	CWE( EmpathConfig::KEY_VARIABLE_FONT,		l_sampleVariable_->font());
	CWE( EmpathConfig::KEY_FIXED_FONT,			l_sampleFixed_->font());
	CWE( EmpathConfig::KEY_UNDERLINE_LINKS,		cb_underlineLinks_->isChecked());
	CWE( EmpathConfig::KEY_FONT_STYLE,
		(unsigned long)(rb_messageFontFixed_->isChecked() ? Fixed : Variable));

	CWE( EmpathConfig::KEY_BACKGROUND_COLOUR,	kcb_backgroundColour_->color());
	CWE( EmpathConfig::KEY_TEXT_COLOUR,			kcb_textColour_->color());
	CWE( EmpathConfig::KEY_LINK_COLOUR,			kcb_linkColour_->color());
	CWE( EmpathConfig::KEY_VISITED_LINK_COLOUR,	kcb_visitedLinkColour_->color());
	CWE( EmpathConfig::KEY_ICON_SET,			cb_iconSet_->currentText());
#undef CWE
}

	void
EmpathDisplaySettingsDialog::loadData()
{
	KConfig * c = kapp->getConfig();
	c->setGroup(EmpathConfig::GROUP_DISPLAY);
	
	QFont font; QColor col;

	font = empathGeneralFont();
	
	l_sampleVariable_->setFont(
		c->readFontEntry(EmpathConfig::KEY_VARIABLE_FONT, &font));
	
	font = empathFixedFont();
	
	l_sampleFixed_->setFont(
		c->readFontEntry(EmpathConfig::KEY_FIXED_FONT, &font));
	
	cb_underlineLinks_->setChecked(c->readBoolEntry(EmpathConfig::KEY_UNDERLINE_LINKS, true));
	
	rb_messageFontVariable_->setChecked(
		((FontStyle)c->readNumEntry(EmpathConfig::KEY_FONT_STYLE, Fixed)) == Variable);

	rb_messageFontFixed_->setChecked(!rb_messageFontVariable_->isChecked());
	
	col = empathWindowColour();
	
	kcb_backgroundColour_->setColor(
		c->readColorEntry(EmpathConfig::KEY_BACKGROUND_COLOUR, &col));
	
	col = empathTextColour();
	
	kcb_textColour_->setColor(
		c->readColorEntry(EmpathConfig::KEY_TEXT_COLOUR, &col));
	
	col = Qt::darkBlue;

	kcb_linkColour_->setColor(
		c->readColorEntry(EmpathConfig::KEY_LINK_COLOUR, &col));

	col = Qt::darkCyan;
	
	kcb_visitedLinkColour_->setColor(
		c->readColorEntry(EmpathConfig::KEY_VISITED_LINK_COLOUR, &col));
	
	// Fill in the icon set combo.
	QDir d(empathDir() + "/pics/");
	
	d.setFilter(QDir::Dirs | QDir::Readable);
	cb_iconSet_->clear();
	
	QString s = c->readEntry(EmpathConfig::KEY_ICON_SET, "standard");
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
			cb_iconSet_->insertItem(*it, index);
	}
	
	if (found) cb_iconSet_->insertItem(s);
	cb_iconSet_->setCurrentItem(cb_iconSet_->count() - 1);
}

	void
EmpathDisplaySettingsDialog::s_OK()
{
	if (!applied_)
		kapp->getConfig()->rollback(true);
	
	kapp->getConfig()->sync();
	delete this;
}

	void
EmpathDisplaySettingsDialog::s_help()
{
	empathInvokeHelp(QString::null, QString::null);
}

	void
EmpathDisplaySettingsDialog::s_apply()
{
	if (applied_) {
		pb_apply_->setText(i18n("&Apply"));
		kapp->getConfig()->rollback(true);
		kapp->getConfig()->reparseConfiguration();
		loadData();
		applied_ = false;
	} else {
		pb_apply_->setText(i18n("&Revert"));
		pb_cancel_->setText(i18n("&Close"));
		applied_ = true;
	}
	saveData();
}

	void
EmpathDisplaySettingsDialog::s_default()
{
	l_sampleVariable_->setFont(empathGeneralFont());
	l_sampleFixed_->setFont(empathFixedFont());
	cb_underlineLinks_->setChecked(true);
	rb_messageFontFixed_->setChecked(true);
	rb_messageFontVariable_->setChecked(false);
	kcb_backgroundColour_->setColor(empathWindowColour());
	kcb_textColour_->setColor(empathTextColour());
	kcb_linkColour_->setColor(Qt::blue);
	kcb_visitedLinkColour_->setColor(Qt::darkCyan);
	saveData();
}
	
	void
EmpathDisplaySettingsDialog::s_cancel()
{
	if (!applied_)
		kapp->getConfig()->rollback(true);
	delete this;
}

