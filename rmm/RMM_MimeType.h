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

#ifndef RMM_MIMETYPE_H
#define RMM_MIMETYPE_H

#include <qcstring.h>

#include <RMM_HeaderBody.h>
#include <RMM_Parameter.h>
#include <RMM_ParameterList.h>
#include <RMM_Enum.h>

namespace RMM {

class RMimeType : public RHeaderBody {

#include "RMM_MimeType_generated.h"

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
