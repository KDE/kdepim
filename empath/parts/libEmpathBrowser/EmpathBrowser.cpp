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
#include <qvaluelist.h>
#include <qlayout.h>
#include <qsplitter.h>

// KDE includes
#include <kconfig.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kglobal.h>
#include <kaction.h>
#include <kinstance.h>

// Local includes
#include "EmpathBrowser.h"
#include "EmpathCustomEvents.h"
#include "Empath.h"

extern "C"
{
    void *init_libEmpathBrowser()
    {
        return new EmpathBrowserPartFactory;
    }
}

KInstance * EmpathBrowserPartFactory::instance_ = 0L;

EmpathBrowserPartFactory::EmpathBrowserPartFactory()
{
    // Empty.
}

EmpathBrowserPartFactory::~EmpathBrowserPartFactory()
{
    delete instance_;
    instance_ = 0L;
}

    QObject *
EmpathBrowserPartFactory::create(
    QObject * parent,
    const char * name,
    const char *,
    const QStringList &
)
{
    QObject * o = new EmpathBrowserPart((QWidget *)parent, name);
    emit objectCreated(o);
    return o;
}

    KInstance *
EmpathBrowserPartFactory::instance()
{
    if (0 == instance_)
        instance_ = new KInstance("EmpathBrowser");

    return instance_;
}

// -------------------------------------------------------------------------

EmpathBrowserPart::EmpathBrowserPart(
    QWidget * parent,
    const char * name
)
    :   KParts::ReadWritePart(parent, name)
{
    setInstance(EmpathBrowserPartFactory::instance());

    widget_ = new EmpathBrowser(parent);
    widget_->setFocusPolicy(QWidget::StrongFocus);
    setWidget(widget_);

    insertChildClient(widget_->folderListWidget());
    insertChildClient(widget_->messageListWidget());
    insertChildClient(widget_->messageViewWidget());

    setXMLFile("EmpathBrowser.rc");
    enableAllActions(false);
}

EmpathBrowserPart::~EmpathBrowserPart()
{
    // Empty.
}

    void
EmpathBrowserPart::enableAllActions(bool)
{
    // STUB
}

    void
EmpathBrowserPart::_initActions()
{
}

// -------------------------------------------------------------------------


EmpathBrowser::EmpathBrowser(QWidget * parent)
    : QWidget(parent, "Browser")
{
    setBackgroundColor(red);
    QSplitter * hSplit = new QSplitter(this, "hSplit");

    KLibFactory * folderListFactory =
        KLibLoader::self()->factory("libEmpathFolderListWidget");

    if (0 != folderListFactory) {

        folderListWidget_ =
            static_cast<KParts::ReadWritePart *>
                (
                    folderListFactory->create(
                        hSplit,
                        "folder list part",
                        "KParts::ReadWritePart"
                    )
                );

    } else {
        
        empathDebug("Argh. Can't load a folder list part.");
        return;
    }

    QSplitter * vSplit = new QSplitter(Qt::Vertical, hSplit, "vSplit");

    KLibFactory * messageListFactory =
        KLibLoader::self()->factory("libEmpathMessageListWidget");
    
    KLibFactory * messageViewFactory =
        KLibLoader::self()->factory("libEmpathMessageViewWidget");


    if (0 != messageListFactory) {

        messageListWidget_ =
            static_cast<KParts::ReadWritePart *>
                (
                    messageListFactory->create(
                        vSplit,
                        "message list part",
                        "KParts::ReadWritePart"
                    )
                );

    } else {
        
        empathDebug("Argh. Can't load a message list part.");
        return;
    }

    if (0 != messageViewFactory) {

        messageViewWidget_ =
            static_cast<KParts::ReadOnlyPart *>
            (
                messageViewFactory->create(
                    vSplit,
                    "message view part",
                    "KParts::ReadOnlyPart"
                )
            );

    } else {
        
        empathDebug("Argh. Can't load a message viewing part.");
        return;
    }

    _connectUp();

    // TODO
//    folderListWidget_->setFocus();
    
    (new QVBoxLayout(this))->addWidget(hSplit);
}

