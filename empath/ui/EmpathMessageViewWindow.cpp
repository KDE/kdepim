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
#include <kmsgbox.h>
#include <kfiledialog.h>

// Local includes
#include "Empath.h"
#include "EmpathUIUtils.h"
#include "EmpathMessageViewWidget.h"
#include "EmpathMessageViewWindow.h"
#include "EmpathFolderChooserDialog.h"
#include "EmpathMessageSourceView.h"
#include <RMM_Message.h>

EmpathMessageViewWindow::EmpathMessageViewWindow(
        const EmpathURL & url, const char * name)
    :    KTMainWindow(name),
        url_(url)
{
    empathDebug("ctor");
    
    messageView_ = new EmpathMessageViewWidget(url_, this, "messageView");
    CHECK_PTR(messageView_);
    messageView_->go();
    
    setView(messageView_, false);
    
    setupMenuBar();
    setupToolBar();
    
    setCaption(i18n("Message View - ") + kapp->getCaption());
    
    updateRects();
    kapp->processEvents();
    show();
}

EmpathMessageViewWindow::~EmpathMessageViewWindow()
{
    empathDebug("dtor");
}

    void
EmpathMessageViewWindow::setupMenuBar()
{
    empathDebug("setting up menu bar");

    fileMenu_    = new QPopupMenu();
    CHECK_PTR(fileMenu_);

    editMenu_    = new QPopupMenu();
    CHECK_PTR(editMenu_);

    messageMenu_    = new QPopupMenu();
    CHECK_PTR(messageMenu_);
    
    helpMenu_    = new QPopupMenu();
    CHECK_PTR(helpMenu_);

    // File menu
    empathDebug("setting up file menu");
    
    fileMenu_->insertItem(i18n("&Close"),
        this, SLOT(s_fileClose()));

    // Edit menu
    
    empathDebug("setting up edit menu");

    editMenu_->insertItem(empathIcon("empath-copy.png"), i18n("&Copy"),
        this, SLOT(s_editCopy()));
    
    editMenu_->insertSeparator();
    
    editMenu_->insertItem(empathIcon("findf.png"), i18n("Find..."),
        this, SLOT(s_editFind()));
    
    editMenu_->insertItem(empathIcon("find.png"), i18n("Find &Again"),
        this, SLOT(s_editFindAgain()));
    
    // Message Menu
    empathDebug("setting up message menu");

    messageMenu_->insertItem(empathIcon("mini-view.png"), i18n("&View source"),
        messageView_, SLOT(s_switchView()));
    
    messageMenu_->insertItem(empathIcon("mini-view.png"), i18n("&New"),
        this, SLOT(s_messageNew()));

    messageMenu_->insertItem(empathIcon("mini-save.png"), i18n("Save &As"),
        this, SLOT(s_messageSaveAs()));

    messageMenu_->insertItem(empathIcon("editcopy.png"), i18n("&Copy to..."),
        this, SLOT(s_messageCopyTo()));
    
    helpMenu_->insertItem(
        i18n("&Contents"),
        this, SLOT(s_help()));

    helpMenu_->insertItem(i18n("&About Empath"),
        empath, SLOT(s_about()));

    helpMenu_->insertSeparator();
    
    helpMenu_->insertItem(
        i18n("About &Qt"),
        this, SLOT(s_aboutQt()));
    
    helpMenu_->insertItem(
        i18n("About &KDE"),
        kapp, SLOT(aboutKDE()));
    
    menuBar()->insertItem(i18n("&File"), fileMenu_);
    menuBar()->insertItem(i18n("&Edit"), editMenu_);
    menuBar()->insertItem(i18n("&Message"), messageMenu_);
    menuBar()->insertSeparator();
    menuBar()->insertItem(i18n("&Help"), helpMenu_);
}

    void
EmpathMessageViewWindow::setupToolBar() 
{
    empathDebug("setting up tool bar");

    QPixmap p = empathIcon("compose.png");
    int i = QMAX(p.width(), p.height());

    KToolBar * tb = new KToolBar(this, "tooly", i + 4);
    CHECK_PTR(tb);
    
    this->addToolBar(tb, 0);

    tb->insertButton(empathIcon("compose.png"), 0, SIGNAL(clicked()),
            this, SLOT(s_messageNew()), true, i18n("Compose"));
    
    tb->insertButton(empathIcon("reply.png"), 0, SIGNAL(clicked()),
            this, SLOT(s_messageReply()), true, i18n("Reply"));
    
    tb->insertButton(empathIcon("forward.png"), 0, SIGNAL(clicked()),
            this, SLOT(s_messageForward()), true, i18n("Forward"));
    
    tb->insertButton(empathIcon("delete.png"), 0, SIGNAL(clicked()),
            this, SLOT(s_messageDelete()), true, i18n("Delete"));
    
    tb->insertButton(empathIcon("save.png"), 0, SIGNAL(clicked()),
            this, SLOT(s_messageSaveAs()), true, i18n("Save"));
}

    void
