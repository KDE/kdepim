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
# pragma interface "RMM_DispositionType.h"
#endif

#ifndef RMM_RDISPOSITIONTYPE_H
#define RMM_RDISPOSITIONTYPE_H

#include <qcstring.h>

#include <RMM_Enum.h>
#include <RMM_HeaderBody.h>
#include <RMM_Defines.h>
#include <RMM_Parameter.h>
#include <RMM_ParameterList.h>

namespace RMM {

class RDispositionType : public RHeaderBody {

    public:
        
#include "generated/RDispositionType_generated.h"

        QCString            filename();
        void                setFilename(const QCString &);
        void                addParameter(RParameter & p);
        RParameterList &    parameterList();
        RMM::DispType        type();

    private:

        RParameterList        parameterList_;
        RMM::DispType        dispType_;
        QCString            filename_;
};

};

#endif //RDISPOSITIONTYPE_H
// vim:ts=4:sw=4:tw=78
