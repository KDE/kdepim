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
#include "EmpathAttachmentViewWidget.h"

EmpathAttachmentViewWidget::EmpathAttachmentViewWidget(QWidget * parent)
    : QIconView(parent, "EmpathAttachmentViewWidget")
{
}

EmpathAttachmentViewWidget::~EmpathAttachmentViewWidget()
{
    // Empty.
}

    void
EmpathAttachmentViewWidget::setMessage(RMM::RBodyPart & message)
{
    clear();
    
    QList<RMM::RBodyPart> body(message.body());

    QListIterator<RMM::RBodyPart> it(body);
    
    for (; it.current(); ++it) {

        QCString type       = it.current()->envelope().contentType().type();
        QCString subType    = it.current()->envelope().contentType().subType();

        QString typeString = QString::fromUtf8(type + '/' + subType);

        KMimeType::Ptr mimeType = KMimeType::mimeType(typeString);

        QIconViewItem * item = new QIconViewItem(this);
        item->setPixmap(mimeType->pixmap(KIconLoader::Medium));
        QString text = QString::fromUtf8(
                it.current()->envelope().contentDescription().asString());

        item->setText(text.isEmpty() ? i18n("No description") : text);
    }
}

// vim:ts=4:sw=4:tw=78
