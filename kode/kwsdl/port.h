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

#ifndef KWSDL_PORT_H
#define KWSDL_PORT_H

#include <tqmap.h>

namespace KWSDL {

class Port
{
  public:
    typedef TQValueList<Port> List;

    class Operation
    {
      public:
        typedef TQValueList<Operation> List;
        typedef TQMap<TQString, Operation> Map;

        Operation();
        Operation( const TQString &name, const TQString &input, const TQString &output );

        void setName( const TQString &name ) { mName = name; }
        TQString name() const { return mName; }

        void setInput( const TQString &input ) { mInput = input; }
        TQString input() const { return mInput; }

        void setOutput( const TQString &output ) { mOutput = output; }
        TQString output() const { return mOutput; }

      private:
        TQString mName;
        TQString mInput;
        TQString mOutput;
    };

    Port();
    Port( const TQString &name );

    void setName( const TQString &name ) { mName = name; }
    TQString name() const { return mName; }

    void addOperation( const Operation &operation );
    Operation operation( const TQString &name ) const;
    Operation::List operations() const;

  private:
    TQString mName;
    Operation::List mOperations;
};

}

#endif
