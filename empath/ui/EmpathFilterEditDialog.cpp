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
#include <klocale.h>

// Local includes
#include "RikGroupBox.h"
#include "EmpathDefines.h"
#include "EmpathMatcher.h"
#include "EmpathMatchPropertiesDialog.h"
#include "EmpathFolderChooserWidget.h"
#include "EmpathFilterActionDialog.h"
#include "EmpathFilterEditDialog.h"
#include "EmpathFilter.h"

EmpathFilterEditDialog::EmpathFilterEditDialog(
		EmpathFilter * filter,
		QWidget * parent,
		const char * name)
	:	QDialog(parent, name, true),
		filter_(filter)
{
	empathDebug("ctor");
	ASSERT(filter_ != 0);
	
	QPushButton	tempButton((QWidget *)0);
	Q_UINT32 h	= tempButton.sizeHint().height();

	rgb_arrives_ = new RikGroupBox(
			i18n("Match expressions"), 8, this, "rgb_arrives");
	CHECK_PTR(rgb_arrives_);
	
	rgb_matches_ = new RikGroupBox(
			i18n("Match expressions"), 8, this, "rgb_matches");
	CHECK_PTR(rgb_matches_);
	
	rgb_action_ = new RikGroupBox(
			i18n("Action to take"), 8, this, "rgb_action");
	CHECK_PTR(rgb_action_);

	w_arrives_ = new QWidget(rgb_arrives_, "w_arrives");
	CHECK_PTR(w_arrives_);
	
	w_matches_ = new QWidget(rgb_matches_, "w_matches");
	CHECK_PTR(w_matches_);

	w_action_ = new QWidget(rgb_action_, "w_action");
	CHECK_PTR(w_action_);

	rgb_arrives_->setWidget(w_arrives_);
	rgb_matches_->setWidget(w_matches_);
	rgb_action_->setWidget(w_action_);

	l_arrivesFolder_ =
		new QLabel(i18n("When new mail arrives in folder"),
				w_arrives_, "l_arrivesFolder");
	CHECK_PTR(l_arrivesFolder_);

	l_arrivesFolder_->setFixedHeight(h);
	
	fcw_arrivesFolder_ =
		new EmpathFolderChooserWidget(w_arrives_, "fcw_arrivesFolder");
	CHECK_PTR(fcw_arrivesFolder_);
	
	rgb_arrives_->setMinimumHeight(h * 5);
	rgb_arrives_->setMaximumHeight(h * 5);

	lb_matches_ = new QListBox(w_matches_, "lb_matches");
	CHECK_PTR(lb_matches_);

	exprButtonBox_ = new KButtonBox(w_matches_, KButtonBox::VERTICAL);
	CHECK_PTR(exprButtonBox_);

	pb_addMatch_ = exprButtonBox_->addButton(i18n("Add expression"));
	CHECK_PTR(pb_addMatch_);

	pb_editMatch_ = exprButtonBox_->addButton(i18n("Edit expression"));
	CHECK_PTR(pb_editMatch_);
	
	pb_removeMatch_ = exprButtonBox_->addButton(i18n("Remove expression"));
	CHECK_PTR(pb_removeMatch_);

	QObject::connect(pb_addMatch_, SIGNAL(clicked()),
			this, SLOT(s_addExpr()));
	
	QObject::connect(pb_editMatch_, SIGNAL(clicked()),
			this, SLOT(s_editExpr()));
	
	QObject::connect(pb_removeMatch_, SIGNAL(clicked()),
			this, SLOT(s_removeExpr()));
	
	exprButtonBox_->layout();
	exprButtonBox_->setFixedWidth(exprButtonBox_->sizeHint().width());
	exprButtonBox_->setMinimumHeight(exprButtonBox_->sizeHint().height());

	l_action_ = new QLabel(w_action_, "l_action");
	CHECK_PTR(l_action_);

	pb_editAction_ = new QPushButton(
			i18n("Edit action"), w_action_, "pb_editAction");
	CHECK_PTR(pb_editAction_);
	
	QObject::connect(pb_editAction_, SIGNAL(clicked()),
			this, SLOT(s_editAction()));
	
	pb_editAction_->setFixedHeight(h);
	pb_editAction_->setFixedWidth(pb_editAction_->sizeHint().width());

	rgb_action_->setMinimumHeight(h * 4);  
	rgb_action_->setMinimumWidth(exprButtonBox_->sizeHint().width() + l_action_->sizeHint().width());
	rgb_matches_->setMinimumHeight(h * 6);  
	rgb_matches_->setMinimumWidth(exprButtonBox_->sizeHint().width() + l_action_->sizeHint().width());
	

	buttonBox_ = new KButtonBox(this);
	CHECK_PTR(buttonBox_);

	buttonBox_->setFixedHeight(h);
	
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

	mainLayout_ = new QGridLayout(this, 4, 1, 10, 10);
	CHECK_PTR(mainLayout_);

	arrivesLayout_ = new QGridLayout(w_arrives_, 2, 1, 10, 10);
	CHECK_PTR(arrivesLayout_);
	
	matchesLayout_ = new QGridLayout(w_matches_, 1, 2, 10, 10);
	CHECK_PTR(matchesLayout_);
	
	actionLayout_ = new QGridLayout(w_action_, 1, 2, 10, 10);
	CHECK_PTR(actionLayout_);

	arrivesLayout_->addWidget(l_arrivesFolder_, 0, 0);
	arrivesLayout_->addWidget(fcw_arrivesFolder_, 1, 0);
	
	matchesLayout_->addWidget(lb_matches_, 0, 0);
	matchesLayout_->addWidget(exprButtonBox_, 0, 1);	
	
	actionLayout_->addWidget(l_action_, 0, 0);
	actionLayout_->addWidget(pb_editAction_, 0, 1);
	
	mainLayout_->addWidget(rgb_arrives_,	0, 0);
	mainLayout_->addWidget(rgb_matches_,	1, 0);
	mainLayout_->addWidget(rgb_action_,		2, 0);
	mainLayout_->addWidget(buttonBox_,		3, 0);

	arrivesLayout_->activate();
	matchesLayout_->activate();
	actionLayout_->activate();
	mainLayout_->activate();
	
	update();
}

