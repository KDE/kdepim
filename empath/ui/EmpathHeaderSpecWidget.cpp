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

// Qt includes
#include <qlineedit.h>
#include <qlabel.h>

// Local includes
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
        QHBox(parent, "HeaderSpecWidget"),
        headerName_(headerName),
        headerBody_(headerBody)
{
    headerNameWidget_ = new QLabel(this);
    setHeaderName(headerName_);
    
    address_ = false;

    RMM::HeaderDataType t(RMM::headerNameToType(headerName_.latin1()));
    
    if (t == RMM::AddressList    ||
        t == RMM::Address        ||
        t == RMM::MailboxList    ||
        t == RMM::Mailbox)
        address_ = true;

    if (address_) {

        EmpathAddressSelectionWidget * addressWidget = 
                new EmpathAddressSelectionWidget(this);

        headerBodyWidget_ = addressWidget->lineEdit();
        
    } else 

        headerBodyWidget_ = new QLineEdit(this, "headerBodyWidget");

    headerBodyWidget_->setText(headerBody_);
    headerBodyWidget_->setFocus();
}

EmpathHeaderSpecWidget::~EmpathHeaderSpecWidget()
{
    // Empty.
}

    void
EmpathHeaderSpecWidget::setHeaderName(const QString & headerName)
{
    headerName_ = headerName;
    headerNameWidget_->setText(headerName_ + ":");
}

    QString
EmpathHeaderSpecWidget::header()
{
    return (headerName() + ": " + headerBody());
}

    void
EmpathHeaderSpecWidget::setHeaderBody(const QString & headerBody)
{
    headerBody_ = headerBody;
    headerBodyWidget_->setText(headerBody_);
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
    QString s = headerNameWidget_->text();
    s.truncate(s.length() - 1); // Remove trailing ':'
    return s;
}

    QString
EmpathHeaderSpecWidget::headerBody()
{
    QString s = headerBodyWidget_->text();

    if (address_) {
        if (!s.contains('@') && !s.isEmpty())
            s += '@' + empath->hostName();
        return s;
    }
    else
        return s;
}

    void
EmpathHeaderSpecWidget::focusInEvent(QFocusEvent *)
{
    headerBodyWidget_->setFocus();
}

    void
EmpathHeaderSpecWidget::keyPressEvent(QKeyEvent * e)
{
    if (e->state() & ControlButton) 
        switch (e->key()) {
        case Key_P:
            emit lineUp();
            break;
        case Key_N:
            emit lineDown();
            break;
        default:
            e->ignore();
        }
    else
        switch (e->key()) {
        case Key_Up:
            emit lineUp();
            break;
        case Key_Down:
            emit lineDown();
            break;
        default:
            e->ignore();
        }
}

// vim:ts=4:sw=4:tw=78
