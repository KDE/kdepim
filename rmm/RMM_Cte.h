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

#ifndef RMM_CTE_H
#define RMM_CTE_H

#include <qcstring.h>

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

#include "RMM_Cte_generated.h"

    public:
        
        RMM::CteType mechanism();
        void setMechanism(RMM::CteType);
        
    private:

        RMM::CteType mechanism_;
};

}

#endif

// vim:ts=4:sw=4:tw=78
