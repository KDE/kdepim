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
#include "EmpathMatcher.h"
#include "EmpathMatchPropertiesDialog.h"
#include "EmpathConfig.h"
#include "Empath.h"
#include "RikGroupBox.h"
		
EmpathMatchPropertiesDialog::EmpathMatchPropertiesDialog(
		QWidget * parent,
		EmpathMatcher * matcher)
	:	QDialog(parent, "matchPropertiesDialog", true),
		matcher_(matcher)
{
	empathDebug("ctor");
	setCaption(i18n("Match expression"));
	
	QLineEdit	tempLineEdit((QWidget *)0);
	Q_UINT32 h	= tempLineEdit.sizeHint().height();
	
	buttonBox_	= new KButtonBox(this);
	CHECK_PTR(buttonBox_);

	buttonBox_->setFixedHeight(h);
	
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

	rgb_choices_	= new RikGroupBox(i18n("Match"), 8, this, "rgb_choices");
	CHECK_PTR(rgb_choices_);
	
	w_choices_	= new QWidget(rgb_choices_,	"w_choices");
	CHECK_PTR(w_choices_);
	
	rgb_choices_->setWidget(w_choices_);

	bg_choices_ =
		new QButtonGroup(this, "bg_choices");
	CHECK_PTR(bg_choices_);

	bg_choices_->hide();
	bg_choices_->setExclusive(true);

	rb_size_	= new QRadioButton(i18n("Size larger than"),
			w_choices_, "rb_size");
	CHECK_PTR(rb_size_);
	
	rb_exprBody_	= new QRadioButton(i18n("Expression in body of message"),
			w_choices_, "rb_exprBody");
	CHECK_PTR(rb_exprBody_);
	
	rb_exprHeader_	= new QRadioButton(i18n("Expression in header"),
			w_choices_, "rb_exprHeader");
	CHECK_PTR(rb_exprHeader_);
	
	rb_attached_	= new QRadioButton(i18n("Has attachment(s)"),
			w_choices_, "rb_attached");
	CHECK_PTR(rb_attached_);
	
	rb_all_	= new QRadioButton(i18n("Any message"),
			w_choices_, "rb_all");
	CHECK_PTR(rb_all_);

	rb_size_->setFixedHeight(h);
	rb_exprBody_->setFixedHeight(h);
	rb_exprHeader_->setFixedHeight(h);
	rb_attached_->setFixedHeight(h);
	rb_all_->setFixedHeight(h);

	int minWidth = 0;
	int x;
	
	// Messy stuff - just works out the min width we need to spare for the
	// radio buttons + their text. Doing this allows for lining up what's on
	// the right hand size - looks less messy. Mess in code = clean i/f. Hmm.
	x = rb_size_->sizeHint().width();
	minWidth = (minWidth < x ? x : minWidth);
	x = rb_exprBody_->sizeHint().width();
	minWidth = (minWidth < x ? x : minWidth);
	x = rb_exprHeader_->sizeHint().width();
	minWidth = (minWidth < x ? x : minWidth);
	x = rb_attached_->sizeHint().width();
	minWidth = (minWidth < x ? x : minWidth);
	x = rb_all_->sizeHint().width();
	minWidth = (minWidth < x ? x : minWidth);

	rb_size_->setMinimumWidth(minWidth);
	rb_exprBody_->setMinimumWidth(minWidth);
	rb_exprHeader_->setMinimumWidth(minWidth);
	rb_attached_->setMinimumWidth(minWidth);
	rb_all_->setMinimumWidth(minWidth);

	idx_size_		= bg_choices_->insert(rb_size_);	
	idx_exprBody_	= bg_choices_->insert(rb_exprBody_);	
	idx_exprHeader_	= bg_choices_->insert(rb_exprHeader_);	
	idx_attached_	= bg_choices_->insert(rb_attached_);	
	idx_all_		= bg_choices_->insert(rb_all_);	

	sb_size_ = new KNumericSpinBox(w_choices_, "sb_size");
	CHECK_PTR(sb_size_);
	sb_size_->setRange(1, 1000);
	sb_size_->setEditable(true);

	sb_size_->setFixedWidth(sb_size_->sizeHint().height() * 3);

	l_kb_ = new QLabel("Kb", w_choices_, "l_kb");
	CHECK_PTR(l_kb_);

	l_kb_->setFixedWidth(l_kb_->sizeHint().width());
	
	le_exprBody_ = new QLineEdit(w_choices_, "le_exprBody_");
	CHECK_PTR(le_exprBody_);

	cb_header_	= new QComboBox(w_choices_, "cb_header");
	CHECK_PTR(cb_header_);

	cb_header_->setMinimumWidth(cb_header_->sizeHint().width());
	
	le_exprHeader_ = new QLineEdit(w_choices_, "le_exprHeader_");
	CHECK_PTR(le_exprHeader_);
	
	QObject::connect(rb_size_, SIGNAL(toggled(bool)),
			sb_size_, SLOT(setEnabled(bool)));
	
	QObject::connect(rb_size_, SIGNAL(toggled(bool)),
			l_kb_, SLOT(setEnabled(bool)));
	
	QObject::connect(rb_exprBody_, SIGNAL(toggled(bool)),
			le_exprBody_, SLOT(setEnabled(bool)));
	
	QObject::connect(rb_exprHeader_, SIGNAL(toggled(bool)),
			cb_header_, SLOT(setEnabled(bool)));
	
	QObject::connect(rb_exprHeader_, SIGNAL(toggled(bool)),
			le_exprHeader_, SLOT(setEnabled(bool)));
	
	rb_size_->setChecked(true);
	rb_exprBody_->setChecked(false);
	rb_exprHeader_->setChecked(false);
	rb_attached_->setChecked(false);
	rb_all_->setChecked(false);

	le_exprBody_->setEnabled(false);
	cb_header_->setEnabled(false);
	le_exprHeader_->setEnabled(false);
	
	rgb_choices_->setMinimumWidth(
			minWidth + cb_header_->width() + le_exprHeader_->width() + 20);
	rgb_choices_->setMinimumHeight(
			h * 9);

	
	// Layouts

	mainLayout_			= new QGridLayout(this, 2, 1, 10, 10);
	CHECK_PTR(mainLayout_);
	
	layout_				= new QGridLayout(w_choices_, 5, 1, 10, 10);
	CHECK_PTR(layout_);

	size_subLayout_				= new QGridLayout(1, 3);
	exprBody_subLayout_			= new QGridLayout(1, 2);
	exprHeader_subLayout_		= new QGridLayout(1, 3);
	
	layout_->addLayout(size_subLayout_,			0, 0);
	layout_->addLayout(exprBody_subLayout_,		1, 0);
	layout_->addLayout(exprHeader_subLayout_,	2, 0);
	layout_->addWidget(rb_attached_,			3, 0);
	layout_->addWidget(rb_all_,					4, 0);

	size_subLayout_->addWidget(rb_size_,	0, 0);
	size_subLayout_->addWidget(sb_size_,	0, 1);
	size_subLayout_->addWidget(l_kb_,		0, 2);
	size_subLayout_->setColStretch(0, 20);
	size_subLayout_->setColStretch(1, 1);
	size_subLayout_->setColStretch(2, 1);
	
	exprBody_subLayout_->addWidget(rb_exprBody_,	0, 0);
	exprBody_subLayout_->addWidget(le_exprBody_,	0, 1);
	exprBody_subLayout_->setColStretch(0, 0);
	exprBody_subLayout_->setColStretch(1, 1);
	
	exprHeader_subLayout_->addWidget(rb_exprHeader_,	0, 0);
	exprHeader_subLayout_->addWidget(cb_header_,		0, 1);
	exprHeader_subLayout_->addWidget(le_exprHeader_,	0, 2);
	exprHeader_subLayout_->setColStretch(0, 0);
	exprHeader_subLayout_->setColStretch(1, 1);
	exprHeader_subLayout_->setColStretch(2, 9);

	mainLayout_->addWidget(rgb_choices_,	0, 0);
	mainLayout_->addWidget(buttonBox_,		1, 0);
	
	size_subLayout_->activate();
	exprBody_subLayout_->activate();
	exprHeader_subLayout_->activate();
	layout_->activate();

	mainLayout_->activate();
	
	resize(	minWidth + cb_header_->width() + le_exprHeader_->width() + 20,
			h * 9);
	
	// Setup according to type of matcher.

	if (matcher_ != 0) {
		
		empathDebug("Matcher exists - Lets choose the correct radio button");
	
		switch (matcher_->type()) {
			
			case AnyMessage:
				empathDebug("AnyMessage");
				rb_all_->setChecked(true);
				break;

			case HasAttachments:
				empathDebug("HasAttachments");
				rb_attached_->setChecked(true);
				break;

			case Size:
				empathDebug("Size");
				rb_size_->setChecked(true);
				empathDebug("Size == " + QString().setNum(matcher_->size()));
				sb_size_->setValue(matcher_->size());
				break;
				
			case BodyExpr:
				empathDebug("BodyExpr");
				rb_exprBody_->setChecked(true);
				le_exprBody_->setText(matcher_->matchExpr());
				break;

			case HeaderExpr:
				empathDebug("HeaderExpr");
				rb_exprHeader_->setChecked(true);
				cb_header_->insertItem(matcher_->matchHeader(), 0);
				cb_header_->setCurrentItem(0);
				le_exprBody_->setText(matcher_->matchExpr());
				break;
				
			default:
				empathDebug("WTF ? Couldn't determine matcher's type");
				break;
		}
	}
}

