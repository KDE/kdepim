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
# pragma implementation "EmpathDisplaySettingsDialog.h"
#endif

// Qt includes
#include <qimage.h>
#include <qdir.h>

// KDE includes
#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kapp.h>
#include <kquickhelp.h>
#include <kfontdialog.h>
#include <kstddirs.h>

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
	
	rgb_list_	= new RikGroupBox(i18n("Message List"), 8, this, "rgb_list");
	CHECK_PTR(rgb_list_);
	
	rgb_view_	= new RikGroupBox(i18n("Message Viewing"), 8, this, "rgb_view");
	CHECK_PTR(rgb_view_);
	
	w_list_	= new QWidget(rgb_list_, "w_list");
	CHECK_PTR(w_list_);
	
	w_view_	= new QWidget(rgb_view_, "w_view");
	CHECK_PTR(w_view_);
	
	rgb_list_->setWidget(w_list_);
	rgb_view_->setWidget(w_view_);

////////////////////////////////////////////////////////////////////////	
// Message view
// 
	l_displayHeaders_ =
		new QLabel(i18n("Display headers"), w_view_, "l_displayHeaders");
	CHECK_PTR(l_displayHeaders_);
	
	l_displayHeaders_->setFixedHeight(h);
	
	le_displayHeaders_ =
		new QLineEdit(w_view_, "le_displayHeaders");
	CHECK_PTR(le_displayHeaders_);
	
	le_displayHeaders_->setFixedHeight(h);
	
	KQuickHelp::add(le_displayHeaders_, i18n(
			"Here you may enter the headers that you\n"
			"want to appear in the block above the message\n"
			"you are reading. The default is:\n"
			"From,Date,Subject\n\n"
			"You must separate the header names by commas (,).\n"
			"This is not case-sensitive, i.e. you can write\n"
			"DATE and 'Date', 'date', 'DaTe' etc will all work\n\n"
			"Note that headers that you specify that do not appear\n"
			"in the message envelope will not be shown at all.\n"
			"This is to save space."));
	
	// Fixed font
	
	l_fixedFont_	=
		new QLabel(i18n("Message font"), w_view_, "l_messageFont");
	CHECK_PTR(l_fixedFont_);
	
	l_fixedFont_->setFixedHeight(h);
	
	l_sampleFixed_		=
		new QLabel(i18n("Sample"), w_view_, "l_sampleFixed");
	CHECK_PTR(l_sampleFixed_);
	
	pb_chooseFixedFont_		=
		new QPushButton(i18n("Choose..."), w_view_, "pb_chooseFixedFont");
	CHECK_PTR(pb_chooseFixedFont_);
	
	QObject::connect(pb_chooseFixedFont_, SIGNAL(clicked()),
			this, SLOT(s_chooseFixedFont()));
	
	KQuickHelp::add(pb_chooseFixedFont_, i18n(
			"Here you may set the font to use for displaying\n"
			"messages. It's best to use a fixed-width font\n"
			"as the rest of the world expects that. Doing\n"
			"this allows people to line up text properly."));
	
	// underline links
	
	cb_underlineLinks_	=
		new QCheckBox(i18n("&Underline Links"), w_view_, "cb_underlineLinks");
	CHECK_PTR(cb_underlineLinks_);
	
	cb_underlineLinks_->setFixedHeight(h);
	
	KQuickHelp::add(cb_underlineLinks_, i18n(
			"Choose whether to have links underlined.\n"
			"Links are email addresses, http:// type\n"
			"addresses, etc. If you're colour blind,\n"
			"this is a smart move."));
		
	// Colours
	
	// Markup colour one
	
	l_quoteColourOne_	=
		new QLabel(i18n("Quote colour one"), w_view_, "l_quoteColourOne");
	CHECK_PTR(l_quoteColourOne_);
	
	l_quoteColourOne_->setFixedHeight(h);
	

	kcb_quoteColourOne_	=
		new KColorButton(w_view_, "kcb_quoteColourOne");
	CHECK_PTR(kcb_quoteColourOne_);

	KQuickHelp::add(kcb_quoteColourOne_, i18n(
			"Choose the primary colour for quoted text.\n"
			"Text can be quoted to multiple depths.\n"
			"Text that's quoted to an odd number, e.g.\n"
			"where the line begins with '\\>	' or '\\> \\> \\> '\n"
			"will be shown in this colour."));	
	
	// Markup colour two
	
	l_quoteColourTwo_	=
		new QLabel(i18n("Quote colour two"), w_view_, "l_quoteColourTwo");
	CHECK_PTR(l_quoteColourTwo_);
	
	l_quoteColourTwo_->setFixedHeight(h);
		

	kcb_quoteColourTwo_	=
		new KColorButton(w_view_, "kcb_quoteColourTwo");
	CHECK_PTR(kcb_quoteColourTwo_);

	KQuickHelp::add(kcb_quoteColourTwo_, i18n(
			"Choose the secondary colour for quoted text.\n"
			"Text can be quoted to multiple depths.\n"
			"Text that's quoted to an even number, e.g.\n"
			"where the line begins with '&gt; &gt; ' or '&gt; &gt; &gt; &gt; '\n"
			"will be shown in this colour."));
	
	// Link
	
	l_linkColour_	=
		new QLabel(i18n("Link Colour"), w_view_, "l_linkColour");
	CHECK_PTR(l_linkColour_);
	
	l_linkColour_->setFixedHeight(h);

	kcb_linkColour_	=
		new KColorButton(w_view_, "kcb_linkColour");
	CHECK_PTR(kcb_linkColour_);
			
	KQuickHelp::add(kcb_linkColour_, i18n(
			"Choose the colour that links in messages\n"
			"are shown in. Links means URLs, including\n"
			"mailto: URLs."));
	
	// Visited link
	
	l_visitedLinkColour_	=
		new QLabel(i18n("Visited Link Colour"), w_view_, "l_visitedColour");
	CHECK_PTR(l_visitedLinkColour_);
	
	l_visitedLinkColour_->setFixedHeight(h);
			
	kcb_visitedLinkColour_	=
		new KColorButton(w_view_, "kcb_visitedColour");
	CHECK_PTR(kcb_visitedLinkColour_);
		
	KQuickHelp::add(kcb_visitedLinkColour_, i18n(
			"Choose the colour that visited links in messages\n"
			"are shown in. Links means URLs, including\n"
			"mailto: URLs."));
	
