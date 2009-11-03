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

#include "attachmentjob.h"
#include "globalpart.h"
#include "infopart.h"
#include "jobbase_p.h"
#include "textpart.h"
#include "maintextjob.h"
#include "multipartjob.h"
#include "cryptojob.h"
#include "signjob.h"
#include "encryptjob.h"
#include "skeletonmessagejob.h"

#include <QTimer>

#include <KDebug>

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
    void signBeforeEncryptJobFinished( KJob *job ); // slot
    void startEncryptJobs( KMime::Content* content );
    
    bool started;
    bool finished;
    bool sign;
    bool encrypt;

    Kleo::CryptoMessageFormat format;
    std::vector<GpgME::Key> signers;
    QList<QPair<QStringList, std::vector<GpgME::Key> > > encData;
    
    // if some of the attachments have a different
    // sign/encrypt policy than the message itself
    bool attachmentsWithDifferentPolicy;
    KMime::Message::List resultMessages;

    // Stuff that the application plays with.
    GlobalPart *globalPart;
    InfoPart *infoPart;
    TextPart *textPart;
    AttachmentPart::List attachmentParts;


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
  SkeletonMessageJob *skeletonJob = new SkeletonMessageJob( infoPart, q );
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
  kDebug() << "encoded content of skeleton" << skeletonMessage->encodedContent();

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
    if( sign || encrypt ) {
        QObject::connect( mainJob, SIGNAL(finished(KJob*)), q, SLOT(contentJobPreCryptFinished(KJob*)) );
    } else {
        QObject::connect( mainJob, SIGNAL(finished(KJob*)), q, SLOT(contentJobFinished(KJob*)) );
    }
  } else {
    // We have attachments.  Create a multipart/mixed content.
    MultipartJob *multipartJob = new MultipartJob( q );
    multipartJob->setMultipartSubtype( "mixed" );
    multipartJob->appendSubjob( mainTextJob );
    foreach( AttachmentPart::Ptr part, attachmentParts ) {
      multipartJob->appendSubjob( new AttachmentJob( part ) );
    } // TODO sign and encrypt each attachment potentially differently
    QObject::connect( mainJob, SIGNAL(finished(KJob*)), q, SLOT(contentJobFinished(KJob*)) );
  }
  q->addSubjob( mainJob );
  mainJob->start();
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
  Q_Q( Composer );
  
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
  for( int i = 0; i < encData.size(); ++i ) {
    QPair<QStringList, std::vector<GpgME::Key> > recipients = encData[ i ];
    EncryptJob* eJob = new EncryptJob( q );
    eJob->setContent( content );
    eJob->setCryptoMessageFormat( format );
    eJob->setEncryptionKeys( recipients.second );

    CryptoJob* makeJob = new CryptoJob( q );

    makeJob->setCryptoMessageFormat( format );
    makeJob->setSignEncrypt( sign, encrypt );
    makeJob->appendSubjob( eJob );
    // old msg is where headers, ct, cte, etc come from
    makeJob->setContent( content );
    makeJob->setRecipients( recipients.first );

    QObject::connect( makeJob, SIGNAL( finished( KJob* ) ), q, SLOT( contentJobFinished( KJob* ) ) );

    q->addSubjob( makeJob );
    makeJob->start();
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
  
  CryptoJob *cjob = dynamic_cast<CryptoJob*>( job );
  if( cjob && encData.size() > 1 ) { // crypto job, need to set custom recipients

    resultContent = cjob->content();
    headers = new KMime::Message;
    headers->setHeader( skeletonMessage->from() );
    headers->setHeader( skeletonMessage->subject() );

    KMime::Headers::To *to = new KMime::Headers::To( headers );
    foreach( const QString &a, cjob->recipients() ) {
      KMime::Types::Mailbox address;
      address.fromUnicodeString( a );
      to->addAddress( address );
    }
    headers->setHeader( to );
  } else { // just use the saved headers from before
    headers = skeletonMessage;
    
    Q_ASSERT( dynamic_cast<ContentJobBase*>( job ) );
    ContentJobBase* contentJob = static_cast<ContentJobBase*>( job );

    resultContent = contentJob->content();
  }
  
  QByteArray allData = headers->head() + resultContent->encodedContent();
  KMime::Message* resultMessage =  new KMime::Message;
  resultMessage->setContent( allData );
  resultMessage->parse(); // Not strictly necessary.
  resultMessages.append( resultMessage );

  q->removeSubjob( job );
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

KMime::Message::List Composer::resultMessages() const
{
  Q_D( const Composer );
  Q_ASSERT( d->finished );
  Q_ASSERT( !error() );
  KMime::Message::List results = d->resultMessages;
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

    
void Composer::start()
{
  QTimer::singleShot( 0, this, SLOT(doStart()) );
}

#include "composer.moc"
