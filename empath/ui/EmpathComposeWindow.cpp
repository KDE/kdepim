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

// KDE includes
#include <kapp.h>
#include <kconfig.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kglobal.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kparts/factory.h>

// Local includes
#include "EmpathComposeWindow.h"
#include "EmpathComposePart.h"
#include "Empath.h"

EmpathComposeWindow::EmpathComposeWindow()
    :   KParts::MainWindow("EmpathComposeWindow")
{
    setXMLFile("EmpathComposeWindow.rc");

    _initActions();

    KLibFactory * viewFactory =
        KLibLoader::self()->factory("libEmpathComposePart");

    if (0 != viewFactory) {

        view_ =
            static_cast<EmpathComposePart *>
                (
                    viewFactory->create(
                        this,
                        "empath view part",
                        "KParts::ReadWritePart"
                    )
                );

        CHECK_PTR(view_);

    } else {
        
        empathDebug("Argh. Can't load view part.");
        return;
    }

    setCentralWidget(view_->widget());
    createGUI(view_);

    show();
}

EmpathComposeWindow::~EmpathComposeWindow()
{
    KConfig * c = KGlobal::config();

    c->setGroup("Display");
    
    c->writeEntry("ComposeWindowWidth", width());
    c->writeEntry("ComposeWindowHeight", height());
}

    void
EmpathComposeWindow::s_toolbarMoved(int pos)
{
    KConfig * c = KGlobal::config();
    
    c->setGroup("Display");
    c->writeEntry("ComposeWindowToolbarPos", pos);
}

    void
EmpathComposeWindow::_initActions()
{
}

    void
EmpathComposeWindow::setComposeForm(const EmpathComposeForm & form)
{
    view_->setForm(form);
}

// vim:ts=4:sw=4:tw=78
#include "EmpathComposeWindow.moc"
