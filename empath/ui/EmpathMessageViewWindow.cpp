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
# pragma implementation "EmpathMessageViewWindow.h"
#endif

// Qt includes
#include <qmessagebox.h>
#include <qfile.h>

// KDE includes
#include <klocale.h>
#include <kapp.h>
#include <kmenubar.h>
#include <kfiledialog.h>

// Local includes
#include "Empath.h"
#include "EmpathUIUtils.h"
#include "EmpathMessageViewWidget.h"
#include "EmpathMessageViewWindow.h"
#include "EmpathFolderChooserDialog.h"

#include <RMM_Message.h>

EmpathMessageViewWindow::EmpathMessageViewWindow(const EmpathURL & url)
    :   KTMainWindow("MessageViewWindow"),
        url_(url)
{
    messageView_ = new EmpathMessageViewWidget(url_, this);
    
    setView(messageView_, false);
    
    setupMenuBar();
    setupToolBar();
    
    setCaption(i18n("Message View"));
    
    updateRects();
    kapp->processEvents();
    show();
    messageView_->s_setMessage(url_);
}

EmpathMessageViewWindow::~EmpathMessageViewWindow()
{
    // Empty.
}

    void
EmpathMessageViewWindow::setupMenuBar()
{
    fileMenu_    = new QPopupMenu;
    editMenu_    = new QPopupMenu;
    messageMenu_ = new QPopupMenu;
    helpMenu_    = new QPopupMenu;

    // File
    
    fileMenu_->insertItem(i18n("&Close"),
        this, SLOT(s_fileClose()));

    // Edit
    
    editMenu_->insertItem(empathIcon("copy"), i18n("&Copy"),
        this, SLOT(s_editCopy()));
    
    editMenu_->insertSeparator();
    
    editMenu_->insertItem(empathIcon("findf"), i18n("Find..."),
        this, SLOT(s_editFind()));
    
    editMenu_->insertItem(empathIcon("find"), i18n("Find &Again"),
        this, SLOT(s_editFindAgain()));
    
    // Message

    messageMenu_->insertItem(empathIcon("menu-view"), i18n("&View source"),
        messageView_, SLOT(s_switchView()));
    
    messageMenu_->insertItem(empathIcon("menu-compose"), i18n("&New"),
        this, SLOT(s_messageNew()));

    messageMenu_->insertItem(empathIcon("menu-save"), i18n("Save &As"),
        this, SLOT(s_messageSaveAs()));

    messageMenu_->insertItem(empathIcon("menu-copy"), i18n("&Copy to..."),
        this, SLOT(s_messageCopyTo()));
    
    // Help

    helpMenu_->insertItem(
        i18n("&Contents"),
        this, SLOT(s_help()));

    helpMenu_->insertItem(i18n("&About Empath"),
        empath, SLOT(s_about()));

    helpMenu_->insertSeparator();
    
    helpMenu_->insertItem(
        i18n("About &Qt"),
        this, SLOT(s_aboutQt()));
    
    menuBar()->insertItem(i18n("&File"), fileMenu_);
    menuBar()->insertItem(i18n("&Edit"), editMenu_);
    menuBar()->insertItem(i18n("&Message"), messageMenu_);
    menuBar()->insertSeparator();
    menuBar()->insertItem(i18n("&Help"), helpMenu_);
}

    void
EmpathMessageViewWindow::setupToolBar() 
{
    QPixmap p = empathIcon("toolbar-compose");
    int i = QMAX(p.width(), p.height());

    KToolBar * tb = new KToolBar(this, "tooly", i + 4);
    
    this->addToolBar(tb, 0);

    tb->insertButton(empathIcon("toolbar-compose"), 0, SIGNAL(clicked()),
            this, SLOT(s_messageNew()), true, i18n("Compose"));
    
    tb->insertButton(empathIcon("toolbar-reply"), 0, SIGNAL(clicked()),
            this, SLOT(s_messageReply()), true, i18n("Reply"));
    
    tb->insertButton(empathIcon("toolbar-forward"), 0, SIGNAL(clicked()),
            this, SLOT(s_messageForward()), true, i18n("Forward"));
    
    tb->insertButton(empathIcon("toolbar-delete"), 0, SIGNAL(clicked()),
            this, SLOT(s_messageDelete()), true, i18n("Delete"));
    
    tb->insertButton(empathIcon("toolbar-save"), 0, SIGNAL(clicked()),
            this, SLOT(s_messageSaveAs()), true, i18n("Save"));
}

    void
EmpathMessageViewWindow::setupStatusBar()
{
    // Empty.
}

    void
EmpathMessageViewWindow::s_fileClose()
{
    delete this;
}

// Edit menu slots
    
    void
EmpathMessageViewWindow::s_editCopy()
{
    // STUB
}

    void
EmpathMessageViewWindow::s_editFindInMessage()
{
    // STUB
}

    void
EmpathMessageViewWindow::s_editFind()
{
    // STUB
}

    void
EmpathMessageViewWindow::s_editFindAgain()
{
    // STUB
}

// Message menu slots

    void
EmpathMessageViewWindow::s_messageNew()
{
    empath->s_compose();
}

    void
EmpathMessageViewWindow::s_messageSaveAs()
{
    empath->saveMessage(url_, this);
}

    void
EmpathMessageViewWindow::s_messageCopyTo()
{
    EmpathFolderChooserDialog fcd(this);

    if (fcd.exec() != QDialog::Accepted)
        return;

    empath->copy(url_, fcd.selected());
}

    void
EmpathMessageViewWindow::s_messageForward()
{
    empath->s_forward(url_);
}

    void
EmpathMessageViewWindow::s_messageBounce()
{
    empath->s_bounce(url_);
}

    void
EmpathMessageViewWindow::s_messageDelete()
{
    empath->remove(url_);
}

    void
EmpathMessageViewWindow::s_messagePrint()
{
    messageView_->s_print();
}

    void
EmpathMessageViewWindow::s_messageReply()
{
    empath->s_reply(url_);
}

    void
EmpathMessageViewWindow::s_messageReplyAll()
{
    empath->s_replyAll(url_);
}

    void
EmpathMessageViewWindow::s_aboutQt()
{
    QMessageBox::aboutQt(this, "aboutQt");
}


// vim:ts=4:sw=4:tw=78
