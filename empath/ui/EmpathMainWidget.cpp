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
# pragma implementation "EmpathMainWidget.h"
#endif

// Qt includes
#include <qheader.h>
#include <qvaluelist.h>
#include <qlayout.h>
#include <qsplitter.h>

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
#include "EmpathIndex.h"

EmpathMainWidget::EmpathMainWidget(QWidget * parent)
    : QWidget(parent, "MainWidget")
{
    QSplitter * hSplit = new QSplitter(this, "hSplit");
    (new QVBoxLayout(this))->addWidget(hSplit);

    EmpathFolderWidget * folderWidget = new EmpathFolderWidget(hSplit);

    QSplitter * vSplit = new QSplitter(Qt::Vertical, hSplit, "vSplit");

    messageListWidget_  = new EmpathMessageListWidget(vSplit);

    EmpathMessageViewWidget * messageViewWidget =
        new EmpathMessageViewWidget(EmpathURL(), vSplit);

    QObject::connect(
        folderWidget,       SIGNAL(showFolder(const EmpathURL &)),
        this,               SLOT(s_showFolder(const EmpathURL &)));
   
    QObject::connect(
        messageListWidget_, SIGNAL(changeView(const QString &)),
        this,               SLOT(s_changeView(const QString &)));
}

EmpathMainWidget::~EmpathMainWidget()
{
    // Empty.
}

    void
EmpathMainWidget::s_showFolder(const EmpathURL & url)
{
    currentFolder_ = url;

    EmpathFolder * f(empath->folder(url));

    if (0 == f) {
        empathDebug("Can't find folder `" + url.asString() + "'");
        return;
    }

    empathDebug("Doing showFolder...");
    messageListWidget_->s_showFolder(f->index()->dict());
}

    void
EmpathMainWidget::s_changeView(const QString & id)
{
    EmpathURL u(currentFolder_);
    u.setMessageID(id);

    messageViewWidget_->s_setMessage(u);
}

// vim:ts=4:sw=4:tw=78
