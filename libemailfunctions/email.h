/*  -*- mode: C++; c-file-style: "gnu" -*-

    This file is part of kdepim.
    Copyright (c) 2004 KDEPIM developers

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

#ifndef EMAIL_H
#define EMAIL_H

#include <qstringlist.h>
#include <qcstring.h>

namespace KPIM {

enum EmailParseResult { AddressOk, AddressEmpty, UnexpectedEnd,
                        UnbalancedParens, MissingDomainPart,
                        UnclosedAngleAddr, UnopenedAngleAddr,
                        TooManyAts, UnexpectedComma,
                        TooFewAts, MissingLocalPart,
                        UnbalancedQuote, NoAddressSpec };

// Helper functions
/** Split a comma separated list of email addresses. */
QStringList splitEmailAddrList(const QString& aStr);

/** Splits the given address into display name, email address and comment.
    Returns AddressOk if no error was encountered. Otherwise an appropriate
    error code is returned. In case of an error the values of displayName,
    addrSpec and comment are undefined.

    @param address      a single email address,
                          example: Joe User (comment1) <joe.user@kde.org> (comment2)
    @param displayName  only out: the display-name of the email address, i.e.
                          "Joe User" in the example; in case of an error the
                          return value is undefined
    @param addrSpec     only out: the addr-spec, i.e. "joe.user@kde.org" in the
                          example; in case of an error the return value is
                          undefined
    @param comment      only out: the space-separated comments, i.e.
                          "comment1 comment2" in the example; in case of an
                          error the return value is undefined
    @return             AddressOk if no error was encountered. Otherwise an
                          appropriate error code is returned.
*/
EmailParseResult splitAddress( const QCString & address,
                               QCString & displayName,
                               QCString & addrSpec,
                               QCString & comment );

/** This is an overloaded member function, provided for convenience. It behaves
    essentially like the above function.

    Splits the given address into display name, email address and comment.
    Returns AddressOk if no error was encountered. Otherwise an appropriate
    error code is returned. In case of an error the values of displayName,
    addrSpec and comment are undefined.

    @param address      a single email address,
                          example: Joe User (comment1) <joe.user@kde.org> (comment2)
    @param displayName  only out: the display-name of the email address, i.e.
                          "Joe User" in the example; in case of an error the
                          return value is undefined
    @param addrSpec     only out: the addr-spec, i.e. "joe.user@kde.org" in the
                          example; in case of an error the return value is
                          undefined
    @param comment      only out: the space-separated comments, i.e.
                          "comment1 comment2" in the example; in case of an
                          error the return value is undefined
    @return             AddressOk if no error was encountered. Otherwise an
                          appropriate error code is returned.
*/
EmailParseResult splitAddress( const QString & address,
                               QString & displayName,
                               QString & addrSpec,
                               QString & comment );

/** Validate email address.
 * Testframework in kdepim/libemailfunctions/tests. */

EmailParseResult isValidEmailAddress( const QString& aStr );

/** Translate the enum errorcodes from emailParseResult
 * into i18n'd strings that can be used for msg boxes. */
QString emailParseResultToString( EmailParseResult errorCode );

/** Check for a simple email address and if it is valid
 * this is used for fields where only a "pure" email
 * is allowed, i,e emails in form xxx@yyy.tld */
bool isValidSimpleEmailAddress( const QString& aStr );

/** Returns the pure email address (addr-spec in RFC2822) of the given address
    (mailbox in RFC2822).

    @param address  an email address, e.g. "Joe User <joe.user@kde.org>"
    @return         the addr-spec of @address, i.e. joe.user@kde.org in the
                      example
*/
QCString getEmailAddress( const QCString & address );

/** This is an overloaded member function, provided for convenience. It behaves
    essentially like the above function.

    Returns the pure email address (addr-spec in RFC2822) of the given address
    (mailbox in RFC2822).

    @param address  an email address, e.g. "Joe User <joe.user@kde.org>"
    @return         the addr-spec of @address, i.e. joe.user@kde.org in the
                      example
*/
QString getEmailAddress( const QString & address );

/** Returns the pure email address (addr-spec in RFC2822) of the first
    email address of a list of addresses.

    @param address  an email address, e.g. "Joe User <joe.user@kde.org>"
    @return         the addr-spec of @address, i.e. joe.user@kde.org in the
                      example
*/
QCString getFirstEmailAddress( const QCString & addresses );

/** This is an overloaded member function, provided for convenience. It behaves
    essentially like the above function.

    Returns the pure email address (addr-spec in RFC2822) of the first
    email address of a list of addresses.

    @param address  an email address, e.g. "Joe User <joe.user@kde.org>"
    @return         the addr-spec of @address, i.e. joe.user@kde.org in the
                      example
*/
QString getFirstEmailAddress( const QString & addresses );

/** Return email address and name from string. Examples:
 * "Stefan Taferner <taferner@kde.org>" returns "taferner@kde.org"
 * and "Stefan Taferner". "joe@nowhere.com" returns "joe@nowhere.com"
 * and "". Note that this only returns the first address.
 * Also note that the return value is TRUE if both the name and the
 * mail are not empty: this does NOT tell you if mail contains a
 * valid email address or just some rubbish.
 */
bool getNameAndMail(const QString& aStr, QString& name, QString& mail);

/**
 * Compare two email addresses. If matchName is false, it just checks
 * the email address, and returns true if this matches. If matchName
 * is true, both the name and the email must be the same.
 */
bool compareEmail( const QString& email1, const QString& email2,
                   bool matchName );

/** Returns a normalized address built from the given parts. The normalized
    address is of one the following forms:
    - displayName (comment) <addrSpec>
    - displayName <addrSpec>
    - comment <addrSpec>
    - addrSpec

    @param displayName  the display name of the address
    @param addrSpec     the actual email address (addr-spec in RFC 2822)
    @param comment      a comment
    @return             a normalized address built from the given parts
 */
QString normalizedAddress( const QString & displayName,
                           const QString & addrSpec,
                           const QString & comment );

/** Decodes the punycode domain part of the given addr-spec if it's an IDN.

    @param addrSpec  a pure 7-bit email address (addr-spec in RFC2822)
    @return          the email address with Unicode domain
 */
QString decodeIDN( const QString & addrSpec );

/** Encodes the domain part of the given addr-spec in punycode if it's an
    IDN.

    @param addrSpec  a pure email address with Unicode domain
    @return          the email address with domain in punycode
 */
QString encodeIDN( const QString & addrSpec );

/** Normalizes all email addresses in the given list and decodes all IDNs.

    @param addresses  a list of email addresses with punycoded IDNs
    @return           the email addresses in normalized form with Unicode IDNs

    @also
 */
QString normalizeAddressesAndDecodeIDNs( const QString & addresses );

/** Normalizes all email addresses in the given list and encodes all IDNs
    in punycode.
 */
QString normalizeAddressesAndEncodeIDNs( const QString & str );

} // namespace

#endif /* EMAIL_H */

