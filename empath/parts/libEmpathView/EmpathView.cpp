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
#include "EmpathView.h"
#include "EmpathCustomEvents.h"
#include "Empath.h"
#include "EmpathMessageListWidget.h"
#include "EmpathFolderListWidget.h"

extern "C"
{
    void *init_libEmpathView()
    {
        return new EmpathViewPartFactory;
    }
}

KInstance * EmpathViewPartFactory::instance_ = 0L;

EmpathViewPartFactory::EmpathViewPartFactory()
{
    // Empty.
}

EmpathViewPartFactory::~EmpathViewPartFactory()
{
    delete instance_;
    instance_ = 0L;
}

    QObject *
EmpathViewPartFactory::create(
    QObject * parent,
    const char * name,
    const char *,
    const QStringList &
)
{
    QObject * o = new EmpathViewPart((QWidget *)parent, name);
    emit objectCreated(o);
    return o;
}

    KInstance *
EmpathViewPartFactory::instance()
{
    if (0 == instance_)
        instance_ = new KInstance("EmpathView");

    return instance_;
}

// -------------------------------------------------------------------------

EmpathViewPart::EmpathViewPart(
    QWidget * parent,
    const char * name
)
    :   KParts::ReadWritePart(parent, name)
{
    setInstance(EmpathViewPartFactory::instance());

    widget_ = new EmpathView(parent);
    widget_->setFocusPolicy(QWidget::StrongFocus);
    setWidget(widget_);

    insertChildClient(widget_->messageViewPart_);

    _initActions();

    setXMLFile("EmpathView.rc");

    connect(
        this,                       SIGNAL(showFolder(const EmpathURL &)),
        widget_->messageListWidget_,  SLOT(s_showFolder(const EmpathURL &)));

//    connect(
//        this,                       SIGNAL(showFolder(const EmpathURL &)),
//        widget_->folderListWidget_, SLOT(s_setActiveFolder(const EmpathURL &)));

    connect(
        widget_->messageListWidget_,SIGNAL(messageActivated(const EmpathURL &)),
        widget_->messageViewPart_,  SLOT(s_changeView(const EmpathURL &)));
}

EmpathViewPart::~EmpathViewPart()
{
    // Empty.
}

    void