///////////////////////////////////////////////////////////////////
// Message list
// 
	cb_threadMessages_ =
		new QCheckBox(i18n("Thread messages"), w_list_, "cb_threadMessages");
	CHECK_PTR(cb_threadMessages_);
	
	KQuickHelp::add(cb_threadMessages_, i18n(
			"If you select this, messages will be 'threaded'\n"
			"this means that when one message is a reply to\n"
			"another, it will be placed in a tree, where it\n"
			"is a branch from the previous message."));
	
	cb_threadMessages_->setFixedHeight(h);
	

	l_sortColumn_ =
		new QLabel(i18n("Message sort column"), w_list_, "l_sortColumn");
	CHECK_PTR(l_sortColumn_);
	
	l_sortColumn_->setFixedHeight(h);
	
	cb_sortColumn_ =
		new QComboBox(w_list_, "cb_sortColumn");
	CHECK_PTR(cb_sortColumn_);
	
	cb_sortColumn_->setFixedHeight(h);
	
	cb_sortColumn_->insertItem(i18n("Subject"),	0);
	cb_sortColumn_->insertItem(i18n("Sender"),	1);
	cb_sortColumn_->insertItem(i18n("Date"),	2);
	cb_sortColumn_->insertItem(i18n("Size"),	3);

	KQuickHelp::add(cb_sortColumn_, i18n(
			"Here you can specify which column the message\n"
			"list will be sorted by, when you start the\n"
			"program."));
	
	cb_sortAscending_ =
		new QCheckBox(i18n("Sort ascending"), w_list_, "cb_sortAscending");
	CHECK_PTR(cb_sortAscending_);
	
	cb_sortAscending_->setFixedHeight(h);
	
	KQuickHelp::add(cb_sortAscending_, i18n(
			"If you select this, the column you specified\n"
			"above will be sorted ascending.\n"
			"Guess what happens if you don't."));
	
	cb_timer_ =
		new QCheckBox(i18n("Mark messages as read after"), w_list_, "cb_timer");
	CHECK_PTR(cb_timer_);
	
	cb_timer_->setFixedHeight(h);
	
	KQuickHelp::add(cb_timer_, i18n(
			"If you check this, messages will be marked\n"
			"as read after you've been looking at them for\n"
			"the time specified"));
	
	sb_timer_ =
		new QSpinBox(0, 60, 1, w_list_, "sb_timer");
	CHECK_PTR(sb_timer_);
	
	sb_timer_->setFixedHeight(h);
	sb_timer_->setSuffix(" " + i18n("seconds"));
	
	KQuickHelp::add(sb_timer_, i18n(
			"If you check this, messages will be marked\n"
			"as read after you've been looking at them for\n"
			"the time specified"));

