/*
    This file is part of libkabc and/or kaddressbook.
    Copyright (c) 2004 Klarälvdalens Datakonsult AB
        <info@klaralvdalens-datakonsult.se>

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

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include "subresource.h"

using namespace Kolab;

SubResource::SubResource( bool active, bool writable, const QString& label,
                          int completionWeight )
  : mActive( active ),  mWritable( writable ), mLabel( label ),
    mCompletionWeight( completionWeight )
{
}

SubResource::~SubResource()
{
}

void SubResource::setActive( bool active )
{
  mActive = active;
}

bool SubResource::active() const
{
  return mActive;
}

void SubResource::setWritable( bool writable )
{
  mWritable = writable;
}

bool SubResource::writable() const
{
  return mWritable;
}

void SubResource::setLabel( const QString& label )
{
  mLabel = label;
}

QString SubResource::label() const
{
  return mLabel;
}

void SubResource::setCompletionWeight( int completionWeight )
{
  mCompletionWeight = completionWeight;
}

int SubResource::completionWeight() const
{
  return mCompletionWeight;
}

StorageReference::StorageReference( const QString& resource, Q_UINT32 sernum )
  : mResource( resource ), mSerialNumber( sernum )
{
}

StorageReference::~StorageReference()
{
}

void StorageReference::setResource( const QString& resource )
{
  mResource = resource;
}

QString StorageReference::resource() const
{
  return mResource;
}

void StorageReference::setSerialNumber( Q_UINT32 serialNumber )
{
  mSerialNumber = serialNumber;
}

Q_UINT32 StorageReference::serialNumber() const
{
  return mSerialNumber;
}