EmpathFilterEditDialog::~EmpathFilterEditDialog()
{
	empathDebug("dtor");
}
		
	void
EmpathFilterEditDialog::s_OK()
{
	empathDebug("s_OK() called");
	filter_->setURL(fcw_arrivesFolder_->selectedURL());
	filter_->save();
	done(OK);
}

	void
EmpathFilterEditDialog::s_cancel()
{
	empathDebug("s_cancel() called");
	done(Cancel);
}

	void
EmpathFilterEditDialog::s_help()
{
	empathDebug("s_help() called");
}

	void
EmpathFilterEditDialog::s_addExpr()
{
	empathDebug("s_addExpr() called");
	EmpathMatcher * m = new EmpathMatcher;
	EmpathMatchPropertiesDialog mpd(this, m);
	
	if (mpd.exec() != OK) return;

	empathDebug("Adding matcher to filter's list");
	filter_->matchExprList()->append(m);
	empathDebug("Adding matcher's description to my listbox");
	lb_matches_->insertItem(m->description());
}

	void
EmpathFilterEditDialog::s_editExpr()
{
	empathDebug("s_editExpr() called");
	
	int i = lb_matches_->currentItem();
	if (i == -1) return; // No item selected.
	
	empathDebug("filter has " +
		QString().setNum(filter_->matchExprList()->count()) +
		" match expressions");

	empathDebug("Current item in list box is " + QString().setNum(i));

	EmpathMatchPropertiesDialog mpd(this, filter_->matchExprList()->at(i));
	mpd.exec();
	
	update();
}

	void
EmpathFilterEditDialog::s_removeExpr()
{
	empathDebug("s_removeExpr() called");

	int i = lb_matches_->currentItem();
	if (i == -1) return; // No item selected.

	filter_->matchExprList()->remove(i);
	update();
}

	void
EmpathFilterEditDialog::s_editAction()
{
	empathDebug("s_editAction() called");
	EmpathFilterActionDialog fDlg(filter_, this, "filterActionDialog");
	if (fDlg.exec() == Cancel) return;
	ASSERT(filter_->actionDescription());
	l_action_->setText(filter_->actionDescription());
}

	void
EmpathFilterEditDialog::update()
{
	ASSERT(filter_);
	
	empathDebug("Setting folder chooser folder");
	
	fcw_arrivesFolder_->setURL(filter_->url());

	empathDebug("Filling in match expressions");

	lb_matches_->clear();

	EmpathMatcherListIterator it(*(filter_->matchExprList()));

	for (; it.current(); ++it) {
		lb_matches_->insertItem(it.current()->description());
	}
	
	empathDebug("Setting action description");
	
	l_action_->setText(filter_->actionDescription());
}

