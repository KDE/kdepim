// kregexp3.h - A KRegExp replacement based on QRegExp from Qt3.x
// Copyright: (c) 2001 Marc Mutz <mutz@kde.org>
// Licence: GPL

#include <qglobal.h>
#include <qregexp.h>

#include <qstring.h>

/** This class is simply there to provide a namespace for some nice
    enhancements of the mighty @ref QRegExp (Qt3 version) regular
    expression engine, namely the method @ref replace, which can be
    used to do search-and-replce like one is used to from perl or sed.
    
    It "simply" adds the ability to define a replacement string which
    contains references to the captured substrings. The following
    constructs are understood, which can be freely mixed in the
    replacement string:

    @sect Sed syntax

    Back references in the replacement string are made using \n
    (backslash-digit), where @p n is a single digit. With this mode of
    operation, only the first nine captured substrings can be
    referenced.

    NOTE: Remember that C++ interprets the backslash in string
    constants, so you have to write a backslash as "\\".

    @sect Perl syntax

    Back references in the replacement string are made using $n
    (dollarsign-digit), where @p n is a single digit. With this mode
    of operation, only the first nine captured substrings can be
    referenced.

    Additionally, Perl supports the syntax ${nn}
    (dollarSign-leftCurlyBrace-digits-rightCurlyBrace), where @p nn
    can be a multi-digit number. 

    In all modes, counting of captured substrings starts with 1 (one)!
    To reference the entire matched string, use $0, ${0} or \0.

    @short A QRegExp (Qt3.x) with a replace() method.
    @author Marc Mutz <mutz@kde.org>
    @see QRegExp
*/

class KRegExp3 : public QRegExp
{
public:
  KRegExp3()
    : QRegExp() {}
  KRegExp3( const QString & pattern,
	    bool caseSensitive = TRUE,
	    bool wildcard = FALSE )
    : QRegExp( pattern, caseSensitive, wildcard ) {}
  KRegExp3( const QRegExp & rx )
    : QRegExp( rx ) {}
  KRegExp3( const KRegExp3 & rx )
    : QRegExp( (QRegExp)rx ) {}

  /** Replaces each matching subpattern in @p str with
      @p replacementStr, inserting captured substrings for
      \n, $n and ${nn} as described in the class documentation.
      @param str The source string.
      @param replacementStr The string which replaces matched
             substrings of @p str.
      @param start Start position for the search.
             If @p start is negative, starts @p -(start) positions
	     from the end of @p str.
      @param global If @p TRUE, requests to replace all occurences
             of the regexp with @p replacementStr; if @p FALSE,
	     only the first occurence will be replaced.
	     Equivalent to the /g switch to perl's s/// operator.
      @return The modified string.
  */
  QString replace( const QString & str,
		   const QString & replacementStr,
		   int start=0, bool global=TRUE );
};