EmpathViewPart::_initActions()
{
    (void) new KAction(
        i18n("&View"),
        "empath_message_view",
        CTRL+Key_Return,
        this,
        SLOT(s_messageView()),
        actionCollection(),
        "messageView"
    );
        
    (void) new KAction(
        i18n("&Compose"),
        "empath_message_compose",
        Key_M,
        this,
        SLOT(s_messageCompose()),
        actionCollection(),
        "messageCompose"
    );
        
    (void) new KAction(
        i18n("&Reply"),
        "empath_message_reply",
        Key_R,
        this,
        SLOT(s_messageReply()),
        actionCollection(),
        "messageReply"
    );
        
    (void) new KAction(
        i18n("Reply to &All"),
        "empath_message_reply_all",
        Key_G,
        this,
        SLOT(s_messageReplyAll()),
        actionCollection(),
        "messageReplyAll"
    );
        
    (void) new KAction(
        i18n("&Forward"),
        "empath_message_forward",
        Key_F,
        this,
        SLOT(s_messageForward()),
        actionCollection(),
        "messageForward"
    );
        
    (void) new KAction(
        i18n("&Delete"),
        "remove",
        Key_D,
        this,
        SLOT(s_messageDelete()),
        actionCollection(),
        "messageDelete"
    );
        
    (void) new KAction(
        i18n("&Bounce"),
        "empath_message_bounce",
        0,
        this,
        SLOT(s_messageBounce()),
        actionCollection(),
        "messageBounce"
    );
        
    (void) new KAction(
        i18n("Save &As..."),
        "empath_message_save_as",
        0,
        this,
        SLOT(s_messageSaveAs()),
        actionCollection(),
        "messageSaveAs"
    );
        
    (void) new KAction(
        i18n("&Copy To..."),
        "empath_message_copy",
        Key_C,
        this,
        SLOT(s_messageCopyTo()),
        actionCollection(),
        "messageCopyTo"
    );
        
    (void) new KAction(
        i18n("&Move To..."),
        "empath_message_move",
        0,
        this,
        SLOT(s_messageMoveTo()),
        actionCollection(),
        "messageMoveTo"
    );
        
    (void) new KAction(
        i18n("Mark..."),
        "empath_message_mark_many",
        0,
        this,
        SLOT(s_messageMarkMany()),
        actionCollection(),
        "messageMarkMany"
    );
        
    (void) new KAction(
        i18n("&Print"),
        "empath_message_print",
        0,
        this,
        SLOT(s_messagePrint()),
        actionCollection(),
        "messagePrint"
    );
        
    (void) new KAction(
        i18n("&Filter"),
        "empath_message_filter",
        0,
        this,
        SLOT(s_messageFilter()),
        actionCollection(),
        "messageFilter"
    );
        
    (void) new KAction(
        i18n("&Expand"),
        "viewmag+",
        0,
        this,
        SLOT(s_threadExpand()),
        actionCollection(),
        "threadExpand"
    );
        
    (void) new KAction(
        i18n("&Collapse"),
        "viewmag-",
        0,
        this,
        SLOT(s_threadCollapse()),
        actionCollection(),
        "threadCollapse"
    );
        
    (void) new KAction(
        i18n("&Previous"),
        "previous",
        CTRL+Key_P,
        this,
        SLOT(s_goPrevious()),
        actionCollection(),
        "goPrevious"
    );
        
    (void) new KAction(
        i18n("&Next"),
        "next",
        CTRL+Key_N,
        this,
        SLOT(s_goNext()),
        actionCollection(),
        "goNext"
    );
        
    (void) new KAction(
        i18n("Next &Unread"),
        "next",
        Key_N,
        this,
        SLOT(s_goNextUnread()),
        actionCollection(),
        "goNextUnread"
    );
        

    (void) new KToggleAction(
        i18n("&Tag"),
        "empath_message_tag",
        0,
        this,
        SLOT(s_messageMark()),
        actionCollection(),
        "messageTag"
     );
        
    (void) new KToggleAction(
        i18n("&Mark as read"),
        "empath_message_mark_read",
        0,
        this,
        SLOT(s_messageMarkRead()),
        actionCollection(),
        "messageMarkRead"
     );
        
    (void) new KToggleAction(
        i18n("Mark as replied"),
        "empath_message_mark_replied",
        0,
        this,
        SLOT(s_messageMarkReplied()),
        actionCollection(),
        "messageMarkReplied"
     );
        
    (void) new KToggleAction(
        i18n("Hide Read"),
        "empath_hide_read",
        0,
        this,
        SLOT(s_toggleHideRead()),
        actionCollection(),
        "hideRead"
     );
        
    (void) new KToggleAction(
        i18n("Thread Messages"),
        "empath_thread",
        0,
        this,
        SLOT(s_toggleThread()),
        actionCollection(),
        "thread"
     );
}

    void
EmpathViewPart::s_showFolder(const EmpathURL & url)
{
    empathDebug("showFolder(" + url.asString() + ")");
    emit(showFolder(url));
}

    void
EmpathViewPart::s_messageView()
{
    empathDebug("STUB");
}

    void
EmpathViewPart::s_messageCompose()
{
    empath->s_compose();
}

    void
EmpathViewPart::s_messageReply()
{
    EmpathURL firstSelected = widget_->messageListWidget_->firstSelected();
    empath->s_reply(firstSelected);
}

    void
EmpathViewPart::s_messageReplyAll()
{
    EmpathURL firstSelected = widget_->messageListWidget_->firstSelected();
    empath->s_replyAll(firstSelected);
}

    void
EmpathViewPart::s_messageForward()
{
    empathDebug("STUB");
//    EmpathURLList l(widget_->messageListWidget_->selection());
//    empath->s_forward(l);
}

    void
EmpathViewPart::s_messageDelete()
{
    empathDebug("STUB");
//    EmpathURLList l(widget_->messageListWidget_->selection());
// empath->remove(l);
}

    void
