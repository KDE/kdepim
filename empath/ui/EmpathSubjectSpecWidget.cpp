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
# pragma implementation "EmpathSubjectSpecWidget.h"
#endif

// KDE includes
#include <klocale.h>

// Local includes
#include "EmpathSubjectSpecWidget.h"

EmpathSubjectSpecWidget::EmpathSubjectSpecWidget(
		QWidget * parent,
		const char * name)
	:	QWidget(parent, name)
{
	empathDebug("ctor");
	QGridLayout * layout =
		new QGridLayout(this, 1, 2, 2, 0);
	CHECK_PTR(layout);

	l_subject_ = new QLabel(this, "l_subject");
	CHECK_PTR(l_subject_);

	l_subject_->setText(i18n("Subject:"));
	
	le_subject_ = new QLineEdit(this, "le_subject");
	CHECK_PTR(le_subject_);
	
	int h = le_subject_->sizeHint().height();

	le_subject_->setFixedHeight(h);
	l_subject_->setFixedHeight(h);
	l_subject_->setFixedWidth(l_subject_->sizeHint().width() + 8);
	
	layout->setColStretch(0, 0);
	layout->setColStretch(1, 7);

	layout->addWidget(l_subject_, 0, 0);
	layout->addWidget(le_subject_, 0, 1);

	layout->activate();
}

EmpathSubjectSpecWidget::~EmpathSubjectSpecWidget()
{
	empathDebug("dtor");
}


	QString
EmpathSubjectSpecWidget::getSubject() const
{
	return le_subject_->text();
}

	void
EmpathSubjectSpecWidget::setSubject(const QString & subject)
{
	le_subject_->setText(subject);
}

//XXX Note: Why doesn't setFocusProxy(le_subject_) work ?

	void
EmpathSubjectSpecWidget::setFocus()
{
	le_subject_->setFocus();
}

