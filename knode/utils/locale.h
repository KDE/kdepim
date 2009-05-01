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
namespace Utilities {

/**
  @brief A set of utilitary methods that deals with character sets.

  Those methods allow that only legal (as in MIME) charsets are used in headers of
  messages that are @em sent (not encoding name coming from KCharsets).

  @par
  See bug #169411, #163524 among other issues for the rational behind this class.
*/
class KNODE_EXPORT Locale {

  public:
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
};

} // namespace Utilities
} // namespace KNode

#endif