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

#ifndef RMM_ENTITY_H
#define RMM_ENTITY_H

#include <qcstring.h>
#include <RMM_MessageComponent.h>

namespace RMM {

/**
 * @short An REntity is the base class of an RBodyPart and an RMessage.
 * An REntity is the base class of an RBodyPart and an RMessage. Note that the
 * RFC822 specification is recursive. That means that an RBodyPart can also be
 * an RMessage, which then in turn contains an RBodyPart !
 */
class REntity : public RMessageComponent {

    public:

        REntity() : RMessageComponent() {}
        REntity(const REntity & e)  : RMessageComponent(e) {}
        REntity(const QCString & s) : RMessageComponent(s) {}
        virtual ~REntity() {}

        virtual const char * className() const { return "REntity"; }
};

}

#endif
// vim:ts=4:sw=4:tw=78
