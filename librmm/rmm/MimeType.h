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

#include <rmm/HeaderBody.h>
#include <rmm/Parameter.h>
#include <rmm/ParameterList.h>
#include <rmm/Enum.h>

namespace RMM {

class MimeType : public HeaderBody {

#include "rmm/MimeType_generated.h"

    public:

        QCString boundary();
        QCString name();

        RMM::MimeGroup group();
        RMM::MimeValue value();

        void setGroup(RMM::MimeGroup);
        void setGroup(const QCString &);
        void setValue(RMM::MimeValue);
        void setValue(const QCString &);

        void setBoundary(const QCString & boundary);
        void setName(const QCString & name);

    private:

        QCString            boundary_;
        QCString            name_;
        
        RMM::MimeGroup      group_;
        RMM::MimeValue      value_;
        
        ParameterList      parameterList_;
};

}

#endif
// vim:ts=4:sw=4:tw=78
