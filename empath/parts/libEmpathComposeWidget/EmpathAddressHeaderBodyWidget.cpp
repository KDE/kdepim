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
#include <qwidget.h>
#include <qlayout.h>
#include <qpushbutton.h>

// KDE includes
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kiconloader.h>

// Local includes
#include "EmpathAddressHeaderBodyWidget.h"

EmpathAddressHeaderBodyWidget::EmpathAddressHeaderBodyWidget(QWidget * parent)
    :   EmpathHeaderBodyWidget(parent),
        le_address_(0)
{
    le_address_ = new KLineEdit(this, "le_address");

    QPushButton * pb_browse = new QPushButton(this, "pb_browse");

    pb_browse->setPixmap(BarIcon("pointingHand")), 
    pb_browse->setFixedWidth(pb_browse->sizeHint().width());

    QHBoxLayout * layout = new QHBoxLayout(this);

    layout->addWidget(le_address_);
    layout->addWidget(pb_browse);

    QObject::connect(
        pb_browse,  SIGNAL(clicked()),
        this,       SLOT(s_browseClicked())
    );

    setFocusProxy(le_address_);
}

EmpathAddressHeaderBodyWidget::~EmpathAddressHeaderBodyWidget()
{
    // Empty.
}

    QString
EmpathAddressHeaderBodyWidget::text() const
{
    return le_address_->text();
}

    void
EmpathAddressHeaderBodyWidget::setText(const QString & s)
{
    le_address_->setText(s);
}

    void
EmpathAddressHeaderBodyWidget::s_browseClicked()
{
    KMessageBox::information(
        this,
        "Empath",
        i18n("Sorry, the addressbook isn't ready for use yet."),
        QString::null,
        false
    );
}

// vim:ts=4:sw=4:tw=78
