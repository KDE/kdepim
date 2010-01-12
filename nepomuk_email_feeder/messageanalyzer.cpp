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

#include <email.h>
#include <personcontact.h>
#include <nepomukfeederagentbase.h>
#include <attachment.h>
#include <nmo.h>
#include <mailboxdataobject.h>

#include <messagecore/messagestatus.h>
#include <messageviewer/objecttreeparser.h>
#include <messageviewer/nodehelper.h>
#include <messageviewer/partmetadata.h>

#include <akonadi/item.h>

#include <kmime/kmime_message.h>

#include <klocalizedstring.h>

#include <Nepomuk/Resource>
#include <Nepomuk/ResourceManager>
#include <Nepomuk/Variant>
#include <Nepomuk/Tag>

#include <Soprano/Client/DBusClient>
#include <Soprano/Client/DBusModel>
#include <Soprano/Model>
#include <Soprano/QueryResultIterator>
#include <Soprano/Vocabulary/NAO>

MessageAnalyzer::MessageAnalyzer(const Akonadi::Item& item, const QUrl& graphUri, NepomukFeederAgentBase* parent) :
  QObject( parent ),
  m_parent( parent ),
  m_item( item ),
  m_email( item.url(), graphUri ),
  m_graphUri( graphUri ),
  m_mainBodyPart( 0 ),
  m_nodeHelper( new MessageViewer::NodeHelper ),
  m_otp( 0 ),
  m_mainModel( 0 )
{
  NepomukFeederAgentBase::setParent( m_email, item );
  m_email.addProperty( Soprano::Vocabulary::NAO::hasSymbol(), Soprano::LiteralValue( "internet-mail" ) );

  processFlags( item.flags() );
  const KMime::Message::Ptr msg = item.payload<KMime::Message::Ptr>();
  processHeaders( msg );

  if ( !msg->body().isEmpty() || !msg->contents().isEmpty() ) {

    if ( Settings::self()->indexEncryptedContent() != Settings::NoIndexing ) {
      m_otp = new MessageViewer::ObjectTreeParser( this, m_nodeHelper );
      m_otp->setAllowAsync( true );
      m_otp->parseObjectTree( item, msg.get() );
    }

    if ( !m_otp || !m_otp->hasPendingAsyncJobs() )
      processContent( msg );

  } else {
    deleteLater();
  }
}

MessageAnalyzer::~MessageAnalyzer()
{
  delete m_otp;
}


void MessageAnalyzer::processContent(const KMime::Message::Ptr& msg)
{
  // before we walk the part node tree, let's see if there is a main plain text body, so we don't interpret that as an attachment later on
  m_mainBodyPart = msg->mainBodyPart( "text/plain" );
  if ( m_mainBodyPart ) {
    const QString text = m_mainBodyPart->decodedText( true, true );
    if ( !text.isEmpty() )
      m_email.setPlainTextMessageContents( QStringList( text ) );
  }

  processPart( msg.get() );
  deleteLater();
}

void MessageAnalyzer::processHeaders(const KMime::Message::Ptr& msg)
{
  if ( msg->subject( false ) ) {
    m_email.setMessageSubject( msg->subject()->asUnicodeString() );
    m_email.setLabel( msg->subject()->asUnicodeString() );
  }

  if ( msg->date( false ) ) {
    m_email.setReceivedDate( msg->date()->dateTime().dateTime() );
  }

  if ( msg->from( false ) ) {
    m_email.setFroms( extractContactsFromMailboxes( msg->from()->mailboxes(), graphUri() ) );
  }

  if ( msg->sender( false ) )
    m_email.setSenders( extractContactsFromMailboxes( msg->sender()->mailboxes(), graphUri() ) );

  if ( msg->to( false ) ) {
    m_email.setTos( extractContactsFromMailboxes( msg->to()->mailboxes(), graphUri() ) );
  }

  if ( msg->cc( false ) ) {
    m_email.setCcs( extractContactsFromMailboxes( msg->cc()->mailboxes(), graphUri() ) );
  }

  if ( msg->bcc( false ) ) {
    m_email.setBccs( extractContactsFromMailboxes( msg->bcc()->mailboxes(), graphUri() ) );
  }

  if ( msg->messageID( false ) ) {
    m_email.setMessageIds( QStringList( msg->messageID()->asUnicodeString() ) );
  }
}

