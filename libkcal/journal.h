/*
    This file is part of libkcal.

    Copyright (c) 2001-2003 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
#ifndef KCAL_JOURNAL_H
#define KCAL_JOURNAL_H

#include "incidence.h"
#include <kdepimmacros.h>

namespace KCal {

/**
  This class provides a Journal in the sense of RFC2445.
*/
class KDE_EXPORT Journal : public Incidence
{
  public:
    typedef ListBase<Journal> List;
  
    Journal();
    ~Journal();
    bool accept( Visitor &v ) { return v.visit(this); }
    bool operator==( const Journal& ) const;

    QCString type() const { return "Journal"; }
    
    /**
      Return copy of this Journal. The caller owns the returned object.
    */
    Journal *clone();

  private:
    class Private;
    Private *d;
};

}

#endif
