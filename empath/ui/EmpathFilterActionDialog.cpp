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

// KDE includes
#include <kapp.h>
#include <klocale.h>

// Local includes
#include "EmpathFolderChooserWidget.h"
#include "EmpathFilterEventHandler.h"
#include "EmpathFilter.h"
#include "EmpathAddressSelectionWidget.h"
#include "EmpathFilterActionDialog.h"
#include "EmpathConfig.h"
#include "Empath.h"
#include "RikGroupBox.h"
		
EmpathFilterActionDialog::EmpathFilterActionDialog(
		EmpathFilter * filter,
		QWidget * parent,
		const char * name)
	:	QDialog(parent, name, true),
		filter_(filter)
{
	empathDebug("ctor");
	
	setCaption(i18n("Filter Action - ") + kapp->getCaption());
	
	QLineEdit	tempLineEdit((QWidget *)0);
	Q_UINT32 h	= tempLineEdit.sizeHint().height();
	
	buttonBox_	= new KButtonBox(this);
	CHECK_PTR(buttonBox_);

	buttonBox_->setMaximumHeight(h);
	
	// Bottom button group
	pb_help_	= buttonBox_->addButton(i18n("&Help"));	
	buttonBox_->addStretch();
	pb_OK_		= buttonBox_->addButton(i18n("&OK"));
	pb_cancel_	= buttonBox_->addButton(i18n("&Cancel"));
	
	buttonBox_->layout();

	QObject::connect(pb_OK_, SIGNAL(clicked()),
			this, SLOT(s_OK()));
	
	QObject::connect(pb_cancel_, SIGNAL(clicked()),
			this, SLOT(s_cancel()));
	
	QObject::connect(pb_help_, SIGNAL(clicked()),
			this, SLOT(s_help()));

	rgb_choices_	= new RikGroupBox(i18n("Actions"), 8, this, "rgb_choices");
	CHECK_PTR(rgb_choices_);
	
	w_choices_	= new QWidget(rgb_choices_,	"w_choices");
	CHECK_PTR(w_choices_);
	
	rgb_choices_->setWidget(w_choices_);

	bg_choices_ =
		new QButtonGroup(this, "bg_choices");
	CHECK_PTR(bg_choices_);

	bg_choices_->hide();
	bg_choices_->setExclusive(true);

	rb_moveFolder_	= new QRadioButton(w_choices_, "rb_moveFolder");
	CHECK_PTR(rb_moveFolder_);
	rb_delete_		= new QRadioButton(w_choices_, "rb_delete");
	CHECK_PTR(rb_delete_);
	rb_ignore_		= new QRadioButton(w_choices_, "rb_ignore");
	CHECK_PTR(rb_ignore_);
	rb_forwardTo_	= new QRadioButton(w_choices_, "rb_forwardTo");
	CHECK_PTR(rb_forwardTo_);

	rb_moveFolder_->setFixedHeight(h);
	rb_delete_->setFixedHeight(h);
	rb_ignore_->setFixedHeight(h);
	rb_forwardTo_->setFixedHeight(h);

	bg_choices_->insert(rb_moveFolder_);
	bg_choices_->insert(rb_delete_);
	bg_choices_->insert(rb_ignore_);
	bg_choices_->insert(rb_forwardTo_);
	
	l_moveFolder_	= new QLabel(i18n("to folder:"), w_choices_, "l_moveFolder");
	CHECK_PTR(l_moveFolder_);
	l_moveFolder_->setFixedWidth(l_moveFolder_->sizeHint().width());
	l_delete_		= new QLabel(i18n("Delete"), w_choices_, "l_delete");
	CHECK_PTR(l_delete_);
	l_ignore_		= new QLabel(i18n("Ignore"), w_choices_, "l_ignore");
	CHECK_PTR(l_ignore_);
	l_forwardTo_	= new QLabel(i18n("Forward to:"), w_choices_, "l_forwardTo");
	CHECK_PTR(l_forwardTo_);
	l_forwardTo_->setFixedWidth(l_forwardTo_->sizeHint().width());
	
	cb_moveOrCopy_	= new QComboBox(w_choices_, "cb_moveOrCopy");
	CHECK_PTR(cb_moveOrCopy_);

	cb_moveOrCopy_->insertItem(i18n("Move"));
	cb_moveOrCopy_->insertItem(i18n("Copy"));
	
	cb_moveOrCopy_->setFixedWidth(cb_moveOrCopy_->sizeHint().width() + 10);
	
	fcw_moveFolder_	= new EmpathFolderChooserWidget(w_choices_, "folderChooser");
	CHECK_PTR(fcw_moveFolder_);

	asw_address_		= new EmpathAddressSelectionWidget(w_choices_, "asw");
	CHECK_PTR(asw_address_);
	
	cb_continue_		=
		new QCheckBox(i18n("Continue matching"), w_choices_, "cb_continue");
	CHECK_PTR(cb_continue_);
	cb_continue_->setFixedHeight(h);

	// Connect the radio buttons to the widgets they control so that the widgets
	// are disabled when the radio buttons are switched off.
	
	QObject::connect(rb_moveFolder_, SIGNAL(toggled(bool)),
			cb_moveOrCopy_, SLOT(setEnabled(bool)));
	
	QObject::connect(rb_moveFolder_, SIGNAL(toggled(bool)),
			fcw_moveFolder_, SLOT(setEnabled(bool)));

	QObject::connect(rb_moveFolder_, SIGNAL(toggled(bool)),
			l_moveFolder_, SLOT(setEnabled(bool)));
	
	QObject::connect(rb_delete_, SIGNAL(toggled(bool)),
			l_delete_, SLOT(setEnabled(bool)));
	
	QObject::connect(rb_ignore_, SIGNAL(toggled(bool)),
			l_ignore_, SLOT(setEnabled(bool)));
	
	QObject::connect(rb_forwardTo_, SIGNAL(toggled(bool)),
			l_forwardTo_, SLOT(setEnabled(bool)));
	
	QObject::connect(rb_forwardTo_, SIGNAL(toggled(bool)),
			asw_address_, SLOT(setEnabled(bool)));

	rb_moveFolder_->setChecked(true);
	rb_delete_->setChecked(false);
	rb_ignore_->setChecked(false);
	rb_forwardTo_->setChecked(false);
	
	// Layouts

	mainLayout_			= new QGridLayout(this, 2, 1, 10, 10);
	CHECK_PTR(mainLayout_);
	
	layout_				= new QGridLayout(w_choices_, 5, 2, 10, 10);
	CHECK_PTR(layout_);
	
	mc_subLayout_				= new QGridLayout(1, 3);
	CHECK_PTR(mc_subLayout_);
	
	ft_subLayout_				= new QGridLayout(1, 2);
	CHECK_PTR(ft_subLayout_);
	
	layout_->addLayout(mc_subLayout_,	0, 1);
	layout_->addLayout(ft_subLayout_,	3, 1);

	mc_subLayout_->addWidget(cb_moveOrCopy_,	0, 0);
	mc_subLayout_->addWidget(l_moveFolder_,		0, 1);
	mc_subLayout_->addWidget(fcw_moveFolder_,	0, 2);

	mc_subLayout_->setColStretch(0, 1);
	mc_subLayout_->setColStretch(1, 1);
	mc_subLayout_->setColStretch(2, 1);

	ft_subLayout_->addWidget(l_forwardTo_,	0, 0);
	ft_subLayout_->addWidget(asw_address_,	0, 1);

	ft_subLayout_->setColStretch(0, 1);
	ft_subLayout_->setColStretch(1, 1);

	layout_->setColStretch(0, 0);
	layout_->setColStretch(1, 10);
	
	layout_->addWidget(rb_moveFolder_,	0, 0);
	layout_->addWidget(rb_delete_,		1, 0);
	layout_->addWidget(rb_ignore_,		2, 0);
	layout_->addWidget(rb_forwardTo_,	3, 0);
	
	layout_->addWidget(l_delete_,		1, 1);
	layout_->addWidget(l_ignore_,		2, 1);
	
	layout_->addMultiCellWidget(cb_continue_, 4, 4, 0, 1);
	
	mainLayout_->addWidget(rgb_choices_,	0, 0);
	mainLayout_->addWidget(buttonBox_,		1, 0);
	
	mc_subLayout_->activate();
	ft_subLayout_->activate();
	layout_->activate();

	mainLayout_->activate();
	
	setMinimumSize(minimumSizeHint());
	resize(minimumSizeHint());
	
	load();
}

