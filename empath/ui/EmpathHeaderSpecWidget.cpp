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

#include "EmpathHeaderSpecWidget.h"
#include "EmpathHeaderNameWidget.h"
#include "EmpathHeaderBodyWidget.h"

EmpathHeaderSpecWidget::EmpathHeaderSpecWidget(
		int headerIndex,
		QWidget * parent,
		const char * name)
	:	QWidget(parent, name)
{
	empathDebug("ctor");
	
	QGridLayout * layout_	
		= new QGridLayout(this, 1, 3, 0, 4);
	CHECK_PTR(layout_);
	
	headerNameWidget_	= new EmpathHeaderNameWidget(
		headerIndex, this, "headerNameWidget");
	CHECK_PTR(headerNameWidget_);

	QObject::connect(headerNameWidget_, SIGNAL(activated(const char *)),
			this, SLOT(s_headerNameActivated(const char *)));
	
	headerBodyWidget_	= new EmpathHeaderBodyWidget(this, "headerNameWidget");
	CHECK_PTR(headerBodyWidget_);
	
	pb_selectRecipients_ = new QPushButton("...", this, "pb_selectRecipients");
	CHECK_PTR(pb_selectRecipients_);
	
	int h = headerBodyWidget_->sizeHint().height();
	headerNameWidget_->setFixedHeight(h);
	headerBodyWidget_->setFixedHeight(h);
	pb_selectRecipients_->setFixedHeight(h);
	pb_selectRecipients_->setFixedWidth(pb_selectRecipients_->sizeHint().width());
	
	QObject::connect(headerBodyWidget_, SIGNAL(textChanged()),
			this, SLOT(s_headerBodyChanged()));

	QObject::connect(headerBodyWidget_, SIGNAL(returnPressed()),
			this, SLOT(s_headerBodyAccepted()));
	
	QObject::connect(pb_selectRecipients_, SIGNAL(clicked()),
			this, SLOT(s_selectRecipients()));
	
	layout_->setColStretch(0, 3);
	layout_->setColStretch(1, 7);
	layout_->setColStretch(2, 1);

	layout_->addWidget(headerNameWidget_, 0, 0);
	layout_->addWidget(headerBodyWidget_, 0, 1);
	layout_->addWidget(pb_selectRecipients_, 0, 2);

	layout_->activate();
	
	headerBodyWidget_->setFocus();
}

EmpathHeaderSpecWidget::~EmpathHeaderSpecWidget()
{
	empathDebug("dtor");
}

	void
EmpathHeaderSpecWidget::setHeaderList(const QStrList & headerList)
{
	headerNameWidget_->clear();
	headerNameWidget_->insertStrList(&headerList);
}

	void
EmpathHeaderSpecWidget::setHeaderName(const QString & headerName)
{
	headerNameWidget_->setHeaderName(headerName);
}

	QString
EmpathHeaderSpecWidget::getHeader() const
{
	QString s;

	s += headerNameWidget_->headerName();
	s += " ";
	s += headerBodyWidget_->getHeaderBody();

	return s;
}

	void
EmpathHeaderSpecWidget::setHeaderBody(const QString & headerBody)
{
	empathDebug("setHeaderBody(" + headerBody + ") called");
	headerBodyWidget_->setHeaderBody(headerBody);
}

	void
EmpathHeaderSpecWidget::s_headerNameActivated(const char * text)
{
	empathDebug("s_headerNameActivated() called");
	emit(nameActivated(text));
}

	void
EmpathHeaderSpecWidget::s_headerNameAccepted()
{
	empathDebug("s_headerNameAccepted() called");
	empathDebug("header name text: \"" + headerNameWidget_->headerName() + "\""); 
}

	void
EmpathHeaderSpecWidget::s_headerBodyChanged()
{
	empathDebug("s_headerBodyChanged() called");
	empathDebug("header body text: \"" + headerBodyWidget_->getHeaderBody() + "\""); 
	emit(_empath_textChanged(0));
}

	void
EmpathHeaderSpecWidget::s_headerBodyAccepted()
{
	empathDebug("s_headerBodyAccepted() called");
	empathDebug("header body text: \"" + headerBodyWidget_->getHeaderBody() + "\""); 
	emit(_empath_returnPressed(0));
}


	void
EmpathHeaderSpecWidget::s_selectRecipients()
{
}