EmpathMatchPropertiesDialog::~EmpathMatchPropertiesDialog()
{
	empathDebug("dtor");
}

	void
EmpathMatchPropertiesDialog::s_OK()
{
	empathDebug("s_OK() called");
	
	if (rb_all_->isChecked())
		matcher_->setType(AnyMessage);
	else if (rb_attached_->isChecked())
		matcher_->setType(HasAttachments);
	else if (rb_size_->isChecked()) {
		matcher_->setType(Size);
		matcher_->setSize(sb_size_->getValue());
	}
	else if (rb_exprBody_->isChecked()) {
		matcher_->setType(BodyExpr);
		matcher_->setMatchExpr(le_exprBody_->text());
		matcher_->setMatchHeader(cb_header_->currentText());
	}
	else if (rb_exprHeader_->isChecked()) {
		matcher_->setType(HeaderExpr);
		matcher_->setMatchExpr(le_exprHeader_->text());
	}
	
	accept();
}

	EmpathMatcher *
EmpathMatchPropertiesDialog::matcher()
{
	return matcher_;
}

	void
EmpathMatchPropertiesDialog::s_cancel()
{
	empathDebug("s_cancel() called");
	reject();
}
	
	void
EmpathMatchPropertiesDialog::s_help()
{
	empathDebug("s_help() called");
}

