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
# pragma interface "RMM_Cte.h"
#endif

#ifndef RMM_CTE_H
#define RMM_CTE_H

#include <qstring.h>
#include <qlist.h>

#include <RMM_Defines.h>
#include <RMM_Enum.h>
#include <RMM_Parameter.h>
#include <RMM_HeaderBody.h>

namespace RMM {

/**
 * An RCte holds a Content-Transfer-Encoding header body. It contains a
 * mechanism. This is likely to be "7bit, "quoted-printable", "base64", "8bit",
 * "binary" or an 'x-token'. An x-token is an extension token and is prefixed
 * by 'X-'.
 */
class RCte : public RHeaderBody {

#include "generated/RCte_generated.h"

    public:
        
        RMM::CteType mechanism();
        void setMechanism(RMM::CteType);
        
    private:

        RMM::CteType mechanism_;
};

}

#endif

// vim:ts=4:sw=4:tw=78
