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
#include <qlcdnumber.h>
#include <qpopupmenu.h>
#include <qwidgetstack.h>

// KDE includes
#include <kapp.h>
#include <kconfig.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kglobal.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kprogress.h>
#include <kparts/factory.h>

// Local includes
#include "EmpathSettingsDialog.h"
#include "EmpathMainWindow.h"
#include "EmpathTask.h"
#include "EmpathFolderCombo.h"
#include "Empath.h"

class EmpathProgressStack : public QWidgetStack
{
    public:

        EmpathProgressStack(QStatusBar * parent)
            :   QWidgetStack(parent)
        {
        }

        QSize sizeHint() const
        { return QSize(QWidgetStack::sizeHint().width(), 16); }

        QSize minimumSizeHint() const { return sizeHint(); }
};


EmpathMainWindow::EmpathMainWindow()
    :   KParts::MainWindow("EmpathMainWindow")
{
    // Resize to previous size.
   
    KConfig * c = KGlobal::config();

    c->setGroup("Display");
    
    int x = c->readNumEntry("MainWindowWidth", 600);
    int y = c->readNumEntry("MainWindowHeight", 400);

    resize(x, y);
    
    progressStack_ = new EmpathProgressStack(statusBar());
    statusBar()->addWidget(progressStack_, 0, true);
    statusBar()->show();

    QObject::connect(
        empath, SIGNAL(newTask(EmpathTask *)),
        this,   SLOT(s_newTask(EmpathTask *)));

    setXMLFile("EmpathMainWindow.rc");

    _initActions();

    KLibFactory * viewFactory =
        KLibLoader::self()->factory("libEmpathView");

    if (0 != viewFactory) {

        view_ =
            static_cast<KParts::ReadWritePart *>
                (
                    viewFactory->create(
                        this,
                        "empath view part",
                        "KParts::ReadWritePart"
                    )
                );

    } else {
        
        empathDebug("Argh. Can't load view part.");
        return;
    }

    setCentralWidget(view_->widget());
    createGUI(view_);

    toolBar()->insertSeparator();
    EmpathFolderCombo * fc = new EmpathFolderCombo(toolBar());
    toolBar()->insertWidget(0, 200, fc);

    connect(fc,         SIGNAL(folderSelected(const EmpathURL &)),
            view_,   SLOT(s_showFolder(const EmpathURL &)));

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
    (void) new EmpathProgressIndicator(t, progressStack_);
    progressStack_->show();
}

    void
EmpathMainWindow::s_toolbarMoved(int pos)
{
    KConfig * c = KGlobal::config();
    
    c->setGroup("Display");
    c->writeEntry("MainWindowToolbarPos", pos);
}

    void
EmpathMainWindow::_initActions()
{
    KStdAction::quit(
        this,
        SLOT(quit()),
        actionCollection(),
        "quit"
    );

    KStdAction::preferences(
        this,
        SLOT(s_settings()),
        actionCollection(),
        "config"
    );
}

    void
EmpathMainWindow::s_settings()
{
    EmpathSettingsDialog::run();
}

    void
EmpathMainWindow::quit()
{
    kapp->quit();
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
#include "EmpathMainWindow.moc"
