/*
    This file is part of libkcal.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef JOURNAL_H
#define JOURNAL_H
//
// Journal component, representing a VJOURNAL object
//

#include "incidence.h"

namespace KCal {

/**
  This class provides a Journal in the sense of RFC2445.
*/
class Journal : public Incidence
{
  public:
    typedef ListBase<Journal> List;
  
    Journal();
    ~Journal();

    QCString type() const { return "Journal"; }
    
    Journal *clone();
};

    bool operator==( const Journal&, const Journal& );
}

#endif
