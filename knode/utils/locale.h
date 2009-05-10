/*
  Copyright 2009 Olivier Trichet <nive@nivalis.org>

  Permission to use, copy, modify, and distribute this software
  and its documentation for any purpose and without fee is hereby
  granted, provided that the above copyright notice appear in all
  copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaim all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

#ifndef KNODE_UTILITIES_LOCALE_H
#define KNODE_UTILITIES_LOCALE_H

#include "knode_export.h"

class KNGroup;
class QByteArray;
class QString;

namespace KNode {

/**
  Namespace for utilitary classes.
*/
namespace Utilities {

/**
  @brief A set of utilitary methods that deals with character sets.

  Those methods allow:

  @li that only legal (as in MIME) charsets are used in headers of
  messages that are @em sent (not encoding name coming from KCharsets).
  @par
  See bug #169411, #163524 among other issues for the rational behind this class.

  @li or that recode 8bit strings to a hopefully proper encoding. This is used for incoming
  messages, specially for data from XOVER call.
*/
class KNODE_EXPORT Locale {

  public:
    // Methods to cleanup charset of out-going message
    /**
      Converts a given charset to a charset that can be used in message headers (e.g. in charset field of a
      Content-Type, or in encoding for From and Subject field).
      This means that this method will always returns a valid encoding as per the
      @link http://www.iana.org/assignments/character-sets IANA charset list @endlink and a
      non empty one.

      @param charset a charset to validate or an empty string to get a suitable default charset.
      @return the equivalent of the @p charset parameter that can be used to encode MIME message
      or "UTF-8" if no suitable character set is found.
    */
    static QString toMimeCharset( const QString &charset );

    /**
      Returns the charset defined in the global configuration.
      No needs to passed it through toMimeCharset().
      This must be used instead of knGlobals.settings()->charset() for
      out-going message.
    */
    static QByteArray defaultCharset();

    /**
      Returns the charset defined in a group configuration.
      This calls defaultCharset() if no default charset is configured for this group.
    */
    static QByteArray defaultCharset( KNGroup *g );


    // Incoming data
    /**
      Try to convert a 8bits string to the equivalent 7bits RFC-2047 string.
      @param s a string that contains either a properly RFC2047-encoded string, or
               a 8bits bytearray read as a ISO-8859-1 string.
      @param g group associated with the input string @p s. used to get a charset to
               try the recoding.
      @param result a bytearray filled with the 7bits equivalent of @p s encoded as defined in RFC-2047.
    */
    static void recodeString( const QString &s, KNGroup *g, QByteArray &result );

    /**
      Convert raw data @p raw to a RFC-2047 string.
      @param raw a raw string that is assume to be encoded with @p charset.
      @param charset the charset to read from @p raw.
      @param result a bytearray filled with the equivalent of @p raw as a RFC-2047 string or @p raw
              if it was already correctly encoded.
    */
    static void encodeTo7Bit( const QByteArray &raw, const QByteArray &charset, QByteArray &result );

};

} // namespace Utilities
} // namespace KNode

#endif
