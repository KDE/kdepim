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
# pragma interface "RMM_MessageComponent.h"
#endif

#ifndef RMM_MESSAGE_COMPONENT_H
#define RMM_MESSAGE_COMPONENT_H

#include <qstring.h>
#include <RMM_Defines.h>

namespace RMM {

/**
 * @short Base class of all message components.
 * An RMessageComponent is the base class of all parts of a message.
 * It provides some abstract methods, which need to be implemented by all
 * derived classes.
 * It encapsulates a string representation, which all derived components have.
 * This representation is parsed to create the subcomponents of a component,
 * and assembled from the subcomponents.
 */
class RMessageComponent {

    public:
        

        virtual ~RMessageComponent();

        RMessageComponent & operator = (const RMessageComponent & m);
        RMessageComponent & operator = (const QCString & s);
        
        bool operator == (RMessageComponent &);
        virtual bool operator == (const QCString &);

        virtual void parse()        = 0L;
        virtual void assemble()        = 0L;
        virtual void _parse()        = 0L;
        virtual void _assemble()    = 0L;
        virtual void createDefault()= 0L;

        QCString asString() { assemble(); return strRep_; }

        const char * className() const { return "RMessageComponent"; }
        
    protected:

        RMessageComponent();
        RMessageComponent(const RMessageComponent & component);
        RMessageComponent(const QCString &);

        QCString            strRep_;
        bool                parsed_;
        bool                assembled_;
};

};

#endif
// vim:ts=4:sw=4:tw=78
