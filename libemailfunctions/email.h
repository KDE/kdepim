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

// Helper functions
/** Split a comma separated list of email addresses. */
QStringList splitEmailAddrList(const QString& aStr);


/** Validate email address.
 * Testframework in kdepim/libemailfunctions/tests. */
enum EmailParseResult { AddressOk, AddressEmpty, UnexpectedEnd,
                            UnbalancedQuote, UnbalancedParens,
                            UnclosedAngleAddr, UnopenedAngleAddr,
                            UnexpectedComma, MissingDomainPart,
                            TooManyAts, TooFewAts,
                            MissingLocalPart };

EmailParseResult isValidEmailAddress( const QString& aStr );

/** Translate the enum errorcodes from emailParseResult
 * into i18n'd strings that can be used for msg boxes. */
QString emailParseResultToString( EmailParseResult errorCode );

/** Return email address from string. Examples:
 * "Stefan Taferner <taferner@kde.org>" returns "taferner@kde.org"
 * "joe@nowhere.com" returns "joe@nowhere.com". Note that this only
 * returns the first address. */
QCString getEmailAddr(const QString& aStr);

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

} // namespace

#endif /* EMAIL_H */

