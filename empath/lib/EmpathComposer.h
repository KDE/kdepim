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
# pragma interface "EmpathComposer.h"
#endif

#ifndef EMPATHCOMPOSER_H
#define EMPATHCOMPOSER_H

// Qt includes
#include <qobject.h>
#include <qcstring.h>
#include <qvaluelist.h>
#include <qmap.h>

// Local includes
#include "EmpathURL.h"
#include "EmpathDefines.h"
#include "EmpathJobInfo.h"
#include "EmpathAttachmentSpec.h"
#include <RMM_Envelope.h>
#include <RMM_Message.h>

/**
 * @short Composer backend
 * 
 * EmpathComposer creates composeforms which can be handled by the composer UI.
 * A composeform contains all the information necessary to compose a message.
 * When replying to a message for example, the body already contains the
 * quoted message.
 * 
 * @authors Wilco Greven, Rik Hemsley.
 */
class EmpathComposer : public QObject
{
    Q_OBJECT
    
    public:

        enum ComposeType 
        {
            ComposeReply,
            ComposeReplyAll,
            ComposeForward,
            ComposeBounce,
            ComposeNormal
        };
 
        /**
         * A composeform is used by the composer UI.
         */
        struct Form 
        {
            ComposeType                         composeType;
            RMM::REnvelope                      visibleHeaders;
            RMM::REnvelope                      invisibleHeaders;
            QCString                            body;
            QValueList<EmpathAttachmentSpec>    attachments;
        };

        /**
         * ctor
         */
        EmpathComposer();

        /**
         * dtor
         */
        ~EmpathComposer();
       
        /**
         * Create a new composeform with the given parameters 
         */
        void newComposeForm(const QString & recipient);
        void newComposeForm(const ComposeType, const EmpathURL &);
        
        /**
         * Create a composeform for sending a bugreport.
         */
        void bugReport();
        
        /**
         * Convert a composeform to a message, so that it can be sent.
         */
        RMM::RMessage message(Form &);
 
    signals:

        /**
         * Signals that a composeform is ready to be handled by the composer
         * UI.
         */
        void composeFormComplete(EmpathComposer::Form);
        
    protected slots:

        void s_jobComplete(EmpathJobInfo);
   
    private:
        
        void _reply     (int id, RMM::RMessage * m);
        void _forward   (int id, RMM::RMessage * m);
        void _bounce    (int id, RMM::RMessage * m);

        void _initVisibleHeaders(Form &);
        
        QCString _referenceHeaders(RMM::RMessage * m);
        QCString _stdHeaders();

        QCString _signature();
        
        QMap<int, Form> jobs_;
       
        QCString referenceHeaders_;

        void _quote(QCString &);
};

#endif
// vim:ts=4:sw=4:tw=78
