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
# pragma implementation "EmpathMainWindow.h"
#endif

// Qt includes
#include <qapplication.h>
#include <qmessagebox.h>
#include <qwidgetlist.h>
#include <qfile.h>

// KDE includes
#include <klocale.h>
#include <kfiledialog.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kapp.h>

// Local includes
#include "EmpathUIUtils.h"
#include "EmpathDefines.h"
#include "EmpathMainWidget.h"
#include "EmpathMainWindow.h"
#include "EmpathMessageListWidget.h"
#include "EmpathMessageViewWindow.h"
#include "EmpathMessageWidget.h"
#include "EmpathFolderWidget.h"
#include "EmpathConfig.h"
#include "EmpathFolderChooserDialog.h"
#include "Empath.h"

EmpathMainWindow::EmpathMainWindow()
    :    KTMainWindow("MainWindow")
{
    // Resize to previous size.
    
    KConfig * c = KGlobal::config();

    using namespace EmpathConfig;

    c->setGroup(GROUP_DISPLAY);
    
    int x = c->readNumEntry(UI_MAIN_WIN_X, 600);
    int y = c->readNumEntry(UI_MAIN_WIN_Y, 400);
    resize(x, y);
    
    menu_       = menuBar();
    status_     = statusBar();    

    mainWidget_ = new EmpathMainWidget(this);
    
    messageListWidget_ = mainWidget_->messageListWidget();

    setView(mainWidget_, false);
    kapp->setMainWidget(this);
    
    _setupMenuBar();
    _setupToolBar();
    _setupStatusBar();
    
    updateRects();
    kapp->processEvents();
    show();
}

EmpathMainWindow::~EmpathMainWindow()
{
    KConfig * c = KGlobal::config();

    using namespace EmpathConfig;
    
    c->setGroup(GROUP_DISPLAY);
    
    c->writeEntry(UI_MAIN_WIN_X, width());
    c->writeEntry(UI_MAIN_WIN_Y, height());
}

    void
EmpathMainWindow::_setupToolBar()
{
    QPixmap p = empathIcon("toolbar-compose");
    int i = QMAX(p.width(), p.height());

    KToolBar * tb = new KToolBar(this, "tooly", i + 4 );

    KConfig * c = KGlobal::config();

    using namespace EmpathConfig;

    c->setGroup(GROUP_DISPLAY);
    
    KToolBar::BarPosition pos =
        (KToolBar::BarPosition)
        c->readNumEntry(UI_MAIN_WIN_TOOL);

    if  (   pos != KToolBar::Top    &&
            pos != KToolBar::Left   &&
            pos != KToolBar::Right  &&
            pos != KToolBar::Bottom )
        pos = KToolBar::Top;

    tb->setBarPos(pos);
    
    this->addToolBar(tb, 0);

    QObject::connect(tb, SIGNAL(moved(BarPosition)),
            this, SLOT(s_toolbarMoved(BarPosition)));

    tb->insertButton(empathIcon("toolbar-compose"), 0, SIGNAL(clicked()),
            this, SLOT(s_messageNew()), true, i18n("Compose"));
    
    tb->insertButton(empathIcon("toolbar-reply"), 0, SIGNAL(clicked()),
            this, SLOT(s_messageReply()), true, i18n("Reply"));
    
    tb->insertButton(empathIcon("toolbar-forward"), 0, SIGNAL(clicked()),
            this, SLOT(s_messageForward()), true, i18n("Forward"));
    
    tb->insertSeparator();
    
    tb->insertButton(empathIcon("toolbar-delete"), 0, SIGNAL(clicked()),
            this, SLOT(s_messageDelete()), true, i18n("Delete"));
    
    tb->insertButton(empathIcon("toolbar-save"), 0, SIGNAL(clicked()),
            this, SLOT(s_messageSaveAs()), true, i18n("Save"));
}

    void
EmpathMainWindow::_setupStatusBar()
{
    empath->s_infoMessage(i18n("Welcome to Empath"));
}

// If the user presses the close button on the title bar, or tries
// to kill us off another way, handle gracefully

    bool
EmpathMainWindow::queryExit()
{
    // FIXME: Check if the user wants to save changes
    
    s_fileQuit();

    return false;
}

    void
EmpathMainWindow::closeEvent(QCloseEvent * e)
{
    e->accept();
    s_fileQuit();
}

// File menu slots

    void
EmpathMainWindow::s_fileSendNew()
{
    empath->sendQueued();
}

    void
EmpathMainWindow::s_fileAddressBook()
{
}
    
    void
EmpathMainWindow::s_fileQuit()
{
    // FIXME: Check if the user wants to save changes.
    
    // Hide all toplevels while we clean up.
    hide();

    QWidgetList * list = QApplication::topLevelWidgets();

    QWidgetListIt it(*list);
    
    for (; it.current(); ++it)
        if (it.current()->isVisible())
            it.current()->hide();
    
    delete list;
    list = 0;
    
    kapp->processEvents();

    delete this;
}

// Folder menu slots

    void
EmpathMainWindow::s_folderNew()
{
    // STUB
}

    void
