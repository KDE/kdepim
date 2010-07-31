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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef EMAIL_H
#define EMAIL_H

#include <tqstringlist.h>
#include <tqcstring.h>

#include <kdepimmacros.h>

/** @file */

/** 
    \brief KPIM holds all kinds of functions specific to KDE PIM.

    The KPIM namespace hides away functions, enums, and other things
    that are KDE PIM specific and that we don't want to have polluting
    the global namespace.
*/
namespace KPIM {

/**
    Result type for splitAddress, isValidEmailAddress.
*/
enum EmailParseResult { AddressOk, AddressEmpty, UnexpectedEnd,
                        UnbalancedParens, MissingDomainPart,
                        UnclosedAngleAddr, UnopenedAngleAddr,
                        TooManyAts, UnexpectedComma,
                        TooFewAts, MissingLocalPart,
                        UnbalancedQuote, NoAddressSpec,
                        DisallowedChar, InvalidDisplayName };

// Helper functions
/** Split a comma separated list of email addresses. */
KDE_EXPORT TQStringList splitEmailAddrList(const TQString& aStr);

/** Splits the given address into display name, email address and comment.
    Returns AddressOk if no error was encountered. Otherwise an appropriate
    error code is returned. In case of an error the values of displayName,
    addrSpec and comment are undefined.

    @param address      a single email address,
                          example: Joe User (comment1) <joe.user@example.org> (comment2)
    @param displayName  only out: the display-name of the email address, i.e.
                          "Joe User" in the example; in case of an error the
                          return value is undefined
    @param addrSpec     only out: the addr-spec, i.e. "joe.user@example.org" in the
                          example; in case of an error the return value is
                          undefined
    @param comment      only out: the space-separated comments, i.e.
                          "comment1 comment2" in the example; in case of an
                          error the return value is undefined
    @return             AddressOk if no error was encountered. Otherwise an
                          appropriate error code is returned.
*/
KDE_EXPORT EmailParseResult splitAddress( const TQCString & address,
                               TQCString & displayName,
                               TQCString & addrSpec,
                               TQCString & comment );

/** This is an overloaded member function, provided for convenience. It behaves
    essentially like the above function.

    Splits the given address into display name, email address and comment.
    Returns AddressOk if no error was encountered. Otherwise an appropriate
    error code is returned. In case of an error the values of displayName,
    addrSpec and comment are undefined.

    @param address      a single email address,
                          example: Joe User (comment1) <joe.user@example.org> (comment2)
    @param displayName  only out: the display-name of the email address, i.e.
                          "Joe User" in the example; in case of an error the
                          return value is undefined
    @param addrSpec     only out: the addr-spec, i.e. "joe.user@example.org" in the
                          example; in case of an error the return value is
                          undefined
    @param comment      only out: the space-separated comments, i.e.
                          "comment1 comment2" in the example; in case of an
                          error the return value is undefined
    @return             AddressOk if no error was encountered. Otherwise an
                          appropriate error code is returned.
*/
KDE_EXPORT EmailParseResult splitAddress( const TQString & address,
                               TQString & displayName,
                               TQString & addrSpec,
                               TQString & comment );

/** Validates an email address in the form of "Joe User" <joe@example.org>.
    Returns AddressOk if no error was encountered. Otherwise an appropriate
    error code is returned.

    @param aStr         a single email address,
                          example: Joe User (comment1) <joe.user@example.org>
    @return             AddressOk if no error was encountered. Otherwise an
                          appropriate error code is returned.
*/
KDE_EXPORT EmailParseResult isValidEmailAddress( const TQString& aStr );

/** Translate the enum errorcodes from emailParseResult
    into i18n'd strings that can be used for msg boxes. 
    
    @param errorCode    the errorCode from isValidEmailEmailAddress().

    @return             An i18n ready string for use in msgboxes. 
*/
KDE_EXPORT TQString emailParseResultToString( EmailParseResult errorCode );

/** Validates an email address in the form of joe@example.org.
    Returns true if no error was encountered.
    This method should be used when the input field should not
    allow a "full" email address with comments and other special
    cases that normally are valid in an email address.

    @param aStr         a single email address,
                          example: joe.user@example.org
    @return             true if no error was encountered. 
*/
KDE_EXPORT bool isValidSimpleEmailAddress( const TQString& aStr );

/** Returns a i18n string to be used in msgboxes 
    this allows for error messages to be the same
    across the board. 
   
    @return             An i18n ready string for use in msgboxes.
*/

KDE_EXPORT TQString simpleEmailAddressErrorMsg(); 

/** Returns the pure email address (addr-spec in RFC2822) of the given address
    (mailbox in RFC2822).

    @param address  an email address, e.g. "Joe User <joe.user@example.org>"
    @return         the addr-spec of @a address, i.e. joe.user@example.org in the
                      example
*/
KDE_EXPORT TQCString getEmailAddress( const TQCString & address );

/** This is an overloaded member function, provided for convenience. It behaves
    essentially like the above function.

    Returns the pure email address (addr-spec in RFC2822) of the given address
    (mailbox in RFC2822).

    @param address  an email address, e.g. "Joe User <joe.user@example.org>"
    @return         the addr-spec of @a address, i.e. joe.user@example.org in the
                      example
*/
KDE_EXPORT TQString getEmailAddress( const TQString & address );

/** Returns the pure email address (addr-spec in RFC2822) of the first
    email address of a list of addresses.

    @param addresses  an email address, e.g. "Joe User <joe.user@example.org>"
    @return         the addr-spec of @a addresses, i.e. joe.user@example.org in the
                      example
*/
KDE_EXPORT TQCString getFirstEmailAddress( const TQCString & addresses );

/** This is an overloaded member function, provided for convenience. It behaves
    essentially like the above function.

    Returns the pure email address (addr-spec in RFC2822) of the first
    email address of a list of addresses.

    @param addresses  an email address, e.g. "Joe User <joe.user@example.org>"
    @return         the addr-spec of @a addresses, i.e. joe.user@example.org in the
                      example
*/
KDE_EXPORT TQString getFirstEmailAddress( const TQString & addresses );

/** Return email address and name from string. Examples:
 * "Stefan Taferner <taferner@example.org>" returns "taferner@example.org"
 * and "Stefan Taferner". "joe@example.com" returns "joe@example.com"
 * and "". Note that this only returns the first address.
 * Also note that the return value is TRUE if both the name and the
 * mail are not empty: this does NOT tell you if mail contains a
 * valid email address or just some rubbish.
 */
KDE_EXPORT bool getNameAndMail(const TQString& aStr, TQString& name, TQString& mail);

/**
 * Compare two email addresses. If matchName is false, it just checks
 * the email address, and returns true if this matches. If matchName
 * is true, both the name and the email must be the same.
 */
KDE_EXPORT bool compareEmail( const TQString& email1, const TQString& email2,
                   bool matchName );

/** Returns a normalized address built from the given parts. The normalized
    address is of one the following forms:
    - displayName (comment) &lt;addrSpec&gt;
    - displayName &lt;addrSpec&gt;
    - comment &lt;addrSpec&gt;
    - addrSpec

    @param displayName  the display name of the address
    @param addrSpec     the actual email address (addr-spec in RFC 2822)
    @param comment      a comment
    @return             a normalized address built from the given parts
 */
KDE_EXPORT TQString normalizedAddress( const TQString & displayName,
                           const TQString & addrSpec,
                           const TQString & comment );

/** Decodes the punycode domain part of the given addr-spec if it's an IDN.

    @param addrSpec  a pure 7-bit email address (addr-spec in RFC2822)
    @return          the email address with Unicode domain
 */
KDE_EXPORT TQString decodeIDN( const TQString & addrSpec );

/** Encodes the domain part of the given addr-spec in punycode if it's an
    IDN.

    @param addrSpec  a pure email address with Unicode domain
    @return          the email address with domain in punycode
 */
KDE_EXPORT TQString encodeIDN( const TQString & addrSpec );

/** Normalizes all email addresses in the given list and decodes all IDNs.

    @param addresses  a list of email addresses with punycoded IDNs
    @return           the email addresses in normalized form with Unicode IDNs

 */
KDE_EXPORT TQString normalizeAddressesAndDecodeIDNs( const TQString & addresses );

/** Normalizes all email addresses in the given list and encodes all IDNs
    in punycode.
 */
KDE_EXPORT TQString normalizeAddressesAndEncodeIDNs( const TQString & str );

/** Add quote characters around the given string if it contains a 
 * character that makes that necessary, in an email name, such as ",".
 */
KDE_EXPORT TQString quoteNameIfNecessary( const TQString& str );

} // namespace

#endif /* EMAIL_H */

