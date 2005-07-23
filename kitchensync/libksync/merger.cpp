/*
    This file is part of KitchenSync.

    Copyright (c) 2003,2004 Holger Freyther <zecke@handhelds.org>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "merger.h"

#include "syncee.h"

namespace KSync {
Merger::Merger()
{}

Merger::~Merger()
{}

QString Merger::synceeType()const
{
  return mString;
}

void Merger::setSynceeType( const QString& str )
{
  mString = str;
}

bool Merger::sameType( SyncEntry* e1, SyncEntry* e2 )
{
  return ( e1->type() == e2->type() );
}

bool Merger::sameType( SyncEntry* e1, SyncEntry* e2, const QString& wish )
{
  return sameType( e1, e2 ) && e1->type() == wish;
}

}


