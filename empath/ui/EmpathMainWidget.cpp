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
# pragma implementation "EmpathMainWidget.h"
#endif

// Qt includes
#include <qheader.h>
#include <qvaluelist.h>
#include <qlayout.h>

// KDE includes
#include <kconfig.h>
#include <kglobal.h>

// Local includes
#include "EmpathURL.h"
#include "EmpathConfig.h"
#include "EmpathMainWidget.h"
#include "EmpathFolderWidget.h"
#include "EmpathMessageViewWidget.h"
#include "EmpathMessageListWidget.h"

EmpathMainWidget::EmpathMainWidget(QWidget * parent)
    : QWidget(parent, "MainWidget")
{
    QVBoxLayout * layout = new QVBoxLayout(this);
    layout->setAutoAdd(true);
    
    hSplit = new QSplitter(this, "hSplit");
    
    EmpathFolderWidget * fw = new EmpathFolderWidget(hSplit);
    
    vSplit = new QSplitter(Qt::Vertical, hSplit, "vSplit");
        
    messageListWidget_ = new EmpathMessageListWidget(vSplit);
    messageListWidget_->listenTo(fw->id());

    messageViewWidget_ = new EmpathMessageViewWidget(EmpathURL(), vSplit);
   
    QObject::connect(
        messageListWidget_, SIGNAL(changeView(const EmpathURL &)),
        messageViewWidget_, SLOT(s_setMessage(const EmpathURL &)));
}

EmpathMainWidget::~EmpathMainWidget()
{
    // Empty.
}

    EmpathMessageListWidget *
EmpathMainWidget::messageListWidget()
{
    return messageListWidget_;
}

    EmpathMessageViewWidget *
EmpathMainWidget::messageViewWidget()
{
    return messageViewWidget_;
}

// vim:ts=4:sw=4:tw=78
