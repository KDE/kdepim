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

#ifndef RMM_MESSAGE_COMPONENT_H
#define RMM_MESSAGE_COMPONENT_H

#include <qcstring.h>

namespace RMM {

/**
 * @short Base class of all message components.
 * A MessageComponent is the base class of all parts of a message. It provides
 * some abstract methods, which need to be implemented by all derived classes.
 * It encapsulates a string representation, which all derived components have.
 * This representation is parsed to create the subcomponents of a component,
 * and assembled from the subcomponents.
 */
class MessageComponent {

    public:

        virtual ~MessageComponent()
        {
            // Empty.
        }

        MessageComponent & operator = (const MessageComponent & m)
        {
            if (this == &m) return *this;    // Avoid a = a.
            assembled_  = m.assembled_;
            parsed_     = m.parsed_;
            strRep_     = m.strRep_.copy();
            return *this;
        }

        MessageComponent & operator = (const QCString & s)
        {
            strRep_ = s.copy();
            parsed_ = assembled_ = false;
            return *this;
        }

        bool operator == (MessageComponent & mc)
        {
            assemble();
            return (strRep_ == mc.asString());
        }

        virtual bool operator == (const QCString & s)
        {
            assemble();
            return (strRep_ == s);
        }

        void parse() 
            { if (!parsed_) { _parse(); parsed_ = true; assembled_ = false; } }
        void assemble() 
            { parse(); if (!assembled_) { _assemble(); assembled_ = true; } }

        virtual void createDefault()    = 0L;

        QCString asString() { assemble(); return strRep_; }

        virtual const char * className() const { return "MessageComponent"; }
        
    protected:

        MessageComponent()
            :   strRep_     (""),
                parsed_     (false),
                assembled_  (false)
        {
            // Empty.
        }

        MessageComponent(const MessageComponent & mc)
            :   strRep_     (mc.strRep_.copy()),
                parsed_     (mc.parsed_),
                assembled_  (mc.assembled_)
        {
            // Empty.
        }

        MessageComponent(const QCString & s)
            :   strRep_     (s.copy()),
                parsed_     (false),
                assembled_  (false)
        {
        }

        virtual void _parse()       = 0L;
        virtual void _assemble()    = 0L;

        QCString            strRep_;
        bool                parsed_;
        bool                assembled_;
};

};

#endif
// vim:ts=4:sw=4:tw=78
