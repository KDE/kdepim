/*
    This file is part of libkcal.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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

#include <kdebug.h>
#include <klocale.h>


#include "person.h"

using namespace KCal;

// Copy of KPIM::getNameAndMail from libkdepim/email.cpp, which we can't link to.
// HEAD has a better solution, with a shared lib.
static bool getNameAndMail(const QString& aStr, QString& name, QString& mail)
{
  name = QString::null;
  mail = QString::null;

  const int len=aStr.length();
  const char cQuotes = '"';

  bool bInComment, bInQuotesOutsideOfEmail;
  int i=0, iAd=0, iMailStart=0, iMailEnd=0;
  QChar c;

  // Find the '@' of the email address
  // skipping all '@' inside "(...)" comments:
  bInComment = false;
  while( i < len ){
    c = aStr[i];
    if( !bInComment ){
      if( '(' == c ){
        bInComment = true;
      }else{
        if( '@' == c ){
          iAd = i;
          break; // found it
        }
      }
    }else{
      if( ')' == c ){
        bInComment = false;
      }
    }
    ++i;
  }

  if( !iAd ){
    // We suppose the user is typing the string manually and just
    // has not finished typing the mail address part.
    // So we take everything that's left of the '<' as name and the rest as mail
    for( i = 0; len > i; ++i ) {
      c = aStr[i];
      if( '<' != c )
        name.append( c );
      else
        break;
    }
    mail = aStr.mid( i+1 );

  }else{

    // Loop backwards until we find the start of the string
    // or a ',' that is outside of a comment
    //          and outside of quoted text before the leading '<'.
    bInComment = false;
    bInQuotesOutsideOfEmail = false;
    for( i = iAd-1; 0 <= i; --i ) {
      c = aStr[i];
      if( bInComment ){
        if( '(' == c ){
          if( !name.isEmpty() )
            name.prepend( ' ' );
          bInComment = false;
        }else{
          name.prepend( c ); // all comment stuff is part of the name
        }
      }else if( bInQuotesOutsideOfEmail ){
        if( cQuotes == c )
          bInQuotesOutsideOfEmail = false;
        name.prepend( c );
      }else{
        // found the start of this addressee ?
        if( ',' == c )
          break;
        // stuff is before the leading '<' ?
        if( iMailStart ){
          if( cQuotes == c )
            bInQuotesOutsideOfEmail = true; // end of quoted text found
          name.prepend( c );
        }else{
          switch( c ){
            case '<':
              iMailStart = i;
              break;
            case ')':
              if( !name.isEmpty() )
                name.prepend( ' ' );
              bInComment = true;
              break;
            default:
              if( ' ' != c )
                mail.prepend( c );
          }
        }
      }
    }

    name = name.simplifyWhiteSpace();
    mail = mail.simplifyWhiteSpace();

    if( mail.isEmpty() )
      return false;

    mail.append('@');

    // Loop forward until we find the end of the string
    // or a ',' that is outside of a comment
    //          and outside of quoted text behind the trailing '>'.
    bInComment = false;
    bInQuotesOutsideOfEmail = false;
    for( i = iAd+1; len > i; ++i ) {
      c = aStr[i];
      if( bInComment ){
        if( ')' == c ){
          if( !name.isEmpty() )
            name.append( ' ' );
          bInComment = false;
        }else{
          name.append( c ); // all comment stuff is part of the name
        }
      }else if( bInQuotesOutsideOfEmail ){
        if( cQuotes == c )
          bInQuotesOutsideOfEmail = false;
        name.append( c );
      }else{
        // found the end of this addressee ?
        if( ',' == c )
          break;
        // stuff is behind the trailing '>' ?
        if( iMailEnd ){
          if( cQuotes == c )
            bInQuotesOutsideOfEmail = true; // start of quoted text found
          name.append( c );
        }else{
          switch( c ){
            case '>':
              iMailEnd = i;
              break;
            case '(':
              if( !name.isEmpty() )
                name.append( ' ' );
              bInComment = true;
              break;
            default:
              if( ' ' != c )
                mail.append( c );
          }
        }
      }
    }
  }

  name = name.simplifyWhiteSpace();
  mail = mail.simplifyWhiteSpace();

  return ! (name.isEmpty() || mail.isEmpty());
}

Person::Person( const QString &fullName )
{
  QString name, email;
  getNameAndMail( fullName, name, email );
  setName( name );
  setEmail( email );
}

Person::Person( const QString &name, const QString &email )
{
  setName( name );
  setEmail( email );
}


bool KCal::operator==( const Person& p1, const Person& p2 )
{
    return ( p1.name() == p2.name() &&
             p1.email() == p2.email() );
}


QString Person::fullName() const
{
  if( mName.isEmpty() ) {
    return mEmail;
  } else {
    if( mEmail.isEmpty() )
      return mName;
    else {
      // Taken from KABC::Addressee::fullEmail
      QString name = mName;
      QRegExp needQuotes( "[^ 0-9A-Za-z\\x0080-\\xFFFF]" );
      if ( name.find( needQuotes ) != -1 )
        return "\"" + name + "\" <" + mEmail + ">";
      else
        return name + " <" + mEmail + ">";
    }
  }
}

bool Person::isEmpty() const
{
  return mEmail.isEmpty() && mName.isEmpty();
}

void Person::setName(const QString &name)
{
  mName = name;
}

void Person::setEmail(const QString &email)
{
  if ( email.startsWith( "mailto:", false ) ) {
    mEmail = email.mid(7);
  } else {
    mEmail = email;
  }
}