EmpathViewPart::s_messageBounce()
{
    empathDebug("STUB");
//    EmpathURLList l(widget_->messageListWidget_->selection());
//    empath->s_bounce(l);
}

    void
EmpathViewPart::s_messageSaveAs()
{
    empathDebug("STUB");
//    EmpathURL firstSelected = widget_->messageListWidget_->firstSelected();
//    empath->saveMessage(firstSelected, this);
}

    void
EmpathViewPart::s_messageCopyTo()
{
//    EmpathURLList l(widget_->messageListWidget_->selection());

    empathDebug("STUB");

//    EmpathURL dest; // = what ?
//    EmpathJobID id = empath->copy(firstSelected, dest, this);
}

    void
EmpathViewPart::s_messageMoveTo()
{
//    EmpathURLList l(widget_->messageListWidget_->selection());

    empathDebug("STUB");

//    EmpathURL dest; // = what ?
//    EmpathJobID id = empath->copy(each url, dest, this);
}

    void
EmpathViewPart::s_messageMarkMany()
{
    empathDebug("STUB");
}

    void
EmpathViewPart::s_messagePrint()
{
    empathDebug("STUB");
//    empath->print(widget_->messageListWidget_->selection());
}

    void
EmpathViewPart::s_messageFilter()
{
    empathDebug("STUB");
//    empath->filter(widget_->messageListWidget_->selection());
}

    void
EmpathViewPart::s_threadExpand()
{
    widget_->messageListWidget_->s_threadExpand();
}

    void
EmpathViewPart::s_threadCollapse()
{
    widget_->messageListWidget_->s_threadCollapse();
}

    void
EmpathViewPart::s_goPrevious()
{
    widget_->messageListWidget_->s_goPrevious();
}

    void
EmpathViewPart::s_goNext()
{
    widget_->messageListWidget_->s_goNext();
}

    void
EmpathViewPart::s_goNextUnread()
{
    widget_->messageListWidget_->s_goNextUnread();
}

    void
EmpathViewPart::s_messageMark()
{
//    EmpathURLList l(widget_->messageListWidget_->selection());
    empathDebug("STUB");
}

    void
EmpathViewPart::s_messageMarkRead()
{
//    EmpathURLList l(widget_->messageListWidget_->selection());
    empathDebug("STUB");
}

    void
EmpathViewPart::s_messageMarkReplied()
{
//    EmpathURLList l(widget_->messageListWidget_->selection());
    empathDebug("STUB");
}

    void
EmpathViewPart::s_toggleHideRead()
{
    widget_->messageListWidget_->s_toggleHideRead();
}

    void
EmpathViewPart::s_toggleThread()
{
    widget_->messageListWidget_->s_toggleThread();
}


// -------------------------------------------------------------------------


EmpathView::EmpathView(QWidget * parent)
    : QWidget(parent, "View")
{
//    QSplitter * hSplit = new QSplitter(this, "hSplit");

//    folderListWidget_ = new EmpathFolderListWidget(hSplit);

//    QSplitter * vSplit = new QSplitter(Qt::Vertical, hSplit, "vSplit");

    QSplitter * vSplit = new QSplitter(Qt::Vertical, this, "vSplit");

    KLibFactory * messageViewFactory =
        KLibLoader::self()->factory("libEmpathMessageViewWidget");

    messageListWidget_ = new EmpathMessageListWidget(vSplit);

    if (0 != messageViewFactory) {

        messageViewPart_ =
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

//    connect(
//        folderListWidget_,  SIGNAL(showFolder(const EmpathURL &)),
//        messageListWidget_, SLOT(s_showFolder(const EmpathURL &)));

    connect(
        messageListWidget_, SIGNAL(showMessage(const EmpathURL &)),
        messageViewPart_,   SLOT(s_showMessage(const EmpathURL &)));

//    folderListWidget_->setFocus();
    messageListWidget_->setFocus();

//    (new QVBoxLayout(this))->addWidget(hSplit);
    (new QVBoxLayout(this))->addWidget(vSplit);
}

EmpathView::~EmpathView()
{
    // Empty.
}

// vim:ts=4:sw=4:tw=78
#include "EmpathView.moc"
