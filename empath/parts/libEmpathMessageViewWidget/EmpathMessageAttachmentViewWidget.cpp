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
#include <klocale.h>
#include <kiconloader.h>
#include <kmimetype.h>
#include <kapp.h>

// Local includes
#include "EmpathMessageAttachmentViewWidget.h"

#include "rmm/ContentType.h"
#include "rmm/ContentDisposition.h"
#include "rmm/ParameterList.h"
#include "rmm/Parameter.h"

EmpathMessageAttachmentViewWidget::EmpathMessageAttachmentViewWidget(
    QWidget * parent
)
    :   QIconView(parent, "EmpathMessageAttachmentViewWidget")
{
    qDebug("Attach view ctor");
    // Empty.
}

EmpathMessageAttachmentViewWidget::~EmpathMessageAttachmentViewWidget()
{
    // Empty.
}

    void
EmpathMessageAttachmentViewWidget::setMessage(RMM::BodyPart & message)
{
    clear();
    
    QList<RMM::BodyPart> body(message.body());

    QListIterator<RMM::BodyPart> it(body);

    ++it; // Ignore first part. That's the main part of the message and
          // not an attachment.
    
    for (; it.current(); ++it) {

        QCString type       = it.current()->envelope().contentType().type();
        QCString subType    = it.current()->envelope().contentType().subType();

        QString typeString = QString::fromUtf8(type + '/' + subType);

        KMimeType::Ptr mimeType = KMimeType::mimeType(typeString);

        QIconViewItem * item = new QIconViewItem(this);
        item->setPixmap(mimeType->pixmap(KIcon::SizeMedium));

        QString text = QString::fromUtf8(
                it.current()->envelope().contentDescription().asString());

        // No Content-Description ?

        if (text.isEmpty()) {
            
            QValueList<RMM::Parameter> l =
                it.current()->envelope().contentType().parameterList().list();

            QValueList<RMM::Parameter>::Iterator it(l.begin());

            for (; it != l.end(); ++it) {

                if ((*it).attribute() == "name") {
                    text = QString::fromUtf8((*it).value());
                    break;
                }
            }
        }
        
        // No 'name=' in Content-Type ?

        if (text.isEmpty()) {

            QString quoted = QString::fromUtf8(
                it.current()->envelope().contentDisposition().filename());

            text = quoted.mid(1, quoted.length() - 2);
        }


        item->setText(text.isEmpty() ? i18n("No description") : text);
    }
}

// vim:ts=4:sw=4:tw=78
