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
#ifndef KCAL_PERSON_H
#define KCAL_PERSON_H

#include <qstring.h>

#include "libkcal_export.h"

namespace KCal {

/**
  This class represents a person. A person has a name and an email address.
*/
class LIBKCAL_EXPORT Person
{
  public:
    Person() {}
    Person( const QString &fullName );
    Person( const QString &name, const QString &email );
    bool isEmpty() const;

    QString fullName( ) const;

    void setName(const QString &);
    QString name() const { return mName; }
    
    void setEmail(const QString &);
    QString email() const { return mEmail; }

  private:
    QString mName;
    QString mEmail;

    class Private;
    Private *d;
};

bool operator==( const Person& p1, const Person& p2 );

}

#endif