/////////////////////////////////////////////////////////////////////////
	
	l_iconSet_ = new QLabel(i18n("Icon set"), this, "l_iconSet");
	CHECK_PTR(l_iconSet_);
	
	l_iconSet_->setFixedHeight(h);
	
	KQuickHelp::add(l_iconSet_, i18n(
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
	
	topLevelLayout_				= new QGridLayout(this, 4, 2, 10, 10);
	CHECK_PTR(topLevelLayout_);

	listGroupLayout_			= new QGridLayout(w_list_, 3, 2, 0, 10);
	CHECK_PTR(listGroupLayout_);
	
	viewGroupLayout_			= new QGridLayout(w_view_, 7, 3, 0, 10);
	CHECK_PTR(viewGroupLayout_);

	topLevelLayout_->setRowStretch(0, 3);
	topLevelLayout_->setRowStretch(1, 7);
	topLevelLayout_->setRowStretch(2, 0);
	topLevelLayout_->setRowStretch(3, 0);
	
	topLevelLayout_->addMultiCellWidget(rgb_list_,		0, 0, 0, 1);
	topLevelLayout_->addMultiCellWidget(rgb_view_,		1, 1, 0, 1);
	topLevelLayout_->addWidget(l_iconSet_,				2, 0);
	topLevelLayout_->addWidget(cb_iconSet_,				2, 1);
	topLevelLayout_->addMultiCellWidget(buttonBox_,		3, 3, 0, 1);
	
	listGroupLayout_->addWidget(l_sortColumn_,			0, 0);
	listGroupLayout_->addWidget(cb_sortColumn_,			0, 1);
	listGroupLayout_->addWidget(cb_sortAscending_,		1, 0);
	listGroupLayout_->addWidget(cb_threadMessages_,		1, 1);
	listGroupLayout_->addWidget(cb_timer_,				2, 0);
	listGroupLayout_->addWidget(sb_timer_,				2, 1);
	listGroupLayout_->activate();
	
	viewGroupLayout_->addWidget(l_displayHeaders_,				0, 0);
	viewGroupLayout_->addMultiCellWidget(le_displayHeaders_,	0, 0, 1, 2);
	viewGroupLayout_->addWidget(l_fixedFont_,					1, 0);
	viewGroupLayout_->addWidget(l_sampleFixed_,					1, 1);
	viewGroupLayout_->addWidget(pb_chooseFixedFont_,			1, 2);
	viewGroupLayout_->addMultiCellWidget(l_quoteColourOne_,		2, 2, 0, 1);
	viewGroupLayout_->addWidget(kcb_quoteColourOne_,			2, 2);
	viewGroupLayout_->addMultiCellWidget(l_quoteColourTwo_,		3, 3, 0, 1);
	viewGroupLayout_->addWidget(kcb_quoteColourTwo_,			3, 2);
	viewGroupLayout_->addMultiCellWidget(l_linkColour_,			4, 4, 0, 1);
	viewGroupLayout_->addWidget(kcb_linkColour_,				4, 2);
	viewGroupLayout_->addMultiCellWidget(l_visitedLinkColour_,	5, 5, 0, 1);
	viewGroupLayout_->addWidget(kcb_visitedLinkColour_,			5, 2);
	viewGroupLayout_->addMultiCellWidget(cb_underlineLinks_,	6, 6, 0, 2);
	viewGroupLayout_->activate();

	topLevelLayout_->activate();
	
	setMinimumSize(minimumSizeHint());
	resize(minimumSizeHint());
};

	void
EmpathDisplaySettingsDialog::s_chooseFixedFont()
{
	QFont fnt = l_sampleFixed_->font();
	KFontDialog d(this);
	d.setFont(fnt);
	if (d.getFont(fnt) == QDialog::Accepted)
		l_sampleFixed_->setFont(fnt);
}

	void
EmpathDisplaySettingsDialog::saveData()
{
	KConfig * c(KGlobal::config());
	c->setGroup(EmpathConfig::GROUP_DISPLAY);
#define CWE c->writeEntry
	CWE( EmpathConfig::KEY_FIXED_FONT,			l_sampleFixed_->font());
	CWE( EmpathConfig::KEY_UNDERLINE_LINKS,		cb_underlineLinks_->isChecked());
	CWE( EmpathConfig::KEY_QUOTE_COLOUR_ONE,	kcb_quoteColourOne_->color());
	CWE( EmpathConfig::KEY_QUOTE_COLOUR_TWO,	kcb_quoteColourTwo_->color());
	CWE( EmpathConfig::KEY_LINK_COLOUR,			kcb_linkColour_->color());
	CWE( EmpathConfig::KEY_VISITED_LINK_COLOUR,	kcb_visitedLinkColour_->color());
	CWE( EmpathConfig::KEY_ICON_SET,			cb_iconSet_->currentText());
	CWE( EmpathConfig::KEY_THREAD_MESSAGES,		cb_threadMessages_->isChecked());
	CWE( EmpathConfig::KEY_MESSAGE_SORT_ASCENDING,cb_sortAscending_->isChecked());
	CWE( EmpathConfig::KEY_SHOW_HEADERS,		le_displayHeaders_->text());
	CWE( EmpathConfig::KEY_MESSAGE_SORT_COLUMN,	cb_sortColumn_->currentItem());
	CWE( EmpathConfig::KEY_MARK_AS_READ,		cb_timer_->isChecked());
	CWE( EmpathConfig::KEY_MARK_AS_READ_TIME,	sb_timer_->value());
#undef CWE
}

	void
EmpathDisplaySettingsDialog::loadData()
{
	KConfig * c = KGlobal::config();
	c->setGroup(EmpathConfig::GROUP_DISPLAY);
	
	QFont font; QColor col;

	font = KGlobal::fixedFont();
	
	l_sampleFixed_->setFont(
		c->readFontEntry(EmpathConfig::KEY_FIXED_FONT, &font));
	
	cb_underlineLinks_->setChecked(
		c->readBoolEntry(EmpathConfig::KEY_UNDERLINE_LINKS, true));
	
	col = kapp->palette().color(QPalette::Normal, QColorGroup::Base);
	
	kcb_quoteColourOne_->setColor(
		c->readColorEntry(EmpathConfig::KEY_QUOTE_COLOUR_ONE, &col));
	
	col = kapp->palette().color(QPalette::Normal, QColorGroup::Text);
	
	kcb_quoteColourTwo_->setColor(
		c->readColorEntry(EmpathConfig::KEY_QUOTE_COLOUR_TWO, &col));
	
	col = Qt::darkBlue;

	kcb_linkColour_->setColor(
		c->readColorEntry(EmpathConfig::KEY_LINK_COLOUR, &col));

	col = Qt::darkCyan;
	
	kcb_visitedLinkColour_->setColor(
		c->readColorEntry(EmpathConfig::KEY_VISITED_LINK_COLOUR, &col));
	
	cb_threadMessages_->setChecked(
		c->readBoolEntry(EmpathConfig::KEY_THREAD_MESSAGES, true));
	
	cb_sortAscending_->setChecked(
		c->readBoolEntry(EmpathConfig::KEY_MESSAGE_SORT_ASCENDING, true));
	
	le_displayHeaders_->setText(
		c->readEntry(EmpathConfig::KEY_SHOW_HEADERS, i18n("From,Date,Subject")));
	cb_sortColumn_->setCurrentItem(
		c->readNumEntry(EmpathConfig::KEY_MESSAGE_SORT_COLUMN, 2));
	
	cb_timer_->setChecked(
		c->readBoolEntry(EmpathConfig::KEY_MARK_AS_READ, true));
	
	sb_timer_->setValue(
		c->readNumEntry(EmpathConfig::KEY_MARK_AS_READ_TIME, 2));
	
	// Fill in the icon set combo.

	cb_iconSet_->clear();
	
	QString s = c->readEntry(EmpathConfig::KEY_ICON_SET, "standard");
	empathDebug("Saved icon set was \"" + s + "\"");

	bool found = false;
	int index = 0;
	
	QStringList el(KGlobal::dirs()->findDirs("pics", ""));

	for (QStringList::ConstIterator it = el.begin(); it != el.end() ; ++it) {
		
		if ((*it).at(0) == ".")
			continue;
		
		if (*it == "mime")
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
	hide();
	if (!applied_)
		s_apply();
	KGlobal::config()->sync();
	delete this;
}

	void
EmpathDisplaySettingsDialog::s_help()
{
	//empathInvokeHelp(QString::null, QString::null);
}

	void
EmpathDisplaySettingsDialog::s_apply()
{
	if (applied_) {
		pb_apply_->setText(i18n("&Apply"));
		KGlobal::config()->rollback(true);
		KGlobal::config()->reparseConfiguration();
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
	l_sampleFixed_->setFont(KGlobal::fixedFont());
	cb_underlineLinks_->setChecked(true);
	kcb_quoteColourOne_->setColor(Qt::darkBlue);
	kcb_quoteColourTwo_->setColor(Qt::darkGreen);
	kcb_linkColour_->setColor(Qt::blue);
	kcb_visitedLinkColour_->setColor(Qt::darkCyan);
	cb_threadMessages_->setChecked(true);
	cb_sortAscending_->setChecked(true);
	le_displayHeaders_->setText(i18n("From,Date,Subject"));
	cb_sortColumn_->setCurrentItem(2);
	cb_timer_->setChecked(true);
	sb_timer_->setValue(2);
	saveData();
}
	
	void
EmpathDisplaySettingsDialog::s_cancel()
{
	if (!applied_)
		KGlobal::config()->rollback(true);
	delete this;
}

	void
EmpathDisplaySettingsDialog::closeEvent(QCloseEvent * e)
{
	e->accept();
	delete this;
}

