/*  -*- c++ -*-
    ksieve_argument.h

    KSieve, the KDE internet mail/usenet news message filtering library.
    Copyright (c) 2002 Marc Mutz <mutz@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; version 2 of the License.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#ifndef __KSIEVE_ARGUMENT_H__
#define __KSIEVE_ARGUMENT_H__

#include <qstringlist.h>
#include <qstring.h>

#include <cassert>

template <typename T> class QValueVector;

#undef None

namespace KSieve {

  class Argument {
  public:
    typedef QValueVector<Argument> List;

    enum Type { None, Number, StringList, Tag };

    Argument()
      : mType( None ), mNumber( 0 ) {}
    Argument( const QStringList & s )
      : mType( StringList ), mStringList( s ), mNumber( 0 ) {}
    Argument( const QString & s )
      : mType( Tag ), mTag( s ), mNumber( 0 ) {}
    Argument( unsigned int i )
      : mType( Number ), mNumber( i ) {}

    bool isNumber() const { return type() == Number; }
    bool isStringList() const { return type() == StringList; }
    bool isTag() const { return type() == Tag; }

    void setNumber( unsigned int i, char quantifier ) {
      mType = Number;
      mNumber = i;
      assert( quantifier == 'k' || quantifier == 'K' || quantifier == 'm' ||
	      quantifier == 'M' || quantifier == 'g' || quantifier == 'G' );
      mQuantifier = quantifier;
    }

    void setStringList( const QStringList & l ) {
      mType = StringList;
      mStringList = l;
    }

    void setTag( const QString & t ) {
      mType = Tag;
      mTag = t;
    }

    QStringList stringList() const {
      assert( isStringList() );
      return mStringList;
    }

    unsigned int number() const {
      assert( isNumber() );
      return mNumber;
    }

    char quantifier() const {
      assert( isNumber() );
      return mQuantifier;
    }
    
    QString tag() const {
      assert( isTag() );
      return mTag;
    }

  protected:
    Type type() const { return mType; }

    Type mType;
    QStringList  mStringList;
    QString      mTag;
    unsigned int mNumber;
    char         mQuantifier;
  };

}; // namespace KSieve

#endif // __KSIEVE_ARGUMENT_H__
