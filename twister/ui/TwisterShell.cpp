/*
    Twister - PIM app for KDE
    
    Copyright 2000
        Rik Hemsley <rik@kde.org>
    
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
#include "TwisterShell.h"

TwisterShell::TwisterShell()
    :   KParts::MainWindow("TwisterShell")
{
    // Resize to previous size.
   
    KConfig * c = KGlobal::config();

    c->setGroup("TwisterDisplay");
    
    int x = c->readNumEntry("MainWindowWidth", 600);
    int y = c->readNumEntry("MainWindowHeight", 400);

    resize(x, y);
    
    statusBar()->show();

    setXMLFile("TwisterShell.rc");

    _initActions();

    KLibFactory * browserFactory =
        KLibLoader::self()->factory("libTwisterBrowser");

    if (0 != browserFactory) {

        browser_ =
            static_cast<KParts::ReadWritePart *>
                (
                    browserFactory->create(
                        this,
                        "twister browser part",
                        "KParts::ReadWritePart"
                    )
                );

    } else {
        
        qDebug("Argh. Can't load browser part.");
        return;
    }

    setView(browser_->widget());
    createGUI(browser_);

    show();
}

TwisterShell::~TwisterShell()
{
    KConfig * c = KGlobal::config();

    c->setGroup("TwisterDisplay");
    
    c->writeEntry("MainWindowWidth", width());
    c->writeEntry("MainWindowHeight", height());
}

    void
TwisterShell::_initActions()
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
#include "TwisterShell.moc"