EmpathMessageViewWindow::setupStatusBar()
{
    empathDebug("setting up status bar");
}

    void
EmpathMessageViewWindow::s_fileClose()
{
    empathDebug("s_fileClose called");
    delete this;
}

// Edit menu slots
    
    void
EmpathMessageViewWindow::s_editCopy()
{
    empathDebug("s_editCopy called");
}

    void
EmpathMessageViewWindow::s_editFindInMessage()
{
    empathDebug("s_editFindInMessage called");
}

    void
EmpathMessageViewWindow::s_editFind()
{
    empathDebug("s_editFind called");
}

    void
EmpathMessageViewWindow::s_editFindAgain()
{
    empathDebug("s_editFindAgain called");
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
    empathDebug("s_messageSaveAs called");
    
    RMM::RMessage * m = empath->message(url_);
    
    if (m == 0) {
        return;
    }
    
    RMM::RMessage message(*m);

    QString saveFilePath =
        KFileDialog::getSaveFileName(
            QString::null, QString::null,
            this, i18n("Empath: Save Message").ascii());

    empathDebug(saveFilePath);
    
    if (saveFilePath.isEmpty()) {
        empathDebug("No filename given");
        return;
    }
    
    QFile f(saveFilePath);
    if (!f.open(IO_WriteOnly)) {
        // Warn user file cannot be opened.
        empathDebug("Couldn't open file for writing");
        KMsgBox(this, "Empath",
    i18n("Sorry I can't write to that file. Please try another filename."),
        KMsgBox::EXCLAMATION, i18n("OK"));
        return;
    }
    empathDebug("Opened " + saveFilePath + " OK");
    
    QDataStream d(&f);
    
    d << message.asString();

    f.close();
    
}

    void
EmpathMessageViewWindow::s_messageCopyTo()
{
    empathDebug("s_messageCopyTo called");

    RMM::RMessage * m(empath->message(url_));
    
    if (m == 0) {
        return;
    }
    
    RMM::RMessage message(*m);
    
    EmpathFolderChooserDialog fcd((QWidget *)0L, "fcd");

    if (fcd.exec() != QDialog::Accepted) {
        empathDebug("copy cancelled");
        return;
    }

    EmpathFolder * copyFolder = empath->folder(fcd.selected());

    if (copyFolder != 0)
        copyFolder->writeMessage(message);
    else {
        empathDebug("Couldn't get copy folder");
    }
}

    void
EmpathMessageViewWindow::s_messageForward()
{
    empathDebug("s_messageForward called");
    empath->s_forward(url_);
}

    void
EmpathMessageViewWindow::s_messageBounce()
{
    empathDebug("s_messageBounce called");
    empath->s_bounce(url_);
}

    void
EmpathMessageViewWindow::s_messageDelete()
{
    empathDebug("s_messageDelete called");
    if (empath->remove(url_)) {
        KMsgBox(this, "Empath",
            i18n("Message Deleted"), KMsgBox::EXCLAMATION, i18n("OK"));
    } else {
        KMsgBox(this, "Empath",
            i18n("Couldn't delete message"), KMsgBox::EXCLAMATION, i18n("OK"));
    }
    s_fileClose();
}

    void
EmpathMessageViewWindow::s_messagePrint()
{
    empathDebug("s_messagePrint called");
    messageView_->s_print();
}

    void
EmpathMessageViewWindow::s_messageReply()
{
    empathDebug("s_messageReply called");
    empath->s_reply(url_);
}

    void
EmpathMessageViewWindow::s_messageReplyAll()
{
    empathDebug("s_messageReplyAll called");
    empath->s_replyAll(url_);
}

    void
EmpathMessageViewWindow::s_aboutQt()
{
    QMessageBox::aboutQt(this, "aboutQt");
}


// vim:ts=4:sw=4:tw=78
