/*
  Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>
  Copyright (C) 2009 Klaralvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
  Copyright (c) 2009 Leo Franchi <lfranchi@kde.org>

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

#include "attachmentjob.h"
#include "globalpart.h"
#include "infopart.h"
#include "jobbase_p.h"
#include "textpart.h"
#include "maintextjob.h"
#include "multipartjob.h"
#include "signjob.h"
#include "encryptjob.h"
#include "signencryptjob.h"
#include "skeletonmessagejob.h"
#include "transparentjob.h"

#include <QTimer>

#include <KDebug>
#include <klocalizedstring.h>

using namespace Message;
using KPIM::AttachmentPart;

class Message::ComposerPrivate : public JobBasePrivate
{
  public:
    ComposerPrivate( Composer *qq )
      : JobBasePrivate( qq )
      , started( false )
      , finished( false )
      , sign( false )
      , encrypt( false )
      , noCrypto( false )
      , globalPart( 0 )
      , infoPart( 0 )
      , textPart( 0 )
      , skeletonMessage( 0 )
      , resultContent( 0 )
    {
    }

    void init();
    void doStart(); // slot
    void composeStep1();
    void skeletonJobFinished( KJob *job ); // slot
    void composeStep2();
    void contentJobFinished( KJob *job ); // slot
    void contentJobPreCryptFinished( KJob *job ); // slot
    void contentJobPreInlineFinished( KJob *job ); // slot
    void signBeforeEncryptJobFinished( KJob *job ); // slot
    void startEncryptJobs( KMime::Content* content );
    void composeWithLateAttachments( KMime::Message* headers, KMime::Content* content, AttachmentPart::List parts, std::vector<GpgME::Key> keys, QStringList recipients );
    void attachmentsFinished( KJob* job ); // slot

    void composeFinalStep( KMime::Content* headers, KMime::Content* content );
    bool started;
    bool finished;
    bool sign;
    bool encrypt;
    bool noCrypto;

    Kleo::CryptoMessageFormat format;
    std::vector<GpgME::Key> signers;
    QList<QPair<QStringList, std::vector<GpgME::Key> > > encData;

    QList<KMime::Message::Ptr> resultMessages;

    // Stuff that the application plays with.
    GlobalPart *globalPart;
    InfoPart *infoPart;
    TextPart *textPart;
    AttachmentPart::List attachmentParts;
    // attachments with different sign/encrypt settings from
    // main message body. added at the end of the process
    AttachmentPart::List lateAttachmentParts; 


    // Stuff that we play with.
    KMime::Message *skeletonMessage;
    KMime::Content *resultContent;

    Q_DECLARE_PUBLIC( Composer )
};

void ComposerPrivate::init()
{
  Q_Q( Composer );

  // We cannot create these in ComposerPrivate's constructor, because
  // their parent q is not fully constructed at that time.
  globalPart = new GlobalPart( q );
  infoPart = new InfoPart( q );
  textPart = new TextPart( q );
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

  // Create skeleton message (containing headers only; no content).
  SkeletonMessageJob *skeletonJob = new SkeletonMessageJob( infoPart, globalPart, q );
  QObject::connect( skeletonJob, SIGNAL(finished(KJob*)), q, SLOT(skeletonJobFinished(KJob*)) );
  q->addSubjob( skeletonJob );
  skeletonJob->start();
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

  composeStep2();
}

void ComposerPrivate::composeStep2()
{
  Q_Q( Composer );

  ContentJobBase *mainJob = 0;
  MainTextJob *mainTextJob = new MainTextJob( textPart, q );
  if( attachmentParts.isEmpty() ) {
    // We have no attachments.  Use the content given by the MainTextJob.
    mainJob = mainTextJob;
  } else {
    // We have attachments.  Create a multipart/mixed content.
    QMutableListIterator<AttachmentPart::Ptr> iter( attachmentParts );
    while( iter.hasNext() ) {
      AttachmentPart::Ptr part = iter.next();
      kDebug() << "Checking attachment crypto policy..." << part->isSigned() << part->isEncrypted();
      if( !noCrypto && ( sign != part->isSigned() || encrypt != part->isEncrypted() ) ) { // different policy
        kDebug() << "got attachment with different crypto policy!";
        lateAttachmentParts.append( part );
        iter.remove();
      }
    }
    MultipartJob *multipartJob = new MultipartJob( q );
    multipartJob->setMultipartSubtype( "mixed" );
    multipartJob->appendSubjob( mainTextJob );
    foreach( AttachmentPart::Ptr part, attachmentParts ) {
      multipartJob->appendSubjob( new AttachmentJob( part ) );
    }
    mainJob = multipartJob;
  }
  if( sign && encrypt && format & Kleo::InlineOpenPGPFormat ) { // needs custom handling--- one SignEncryptJob by itself
    kDebug() << "sending to sign/enc inline job!";
    QObject::connect( mainJob, SIGNAL(finished(KJob*)), q, SLOT(contentJobPreInlineFinished(KJob*)) );
  } else if( sign || encrypt ) {
    QObject::connect( mainJob, SIGNAL(finished(KJob*)), q, SLOT(contentJobPreCryptFinished(KJob*)) );
  } else {
    QObject::connect( mainJob, SIGNAL(finished(KJob*)), q, SLOT(contentJobFinished(KJob*)) );
  }
  q->addSubjob( mainJob );
  mainJob->start();
}


void ComposerPrivate::contentJobPreInlineFinished( KJob *job )
{
  Q_Q( Composer );

  Q_ASSERT( format & Kleo::InlineOpenPGPFormat );
  Q_ASSERT( sign && encrypt ); // for safety... we shouldn't be here otherwise
  Q_ASSERT( dynamic_cast<ContentJobBase*>( job ) );
  ContentJobBase *cjob = static_cast<ContentJobBase*>( job );

  kDebug() << "creaeting inline signandenc job";
  if( encData.size() == 0 ) { // no key data! bail!
    q->setErrorText( i18n( "No key data for recipients found." ) );
    q->setError( Composer::IncompleteError );
    q->emitResult();
    return;
  }


  for( int i = 0; i < encData.size(); ++i ) {
    QPair<QStringList, std::vector<GpgME::Key> > recipients = encData[ i ];
    kDebug() << "got first list of recipients:" << recipients.first;
    SignEncryptJob* seJob = new SignEncryptJob( q );
    seJob->setContent( cjob->content() );
    seJob->setCryptoMessageFormat( format );
    seJob->setEncryptionKeys( recipients.second );
    seJob->setSigningKeys( signers );
    seJob->setRecipients( recipients.first );

    QObject::connect( seJob, SIGNAL( finished( KJob* ) ), q, SLOT( contentJobFinished( KJob* ) ) );

    q->addSubjob( seJob );
    seJob->start();
  }
}


void ComposerPrivate::contentJobPreCryptFinished( KJob *job )
{
  Q_Q( Composer );

  // we're signing or encrypting or both, so add an additional job to the process
  Q_ASSERT( dynamic_cast<ContentJobBase*>( job ) );
  ContentJobBase *cjob = static_cast<ContentJobBase*>( job );

  
  if( sign ) {
    SignJob* sJob = new SignJob( q );
    sJob->setContent( cjob->content() );
    sJob->setCryptoMessageFormat( format );
    sJob->setSigningKeys( signers );

    if( encrypt ) {
      QObject::connect( sJob, SIGNAL( finished( KJob* ) ), q, SLOT( signBeforeEncryptJobFinished( KJob* ) ) );
    } else {
      QObject::connect( sJob, SIGNAL( finished( KJob* ) ), q, SLOT( contentJobFinished( KJob* ) ) );
    }
    q->addSubjob( sJob );
    sJob->start();
    
  } else if( encrypt ) {
    // just encrypting, so setup the jobs directly
    startEncryptJobs( cjob->content() );
  }
    
}

void ComposerPrivate::signBeforeEncryptJobFinished( KJob* job )
{
  
  if( job->error() ) {
    return; // KCompositeJob takes care of the error.
  }

  Q_ASSERT( dynamic_cast<ContentJobBase*>( job ) );
  ContentJobBase *cjob = static_cast<ContentJobBase*>( job );

  // cjob holds the signed content, now we encrypt per recipient
  startEncryptJobs( cjob->content() );
         
}

void ComposerPrivate::startEncryptJobs( KMime::Content* content ) {
  Q_Q( Composer );

  // each SplitInfo holds a list of recipients/keys, if there is more than
  // one item in it then it means there are secondary recipients that need
  // different messages w/ clean headers
  kDebug() << "starting enc jobs";
  kDebug() << "format:" << format;
  kDebug() << "enc data:" << encData.size();

  if( encData.size() == 0 ) { // no key data! bail!
    q->setErrorText( i18n( "No key data for recipients found." ) );
    q->setError( Composer::IncompleteError );
    q->emitResult();
    return;
  }

  for( int i = 0; i < encData.size(); ++i ) {
    QPair<QStringList, std::vector<GpgME::Key> > recipients = encData[ i ];
    kDebug() << "got first list of recipients:" << recipients.first;
    EncryptJob* eJob = new EncryptJob( q );
    eJob->setContent( content );
    eJob->setCryptoMessageFormat( format );
    eJob->setEncryptionKeys( recipients.second );
    eJob->setRecipients( recipients.first );
    
    QObject::connect( eJob, SIGNAL( finished( KJob* ) ), q, SLOT( contentJobFinished( KJob* ) ) );

    q->addSubjob( eJob );
    eJob->start();
  }

}

void ComposerPrivate::contentJobFinished( KJob *job )
{
  Q_Q( Composer );
  
  if( job->error() ) {
    return; // KCompositeJob takes care of the error.
  }
  kDebug() << "composing final message";

  KMime::Message* headers;
  KMime::Content* resultContent;
  std::vector<GpgME::Key> keys;
  QStringList recipients;

  Q_ASSERT( dynamic_cast<ContentJobBase*>( job ) == static_cast<ContentJobBase*>( job ) );
  ContentJobBase* contentJob = static_cast<ContentJobBase*>( job );

  // create the final headers and body,
  // taking into account secondary recipients for encryption
  if( encData.size() > 1 ) { // crypto job with secondary recipients..
    Q_ASSERT( dynamic_cast<AbstractEncryptJob*>( job ) ); // we need to get the recipients for this job
    AbstractEncryptJob* eJob = dynamic_cast<AbstractEncryptJob*>( job );

    keys = eJob->encryptionKeys();
    recipients = eJob->recipients();
    
    resultContent = contentJob->content(); // content() comes from superclass
    headers = new KMime::Message;
    headers->setHeader( skeletonMessage->from() );
    headers->setHeader( skeletonMessage->to() );
    headers->setHeader( skeletonMessage->subject() );

    KMime::Headers::Bcc *bcc = new KMime::Headers::Bcc( headers );
    foreach( const QString &a, eJob->recipients() ) {
      KMime::Types::Mailbox address;
      address.fromUnicodeString( a );
      bcc->addAddress( address );
    }

    kDebug() << "got one of multiple messages sending to:" << bcc->asUnicodeString();
    kDebug() << "sending to recipients:" << recipients;
    headers->setHeader( bcc );
    headers->assemble();
  } else { // just use the saved headers from before
    if( encData.size() > 0 ) {
      kDebug() << "setting enc data:" << encData[ 0 ].first << "with num keys:" << encData[ 0 ].second.size();
      keys = encData[ 0 ].second;
      recipients = encData[ 0 ].first;
    }
    
    headers = skeletonMessage;
    resultContent = contentJob->content();
  }
  // manually remove the subjob so we can check if we have any left later
  q->removeSubjob( job );
  
  if( lateAttachmentParts.isEmpty() ) {
    composeFinalStep( headers, resultContent );
  } else {
    composeWithLateAttachments( headers, resultContent, lateAttachmentParts, keys, recipients );
  }
  
}

void ComposerPrivate::composeWithLateAttachments( KMime::Message* headers, KMime::Content* content, AttachmentPart::List parts, std::vector<GpgME::Key> keys, QStringList recipients ) 
{
  Q_Q( Composer );

  MultipartJob* multiJob = new MultipartJob( q );
  multiJob->setMultipartSubtype( "mixed" );

  // wrap the content into a job for the multijob to handle it
  TransparentJob* tJob = new TransparentJob( q );
  tJob->setContent( content );
  multiJob->appendSubjob( tJob );
  multiJob->setExtraContent( headers );
  
  kDebug() << "attachment encr key size:" << keys.size() << recipients;

  // operate correctly on each attachment that has a different crypto policy than body.
  foreach( AttachmentPart::Ptr attachment, parts ) {
    AttachmentJob* attachJob = new AttachmentJob( attachment, q );

    kDebug() << "got a late attachment";
    if( attachment->isSigned() ) {
      kDebug() << "adding signjob for late attachment";
      SignJob* sJob = new SignJob( q );
      sJob->setCryptoMessageFormat( format );
      sJob->setSigningKeys( signers );

      sJob->appendSubjob( attachJob );
      if( attachment->isEncrypted() ) {
        kDebug() << "adding sign + encrypt job for late attachment";
        EncryptJob* eJob = new EncryptJob( q );
        eJob->setCryptoMessageFormat( format );
        eJob->setEncryptionKeys( keys );
        eJob->setRecipients( recipients );

        eJob->appendSubjob( sJob );

        multiJob->appendSubjob( eJob );
      } else {
        kDebug() << "Just signing late attachment";
        multiJob->appendSubjob( sJob );
      }
    } else if( attachment->isEncrypted() ) { // only encryption
      kDebug() << "just encrypting late attachment";
      EncryptJob* eJob = new EncryptJob( q );
      eJob->setCryptoMessageFormat( format );
      eJob->setEncryptionKeys( keys );
      eJob->setRecipients( recipients );

      eJob->appendSubjob( attachJob );
      multiJob->appendSubjob( eJob );
    } else {
      kDebug() << "attaching plain non-crypto attachment";
      AttachmentJob* attachJob = new AttachmentJob( attachment, q );
      multiJob->appendSubjob( attachJob );
    }
  }

  QObject::connect( multiJob, SIGNAL( finished( KJob* ) ), q, SLOT( attachmentsFinished( KJob* ) ) );

  q->addSubjob( multiJob );
  multiJob->start();
}

void ComposerPrivate::attachmentsFinished( KJob* job ) {
  Q_Q( Composer );

  if( job->error() ) {
    return; // KCompositeJob takes care of the error.
  }
  kDebug() << "composing final message with late attachments";

  Q_ASSERT( dynamic_cast<ContentJobBase*>( job ) );
  ContentJobBase* contentJob = static_cast<ContentJobBase*>( job );

  KMime::Content* content = contentJob->content();
  KMime::Content* headers = contentJob->extraContent();

  q->removeSubjob( job );
  composeFinalStep( headers, content );
  
}

void ComposerPrivate::composeFinalStep( KMime::Content* headers, KMime::Content* content )
{
  Q_Q( Composer );

  content->assemble();

  QByteArray allData = headers->head() + content->encodedContent();
  KMime::Message::Ptr resultMessage( new KMime::Message );
  resultMessage->setContent( allData );
  resultMessage->parse(); // Not strictly necessary.
  resultMessages.append( resultMessage );

  kDebug() << "still have subjobs:" << q->hasSubjobs() << "num:" << q->subjobs().size();
  if( !q->hasSubjobs() ) {
    finished = true;
    q->emitResult();
  }
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

QList<KMime::Message::Ptr> Composer::resultMessages() const
{
  Q_D( const Composer );
  Q_ASSERT( d->finished );
  Q_ASSERT( !error() );
  QList<KMime::Message::Ptr> results = d->resultMessages;
  return results;
}

GlobalPart *Composer::globalPart()
{
  Q_D( Composer );
  return d->globalPart;
}

InfoPart* Composer::infoPart()
{
  Q_D( Composer );
  return d->infoPart;
}

TextPart *Composer::textPart()
{
  Q_D( Composer );
  return d->textPart;
}

AttachmentPart::List Composer::attachmentParts()
{
  Q_D( Composer );
  return d->attachmentParts;
}

void Composer::addAttachmentPart( AttachmentPart::Ptr part )
{
  Q_D( Composer );
  Q_ASSERT( !d->started );
  Q_ASSERT( !d->attachmentParts.contains( part ) );
  d->attachmentParts.append( part );
}

void Composer::addAttachmentParts( const AttachmentPart::List &parts )
{
  foreach( AttachmentPart::Ptr part, parts ) {
    addAttachmentPart( part );
  }
}

void Composer::removeAttachmentPart( AttachmentPart::Ptr part )
{
  Q_D( Composer );
  Q_ASSERT( !d->started );
  if( d->attachmentParts.contains( part ) ) {
    d->attachmentParts.removeAll( part );
  } else {
    kError() << "Unknown attachment part" << part;
    Q_ASSERT( false );
    return;
  }
}

void Composer::setSignAndEncrypt( const bool doSign, const bool doEncrypt )
{
  Q_D( Composer );
  d->sign = doSign;
  d->encrypt = doEncrypt;
}


void Composer::setMessageCryptoFormat( Kleo::CryptoMessageFormat format )
{
  Q_D( Composer );

  d->format = format;
}

void Composer::setSigningKeys( std::vector<GpgME::Key>& signers )
{
  Q_D( Composer );

  d->signers = signers;
}

void Composer::setEncryptionKeys( QList<QPair<QStringList, std::vector<GpgME::Key> > > encData )
{
  Q_D( Composer );

  d->encData = encData;
}

void Composer::setNoCrypto(bool noCrypto)
{
  Q_D( Composer );

  d->noCrypto = noCrypto;
}

    
void Composer::start()
{
  QTimer::singleShot( 0, this, SLOT(doStart()) );
}

#include "composer.moc"
