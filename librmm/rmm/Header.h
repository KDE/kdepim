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

#ifndef RMM_HEADER_H
#define RMM_HEADER_H

#include <qlist.h>

#include <rmm/Enum.h>
#include <rmm/HeaderBody.h>
#include <rmm/Defines.h>

namespace RMM {

/**
 * A Header encapsulates an header name and a HeaderBody.
 */
class Header : public MessageComponent
{

#include "rmm/Header_generated.h"
        
    public:
        
        QCString headerName();
        RMM::HeaderType headerType();
        HeaderBody * headerBody();

        void setName(const QCString & name);
        void setType(RMM::HeaderType t);
        void setBody(HeaderBody * b);

    private:
 
        HeaderBody * _newHeaderBody(RMM::HeaderType);
        void _replaceHeaderBody(RMM::HeaderType, HeaderBody *);
        
        QCString        headerName_;
        RMM::HeaderType headerType_;
        HeaderBody *   headerBody_;
};

typedef QList<Header> HeaderList;
typedef QListIterator<Header> HeaderListIterator;

}

#endif

// vim:ts=4:sw=4:tw=78
