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
# pragma implementation "EmpathPathSelectWidget.h"
#endif

// Qt includes
#include <qlayout.h>

// KDE includes
#include <kfiledialog.h>

// Local includes
#include "EmpathPathSelectWidget.h"
#include "EmpathUIUtils.h"

EmpathPathSelectWidget::EmpathPathSelectWidget
    (const QString & initialPath, QWidget * parent)
    :   QWidget(parent, "PathSelectWidget")
{
    le_path_    = new QLineEdit(initialPath, this);
    pb_select_  = new QPushButton(this);
    
    pb_select_->setPixmap(empathIcon("misc-browse"));
    pb_select_->setFixedWidth(pb_select_->sizeHint().height());

    QObject::connect(pb_select_, SIGNAL(clicked()), this, SLOT(s_browse()));

    QHBoxLayout * layout = new QHBoxLayout(this);
    layout->addWidget(le_path_);
    layout->addWidget(pb_select_);
}

EmpathPathSelectWidget::~EmpathPathSelectWidget()
{
}

    QString
EmpathPathSelectWidget::path() const
{
    return le_path_->text();
}

    void
EmpathPathSelectWidget::setPath(const QString & s)
{
    le_path_->setText(s);
    emit(changed(s));
}
        
    void
EmpathFileSelectWidget::s_browse()
{
    QString s =
        KFileDialog::getOpenFileName(le_path_->text(), QString::null, this);
    
    if (!s.isEmpty())
        le_path_->setText(s);
}

    void
EmpathDirSelectWidget::s_browse()
{
    QString s =
        KFileDialog::getDirectory(le_path_->text(), this);
    
    if (!s.isEmpty())
        le_path_->setText(s);
}


// vim:ts=4:sw=4:tw=78