void MessageAnalyzer::processPart(KMime::Content* content)
{
  bool resetModel = false;
  if ( Settings::self()->indexEncryptedContent() == Settings::EncryptedIndex && !m_mainModel ) {
    const MessageViewer::PartMetaData metaData = m_nodeHelper->partMetaData( content );
    kDebug() << content << metaData.isEncrypted << metaData.keyId;
    if ( metaData.isEncrypted && !metaData.keyId.isEmpty() ) {
      Soprano::Client::DBusClient dbusClient( "org.kde.nepomuk.services.nepomukstorage" );
      Soprano::Model* cryptoModel = dbusClient.createModel( metaData.keyId );
      if ( cryptoModel ) {
        m_mainModel = Nepomuk::ResourceManager::instance()->mainModel();
        Nepomuk::ResourceManager::instance()->setOverrideMainModel( cryptoModel );
        resetModel = true;
      } else {
        kWarning() << "Could not obtain cryto index model";
      }
    }
  }

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
    m_parent->indexData( m_email.uri(), content->decodedContent(), m_item.modificationTime() );
  }

  // attachment -> delegate to strigi
  else {
    const KMime::ContentIndex index = content->index();
    KUrl attachmentUrl = m_email.uri();
    attachmentUrl.setHTMLRef( index.toString() );
    kDebug() << attachmentUrl;
    NepomukFast::Attachment attachment( attachmentUrl, graphUri() );
    attachment.addProperty( Vocabulary::NIE::isPartOf(), m_email.uri() );
    attachment.addProperty( Soprano::Vocabulary::NAO::hasSymbol(), Soprano::LiteralValue( "mail-attachment" ) );
    if ( !content->contentType()->name().isEmpty() )
      attachment.setLabel( content->contentType()->name() );
    else if ( content->contentDisposition( false ) && !content->contentDisposition()->filename().isEmpty() )
      attachment.setLabel( content->contentDisposition()->filename() );
    if ( content->contentDescription( false ) && !content->contentDescription()->asUnicodeString().isEmpty() )
      attachment.addProperty( Vocabulary::NIE::description(), Soprano::LiteralValue( content->contentDescription()->asUnicodeString() ) );
    m_email.addAttachment( attachment );
    m_parent->indexData( attachmentUrl, content->decodedContent(), m_item.modificationTime() );
  }

  if ( resetModel ) {
    Nepomuk::ResourceManager::instance()->setOverrideMainModel( 0 );
    m_mainModel = 0;
  }
}

QList< NepomukFast::Contact > MessageAnalyzer::extractContactsFromMailboxes(const KMime::Types::Mailbox::List& mbs, const QUrl&graphUri )
{
  QList<NepomukFast::Contact> contacts;

  foreach( const KMime::Types::Mailbox& mbox, mbs ) {
    if ( mbox.hasAddress() ) {
      const NepomukFast::Contact c = NepomukFeederAgentBase::findOrCreateContact( QString::fromLatin1( mbox.address() ), mbox.name(), graphUri );
      contacts << c;
    }
  }

  return contacts;
}

void MessageAnalyzer::processFlags(const Akonadi::Item::Flags& flags)
{
  KPIM::MessageStatus status;
  status.setStatusFromFlags( flags );

  // the \Seen flag is in MailboxDataObject instead of Email...
  NepomukFast::MailboxDataObject mdb( m_item.url(), graphUri() );
  mdb.setIsReads( QList<bool>() << status.isRead() );

  if ( status.isImportant() )
    addTranslatedTag( "important", i18n("Important"), "mail-mark-important" );
  if ( status.isToAct() )
    addTranslatedTag( "todo", i18n("To Do"), "mail-mark-task" );
  if ( status.isWatched() )
    addTranslatedTag( "watched", i18n("Watched") );
}


void MessageAnalyzer::addTranslatedTag(const char* tagName, const QString& tagLabel, const QString &icon )
{
 Nepomuk::Tag tag( QString::fromLatin1( tagName ) );
 if ( tag.label().isEmpty() )
   tag.setLabel( tagLabel );
 if ( tag.symbols().isEmpty() && !icon.isEmpty() )
   tag.addSymbol( icon );
  m_email.addProperty( Soprano::Vocabulary::NAO::hasTag(), tag.resourceUri() );
}

void MessageAnalyzer::update(MessageViewer::Viewer::UpdateMode mode)
{
  kDebug() << m_otp->hasPendingAsyncJobs();
  const KMime::Message::Ptr msg = m_item.payload<KMime::Message::Ptr>();
  m_otp->parseObjectTree( m_item, msg.get() );
  if ( !m_otp->hasPendingAsyncJobs() )
    processContent( msg );
}

#include "messageanalyzer.moc"
