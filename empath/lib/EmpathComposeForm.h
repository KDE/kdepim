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
#include <qcstring.h>
#include <qvaluelist.h>

// Local includes
#include "EmpathEnum.h"
#include "EmpathAttachmentSpec.h"
#include <RMM_Envelope.h>
#include <RMM_Header.h>

/**
 * A composeform is used by the composer UI.
 */
class EmpathComposeForm 
{
    public:

        EmpathComposeForm();
        ~EmpathComposeForm();
        EmpathComposeForm(const EmpathComposeForm &);
        EmpathComposeForm & operator = (const EmpathComposeForm &);

        ComposeType     composeType()       const;
        RMM::REnvelope  visibleHeaders()    const;
        RMM::REnvelope  invisibleHeaders()  const;
        QCString        body()              const;
        EmpathAttachmentSpecList attachmentList() const;

        void setComposeType         (ComposeType);
        void setVisibleHeaders      (RMM::REnvelope);
        void setInvisibleHeaders    (RMM::REnvelope);
        void setBody                (const QCString &);
        void setAttachmentList      (EmpathAttachmentSpecList);

        void addAttachment          (EmpathAttachmentSpec);
        void removeAttachment       (EmpathAttachmentSpec);

        void addVisibleHeader       (RMM::RHeader);
        void addInvisibleHeader     (RMM::RHeader);

        void setHeader(QCString name, QCString body, bool vis = true);

    private:

        ComposeType                 composeType_;
        RMM::REnvelope              visibleHeaders_;
        RMM::REnvelope              invisibleHeaders_;
        QCString                    body_;
        EmpathAttachmentSpecList    attachments_;
};

#endif
// vim:ts=4:sw=4:tw=78