EmpathMainWindow::s_folderEdit()
{
    // STUB
}

    void
EmpathMainWindow::s_folderClear()
{
    // STUB
}

    void
EmpathMainWindow::s_folderDelete()
{
    // STUB
}


// Message menu slots

    void
EmpathMainWindow::s_messageNew()
{
    empath->s_compose();
}

    void
EmpathMainWindow::s_messageReply()
{
    if (!_messageSelected()) return;
    empath->s_reply(messageListWidget_->firstSelectedMessage());
}

    void
EmpathMainWindow::s_messageReplyAll()
{
    if (!_messageSelected()) return;
    empath->s_replyAll(messageListWidget_->firstSelectedMessage());
}

    void
EmpathMainWindow::s_messageForward()
{
    if (!_messageSelected()) return;
    empath->s_forward(messageListWidget_->firstSelectedMessage());
}

    void
EmpathMainWindow::s_messageBounce()
{
    if (!_messageSelected()) return;
    empath->s_bounce(messageListWidget_->firstSelectedMessage());
}

    void
EmpathMainWindow::s_messageDelete()
{
    if (!_messageSelected()) return;
    messageListWidget_->s_messageDelete();
}

    void
EmpathMainWindow::s_messageSaveAs()
{
    if (!_messageSelected()) return;

    empath->saveMessage(messageListWidget_->firstSelectedMessage(), this);
}

    void
EmpathMainWindow::s_messageCopyTo()
{
    if (!_messageSelected()) return;

    EmpathFolderChooserDialog fcd(this);

    if (fcd.exec() != QDialog::Accepted)
        return;

    empath->copy(messageListWidget_->firstSelectedMessage(), fcd.selected());
}

    void
EmpathMainWindow::s_messageMoveTo()
{
    EmpathFolderChooserDialog fcd(this);

    if (fcd.exec() != QDialog::Accepted)
        return;

    empath->move
        (messageListWidget_->firstSelectedMessage(), fcd.selected());
}

    void
EmpathMainWindow::s_messagePrint()
{
    if (!_messageSelected()) return;
//    mainWidget_->messageViewWidget()->s_print();
}

    void
EmpathMainWindow::s_messageFilter()
{
    if (!_messageSelected()) return;
    empath->filter(messageListWidget_->firstSelectedMessage());
}

    void
EmpathMainWindow::s_messageView()
{
    if (!_messageSelected()) return;
    
    EmpathMessageViewWindow * messageViewWindow =
        new EmpathMessageViewWindow(messageListWidget_->firstSelectedMessage());

    messageViewWindow->show();
}

    void
EmpathMainWindow::s_help()
{
    // STUB
}

    void
EmpathMainWindow::s_about()
{
    empath->s_about(this);
}

    void
EmpathMainWindow::s_aboutQt()
{
    QMessageBox::aboutQt(this, "aboutQt");
}

    void
EmpathMainWindow::statusMessage(const QString & messageText, int seconds)
{
    status_->message(messageText, seconds);
}

    void
EmpathMainWindow::clearStatusMessage()
{
    status_->clear();
}

    void
EmpathMainWindow::s_toolbarMoved(BarPosition pos)
{
    KConfig * c = KGlobal::config();
    
    using namespace EmpathConfig;
    
    c->setGroup(GROUP_DISPLAY);
    c->writeEntry(UI_MAIN_WIN_TOOL, (int)pos);
}

    void
EmpathMainWindow::newMailArrived()
{
    messageListWidget_->update();
}

    EmpathMessageListWidget *
EmpathMainWindow::messageListWidget()
{
    return messageListWidget_;
}

    void
EmpathMainWindow::s_dumpWidgetList()
{
    // STUB
}

    void
EmpathMainWindow::s_editSelectTagged()
{
    messageListWidget_->selectTagged();
}

    void
EmpathMainWindow::s_editSelectRead()
{
    messageListWidget_->selectRead();
}

    void
EmpathMainWindow::s_editSelectAll()
{
    messageListWidget_->selectAll();
}

    void
EmpathMainWindow::s_editInvertSelection()
{
    messageListWidget_->selectInvert();
}

    bool
EmpathMainWindow::_messageSelected()
{
    if (messageListWidget_->firstSelectedMessage() == EmpathURL("orphaned")) {
        QMessageBox::information(this, "Empath",
            i18n("Please select a message first"), i18n("OK"));
        return false;
    }

    return true;
}

    void
EmpathMainWindow::s_setupDisplay()
{ empath->s_setupDisplay(this); }

        void
EmpathMainWindow::s_setupIdentity()
{ empath->s_setupIdentity(this); }

        void
EmpathMainWindow::s_setupComposing()
{ empath->s_setupComposing(this); }

        void
EmpathMainWindow::s_setupSending()
{ empath->s_setupSending(this); }

        void
EmpathMainWindow::s_setupAccounts()
{ empath->s_setupAccounts(this); }

        void
EmpathMainWindow::s_setupFilters()
{ empath->s_setupFilters(this); }


#include "EmpathMainWindowMenus.cpp"

// vim:ts=4:sw=4:tw=78
