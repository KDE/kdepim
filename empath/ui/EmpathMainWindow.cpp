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
#include <qlayout.h>
#include <qlabel.h>
#include <qpopupmenu.h>
#include <qwidgetstack.h>

// KDE includes
#include <kconfig.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kglobal.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kprogress.h>
#include <kparts/factory.h>

// Local includes
#include "EmpathMainWindow.h"
#include "EmpathTask.h"
#include "Empath.h"

EmpathMainWindow::EmpathMainWindow()
    :   KParts::MainWindow("EmpathMainWindow")
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

    _initActions();

    KLibFactory * browserFactory =
        KLibLoader::self()->factory("libEmpathBrowser");

    if (0 != browserFactory) {

        browser_ =
            static_cast<KParts::ReadWritePart *>
                (
                    browserFactory->create(
                        this,
                        "empath browser part",
                        "KParts::ReadWritePart"
                    )
                );

    } else {
        
        empathDebug("Argh. Can't load browser part.");
        return;
    }

    setView(browser_->widget());
    createGUI(browser_);

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
EmpathMainWindow::_initActions()
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

// --------------------------------------------------------------------------

EmpathProgressIndicator::EmpathProgressIndicator
    (EmpathTask * t, QWidgetStack * parent)
    :   QWidget(parent, "ProgressIndicator")
{
    parent->addWidget(this, (int)this);

    QHBoxLayout * layout = new QHBoxLayout(this, 0, 6);

    progress_ =
        new KProgress(t->pos(), t->max(), 0, KProgress::Horizontal, this);

    progress_->setFixedWidth(120);

    QLabel * l = new QLabel(t->name(), this);

    layout->addWidget(progress_);
    layout->addWidget(l);
    layout->addStretch(10);

    QObject::connect(
        t,          SIGNAL(posChanged(int)),
        progress_,  SLOT(setValue(int))
    );

    QObject::connect(
        t,          SIGNAL(maxChanged(int)),
        SLOT(s_setMaxValue(int))
    );

    QObject::connect(
        t,          SIGNAL(addOne()),
        this,       SLOT(s_incValue())
    );

    QObject::connect(
        t,          SIGNAL(finished()),
        this,       SLOT(s_finished())
    );

    show();
}

EmpathProgressIndicator::~EmpathProgressIndicator()
{
    // Empty.
}

    void
EmpathProgressIndicator::s_setMaxValue(int v)
{
    progress_->setRange(progress_->minValue(), v);
}

    void
EmpathProgressIndicator::s_incValue()
{
    progress_->advance(1);
}

    void
EmpathProgressIndicator::s_finished()
{
    delete this;
}

// vim:ts=4:sw=4:tw=78
