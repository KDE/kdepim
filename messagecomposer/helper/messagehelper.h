
/* -*- mode: C++; c-file-style: "gnu" -*-
  Copyright (C) 2009 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
  Copyright (c) 2009 Andras Mantia <andras@kdab.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef KMAIL_MESSAGE_HELPER_H
#define KMAIL_MESSAGE_HELPER_H

#include <akonadi/kmime/messagestatus.h>
#include "messagecomposer_export.h"

#include <kmime/kmime_headers.h>
#include <kmime/kmime_message.h>
#include <item.h>

namespace KPIMIdentities {
class IdentityManager;
}

namespace KMime {
class Message;
}

/**
 * Contains random helper methods when dealing with messages.
 * TODO: cleanup and organize, along with similar methods in messageviewer.
 */
namespace MessageHelper {

/** Initialize header fields. Should be called on new messages
    if they are not set manually. E.g. before composing. Calling
    of setAutomaticFields(), see below, is still required. */
void MESSAGECOMPOSER_EXPORT initHeader( const KMime::Message::Ptr &message, const KPIMIdentities::IdentityManager* identMan, uint id = 0 );

/** Set the from, to, cc, bcc, encrytion etc headers as specified in the
  * given identity. */
void MESSAGECOMPOSER_EXPORT applyIdentity( const KMime::Message::Ptr &message, const KPIMIdentities::IdentityManager* identMan,  uint id );

/** Initialize headers fields according to the identity and the transport
   header of the given original message */
void MESSAGECOMPOSER_EXPORT initFromMessage(const KMime::Message::Ptr &msg, const KMime::Message::Ptr &orgiMsg, KPIMIdentities::IdentityManager*,
                                            uint id, bool idHeaders = true);

KMime::Types::AddrSpecList MESSAGECOMPOSER_EXPORT extractAddrSpecs( const KMime::Message::Ptr &msg, const QByteArray &header );

/** Check for prefixes @p prefixRegExps in #subject(). If none
      is found, @p newPrefix + ' ' is prepended to the subject and the
      resulting string is returned. If @p replace is true, any
      sequence of whitespace-delimited prefixes at the beginning of
      #subject() is replaced by @p newPrefix
  **/
QString MESSAGECOMPOSER_EXPORT cleanSubject( const KMime::Message::Ptr &msg, const QStringList &prefixRegExps, bool replace,
                                             const QString &newPrefix );

/** Return this mails subject, with all "forward" and "reply"
      prefixes removed */
QString MESSAGECOMPOSER_EXPORT cleanSubject( const KMime::Message::Ptr &msg );

/** Return this mails subject, formatted for "forward" mails */
QString MESSAGECOMPOSER_EXPORT forwardSubject( const KMime::Message::Ptr &msg );

/** Return this mails subject, formatted for "reply" mails */
QString MESSAGECOMPOSER_EXPORT replySubject( const KMime::Message::Ptr &msg );
/** Check for prefixes @p prefixRegExps in @p str. If none
      is found, @p newPrefix + ' ' is prepended to @p str and the
      resulting string is returned. If @p replace is true, any
      sequence of whitespace-delimited prefixes at the beginning of
      @p str is replaced by @p newPrefix.
  **/
QString MESSAGECOMPOSER_EXPORT replacePrefixes( const QString& str,
                                                const QStringList& prefixRegExps,
                                                bool replace,
                                                const QString& newPrefix );

/** Set fields that are either automatically set (Message-id)
    or that do not change from one message to another (MIME-Version).
    Call this method before sending *after* all changes to the message
    are done because this method does things different if there are
    attachments / multiple body parts. */
void MESSAGECOMPOSER_EXPORT setAutomaticFields( const KMime::Message::Ptr &msg, bool isMultipart=false );

/** Creates reference string for reply to messages.
   *  reference = original first reference + original last reference + original msg-id
   */
QByteArray MESSAGECOMPOSER_EXPORT getRefStr( const KMime::Message::Ptr &msg );

QString MESSAGECOMPOSER_EXPORT ccStrip( const KMime::Message::Ptr &msg );
QString MESSAGECOMPOSER_EXPORT toStrip( const KMime::Message::Ptr &msg );
QString MESSAGECOMPOSER_EXPORT fromStrip( const KMime::Message::Ptr &msg );

/** Returns @p str with all "forward" and "reply" prefixes stripped off.
  **/
QString MESSAGECOMPOSER_EXPORT stripOffPrefixes( const QString& str );

/**
   * Skip leading keyword if keyword has given character at it's end
   * (e.g. ':' or ',') and skip the following blanks (if any) too.
   * If keywordFound is specified it will be true if a keyword was skipped
   * and false otherwise. */
QString MESSAGECOMPOSER_EXPORT skipKeyword( const QString& str, QChar sepChar = QLatin1Char( ':' ),
                                            bool *keywordFound = 0 );

}



#endif
