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
#include <qpopupmenu.h>
#include <qwidgetstack.h>

// KDE includes
#include <kconfig.h>
#include <kglobal.h>
#include <kaction.h>
#include <kstdaction.h>

// Local includes
#include "EmpathTask.h"
#include "EmpathMainWindow.h"
#include "EmpathMainWidget.h"
#include "EmpathProgressIndicator.h"
#include "Empath.h"

EmpathMainWindow::EmpathMainWindow()
    :   KParts::MainWindow("MainWindow")
{
    // Resize to previous size.
   
    KConfig * c = KGlobal::config();

    c->setGroup("Display");
    
    int x = c->readNumEntry("MainWindowWidth", 600);
    int y = c->readNumEntry("MainWindowHeight", 400);

    resize(x, y);
    
    progressStack_ = new QWidgetStack(statusBar());
    statusBar()->addWidget(progressStack_, width());
    statusBar()->show();

    progressStack_->hide();

    QObject::connect(
        empath, SIGNAL(newTask(EmpathTask *)),
        this,   SLOT(s_newTask(EmpathTask *)));

    setXMLFile("EmpathMainWindow.rc");

    _setupActions();

    mainWidget_ = new EmpathMainWidget(this);
    setView(mainWidget_);

    createGUI(mainWidget_->messageListWidget());
    createGUI(mainWidget_->messageViewWidget());

    show();
}

EmpathMainWindow::~EmpathMainWindow()
{
    KConfig * c = KGlobal::config();

    c->setGroup("Display");
    
    c->writeEntry("MainWindowWidth", width());
    c->writeEntry("MainWindowHeight", height());
}

    void
EmpathMainWindow::s_newTask(EmpathTask * t)
{
    new EmpathProgressIndicator(t, progressStack_);
    progressStack_->show();
}

    void
EmpathMainWindow::s_toolbarMoved(BarPosition pos)
{
    KConfig * c = KGlobal::config();
    
    c->setGroup("Display");
    c->writeEntry("MainWindowToolbarPos", (int)pos);
}

    void
EmpathMainWindow::_setupActions()
{
    KStdAction::preferences(
        this,
        SLOT(s_settings()),
        actionCollection(),
        "settings"
    );

    KStdAction::saveOptions(
        this,
        SLOT(s_saveSettings()),
        actionCollection(),
        "saveSettings"
    );
}

// vim:ts=4:sw=4:tw=78
