/*
  Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>

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

#include "composer.h"

#include "attachmentpart.h"
#include "finalmessage_p.h"
#include "globalpart.h"
#include "infopart.h"
#include "jobbase_p.h"
#include "textpart.h"
#include "maintextjob.h"
#include "multipartjob.h"
#include "skeletonmessagejob.h"

#include <QTimer>

#include <KDebug>

using namespace MessageComposer;
using namespace KMime;

class MessageComposer::ComposerPrivate : public JobBasePrivate
{
  public:
    ComposerPrivate( Composer *qq )
      : JobBasePrivate( qq )
      , started( false )
      , finished( false )
      , globalPart( 0 )
      , infoPart( 0 )
      , textPart( 0 )
      , skeletonMessage( 0 )
      , contentBeforeCrypto( 0 )
#if 0
      , pendingCryptoJobs( 0 )
#endif
    {
    }

    void init();
    void doStart(); // slot
    void composeStep1();
    void splitAttachmentsIntoEarlyLate();
    void skeletonJobFinished( KJob *job ); // slot
    void composeStep2();
    void beforeCryptoJobFinished( KJob *job ); // slot
    void composeStep3();
    FinalMessage *createFinalMessage( Content *content );

    bool started;
    bool finished;
    FinalMessage::List messages;

    // Stuff that the application plays with.
    GlobalPart *globalPart;
    InfoPart *infoPart;
    TextPart *textPart;
    AttachmentPart::List attachmentParts;

    // Stuff that we play with.
    AttachmentPart::List earlyAttachments;
    AttachmentPart::List lateAttachments;
    Message *skeletonMessage;
    Content *contentBeforeCrypto;
    // TODO crypto: for each resulting message, keep track of its recipient.
    // See keyresolver in kmail...
    // We need a structure to keep track of a recipient (list) as well as type
    // (to or cc or bcc), and createFinalMessage needs to honour it.
#if 0
    int pendingCryptoJobs;
    QList<Content*> contentsAfterCrypto;
#endif

    Q_DECLARE_PUBLIC( Composer )
};

void ComposerPrivate::init()
{
  Q_Q( Composer );
  globalPart = new GlobalPart( q );
  infoPart = new InfoPart( q );
  textPart = new TextPart( q );
  // FIXME: If I do these in ComposerPrivate's constructor, I get weird
  // "Cannot create children for a parent that is in a different thread"
  // errors. WTF? Something to do with construction order... I think q is
  // not fully constructed by the time it is passed as a parent for these
  // part QObjects.
}

void ComposerPrivate::doStart()
{
  Q_ASSERT( !started );
  started = true;
  composeStep1();
}

void ComposerPrivate::composeStep1()
{
  Q_Q( Composer );

  // Split the attachments into early and late.  (see DESIGN)
  splitAttachmentsIntoEarlyLate();

  // Create skeleton message (containing headers only; no content).
  SkeletonMessageJob *skeletonJob = new SkeletonMessageJob( infoPart, q );
  QObject::connect( skeletonJob, SIGNAL(finished(KJob*)), q, SLOT(skeletonJobFinished(KJob*)) );
  q->addSubjob( skeletonJob );
  skeletonJob->start();
}

void ComposerPrivate::splitAttachmentsIntoEarlyLate()
{
  // TODO
  earlyAttachments = attachmentParts;
}

void ComposerPrivate::skeletonJobFinished( KJob *job )
{
  if( job->error() ) {
    return; // KCompositeJob takes care of the error.
  }

  Q_ASSERT( dynamic_cast<SkeletonMessageJob*>( job ) );
  SkeletonMessageJob *sjob = static_cast<SkeletonMessageJob*>( job );
  // SkeletonMessageJob is a special job creating a Message instead of a Content.
  Q_ASSERT( skeletonMessage == 0 );
  skeletonMessage = sjob->message();
  Q_ASSERT( skeletonMessage );
  skeletonMessage->assemble();
  kDebug() << "encoded content of skeleton" << skeletonMessage->encodedContent();

  composeStep2();
}

void ComposerPrivate::composeStep2()
{
  Q_Q( Composer );

  ContentJobBase *beforeCryptoJob = 0;
  // Create contentBeforeCrypto from the main text part and early attachments.
  if( earlyAttachments.isEmpty() ) {
    // We have no attachments.  Use whatever content the textPart gives us.
    beforeCryptoJob = new MainTextJob( textPart, q );
  } else {
    // We have attachments.  Create a multipart/mixed content.
    // TODO
    Q_ASSERT( false );
#if 0
    beforeCryptoJob = new MultipartJob( q );
    beforeCryptoJob->setMultipartSubtype( "mixed" );
    new MainTextJob( textPart, beforeCryptoJob );
    foreach( AttachmentPart *part, earlyAttachments ) {
      new AttachmentJob( part, beforeCryptoJob );
    }
#endif
  }
  QObject::connect( beforeCryptoJob, SIGNAL(finished(KJob*)), q, SLOT(beforeCryptoJobFinished(KJob*)) );
  q->addSubjob( beforeCryptoJob );
  beforeCryptoJob->start();
}

void ComposerPrivate::beforeCryptoJobFinished( KJob *job )
{
  if( job->error() ) {
    return; // KCompositeJob takes care of the error.
  }

  Q_ASSERT( dynamic_cast<ContentJobBase*>( job ) );
  ContentJobBase *cjob = static_cast<ContentJobBase*>( job );
  contentBeforeCrypto = cjob->content();
  contentBeforeCrypto->assemble();

  composeStep3();
}

void ComposerPrivate::composeStep3()
{
  Q_Q( Composer );

  // (temporary until crypto) Compose final message.
  FinalMessage *msg = createFinalMessage( contentBeforeCrypto );
  delete contentBeforeCrypto;
  contentBeforeCrypto = 0;
  messages << msg;
  kDebug() << "Finished composing the single lousy unencrypted message.";
  finished = true;
  q->emitResult();

#if 0
  // The contentBeforeCrypto is done; now create contentsAfterCrypto.
  QList<Job*> afterCryptoJobs = createAfterCryptoJobs();
  pendingCryptoJobs = afterCryptoJobs.count();
  foreach( const Job *cryptoJob, afterCryptoJobs ) {
    connect( cryptoJob, SIGNAL(finished(KJob*)), q, SLOT(afterCryptoJobFinished(KJob*)) );
    cryptoJob->start();
  }
#endif
}

#if 0
void ComposerPrivate::afterCryptoJobFinished( KJob *job )
{
  if( job->error() ) {
    return; // KCompositeJob takes care of the error.
  }

  Q_ASSERT( dynamic_cast<Job*>( job ) );
  Job *cryptoJob = static_cast<Job*>( job );
  contentsAfterCrypto << cryptoJob->content();
  pendingCryptoJobs--;
  Q_ASSERT( pendingCryptoJobs >= 0 );
  if( pendingCryptoJobs == 0 ) {
    // All contentsAfterCrypto are done; now add the late attachments.
  }
}
#endif

FinalMessage *ComposerPrivate::createFinalMessage( Content *content )
{
  Q_ASSERT( skeletonMessage );
  Message *message = new Message;
  // Merge the headers from skeletonMessage with the headers + content
  // of beforeCryptoJobFinished.  FIXME HACK There should be a better way.
  QByteArray allData = skeletonMessage->head() + content->encodedContent();
  message->setContent( allData );
  message->parse();
#if 0 // this was the second attempt
  QByteArray head = skeletonMessage->head() + content->head();
  kDebug() << "head" << head;
  QByteArray body = content->body();
  kDebug() << "body" << body;
  message->setHead( head );
  message->setBody( body );

  // With the above, for some reason the CTE thinks it has already encoded
  // the content, and so we get non-encoded content :-/
#endif
#if 0 // this was the first attempt
  // Copy the content.  FIXME There should be an easier way.
  content->assemble();
  message->setContent( content->encodedContent() );
  kDebug() << "encoded content" << content->encodedContent();
  
  // Extract the headers from the skeletonMessage and copy them.
  // FIXME There should be an easier way.
  QByteArray head = skeletonMessage->head();
  while( true ) {
    Headers::Base *header = skeletonMessage->nextHeader( head ); // Shouldn't this be static?
    if( !header ) {
      break;
    }
    kDebug() << "header from skeleton" << header->as7BitString();
    message->setHeader( header );
    header->setParent( message );
    kDebug() << "created header @" << header;

    // The above SIGSEGVs for a reason I don't understand.
    // (when Akonadi tries to serialize the item)
  }
#endif

  // Assemble the FinalMessage.
#if 0 // part of 2nd attempt debugging
  //message->parse();
  {
    kDebug() << "for the bloody content:";
    Headers::ContentTransferEncoding *cte = content->contentTransferEncoding( false );
    kDebug() << "CTE" << (cte ? cte->as7BitString() : "NULL");
    if( cte ) {
      kDebug() << "decoded" << cte->decoded() << "needToEncode" << cte->needToEncode();
    }
  }
  {
    kDebug() << "for the bloody message:";
    Headers::ContentTransferEncoding *cte = message->contentTransferEncoding( false );
    kDebug() << "CTE" << (cte ? cte->as7BitString() : "NULL");
    if( cte ) {
      kDebug() << "decoded" << cte->decoded() << "needToEncode" << cte->needToEncode();
    }
  }
#endif
  //message->assemble();
  kDebug() << "encoded message after assembly" << message->encodedContent();
  FinalMessage *finalMessage = new FinalMessage( message );
  finalMessage->d->hasCustomHeaders = false; // TODO save those if not sending...
  finalMessage->d->transportId = infoPart->transportId();

  // TODO will need to change this for crypto...
  finalMessage->d->from = infoPart->from();
  finalMessage->d->to = infoPart->to();
  finalMessage->d->cc = infoPart->cc();
  finalMessage->d->bcc = infoPart->bcc();

  return finalMessage;
}



Composer::Composer( QObject *parent )
  : JobBase( *new ComposerPrivate( this ), parent )
{
  Q_D( Composer );
  d->init();
}

Composer::~Composer()
{
}

FinalMessage::List Composer::messages() const
{
  Q_D( const Composer );
  Q_ASSERT( d->finished );
  Q_ASSERT( !error() );
  return d->messages;
}

GlobalPart *Composer::globalPart()
{
  Q_D( Composer );
  return d->globalPart;
}

InfoPart *Composer::infoPart()
{
  Q_D( Composer );
  return d->infoPart;
}

TextPart *Composer::textPart()
{
  Q_D( Composer );
  return d->textPart;
}

QList<AttachmentPart*> Composer::attachmentParts()
{
  Q_D( Composer );
  return d->attachmentParts;
}

void Composer::addAttachmentPart( AttachmentPart *part )
{
  Q_D( Composer );
  Q_ASSERT( !d->started );
  Q_ASSERT( !d->attachmentParts.contains( part ) );
  d->attachmentParts.append( part );
  part->setParent( this );
}

void Composer::removeAttachmentPart( AttachmentPart *part, bool del )
{
  Q_D( Composer );
  Q_ASSERT( !d->started );
  if( d->attachmentParts.contains( part ) ) {
    d->attachmentParts.removeAll( part );
  } else {
    kWarning() << "Unknown attachment part" << part;
    Q_ASSERT( false );
    return;
  }

  if( del ) {
    delete part;
  } else {
    part->setParent( 0 );
  }
}

void Composer::start()
{
  QTimer::singleShot( 0, this, SLOT(doStart()) );
}

#include "composer.moc"
