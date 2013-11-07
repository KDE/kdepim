/*
    Copyright (c) 2006, 2009 Volker Krause <vkrause@kde.org>
    Copyright (c) 2008 Sebastian Trueg <trueg@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "messageanalyzer.h"

#include "settings.h"

#include <akonadi/item.h>
#include <akonadi/kmime/messagestatus.h>
#include <kglobal.h>
#include <klocalizedstring.h>
#include <kmime/kmime_message.h>
#include <messageviewer/viewer/objecttreeparser.h>
#include <messageviewer/viewer/nodehelper.h>
#include <messageviewer/viewer/partmetadata.h>

#include <QtCore/QDir>
#include <QtCore/QMetaMethod>

#include <boost/scoped_ptr.hpp>

using namespace StrigiEndAnalyzer;

MessageAnalyzer::MessageAnalyzer( const KMime::Message::Ptr &message, MailEndAnalyzer* analyzer )
  : m_analyzer( analyzer ),
    m_message( message ),
    m_mainBodyPart( 0 ),
    m_nodeHelper( new MessageViewer::NodeHelper ),
    m_otp( 0 )
{
}

MessageAnalyzer::~MessageAnalyzer()
{
  delete m_otp;
  delete m_nodeHelper;
}

void MessageAnalyzer::analyze()
{
  processHeaders( m_message );

  if ( !m_message->body().isEmpty() || !m_message->contents().isEmpty() ) {

    if ( Settings::self()->indexEncryptedContent() != Settings::NoIndexing ) {
      m_otp = new MessageViewer::ObjectTreeParser( this, m_nodeHelper );
      m_otp->setAllowAsync( true );
      m_otp->parseObjectTree( m_message.get() );
    }

    if ( !m_otp || !m_otp->hasPendingAsyncJobs() )
      processContent( m_message );

  } else {
    emit finished();
  }
}

void MessageAnalyzer::processContent( const KMime::Message::Ptr &message )
{
  // before we walk the part node tree, let's see if there is a main plain text body, so we don't interpret that as an attachment later on
  m_mainBodyPart = message->mainBodyPart( "text/plain" );
  if ( m_mainBodyPart ) {
    const QString text = m_mainBodyPart->decodedText( true, true );
    if ( !text.isEmpty() ) {
      m_analyzer->addValue( MailEndAnalyzer::MessageContentField, text );
    }
  }

  processPart( message.get() );
  emit finished();
}

void MessageAnalyzer::processHeaders( const KMime::Message::Ptr &message )
{
  if ( message->subject( false ) )
    m_analyzer->addValue( MailEndAnalyzer::SubjectField, message->subject()->asUnicodeString() );

  if ( message->date( false ) )
    m_analyzer->addValue( MailEndAnalyzer::SentDateField, message->date()->dateTime().date().toString( "yyyyMMdd" ) );

  if ( message->from( false ) )
    addMailboxValues( MailEndAnalyzer::FromField, message->from()->mailboxes() );

  if ( message->sender( false ) )
    addMailboxValues( MailEndAnalyzer::SenderField, message->sender()->mailboxes() );

  if ( message->to( false ) )
    addMailboxValues( MailEndAnalyzer::ToField, message->to()->mailboxes() );

  if ( message->cc( false ) )
    addMailboxValues( MailEndAnalyzer::CcField, message->cc()->mailboxes() );

  if ( message->bcc( false ) )
    addMailboxValues( MailEndAnalyzer::BccField, message->bcc()->mailboxes() );

  if ( message->inReplyTo( false ) )
    m_analyzer->addValue( MailEndAnalyzer::InReplyToField, message->inReplyTo()->asUnicodeString() );

  if ( message->references( false ) )
    m_analyzer->addValue( MailEndAnalyzer::ReferencesField, message->references()->asUnicodeString() );

  if ( message->messageID( false ) )
    m_analyzer->addValue( MailEndAnalyzer::MessageIdField, message->messageID()->asUnicodeString() );
}

void MessageAnalyzer::processPart( KMime::Content *content )
{
  // multipart -> recurse
  if ( content->contentType()->isMultipart() ) {
    if ( content->contentType()->isSubtype( "encrypted" ) && Settings::self()->indexEncryptedContent() == Settings::NoIndexing )
      return;
    // TODO what about multipart/alternative?
    foreach ( KMime::Content* child, content->contents() )
      processPart( child );
  }

  // plain text main body part, we already dealt with that
  else if ( content == m_mainBodyPart ) {
  }

  // non plain text main body part, let strigi figure out what to do about that
  else if ( !m_mainBodyPart ) {
    m_mainBodyPart = content;
    //TODO: index data?
  }

  // attachment -> delegate to strigi
  else {
    //TODO: index data?
  }
}

void MessageAnalyzer::addMailboxValues( MailEndAnalyzer::Field field, const KMime::Types::Mailbox::List &mailboxes )
{
  foreach ( const KMime::Types::Mailbox &mailbox, mailboxes ) {
    if ( mailbox.hasAddress() )
      m_analyzer->addValue( field, mailbox.prettyAddress() );
  }
}

void MessageAnalyzer::update( MessageViewer::Viewer::UpdateMode mode )
{
  Q_UNUSED( mode );
  m_otp->parseObjectTree( m_message.get() );
  if ( !m_otp->hasPendingAsyncJobs() )
    processContent( m_message );
}

