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

#ifdef __GNUG__
# pragma implementation "EmpathAddressSelectionWidget.h"
#endif

// Qt includes
#include <qpushbutton.h>
#include <qmessagebox.h> 
#include <qlayout.h> 

// KDE includes
#include <kiconloader.h>
#include <klocale.h>
#include <klineedit.h>

// Local includes
#include "EmpathDefines.h"
#include "EmpathUIUtils.h"
#include "EmpathAddressSelectionWidget.h"
#include "Empath.h"

EmpathAddressSelectionWidget::EmpathAddressSelectionWidget(QWidget * parent)
    :   EmpathHeaderBodyWidget(parent, "AddressSelectionWidget")
{
    le_address_ = new KLineEdit(this, "le_address");
    pb_browse_  = new QPushButton(this, "pb_browse");

    /*
    le_address_->setCompletionKey(Qt::Key_Control + Qt::Key_Tab);
    le_address_->setRotateUpKey(Qt::Key_Control + Qt::Key_Up);
    le_address_->setRotateDownKey(Qt::Key_Control + Qt::Key_Down);
    */
    
    pb_browse_->setPixmap(empathIcon("misc-point")), 
    pb_browse_->setFixedWidth(pb_browse_->sizeHint().width());

    QHBoxLayout * layout = new QHBoxLayout(this);

    layout->addWidget(le_address_);
    layout->addWidget(pb_browse_);

    QObject::connect(le_address_, SIGNAL(textChanged(const QString&)),
            this, SLOT(s_textChanged(const QString&)));

    // FIXME
    QObject::connect(le_address_, SIGNAL(returnPressed()),
            this, SLOT(s_lostFocus()));

    QObject::connect(pb_browse_, SIGNAL(clicked()),
            this, SLOT(s_browseClicked()));

    setFocusProxy(le_address_);
}

EmpathAddressSelectionWidget::~EmpathAddressSelectionWidget()
{
    // Empty.
}

    QString
EmpathAddressSelectionWidget::text() const
{
    return le_address_->text();
}

    void
EmpathAddressSelectionWidget::setText(const QString & address)
{
    le_address_->setText(address);
}

    void
EmpathAddressSelectionWidget::s_textChanged(const QString&)
{
    // STUB
}

    void
EmpathAddressSelectionWidget::s_lostFocus()
{
    // add a host part to the address if necessary
    QString s = le_address_->text();
    
    if (!s.contains('@') && !s.isEmpty()) {
        s += '@' + empath->hostName();
        le_address_->setText(s);
    }
}

    void
EmpathAddressSelectionWidget::s_browseClicked()
{
    QMessageBox::information(this, "Empath",
        i18n("Sorry, the addressbook isn't ready for use yet."), i18n("OK"));
}

// vim:ts=4:sw=4:tw=78
