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

#ifndef RMM_DATE_TIME_H
#define RMM_DATE_TIME_H

#include <qcstring.h>
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

#include "RMM_DateTime_generated.h"

    public:
        
        QDateTime qdt() { parse(); return qdate_; }
        void setTimeZone(const QCString &);
    
        QCString    timeZone();
        Q_UINT32    asUnixTime();

    private:

        QCString    zone_;
        QDateTime   qdate_;
};

}

#endif //RDATETIME_H
// vim:ts=4:sw=4:tw=78
