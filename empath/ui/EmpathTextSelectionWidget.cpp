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
# pragma implementation "EmpathTextSelectionWidget.h"
#endif

// Qt includes
#include <qlayout.h> 

// KDE includes
#include <klineedit.h>

// Local includes
#include "EmpathTextSelectionWidget.h"

EmpathTextSelectionWidget::EmpathTextSelectionWidget(QWidget * parent)
    :   EmpathHeaderBodyWidget(parent, "TextSelectionWidget")
{
    le_text_ = new KLineEdit(this, "le_text");

    QHBoxLayout * layout = new QHBoxLayout(this);
    
    layout->addWidget(le_text_);
    
    QObject::connect(le_text_, SIGNAL(textChanged(const QString&)),
            this, SLOT(s_textChanged(const QString&)));
    
    // FIXME
    QObject::connect(le_text_, SIGNAL(returnPressed()),
            this, SLOT(s_lostFocus()));

    setFocusProxy(le_text_);
}

EmpathTextSelectionWidget::~EmpathTextSelectionWidget()
{
    // Empty.
}

    QString
EmpathTextSelectionWidget::text() const
{
    return le_text_->text();
}

    void
EmpathTextSelectionWidget::setText(const QString & text)
{
    le_text_->setText(text);
}

    void
EmpathTextSelectionWidget::s_textChanged(const QString&)
{
    // STUB
}

    void
EmpathTextSelectionWidget::s_lostFocus()
{
    // STUB
}

// vim:ts=4:sw=4:tw=78
