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
# pragma implementation "EmpathMainWindow.h"
#endif

// Qt includes
#include <qpopupmenu.h>
#include <qwidgetstack.h>

// KDE includes
#include <klocale.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kmenubar.h>
#include <ktoolbar.h>

// Local includes
#include "EmpathTask.h"
#include "EmpathMainWidget.h"
#include "EmpathMainWindow.h"
#include "EmpathMessageListWidget.h"
#include "EmpathProgressIndicator.h"
#include "EmpathUI.h"
#include "EmpathConfig.h"
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
    
    progressStack_ = new QWidgetStack(statusBar());
    statusBar()->insertWidget(progressStack_, width(), 0);
    statusBar()->show();

    progressStack_->hide();

    QObject::connect(
        empath, SIGNAL(newTask(EmpathTask *)),
        this,   SLOT(s_newTask(EmpathTask *)));

    mainWidget_ = new EmpathMainWidget(this);

    setView(mainWidget_, false);
   
    _setupActions();

    empath->s_infoMessage(i18n("Welcome to Empath"));
    
    updateRects();

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
EmpathMainWindow::s_newTask(EmpathTask * t)
{
    new EmpathProgressIndicator(t, progressStack_);
    progressStack_->show();
}

    void
EmpathMainWindow::statusMessage(const QString & messageText, int seconds)
{
    statusBar()->message(messageText, seconds);
}

    void
EmpathMainWindow::clearStatusMessage()
{
    statusBar()->clear();
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
EmpathMainWindow::_setupActions()
{
#define PLUGMSGACTION(a) \
    actionCollection()->insert(mainWidget_->messageListWidget()->actionCollection()->action(a)) 

    PLUGMSGACTION("messageCompose");
    PLUGMSGACTION("messageView");
    PLUGMSGACTION("messageTag");
    PLUGMSGACTION("messageMarkRead");
    PLUGMSGACTION("messageMarkReplied");
    PLUGMSGACTION("messageReply");
    PLUGMSGACTION("messageReplyAll");
    PLUGMSGACTION("messageForward");
    PLUGMSGACTION("messageDelete");
    PLUGMSGACTION("messageSaveAs");
    PLUGMSGACTION("messageFilter");

    PLUGMSGACTION("goPrevious");
    PLUGMSGACTION("goNext");
    PLUGMSGACTION("goNextUnread");

#undef PLUGMSGACTION

    createGUI("EmpathMainWindow.rc");
}

// vim:ts=4:sw=4:tw=78
