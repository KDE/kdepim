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

#ifndef RMM_MESSAGE_H
#define RMM_MESSAGE_H

#include <RMM_Entity.h>
#include <RMM_Envelope.h>
#include <RMM_BodyPart.h>
#include <RMM_Defines.h>
#include <RMM_Enum.h>

namespace RMM {

class RMessage : public RBodyPart {

#include "RMM_Message_generated.h"

    public:
        
        QCString recipientListAsPlainString();

        void        addPart(RBodyPart & bp);
        void        removePart(RBodyPart & part);
        
        bool hasParentMessageID();

        void setStatus(RMM::MessageStatus status);
        RMM::MessageStatus status();
        
    protected:
        
        RMM::MessageStatus    status_;
};

}

#endif

// vim:ts=4:sw=4:tw=78
