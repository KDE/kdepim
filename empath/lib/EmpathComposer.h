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


#ifndef EMPATHCOMPOSER_H
#define EMPATHCOMPOSER_H

// Qt includes
#include <qobject.h>
#include <qcstring.h>
#include <qmap.h>

// Local includes
#include "EmpathURL.h"
#include "EmpathEnum.h"
#include "EmpathJob.h"
#include "EmpathComposeForm.h"
#include <rmm/Message.h>

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

        enum HeaderStyle { Visible, Invisible };

        static EmpathComposer * instance()
        {
            if (0 == THIS)
                THIS = new EmpathComposer;

            return THIS;
        }

        /**
         * Create a new composeform with the given parameters 
         */
        void newComposeForm(const QString & recipient);
        void newComposeForm(
            EmpathComposeForm::ComposeType,
            const EmpathURL &
        );

        /**
         * Convert a composeform to a message, so that it can be sent.
         */
        RMM::Message message(EmpathComposeForm);

        static EmpathComposer * THIS;

    protected:

        EmpathComposer();
        ~EmpathComposer();

        bool event(QEvent *);

    signals:

        /**
         * Signals that a composeform is ready to be handled by the composer
         * UI.
         */
        void composeFormComplete(const EmpathComposeForm &);

    private:

        void _reply     (EmpathJobID, RMM::Message);
        void _forward   (EmpathJobID, RMM::Message);
        void _bounce    (EmpathJobID, RMM::Message);

        void _initVisibleHeaders(EmpathComposeForm &);

        QString _referenceHeaders(RMM::Message m);
        QString _stdHeaders();

        QString _signature();

        QMap<EmpathJobID, EmpathComposeForm> jobList_;

        QString referenceHeaders_;

        void _quote(QString &);
};

#endif
// vim:ts=4:sw=4:tw=78
