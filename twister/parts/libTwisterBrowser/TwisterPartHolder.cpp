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

// Qt includes
#include <qlabel.h>

// KDE includes
#include <kconfig.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kglobal.h>
#include <kaction.h>
#include <kinstance.h>
#include <kparts/part.h>
#include <klibloader.h>

// Local includes
#include "TwisterPartHolder.h"

TwisterPartHolder::TwisterPartHolder(QWidget * parent)
    :   QWidgetStack(parent, "TwisterPartHolder"),
        mailWidget_(0L),
        calendarWidget_(0L)
{
}

TwisterPartHolder::~TwisterPartHolder()
{
    // Empty.
}

    void
TwisterPartHolder::s_switchWidget(const QString & name)
{
    qDebug("TwisterPartHolder::s_switchWidget(%s)", name.ascii());
    if (name == "Inbox") {

        if (0 == mailWidget_)
            _loadMailWidget();

        raiseWidget(mailWidget_);
    }

    else if (name == "Calendar") {

        if (0 == calendarWidget_)
            _loadCalendarWidget();

        raiseWidget(calendarWidget_);
    }

    else
        qDebug("Don't know about a widget called %s", name.ascii());
}

    void
TwisterPartHolder::_loadMailWidget()
{
    KLibFactory * mailFactory =
        KLibLoader::self()->factory("libEmpathBrowser");

    if (0 != mailFactory) {

        KParts::ReadWritePart * mailPart =
            static_cast<KParts::ReadWritePart *>
                (
                    mailFactory->create(
                        this,
                        "mail browser part",
                        "KParts::ReadWritePart"
                    )
                );
    
        mailWidget_ = mailPart->widget();

    } else {
        
        qDebug("Argh. Can't load mail part.");
        return;
    }
}

    void
TwisterPartHolder::_loadCalendarWidget()
{
    calendarWidget_ =
        new QLabel(
            "This will be KOrganizer's calendar view when it's a kpart",
            this
        );
}


// vim:ts=4:sw=4:tw=78
#include "TwisterPartHolder.moc"
