/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Holger Freyther <zecke@handhelds.org>

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

#include "kontainer.h"

Kontainer::Kontainer(const QString& first,  const QString& second)
{
    m_first = first;
    m_second = second;
}

Kontainer::Kontainer( const Kontainer &tain )
{
    (*this) = tain;
}

Kontainer::~Kontainer()
{
}

QString Kontainer::first() const
{
    return m_first;
}

QString Kontainer::second() const
{
    return m_second;
}

Kontainer &Kontainer::operator=( const Kontainer &con )
{
    m_first = con.m_first;
    m_second = con.m_second;
    return *this;
}

bool operator== ( const Kontainer &a ,  const Kontainer &b )
{
    if ( a.first() == b.first() &&  a.second() == b.second() )
        return true;

    return false;
}
