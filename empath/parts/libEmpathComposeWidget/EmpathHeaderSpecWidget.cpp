/*
    Empath - Mailer for KDE
    
    Copyright 1999, 2000
        Rik Hemsley <rik@kde.org>
        Wilco Greven <j.w.greven@student.utwente.nl>
    
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

// Qt includes
#include <qlabel.h>

// Local includes
#include "EmpathHeaderSpecWidget.h"
#include "EmpathAddressHeaderBodyWidget.h"
#include "EmpathTextHeaderBodyWidget.h"
#include "EmpathDefines.h"

EmpathHeaderSpecWidget::EmpathHeaderSpecWidget(
        const QString & headerName,
        const QString & headerBody, 
        QWidget * parent
)
    :
        QHBox(parent, "EmpathHeaderSpecWidget"),
        headerNameWidget_(0),
        headerBodyWidget_(0),
        headerName_(headerName),
        headerBody_(headerBody)
{
    headerNameWidget_ = new QLabel(this);

    if (
        (0 == stricmp(headerName.utf8(), "To")) ||
        (0 == stricmp(headerName.utf8(), "Cc")) ||
        (0 == stricmp(headerName.utf8(), "Bcc"))
    )
        headerBodyWidget_ = new EmpathAddressHeaderBodyWidget(this);
    else
        headerBodyWidget_ = new EmpathTextHeaderBodyWidget(this);

    empathDebug("headerName: " + headerName);
    empathDebug("headerBody: " + headerBody);
    headerNameWidget_->setText(headerName_ + ": ");
    headerBodyWidget_->setText(headerBody_);
    
    headerNameWidget_->setFocusProxy(headerBodyWidget_);
    setFocusProxy(headerBodyWidget_);
}

EmpathHeaderSpecWidget::~EmpathHeaderSpecWidget()
{
    // Empty.
}

    int
EmpathHeaderSpecWidget::sizeOfColumnOne() const
{
    return (headerNameWidget_->sizeHint().width());
}

    void
EmpathHeaderSpecWidget::setColumnOneSize(int i)
{
    headerNameWidget_->setFixedWidth(i);
}

    QString
EmpathHeaderSpecWidget::headerName() const
{
    QString s = headerNameWidget_->text();
    s.truncate(s.length() - 1); // Remove trailing ':'
    return s;
}

    QString
EmpathHeaderSpecWidget::headerBody() const
{
    return headerBodyWidget_->text();
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
            emit goUp();
            break;
        case Key_N:
            emit goDown();
            break;
        default:
            e->ignore();
        }
    else
        switch (e->key()) {
        case Key_Up:
            emit goUp();
            break;
        case Key_Down:
            emit goDown();
            break;
        case Key_Return:
            emit goDown();
            break;
        default:
            e->ignore();
        }
}

// vim:ts=4:sw=4:tw=78
#include "EmpathHeaderSpecWidget.moc"
