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
# pragma interface "EmpathComposeForm.h"
#endif

#ifndef EMPATHCOMPOSEFORM_H
#define EMPATHCOMPOSEFORM_H

// Qt includes
#include <qstring.h>

// Local includes
#include "EmpathAttachmentSpec.h"

/**
 * A composeform is used by the composer + the composer UI.
 */
class EmpathComposeForm 
{
    public:

        enum ComposeType {
            Reply,
            ReplyAll,
            Forward,
            Normal,
            Bounce
        };

        EmpathComposeForm();
        EmpathComposeForm(const EmpathComposeForm &);
        EmpathComposeForm & operator = (const EmpathComposeForm &);

        ~EmpathComposeForm();

        ComposeType                 composeType()       const;
        QMap<QString, QString>      visibleHeaders()    const;
        QMap<QString, QString>      invisibleHeaders()  const;
        QString                     body()              const;
        EmpathAttachmentSpecList    attachmentList()    const;

        void setComposeType         (ComposeType);
        void setVisibleHeaders      (const QMap<QString, QString> &);
        void setInvisibleHeaders    (const QMap<QString, QString> &);
        void setBody                (const QString &);

        void setAttachmentList      (const EmpathAttachmentSpecList &);

        void addAttachment          (const EmpathAttachmentSpec &);
        void removeAttachment       (const EmpathAttachmentSpec &);

        void addVisibleHeader       (const QString & name, const QString &body);
        void addInvisibleHeader     (const QString & name, const QString &body);

        void setHeader(
            const QString & name,
            const QString & body,
            bool vis = true
        );

    private:

        ComposeType                 composeType_;
        QMap<QString, QString>      visibleHeaders_;
        QMap<QString, QString>      invisibleHeaders_;
        QString                     body_;
        EmpathAttachmentSpecList    attachments_;
};

#endif
// vim:ts=4:sw=4:tw=78