EmpathFilterActionDialog::~EmpathFilterActionDialog()
{
	empathDebug("dtor");
}

	void
EmpathFilterActionDialog::load()
{
	EmpathFilterEventHandler * handler = filter_->eventHandler();
	if (!handler) return;
	
	switch (handler->actionType()) {
		
		case MoveFolder:
			cb_moveOrCopy_->setCurrentItem(0);
			fcw_moveFolder_->setURL(handler->moveOrCopyFolder());
			rb_moveFolder_->setChecked(true);
			break;
		
		case CopyFolder:
			cb_moveOrCopy_->setCurrentItem(1);
			fcw_moveFolder_->setURL(handler->moveOrCopyFolder());
			rb_moveFolder_->setChecked(true);
			break;
			
		case Delete:
			rb_delete_->setChecked(true);
			break;
			
		case Ignore:
			rb_ignore_->setChecked(true);
			break;
			
		case Forward:
			rb_forwardTo_->setChecked(true);
			asw_address_->setAddress(handler->forwardAddress());
			break;
			
		default:
			break;
	}
}

	void
EmpathFilterActionDialog::s_OK()
{
	empathDebug("s_OK() called");
	
	EmpathFilterEventHandler * handler = new EmpathFilterEventHandler;
	
	if (rb_moveFolder_->isChecked())
		if (cb_moveOrCopy_->currentItem() == 0)
			handler->setMoveFolder(fcw_moveFolder_->selectedURL());
		else
			handler->setCopyFolder(fcw_moveFolder_->selectedURL());
	
	else if (rb_delete_->isChecked())
		handler->setDelete();
	
	else if (rb_ignore_->isChecked())
		handler->setIgnore();
	
	else if (rb_forwardTo_->isChecked())
		handler->setForward(asw_address_->selectedAddress());

	
	filter_->setEventHandler(handler);
	
	accept();
}

	void
EmpathFilterActionDialog::s_cancel()
{
	empathDebug("s_cancel() called");
	reject();
}
	
	void
EmpathFilterActionDialog::s_help()
{
	empathDebug("s_help() called");
}

