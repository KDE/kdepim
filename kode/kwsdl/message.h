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

#ifndef KWSDL_MESSAGE_H
#define KWSDL_MESSAGE_H

#include <tqmap.h>

namespace KWSDL {

class Message
{
  public:
    typedef TQValueList<Message> List;

    class Part
    {
      public:
        typedef TQMap<TQString, Part> Map;
        typedef TQValueList<Part> List;

        Part();
        Part( const TQString &name, const TQString &type );

        void setName( const TQString &name ) { mName = name; }
        TQString name() const { return mName; }

        void setType( const TQString &type ) { mType = type; }
        TQString type() const { return mType; }

      private:
        TQString mName;
        TQString mType;
    };

    Message();
    Message( const TQString &name );

    void setName( const TQString &name );
    TQString name() const;

    void addPart( const Part &part );
    Part part( const TQString &name ) const;
    Part::List parts() const;

  private:
    TQString mName;
    Part::List mParts;
};

}

#endif
