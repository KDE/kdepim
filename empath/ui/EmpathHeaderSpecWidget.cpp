/*
    Empath - Mailer for KDE
    
    Copyright (C) 1998, 1999 Rik Hemsley rik@kde.org
    
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
# pragma implementation "EmpathHeaderSpecWidget.h"
#endif

#include "EmpathHeaderSpecWidget.h"
#include "EmpathAddressSelectionWidget.h"
#include "EmpathUIUtils.h"
#include "Empath.h"

#include <RMM_Enum.h>

EmpathHeaderSpecWidget::EmpathHeaderSpecWidget(
        const QString & headerName,
        const QString & headerBody,
        QWidget * parent,
        const char * name)
    :
        QWidget(parent, name),
        headerName_(headerName),
        headerBody_(headerBody)
{
    empathDebug("ctor");
    
    QGridLayout * layout_    
        = new QGridLayout(this, 1, 2, 0, 4);
    CHECK_PTR(layout_);
    
    headerNameWidget_    = new QLabel(this, "headerNameWidget");
    CHECK_PTR(headerNameWidget_);
    
    headerNameWidget_->setText(headerName_ + ":");
    
    address_ = false;

    RMM::HeaderDataType t(RMM::headerNameToType(headerName_.ascii()));
    
    if (t == RMM::AddressList    ||
        t == RMM::Address        ||
        t == RMM::MailboxList    ||
        t == RMM::Mailbox)
        address_ = true;

    if (address_) {

        headerBodyWidget_    =
            new EmpathAddressSelectionWidget(this, "headerNameWidget");
        CHECK_PTR(headerBodyWidget_);
        
        ((EmpathAddressSelectionWidget *)headerBodyWidget_)->
            setText(headerBody_);
    
    } else {

        headerBodyWidget_    = new QLineEdit(this, "headerNameWidget");
        CHECK_PTR(headerBodyWidget_);
        ((QLineEdit *)headerBodyWidget_)->setText(headerBody_);
    }
    
    
    layout_->addWidget(headerNameWidget_, 0, 0);
    layout_->addWidget(headerBodyWidget_, 0, 1);

    layout_->activate();
    
    headerBodyWidget_->setFocus();
    
    int h = headerBodyWidget_->sizeHint().height();
    setFixedHeight(h);
}

EmpathHeaderSpecWidget::~EmpathHeaderSpecWidget()
{
    empathDebug("dtor");
}

    void
EmpathHeaderSpecWidget::setHeaderName(const QString & headerName)
{
    headerName_ = headerName;
    headerNameWidget_->setText(headerName_);
}

    QString
EmpathHeaderSpecWidget::header()
{
//    return (headerNameWidget_->text() + " " + headerBodyWidget_->text());
    return QString::null;
}

    void
EmpathHeaderSpecWidget::setHeaderBody(const QString & headerBody)
{
    empathDebug("setHeaderBody(" + headerBody + ")");
    headerBody_ = headerBody;
    if (address_)
        ((EmpathAddressSelectionWidget *)headerBodyWidget_)->
            setText(headerBody_);
    else
        ((QLineEdit *)headerBodyWidget_)->setText(headerBody_);
}

    int
EmpathHeaderSpecWidget::sizeOfColumnOne()
{
    return (headerNameWidget_->sizeHint().width());
}

    void
EmpathHeaderSpecWidget::setColumnOneSize(int i)
{
    headerNameWidget_->setFixedWidth(i);
}

    QString
EmpathHeaderSpecWidget::headerName()
{
    return headerNameWidget_->text();
}

    QString
EmpathHeaderSpecWidget::headerBody()
{
    if (address_) {
        QString s = ((EmpathAddressSelectionWidget *)headerBodyWidget_)->text();
        if (!s.contains('@') && !s.isEmpty())
            s += '@' + empath->hostName();
        return s;
    }
    else
        return ((QLineEdit *)headerBodyWidget_)->text();
}

// vim:ts=4:sw=4:tw=78
