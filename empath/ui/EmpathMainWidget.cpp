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
#include "Empath.h"
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
    messageViewWidget_  = new EmpathMessageViewWidget(EmpathURL(), vSplit);

    QObject::connect(
        folderWidget,       SIGNAL(showFolder(const EmpathURL &)),
        this,               SLOT(s_showFolder(const EmpathURL &)));
   
    QObject::connect(
        messageListWidget_, SIGNAL(changeView(const QString &)),
        this,               SLOT(s_changeView(const QString &)));

    QObject::connect(
        messageListWidget_, SIGNAL(compose()),
        empath,             SLOT(s_compose()));

    QObject::connect(
        messageListWidget_, SIGNAL(reply(const QString &)),
        this,               SLOT(s_reply(const QString &)));
    
    QObject::connect(
        messageListWidget_, SIGNAL(replyAll(const QString &)),
        this,               SLOT(s_replyAll(const QString &)));

    QObject::connect(
        messageListWidget_, SIGNAL(forward(const QString &)),
        this,               SLOT(s_forward(const QString &)));

    QObject::connect(
        messageListWidget_, SIGNAL(bounce(const QString &)),
        this,               SLOT(s_bounce(const QString &)));

    QObject::connect(
        messageListWidget_, SIGNAL(remove(const QStringList &)),
        this,               SLOT(s_remove(const QStringList &)));

    QObject::connect(
        messageListWidget_, SIGNAL(save(const QString &)),
        this,               SLOT(s_save(const QString &)));

    QObject::connect(
        messageListWidget_, SIGNAL(copy(const QStringList &)),
        this,               SLOT(s_copy(const QStringList &)));

    QObject::connect(
        messageListWidget_, SIGNAL(move(const QStringList &)),
        this,               SLOT(s_move(const QStringList &)));

    QObject::connect(
        messageListWidget_, SIGNAL(print(const QStringList &)),
        this,               SLOT(s_print(const QStringList &)));

    QObject::connect(
        messageListWidget_, SIGNAL(filter(const QStringList &)),
        this,               SLOT(s_filter(const QStringList &)));
    
    QObject::connect(
        messageListWidget_, SIGNAL(view(const QString &)),
        this,               SLOT(s_view(const QString &)));
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

    messageListWidget_->s_showFolder(f->index()->dict());
}

    void
EmpathMainWidget::s_changeView(const QString & id)
{
    EmpathURL u(currentFolder_);
    u.setMessageID(id);

    messageViewWidget_->s_setMessage(u);
}

    void
EmpathMainWidget::s_reply(const QString & id)
{
    EmpathURL u(currentFolder_);
    u.setMessageID(id);

    empath->s_reply(u);
}

    void
EmpathMainWidget::s_replyAll(const QString & id)
{
    EmpathURL u(currentFolder_);
    u.setMessageID(id);

    empath->s_replyAll(u);
}

    void
EmpathMainWidget::s_forward(const QString & id)
{
    EmpathURL u(currentFolder_);
    u.setMessageID(id);

    empath->s_forward(u);
}

    void
EmpathMainWidget::s_bounce(const QString & id)
{
    EmpathURL u(currentFolder_);
    u.setMessageID(id);

    empath->s_bounce(u);
}

    void
EmpathMainWidget::s_remove(const QStringList & IDList)
{
    empath->remove(currentFolder_, IDList);
}

    void
EmpathMainWidget::s_save(const QString & id)
{
    EmpathURL u(currentFolder_);
    u.setMessageID(id);
    empathDebug("STUB");
}

    void
EmpathMainWidget::s_copy(const QStringList & IDList)
{
    empathDebug("STUB");
}

    void
EmpathMainWidget::s_move(const QStringList &)
{
    empathDebug("STUB");
}

    void
EmpathMainWidget::s_print(const QStringList &)
{
    empathDebug("STUB");
}

    void
EmpathMainWidget::s_filter(const QStringList &)
{
    empathDebug("STUB");
}

    void
EmpathMainWidget::s_view(const QString &)
{
    empathDebug("STUB");
}


// vim:ts=4:sw=4:tw=78
