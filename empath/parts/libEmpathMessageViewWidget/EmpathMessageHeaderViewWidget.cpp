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
#include <qstrlist.h>

// KDE includes
#include <kconfig.h>
#include <klocale.h>
#include <kglobal.h>

// Local includes
#include "EmpathMessageHeaderViewWidget.h"

#include <rmm/Header.h>
#include <rmm/DateTime.h>

EmpathMessageHeaderViewWidget::EmpathMessageHeaderViewWidget(QWidget * parent)
    :   QLabel(parent, "EmpathMessageHeaderViewWidget")
{
    qDebug("Header view ctor");
    setTextFormat(Qt::RichText);
}

EmpathMessageHeaderViewWidget::~EmpathMessageHeaderViewWidget()
{
    // Empty.
}

    void
EmpathMessageHeaderViewWidget::useEnvelope(RMM::Envelope & e)
{
    KConfig * c(KGlobal::config());

    c->setGroup("EmpathMessageHeaderViewWidget");
    
    QStrList l;

    c->readListEntry("VisibleHeaders", l, ',');
    
    if (l.isEmpty()) {
        // TODO: Take this out. The user might really want no headers.
        l.append("From");
        l.append("Date");
        l.append("Subject");
    }

    qDebug("%d headers to view", l.count());

    QString text;

    QStrListIterator it(l);
    
    for (; it.current() ; ++it) {
        
        QCString s = it.current();
        s = s.stripWhiteSpace();
    
        RMM::Header h(e.get(s));

        text += "<b>";
        text += QString::fromUtf8(h.headerName()) + ": ";
        text += "</b>";
    
        if (RMM::headerTypesTable[h.headerType()] == RMM::ClassDateTime) {

            RMM::DateTime * date = static_cast<RMM::DateTime *>(h.headerBody());

            text += KGlobal::locale()->formatDateTime(date->qdt());

        } else
            // FIXME
            text += QString::fromUtf8(h.headerBody()->asString());

        text += "<br>";
    }

    setText(text);
}

// vim:ts=4:sw=4:tw=78
