/*  -*- c++ -*-
    ksieve_argument.h

    This file is part of KSieve,
    the KDE internet mail/usenet news message filtering library.
    Copyright (c) 2002 Marc Mutz <mutz@kde.org>

    KSieve is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License, version 2, as
    published by the Free Software Foundation.

    KSieve is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

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
