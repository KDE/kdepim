/*
    This file is part of libkcal.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KCAL_FREEBUSYURLSTORE_H
#define KCAL_FREEBUSYURLSTORE_H

#include <qstring.h>

#include "libkcal_export.h"

class KConfig;

namespace KCal {

class LIBKCAL_EXPORT FreeBusyUrlStore
{
  public:
    static FreeBusyUrlStore *self();
    ~FreeBusyUrlStore();

    void writeUrl( const QString &email, const QString &url );

    QString readUrl( const QString &email );
  
    void sync();
  
  private:
    FreeBusyUrlStore();
  
    static FreeBusyUrlStore *mSelf;

    KConfig *mConfig;
};

}

#endif