EmpathBrowser::~EmpathBrowser()
{
    // Empty.
}

    void
EmpathBrowser::s_showFolder(const EmpathURL & url)
{
    empathDebug(url.asString());
    currentFolder_ = url;
    emit(setIndex(currentFolder_));
}

    void
EmpathBrowser::s_changeView(const QString & id)
{
    empathDebug("s_changeView(" + id + ")");
    EmpathURL u(currentFolder_);
    u.setMessageID(id);
    empath->retrieve(u, this);
}

    bool
EmpathBrowser::event(QEvent * e)
{
    switch (e->type()) {

        case EmpathMessageRetrievedEventT:
            {
                EmpathMessageRetrievedEvent * ev =
                    static_cast<EmpathMessageRetrievedEvent *>(e);

                if (ev->success()) {
                    RMM::Message m = ev->message();
                    emit(changeView(m));
                }
            }
            return true;
            break;

        default:
            break;
    }

    return QWidget::event(e);
}

    void
EmpathBrowser::s_reply(const QString & id)
{
    empathDebug(id);
    EmpathURL u(currentFolder_);
    u.setMessageID(id);

    empath->s_reply(u);
}

    void
EmpathBrowser::s_replyAll(const QString & id)
{
    EmpathURL u(currentFolder_);
    u.setMessageID(id);

    empath->s_replyAll(u);
}

    void
EmpathBrowser::s_forward(const QString & id)
{
    EmpathURL u(currentFolder_);
    u.setMessageID(id);

    empath->s_forward(u);
}

    void
EmpathBrowser::s_bounce(const QString & id)
{
    EmpathURL u(currentFolder_);
    u.setMessageID(id);

    empath->s_bounce(u);
}

    void
EmpathBrowser::s_remove(const QStringList & IDList)
{
    empath->remove(currentFolder_, IDList);
}

    void
EmpathBrowser::s_save(const QString & id)
{
    EmpathURL u(currentFolder_);
    u.setMessageID(id);
    empathDebug("STUB");
}

    void
EmpathBrowser::s_copy(const QStringList &)
{
    empathDebug("STUB");
}

    void
EmpathBrowser::s_move(const QStringList &)
{
    empathDebug("STUB");
}

    void
EmpathBrowser::s_print(const QStringList &)
{
    empathDebug("STUB");
}

    void
EmpathBrowser::s_filter(const QStringList &)
{
    empathDebug("STUB");
}

    void
EmpathBrowser::s_view(const QString &)
{
    empathDebug("STUB");
}

    void
EmpathBrowser::s_toggleHideRead()
{
    emit(toggleHideRead());
    emit(setIndex(currentFolder_));
}

    void
EmpathBrowser::s_toggleThread()
{
    emit(toggleThread());
    emit(setIndex(currentFolder_));
}

    void
EmpathBrowser::_connectUp()
{
    QObject::connect(
        folderListWidget_,  SIGNAL(showFolder(const EmpathURL &)),
        this,               SLOT(s_showFolder(const EmpathURL &)));
   
    QObject::connect(
        messageListWidget_, SIGNAL(messageActivated(const QString &)),
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

    QObject::connect(
        this,               SIGNAL(changeView(RMM::Message &)),
        messageViewWidget_, SLOT(s_setMessage(RMM::Message &)));

    QObject::connect(
        this,               SIGNAL(setIndex(const EmpathURL &)),
        messageListWidget_, SLOT(s_setIndex(const EmpathURL &)));

    QObject::connect(
        this,               SIGNAL(toggleHideRead()),
        messageListWidget_, SLOT(s_toggleHideRead()));
    
    QObject::connect(
        this,               SIGNAL(toggleThread()),
        messageListWidget_, SLOT(s_toggleThread()));
}


// vim:ts=4:sw=4:tw=78
