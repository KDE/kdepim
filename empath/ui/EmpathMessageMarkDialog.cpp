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
# pragma implementation "EmpathMessageMarkDialog.h"
#endif

// KDE includes
#include <klocale.h>
#include <kapp.h>
#include <kquickhelp.h>

// Local includes
#include "EmpathMessageMarkDialog.h"
#include "EmpathUtilities.h"
#include "EmpathUIUtils.h"
		
EmpathMessageMarkDialog::EmpathMessageMarkDialog(
		QWidget * parent,
		const char * name)
	:	QDialog(parent, name, true)
{
	empathDebug("ctor");
	setCaption(i18n("Mark Messages - ") + kapp->getCaption());

	typeButtonGroup_	= new QButtonGroup(this, "typeButtonGroup");
	CHECK_PTR(typeButtonGroup_);
	
	typeButtonGroup_->hide();
	typeButtonGroup_->setExclusive(true);

	stateButtonGroup_	= new QButtonGroup(this, "stateButtonGroup");
	CHECK_PTR(stateButtonGroup_);
	
	stateButtonGroup_->hide();
	stateButtonGroup_->setExclusive(true);

	rgb_type_	= new RikGroupBox(i18n("Mark type"), 8, this, "rgb_type");
	CHECK_PTR(rgb_type_);
	
	w_type_			= new QWidget(rgb_type_,	"w_type");
	CHECK_PTR(w_type_);
	
	rgb_type_->setWidget(w_type_);
	
	rgb_state_	= new RikGroupBox(i18n("State"), 8, this, "rgb_state");
	CHECK_PTR(rgb_state_);
	
	w_state_			= new QWidget(rgb_state_,	"w_state");
	CHECK_PTR(w_state_);
	
	rgb_state_->setWidget(w_state_);
	
	// Mark type group box

	rb_tagged_	=
		new QRadioButton(i18n("Tagged"), w_type_, "rb_tagged");
	CHECK_PTR(rb_tagged_);
	
	KQuickHelp::add(rb_tagged_, i18n(
			"Tagging messages is for your own\n"
			"benefit. You can use this to simply\n"
			"remember which messages you are interested\n"
			"in; you can select 'tagged messages' for\n"
			"some operations."));
	
	int h = rb_tagged_->sizeHint().height();
	
	rb_replied_	=
		new QRadioButton(i18n("Replied"), w_type_, "rb_replied");
	CHECK_PTR(rb_replied_);
	
	KQuickHelp::add(rb_replied_, i18n(
			"You can mark a message as replied to\n"
			"if you wish. Why you'd want to is your\n"
			"business."));
	
	rb_read_	=
		new QRadioButton(i18n("Read"), w_type_, "rb_read");
	CHECK_PTR(rb_read_);
	
	KQuickHelp::add(rb_read_, i18n(
			"Marking messages as read is useful when\n"
			"you know you're not interested in some messages\n"
			"and don't want to read them. They'll henceforth\n"
			"be treated just the same as those you really have\n"
			"read."));

	rb_tagged_->setFixedHeight(h);
	rb_replied_->setFixedHeight(h);
	rb_read_->setFixedHeight(h);
	
	rb_tagged_->setChecked(true);
		
	// State group box

	rb_on_	=
		new QRadioButton(i18n("On"), w_state_, "rb_on");
	CHECK_PTR(rb_on_);
	
	KQuickHelp::add(rb_on_, i18n(
			"If you select this then every selected message\n"
			"will have its state switched on. For example,\n"
			"if you select 'Tag' and 'On' then untagged messages\n"
			"will become tagged."));
	
	rb_off_	=
		new QRadioButton(i18n("Off"), w_state_, "rb_off");
	CHECK_PTR(rb_off_);
	
	KQuickHelp::add(rb_off_, i18n(
			"If you select this then every selected message\n"
			"will have its state switched off. For example,\n"
			"if you select 'Tag' and 'Off' then tagged messages\n"
			"will become untagged."));
	
	rb_toggle_	=
		new QRadioButton(i18n("Toggle"), w_state_, "rb_toggle");
	CHECK_PTR(rb_toggle_);
	
	KQuickHelp::add(rb_toggle_, i18n(
			"If you select this then every selected message\n"
			"will have its state toggled. For example, if you\n"
			"select 'Tag' and 'Toggle' then tagged messages\n"
			"will be untagged, and vice versa."));
	
	rb_on_->setChecked(true);
	
	rb_on_->setFixedHeight(h);
	rb_off_->setFixedHeight(h);
	rb_toggle_->setFixedHeight(h);
	
	l_tagged_	= new QLabel(w_type_);
	CHECK_PTR(l_tagged_);
	
	l_tagged_->setPixmap(empathIcon("tree-marked.png"));
	
	l_replied_	= new QLabel(w_type_);
	
	CHECK_PTR(l_replied_);
	
	l_replied_->setPixmap(empathIcon("tree-replied.png"));
	
	l_read_		= new QLabel(w_type_);
	CHECK_PTR(l_read_);

	l_read_->setPixmap(empathIcon("tree-read.png"));
	
	stateButtonGroup_->insert(rb_tagged_,	RMM::Marked);
	stateButtonGroup_->insert(rb_replied_,	RMM::Replied);
	stateButtonGroup_->insert(rb_read_,		RMM::Read);

	typeButtonGroup_->insert(rb_on_,		On);
	typeButtonGroup_->insert(rb_off_,		Off);
	typeButtonGroup_->insert(rb_toggle_,	Toggle);

	// Button box
	
	buttonBox_ = new KButtonBox(this);
	CHECK_PTR(buttonBox_);
	
	pb_Help_	= buttonBox_->addButton(i18n("&Help"));
	buttonBox_->addStretch();
	pb_OK_		= buttonBox_->addButton(i18n("&OK"));
	pb_Cancel_	= buttonBox_->addButton(i18n("&Cancel"));
	buttonBox_->layout();
	
	buttonBox_->setFixedHeight(buttonBox_->height());

	QObject::connect(pb_Help_, SIGNAL(clicked()),
			this, SLOT(s_Help()));
	
	QObject::connect(pb_OK_, SIGNAL(clicked()),
			this, SLOT(s_OK()));
	
	QObject::connect(pb_Cancel_, SIGNAL(clicked()),
			this, SLOT(s_Cancel()));
	
	// Layouts
	
	topLevelLayout_		= new QGridLayout(this,		2, 2, 10, 10);
	CHECK_PTR(topLevelLayout_);
	
	typeGroupLayout_	= new QGridLayout(w_type_,	3, 2, 0, 10);
	CHECK_PTR(typeGroupLayout_);
	
	stateGroupLayout_	= new QGridLayout(w_state_,	3, 1, 0, 10);
	CHECK_PTR(typeGroupLayout_);
	
	topLevelLayout_->setRowStretch(0, 3);
	topLevelLayout_->setRowStretch(1, 1);

	topLevelLayout_->addWidget(rgb_type_,	0, 0);
	topLevelLayout_->addWidget(rgb_state_,	0, 1);
	topLevelLayout_->addMultiCellWidget(buttonBox_,	1, 1, 0, 1);
	
	typeGroupLayout_->addWidget(l_tagged_,	0, 0);
	typeGroupLayout_->addWidget(l_replied_,	1, 0);
	typeGroupLayout_->addWidget(l_read_,	2, 0);

	typeGroupLayout_->addWidget(rb_tagged_,	0, 1);
	typeGroupLayout_->addWidget(rb_replied_,1, 1);
	typeGroupLayout_->addWidget(rb_read_,	2, 1);
	
	stateGroupLayout_->addWidget(rb_on_,	0, 0);
	stateGroupLayout_->addWidget(rb_off_,	1, 0);
	stateGroupLayout_->addWidget(rb_toggle_,2, 0);

	typeGroupLayout_->activate();
	stateGroupLayout_->activate();

	topLevelLayout_->activate();
}

	void
EmpathMessageMarkDialog::s_OK()
{
	accept();
}

	void
EmpathMessageMarkDialog::s_Cancel()
{
	reject();
}

	void
EmpathMessageMarkDialog::s_Help()
{
	//empathInvokeHelp("","");
}

	EmpathMessageMarkDialog::MarkType
EmpathMessageMarkDialog::markType()
{
	return (
		(EmpathMessageMarkDialog::MarkType)
		(typeButtonGroup_->id(typeButtonGroup_->selected())));
}

	RMM::MessageStatus
EmpathMessageMarkDialog::status()
{
	return (
		(RMM::MessageStatus)
		(stateButtonGroup_->id(stateButtonGroup_->selected())));
}

