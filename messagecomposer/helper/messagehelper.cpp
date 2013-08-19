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

#include "messagehelper.h"
#include "kdepim-version.h"
#include "utils/util.h"
#include "settings/messagecomposersettings.h"

#include "messagecore/misc/mailinglist.h"
#include "messagecore/utils/stringutil.h"

#include <KDateTime>
#include <KProtocolManager>
#include <KMime/Message>
#include <kmime/kmime_mdn.h>
#include <kmime/kmime_dateformatter.h>
#include <kmime/kmime_headers.h>
#include <kpimidentities/identitymanager.h>
#include <kpimidentities/identity.h>
#include <KPIMUtils/Email>

using namespace MessageCore;

namespace MessageHelper {

void initHeader( const KMime::Message::Ptr &message, const KPIMIdentities::IdentityManager* identMan, uint id )
{
  applyIdentity( message, identMan, id );
  message->to()->clear();
  message->subject()->clear();
  message->date()->setDateTime( KDateTime::currentLocalDateTime() );

  // user agent, e.g. KMail/1.9.50 (Windows/5.0; KDE/3.97.1; i686; svn-762186; 2008-01-15)
  QStringList extraInfo;
#if defined KDEPIM_GIT_REVISION_STRING && defined KDEPIM_GIT_LAST_CHANGE
  extraInfo << QString::fromLocal8Bit(KDEPIM_GIT_REVISION_STRING) << QString::fromLocal8Bit(KDEPIM_GIT_LAST_CHANGE);
#else
#error forgot to include kdepim-version.h
#endif

  message->userAgent()->fromUnicodeString( KProtocolManager::userAgentForApplication( QString::fromLocal8Bit("KMail"), QString::fromLocal8Bit(KDEPIM_GIT_REVISION_STRING), extraInfo ), QLatin1String("utf-8").latin1() );
  // This will allow to change Content-Type:
  message->contentType()->setMimeType( "text/plain" );
}



void initFromMessage( const KMime::Message::Ptr &msg, const KMime::Message::Ptr &origMsg,
                      KPIMIdentities::IdentityManager* identMan, uint id, bool idHeaders )
{
  if ( idHeaders )
    MessageHelper::initHeader( msg, identMan, id );
  else {
    KMime::Headers::Generic *header = new KMime::Headers::Generic( "X-KMail-Identity", msg.get(),
                                                                   QString::number( id ), "utf-8" );
    msg->setHeader( header );
  }

  if ( origMsg->headerByType("X-KMail-Transport") ) {
    const QString transport = origMsg->headerByType("X-KMail-Transport")->asUnicodeString();
    KMime::Headers::Generic *header = new KMime::Headers::Generic( "X-KMail-Transport", msg.get(),
                                                                   transport, "utf-8" );
    msg->setHeader( header );
  }
}


void applyIdentity( const KMime::Message::Ptr &message, const KPIMIdentities::IdentityManager* identMan, uint id )
{
  const KPIMIdentities::Identity & ident =
    identMan->identityForUoidOrDefault( id );

  if ( ident.fullEmailAddr().isEmpty() )
    message->from()->clear();
  else
    message->from()->addAddress(ident.primaryEmailAddress().toUtf8(), ident.fullName());

  if ( ident.replyToAddr().isEmpty() )
    message->replyTo()->clear();
  else {
    message->replyTo()->addAddress(ident.replyToAddr().toUtf8());
  }

  if ( ident.bcc().isEmpty() )
    message->bcc()->clear();
  else {
    const KMime::Types::Mailbox::List mailboxes = MessageCore::StringUtil::mailboxListFromUnicodeString( ident.bcc() );
    foreach ( const KMime::Types::Mailbox &mailbox, mailboxes )
      message->bcc()->addAddress( mailbox );
  }

  if ( ident.cc().isEmpty() )
    message->cc()->clear();
  else {
    const KMime::Types::Mailbox::List mailboxes = MessageCore::StringUtil::mailboxListFromUnicodeString( ident.cc() );
    foreach ( const KMime::Types::Mailbox &mailbox, mailboxes )
      message->cc()->addAddress( mailbox );
  }

  if ( ident.organization().isEmpty() )
    message->removeHeader("Organization");
  else {
    KMime::Headers::Organization * const organization
           = new KMime::Headers::Organization( message.get(), ident.organization(), "utf-8" );
    message->setHeader( organization );
  }

  if (ident.isDefault())
    message->removeHeader("X-KMail-Identity");
  else {
    KMime::Headers::Generic *header = new KMime::Headers::Generic( "X-KMail-Identity", message.get(), QString::number( ident.uoid() ), "utf-8" );
    message->setHeader( header );
  }

  if (ident.transport().isEmpty())
    message->removeHeader("X-KMail-Transport");
  else {
    KMime::Headers::Generic *header = new KMime::Headers::Generic( "X-KMail-Transport", message.get(), ident.transport(), "utf-8" );
    message->setHeader( header );
  }

  if (ident.fcc().isEmpty())
    message->removeHeader("X-KMail-Fcc");
  else {
    KMime::Headers::Generic *header = new KMime::Headers::Generic( "X-KMail-Fcc", message.get(), ident.fcc(), "utf-8" );
    message->setHeader( header );
  }

  if (ident.drafts().isEmpty())
    message->removeHeader("X-KMail-Drafts");
  else {
    KMime::Headers::Generic *header = new KMime::Headers::Generic( "X-KMail-Drafts", message.get(), ident.drafts(), "utf-8" );
    message->setHeader( header );
  }

  if (ident.templates().isEmpty())
    message->removeHeader("X-KMail-Templates");
  else {
    KMime::Headers::Generic *header = new KMime::Headers::Generic( "X-KMail-Templates", message.get(), ident.templates(), "utf-8" );
    message->setHeader( header );
  }

  if (ident.disabledFcc()) {
      KMime::Headers::Generic *header = new KMime::Headers::Generic( "X-KMail-FccDisabled", message.get(), ident.templates(), "utf-8" );
      message->setHeader( header );
  } else {
      message->removeHeader("X-KMail-FccDisabled");
  }
}

KMime::Types::AddrSpecList extractAddrSpecs( const KMime::Message::Ptr &msg, const QByteArray & header )
{
  KMime::Types::AddrSpecList result;
  if ( !msg->headerByType( header ) )
    return result;

  KMime::Types::AddressList al =
      MessageCore::StringUtil::splitAddressField( msg->headerByType( header )->asUnicodeString().toUtf8() );
  KMime::Types::AddressList::const_iterator alend( al.constEnd() );
  for ( KMime::Types::AddressList::const_iterator ait = al.constBegin() ; ait != alend ; ++ait ) {
    KMime::Types::MailboxList::const_iterator mitEnd( (*ait).mailboxList.constEnd() );
    for ( KMime::Types::MailboxList::const_iterator mit = (*ait).mailboxList.constBegin() ; mit != mitEnd ; ++mit ) {
      result.push_back( (*mit).addrSpec() );
    }
  }
  return result;
}

QString cleanSubject( const KMime::Message::Ptr &msg )
{
  return cleanSubject( msg, MessageComposer::MessageComposerSettings::self()->replyPrefixes() + MessageComposer::MessageComposerSettings::self()->forwardPrefixes(),
                       true, QString() ).trimmed();
}

QString cleanSubject( const KMime::Message::Ptr &msg, const QStringList & prefixRegExps,
                      bool replace, const QString & newPrefix )
{
  return replacePrefixes( msg->subject()->asUnicodeString(), prefixRegExps, replace,
                                     newPrefix );
}

QString forwardSubject( const KMime::Message::Ptr &msg )
{
  return cleanSubject( msg, MessageComposer::MessageComposerSettings::self()->forwardPrefixes(),
                       MessageComposer::MessageComposerSettings::self()->replaceForwardPrefix(), QString::fromLatin1("Fwd:") );
}

QString replySubject( const KMime::Message::Ptr &msg )
{
  return cleanSubject( msg, MessageComposer::MessageComposerSettings::self()->replyPrefixes(),
                       MessageComposer::MessageComposerSettings::self()->replaceReplyPrefix(), QString::fromLatin1("Re:") );
}

QString replacePrefixes( const QString& str, const QStringList &prefixRegExps,
                         bool replace, const QString &newPrefix )
{
  bool recognized = false;
  // construct a big regexp that
  // 1. is anchored to the beginning of str (sans whitespace)
  // 2. matches at least one of the part regexps in prefixRegExps
  QString bigRegExp = QString::fromLatin1("^(?:\\s+|(?:%1))+\\s*")
                      .arg( prefixRegExps.join(QString::fromLatin1(")|(?:")) );
  QRegExp rx( bigRegExp, Qt::CaseInsensitive );
  if ( rx.isValid() ) {
      QString tmp = str;
      if ( rx.indexIn( tmp ) == 0 ) {
        recognized = true;
        if ( replace )
          return tmp.replace( 0, rx.matchedLength(), newPrefix + QString::fromLatin1( " " ) );
      }
  } else {
    kWarning() << "bigRegExp = \""
                   << bigRegExp << "\"\n"
                   << "prefix regexp is invalid!";
    // try good ole Re/Fwd:
    recognized = str.startsWith( newPrefix );
  }

  if ( !recognized )
    return newPrefix + QString::fromLatin1(" ") + str;
  else
    return str;
}


void setAutomaticFields(const KMime::Message::Ptr &msg, bool aIsMulti)
{
  msg->setHeader( new KMime::Headers::Generic( "MIME-Version", msg.get(), QLatin1String("1.0"), QLatin1String("utf-8").latin1() ) );

  if (aIsMulti || msg->contents().size() > 1)
  {
    // Set the type to 'Multipart' and the subtype to 'Mixed'
    msg->contentType()->setMimeType( "multipart/mixed" );
    // Create a random printable string and set it as the boundary parameter
    msg->contentType()->setBoundary( KMime::multiPartBoundary() );
  }
}

QString ccStrip( const KMime::Message::Ptr &msg )
{
  return MessageCore::StringUtil::stripEmailAddr( msg->cc()->asUnicodeString() );
}

QString toStrip( const KMime::Message::Ptr &msg )
{
  return MessageCore::StringUtil::stripEmailAddr( msg->to()->asUnicodeString() );
}

QString fromStrip( const KMime::Message::Ptr &msg )
{
  return MessageCore::StringUtil::stripEmailAddr( msg->from()->asUnicodeString() );
}


QString stripOffPrefixes( const QString& str )
{
  return replacePrefixes( str, MessageComposer::MessageComposerSettings::self()->replyPrefixes() + MessageComposer::MessageComposerSettings::self()->forwardPrefixes(),
                          true, QString() ).trimmed();
}

QString skipKeyword( const QString& aStr, QChar sepChar,
                               bool* hasKeyword)
{
  QString str = aStr;

  while (str[0] == QChar::fromLatin1(' ')) str.remove(0,1);
  if (hasKeyword) *hasKeyword=false;

  unsigned int i = 0, maxChars = 3;
  unsigned int strLength(str.length());
  for (i=0; i < strLength && i < maxChars; ++i)
  {
    if (str[i] < QChar::fromLatin1('A') || str[i] == sepChar) break;
  }

  if (str[i] == sepChar) // skip following spaces too
  {
    do {
      ++i;
    } while (str[i] == QChar::fromLatin1(' '));
    if (hasKeyword) *hasKeyword=true;
    return str.mid(i);
  }
  return str;
}


}
