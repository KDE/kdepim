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
# pragma interface "RMM_DateTime.h"
#endif

#ifndef RMM_RDATETIME_H
#define RMM_RDATETIME_H

#include <qstring.h>
#include <qdatastream.h>
#include <qdatetime.h>

#include <RMM_Defines.h>
#include <RMM_HeaderBody.h>
#include <RMM_Enum.h>

namespace RMM {

/**
 * An RDateTime encapsulates a time value. It is basically a QDateTime with an
 * added 'asUnixTime()' method and knowledge of time zones.
 */ 
class RDateTime : public RHeaderBody {

    public:
        
#include "generated/RDateTime_generated.h"

        friend QDataStream & operator >> (QDataStream & s, RDateTime & dt);
        friend QDataStream & operator << (QDataStream & s, RDateTime & dt);

        QDateTime qdt() { parse(); return qdate_; }
        void    setTimeZone    (const QCString &);
    
        QCString     timeZone();
        Q_UINT32    asUnixTime();

    private:

        QCString    zone_;
        QDateTime    qdate_;
        
        bool        parsed_;
        bool        assembled_;
};

}

#endif //RDATETIME_H
// vim:ts=4:sw=4:tw=78
