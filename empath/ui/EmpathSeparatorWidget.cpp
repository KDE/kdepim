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
# pragma implementation "EmpathSeparatorWidget.h"
#endif

// Qt includes
#include <qlabel.h>
#include <qlayout.h>

// Local includes
#include "EmpathSeparatorWidget.h"

EmpathSeparatorWidget::EmpathSeparatorWidget
        (const QString & text, QWidget * parent)
    :   QWidget(parent, "Separator")
{
    l_text_ = new QLabel(text, this);
    QLabel * f = new QLabel(this);
    
    QHBoxLayout * layout = new QHBoxLayout(this);
    
    layout->addWidget(l_text_);
    layout->addWidget(f);

    f->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    setFixedHeight(l_text_->sizeHint().height());

    l_text_->setFixedWidth(l_text_->sizeHint().width() + 10);
}

EmpathSeparatorWidget::~EmpathSeparatorWidget()
{
}

    QString
EmpathSeparatorWidget::text() const
{
    return l_text_->text();
}

    void
EmpathSeparatorWidget::setText(const QString & s)
{
    l_text_->setText(s);
}
        
// vim:ts=4:sw=4:tw=78
