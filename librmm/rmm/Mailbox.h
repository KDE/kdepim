/* This file is part of the KDE project

   Copyright (C) 1999, 2000 Rik Hemsley <rik@kde.org>
             (C) 1999, 2000 Wilco Greven <j.w.greven@student.utwente.nl>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef RMM_MAILBOX_H
#define RMM_MAILBOX_H

#include <qcstring.h>
#include <rmm/MessageComponent.h>

namespace RMM {

/**
 * A Mailbox holds either a (phrase route-addr) or (localpart domain).
 * (localpart domain) is called an addr-spec by RFC822. You can see which type
 * this is by calling phrase().isEmpty(). If it's empty, you have an
 * addr-spec.
 */
class Mailbox : public MessageComponent {

#include "rmm/Mailbox_generated.h"
        
    public:

        void setPhrase(const QCString &);
        void setRoute(const QCString &);
        void setLocalPart(const QCString &);
        void setDomain(const QCString &);

        QCString phrase();
        QCString route();
        QCString localPart();
        QCString domain();

    private:

        QCString phrase_;
        QCString route_;
        QCString localPart_;
        QCString domain_;
};

}


#endif //RMAILBOX_H
// vim:ts=4:sw=4:tw=78
