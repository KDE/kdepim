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

// KDE includes
#include <kconfig.h>
#include <kglobal.h>

// Local includes
#include "EmpathConfig.h"
#include "EmpathMainWidget.h"
#include "EmpathLeftSideWidget.h"
#include "EmpathMessageViewWidget.h"
#include "EmpathMessageListWidget.h"

EmpathMainWidget::EmpathMainWidget(QWidget * parent)
    : QWidget(parent, "MainWidget")
{
    hSplit = new QSplitter(this, "hSplit");
    
    vSplit = new QSplitter(Qt::Vertical, hSplit, "vSplit");
        
    messageListWidget_ = new EmpathMessageListWidget(vSplit);

    messageViewWidget_ = new EmpathMessageViewWidget(EmpathURL(), vSplit);

    leftSideWidget_ =
        new EmpathLeftSideWidget(messageListWidget_, hSplit, "leftSideWidget");
    
    hSplit->moveToFirst(leftSideWidget_);

    messageListWidget_->update();
    
    QObject::connect(messageListWidget_, SIGNAL(changeView(const EmpathURL &)),
            this, SLOT(s_displayMessage(const EmpathURL &)));
    
    messageListWidget_->setSignalUpdates(true);
    
    QValueList<int> vSizes;
    QValueList<int> hSizes;
    
    KConfig * c = KGlobal::config();
    using namespace EmpathConfig;
    c->setGroup(EmpathConfig::GROUP_DISPLAY);
    
    vSizes.append(c->readNumEntry(UI_MAIN_W_V, width() / 2));
    hSizes.append(c->readNumEntry(UI_MAIN_W_H, height() / 2));
    
    vSplit->setSizes(vSizes);
    hSplit->setSizes(hSizes);
}

EmpathMainWidget::~EmpathMainWidget()
{
    KConfig * c = KGlobal::config();
    
    using namespace EmpathConfig;

    c->setGroup(GROUP_DISPLAY);
    
    c->writeEntry(UI_MAIN_W_V, vSplit->sizes()[0]);
    c->writeEntry(UI_MAIN_W_H, hSplit->sizes()[0]);
    
    c->sync();
}

    void
EmpathMainWidget::resizeEvent(QResizeEvent * e)
{
    resize(e->size());
    hSplit->resize(e->size());
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

    void
EmpathMainWidget::s_displayMessage(const EmpathURL & url)
{
    messageViewWidget_->s_setMessage(url);
}

// vim:ts=4:sw=4:tw=78
