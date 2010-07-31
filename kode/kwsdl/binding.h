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

#ifndef KWSDL_BINDING_H
#define KWSDL_BINDING_H

#include <tqmap.h>

namespace KWSDL {

class Binding
{
  public:
    typedef TQValueList<Binding> List;

    class Operation
    {
      public:
        typedef TQValueList<Operation> List;

        class Item
        {
          public:
            void setUse( const TQString &use ) { mUse = use; }
            TQString use() const { return mUse; }

            void setNameSpace( const TQString &nameSpace ) { mNameSpace = nameSpace; }
            TQString nameSpace() const { return mNameSpace; }

            void setEncodingStyle( const TQString &encodingStyle ) { mEncodingStyle = encodingStyle; }
            TQString encodingStyle() const { return mEncodingStyle; }

          private:
            TQString mUse;
            TQString mNameSpace;
            TQString mEncodingStyle;
        };

        Operation();
        Operation( const TQString &name, const TQString &action );

        void setName( const TQString &name ) { mName = name; }
        TQString name() const { return mName; }

        void setAction( const TQString &action ) { mAction = action; }
        TQString action() const { return mAction; }

        void setInput( const Item &item ) { mInput = item; }
        Item input() const { return mInput; }

        void setOutput( const Item &item ) { mOutput = item; }
        Item output() const { return mOutput; }

      private:
        TQString mName;
        TQString mAction;
        Item mInput;
        Item mOutput;
    };

    Binding();
    Binding( const TQString &name, const TQString &type );

    void setName( const TQString &name ) { mName = name; }
    TQString name() const { return mName; }

    void setType( const TQString &type ) { mType = type; }
    TQString type() const { return mType; }

    void setStyle( const TQString &style ) { mStyle = style; }
    TQString style() const { return mStyle; }

    void setTransport( const TQString &transport ) { mTransport = transport; }
    TQString transport() const { return mTransport; }

    void addOperation( const Operation &operation );
    Operation operation( const TQString &name ) const;
    Operation::List operations() const;

  private:
    TQString mName;
    TQString mType;
    TQString mStyle;
    TQString mTransport;
    Operation::List mOperations;
};

}

#endif
