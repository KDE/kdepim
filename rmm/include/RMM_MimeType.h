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
# pragma interface "RMM_MimeType.h"
#endif

#ifndef RMM_MIMETYPE_H
#define RMM_MIMETYPE_H

#include <qstring.h>
#include <qlist.h>

#include <RMM_HeaderBody.h>
#include <RMM_Parameter.h>
#include <RMM_ParameterList.h>
#include <RMM_Enum.h>
#include <RMM_Defines.h>

namespace RMM {

class RMimeType : public RHeaderBody {

#include "generated/RMimeType_generated.h"

    public:

        QCString boundary();
        QCString name();

        RMM::MimeType type();
        RMM::MimeSubType subType();

        void setType(RMM::MimeType);
        void setType(const QCString &);
        void setSubType(RMM::MimeSubType);
        void setSubType(const QCString &);

        void setBoundary(const QCString & boundary);
        void setName(const QCString & name);

    private:

        QCString            boundary_;
        QCString            name_;
        
        RMM::MimeType       type_;
        RMM::MimeSubType    subType_;
        
        RParameterList      parameterList_;
};

}

#endif
// vim:ts=4:sw=4:tw=78
