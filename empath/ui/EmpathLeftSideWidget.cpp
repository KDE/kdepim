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
# pragma implementation "EmpathLeftSideWidget.h"
#endif

// Local includes
#include "EmpathLeftSideWidget.h"
#include "EmpathFolderWidget.h"
#include "EmpathTaskWidget.h"
#include "EmpathMessageListWidget.h"

EmpathLeftSideWidget::EmpathLeftSideWidget(
        EmpathMessageListWidget * messageListWidget,
        QWidget * parent, const char * name)
    :    QWidget(parent, name)
{
    layout_ = new QGridLayout(this, 2, 1, 0, 0);
    CHECK_PTR(layout_);
    
    folderWidget_ = new EmpathFolderWidget(this, "folderWidget", true);
    CHECK_PTR(folderWidget_);
    
    QObject::connect(
            folderWidget_,        SIGNAL(showFolder(const EmpathURL &)),
            messageListWidget,    SLOT(s_showFolder(const EmpathURL &)));
    
    QObject::connect(
            messageListWidget,    SIGNAL(showing()),
            folderWidget_,        SLOT(s_showing()));
    
    taskWidget_ = new EmpathTaskWidget(this, "taskWidget");
    CHECK_PTR(taskWidget_);
    
    layout_->addWidget(folderWidget_,    0, 0);
    layout_->addWidget(taskWidget_,        1, 0);
    
    layout_->activate();
}

EmpathLeftSideWidget::~EmpathLeftSideWidget()
{
    // Empty.
}

// vim:ts=4:sw=4:tw=78
