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
#include <kiconloader.h>
#include <kmsgbox.h>
#include <klocale.h>

// Local includes
#include "EmpathDefines.h"
#include "EmpathUIUtils.h"
#include "EmpathAddressSelectionWidget.h"

EmpathAddressSelectionWidget::EmpathAddressSelectionWidget(
		QWidget * parent,
		const char * name)
	:
		QWidget(parent, name)
{
	empathDebug("ctor");
	
	layout_		= new QGridLayout(this, 1, 2, 0, 10);
	CHECK_PTR(layout_);
	le_address_	= new QLineEdit(this, "le_address");
	CHECK_PTR(le_address_);
	pb_browse_	= new QPushButton(this, "pb_browse");
	CHECK_PTR(pb_browse_);

	pb_browse_->setPixmap(empathIcon("point.png")), 
   	
	pb_browse_->setFixedWidth(le_address_->sizeHint().height());

	layout_->setColStretch(0, 10);
	layout_->setColStretch(1, 0);
	
	layout_->addWidget(le_address_,	0, 0);
	layout_->addWidget(pb_browse_,	0, 1);

	QObject::connect(le_address_, SIGNAL(textChanged()),
			this, SLOT(s_textChanged()));
	
	// FIXME
	QObject::connect(le_address_, SIGNAL(returnPressed()),
			this, SLOT(s_lostFocus()));

	QObject::connect(pb_browse_, SIGNAL(clicked()),
			this, SLOT(s_browseClicked()));

	layout_->activate();
}

EmpathAddressSelectionWidget::~EmpathAddressSelectionWidget()
{
	empathDebug("dtor");
}

	QString
EmpathAddressSelectionWidget::selectedAddress() const
{
	return le_address_->text();
}

	void
EmpathAddressSelectionWidget::setAddress(const QString & address)
{
	le_address_->setText(address);
}

	void
EmpathAddressSelectionWidget::s_textChanged()
{
	empathDebug("s_textChanged() called");
}

	void
EmpathAddressSelectionWidget::s_lostFocus()
{
	empathDebug("s_lostFocus() called");
}

	void
EmpathAddressSelectionWidget::s_browseClicked()
{
	empathDebug("s_browseClicked() called");
	KMsgBox::message(0, "Empath", i18n("Sorry, the addressbook isn't ready for use yet."), KMsgBox::EXCLAMATION, i18n("OK"));
}

