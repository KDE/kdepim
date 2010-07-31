/*
    This file is part of KDE.

    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>

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

#ifndef KWSDL_SERVICE_H
#define KWSDL_SERVICE_H

#include <tqmap.h>
#include <tqstringlist.h>

namespace KWSDL {

class Service
{
  public:
    class Port
    {
      public:
        typedef TQValueList<Port> List;

        TQString mName;
        TQString mBinding;
        TQString mLocation;
    };

    Service();
    Service( const TQString &name );

    void setName( const TQString &name ) { mName = name; }
    TQString name() const { return mName; }

    void addPort( const Port &port );
    Port port( const TQString &name ) const;
    Port::List ports() const;

  private:
    TQString mName;
    TQMap<TQString, Port> mPorts;
};

}

#endif
