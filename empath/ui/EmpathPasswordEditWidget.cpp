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
# pragma implementation "EmpathPasswordEditWidget.h"
#endif

// Qt includes
#include <qlayout.h>

// Local includes
#include "EmpathPasswordEditWidget.h"

EmpathPasswordEditWidget::EmpathPasswordEditWidget
        (const QString & initialPath, QWidget * parent)
    :   QWidget(parent, "PasswordEditWidget")
{
    QBoxLayout * layout = new QHBoxLayout(this);
    
    le_pass_ = new QLineEdit(initialPath, this);
    pb_echoMode_ = new QPushButton(this);
    
    layout->addWidget(le_pass_);
    layout->addWidget(pb_echoMode_);
    
    pb_echoMode_->setText("*");
    pb_echoMode_->setToggleButton(true);
    pb_echoMode_->setOn(false);
    pb_echoMode_->setFixedWidth(pb_echoMode_->sizeHint().height());

    QObject::connect(
        pb_echoMode_, SIGNAL(toggled(bool)), SLOT(s_switchEchoMode(bool)));
    
    le_pass_->setEchoMode(QLineEdit::NoEcho);
}

EmpathPasswordEditWidget::~EmpathPasswordEditWidget()
{
}

    QString
EmpathPasswordEditWidget::text() const
{
    return le_pass_->text();
}

    void
EmpathPasswordEditWidget::setText(const QString & s)
{
    le_pass_->setText(s);
}
        
    void
EmpathPasswordEditWidget::s_switchEchoMode(bool b)
{
    le_pass_->setEchoMode(b ? QLineEdit::Password : QLineEdit::NoEcho);
    pb_echoMode_->setText(b ? "" : "*");
}

// vim:ts=4:sw=4:tw=78
