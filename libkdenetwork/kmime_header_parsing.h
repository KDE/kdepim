/*  -*- c++ -*-
    kmime_header_parsing.h

    This file is part of KMime, the KDE internet mail/usenet news message library.
    Copyright (c) 2001-2002 Marc Mutz <mutz@kde.org>

    KMime is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License, version 2, as
    published by the Free Software Foundation.

    KMime is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this library with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#ifndef __KMIME_HEADER_PARSING_H__
#define __KMIME_HEADER_PARSING_H__

#include <qstring.h>
#include <qpair.h>
#include <qvaluelist.h>

#include <time.h>

template <typename K, typename V> class QMap;
class QStringList;

namespace KMime {

namespace Types {

  // for when we can't make up our mind what to use...
  struct QStringOrQPair {
    QStringOrQPair() : qstring(), qpair(0,0) {}
    QString qstring;
    QPair<const char*,int> qpair;
  };

  struct AddrSpec {
    QString asString() const;
    QString localPart;
    QString domain;
  };
  typedef QValueList<AddrSpec> AddrSpecList;

  struct Mailbox {
    QString displayName;
    AddrSpec addrSpec;
  };
  typedef QValueList<Mailbox> MailboxList;

  struct Address {
    QString displayName;
    MailboxList mailboxList;
  };
  typedef QValueList<Address> AddressList;

  struct DateTime {
    time_t time;            // secs since 1.1.1970, 0:00 UTC/GMT
    long int secsEastOfGMT; // timezone
    bool timeZoneKnown;     // do we know the timezone? (e.g. on -0000)
  };

} // namespace KMime::Types

namespace HeaderParsing {

  /** Parse the encoded word in @p str pointed to by @p pos
      (actually, @p pos-2, see below).
      
      @param str the source string
      @param pos in: the starting position (must already point to the
                   character following the initial '=?';
		   out: the new postion
      @param ok  only out: if true, the encoded-word was correct up
	           to and including the encoding specifier. The
		   encoded-text is quite generously parsed and @p ok
		   is still set to @p true when e.g. the encoded-word
		   appears to be truncated or contains whitespace.
      @return the decoded string the encoded word represented.
  */
  bool parseEncodedWord( const char* & scursor, const char * const send,
			 QString & result, QCString & language );
  //
  // The parsing squad:
  //

  /** You may or may not have already started parsing into the
      atom. This function will go on where you left off. */
  bool parseAtom( const char* & scursor, const char * const send,
		  QString & result, bool allow8Bit=false );
  bool parseAtom( const char* & scursor, const char * const send,
		  QPair<const char*,int> & result, bool allow8Bit=false );
  /** You may or may not have already started parsing into the
      token. This function will go on where you left off. */
  bool parseToken( const char* & scursor, const char * const send,
		   QString & result, bool allow8Bit=false );
  bool parseToken( const char* & scursor, const char * const send,
		   QPair<const char*,int> & result, bool allow8Bit=false );
  /** @p scursor must be positioned after the opening openChar. */
  bool parseGenericQuotedString( const char* & scursor, const char* const send,
				 QString & result, bool isCRLF,
				 const char openChar='"',
				 const char closeChar='"' );
  /** @p scursor must be positioned right after the opening '(' */
  bool parseComment( const char* & scursor, const char * const send,
		     QString & result, bool isCRLF=false, bool reallySave=true );
  /** You may or may not have already started parsing into the phrase,
      but only if it starts with atext. If you setup this function to
      parse a phrase starting with an encoded-word or quoted-string,
      @p scursor has to point to the char introducing the encoded-word
      or quoted-string, resp. */
  bool parsePhrase( const char* & scursor, const char * const send,
		    QString & result, bool isCRLF=false );
  /** You may or may not have already started parsing into the initial
      atom, but not up to it's end. */
  bool parseDotAtom( const char* & scursor, const char * const send,
		     QString & result, bool isCRLF=false );

  /** Eats comment-folding-white-space, skips whitespace, folding and
      comments (even nested ones) and stops at the next non-CFWS
      character. After calling this function, you should check whether
      @p scursor == @p send (end of header reached).

      If a comment with unbalanced parantheses is encountered, @p
      scursor is being positioned on the opening '(' of the outmost
      comment.
  */
  void eatCFWS( const char* & scursor, const char * const send, bool isCRLF );

  bool parseDomain( const char* & scursor, const char * const send,
		    QString & result, bool isCRLF=false );
  bool parseObsRoute( const char* & scursor, const char * const send,
		      QStringList & result,
		      bool isCRLF=false, bool save=false );
  bool parseAddrSpec( const char* & scursor, const char * const send,
		      Types::AddrSpec & result, bool isCRLF=false );
  bool parseAngleAddr( const char* & scursor, const char * const send,
		       Types::AddrSpec & result, bool isCRLF=false );
  bool parseMailbox( const char* & scursor, const char * const send,
		     Types::Mailbox & result, bool isCRLF=false );
  bool parseGroup( const char* & scursor, const char * const send,
		   Types::Address & result, bool isCRLF=false );
  bool parseAddress( const char* & scursor, const char * const send,
		     Types::Address & result, bool isCRLF=false );
  bool parseAddressList( const char* & scursor, const char * const send,
			 Types::AddressList & result, bool isCRLF=false );

  bool parseParameter( const char* & scursor, const char * const send,
		       QPair<QString,Types::QStringOrQPair> & result,
		       bool isCRLF=false );
  bool parseParameterList( const char* & scursor, const char * const send,
			   QMap<QString,QString> & result, bool isCRLF=false );

  bool parseRawParameterList( const char* & scursor, const char * const send,
			      QMap<QString,Types::QStringOrQPair> & result,
			      bool isCRLF=false );
  
  bool parseTime( const char* & scursor, const char * const send,
		  int & hour, int & min, int & sec, long int & secsEastOfGMT,
		  bool & timeZoneKnown, bool isCRLF=false );

  bool parseDateTime( const char* & scursor, const char * const send,
		      Types::DateTime & result, bool isCRLF=false );

#if 0
  bool tryToMakeAnySenseOfDateString( const char* & scursor,
				      const char * const send,
				      time_t & result, bool isCRLF=false );
#endif

} // namespace HeaderParsing

} // namespace KMime


#endif // __KMIME_HEADER_PARSING_H__

