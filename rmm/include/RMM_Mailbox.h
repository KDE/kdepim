/*
    Empath - Mailer for KDE
    
    Copyright (C) 1998, 1999 Rik Hemsley rik@kde.org
    
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
# pragma interface "RMM_Mailbox.h"
#endif

#ifndef RMM_RMAILBOX_H
#define RMM_RMAILBOX_H

#include <qcstring.h>
#include <RMM_MessageComponent.h>

namespace RMM {

/**
 * An RMailbox holds either a (phrase route-addr) or (localpart domain).
 * (localpart domain) is called an addr-spec by RFC822.
 * You can see which type this is by calling phrase().isEmpty(). If it's empty,
 * you have an addr-spec.
 */
class RMailbox : public RMessageComponent {

#include "generated/RMailbox_generated.h"
        
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
