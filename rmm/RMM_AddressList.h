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

#ifndef RMM_ADDRESS_LIST_H
#define RMM_ADDRESS_LIST_H

#include <qcstring.h>
#include <qvaluelist.h>
#include <RMM_Address.h>
#include <RMM_Defines.h>

namespace RMM {

/**
 * @short Simple encapsulation of a list of RAddress, which is also an
 * RHeaderBody.
 */
class RAddressList : public RHeaderBody {

#include "RMM_AddressList_generated.h"

    public:
        
        RAddress at(unsigned int);
        unsigned int count();

    private:
        
        QValueList<RAddress> list_;
};

}

#endif //RADDRESSLIST_H
// vim:ts=4:sw=4:tw=78
