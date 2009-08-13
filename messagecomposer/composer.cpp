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
using KPIM::AttachmentPart;

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
      , resultContent( 0 )
    {
    }

    void init();
    void doStart(); // slot
    void composeStep1();
    void skeletonJobFinished( KJob *job ); // slot
    void composeStep2();
    void contentJobFinished( KJob *job ); // slot
    void composeStep3();

    bool started;
    bool finished;
    KMime::Message::Ptr resultMessage;

    // Stuff that the application plays with.
    GlobalPart *globalPart;
    InfoPart *infoPart;
    TextPart *textPart;
    AttachmentPart::List attachmentParts;

    // Stuff that we play with.
    Message *skeletonMessage;
    Content *resultContent;

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
  } else {
    // We have attachments.  Create a multipart/mixed content.
    MultipartJob *multipartJob = new MultipartJob( q );
    multipartJob->setMultipartSubtype( "mixed" );
    multipartJob->appendSubjob( mainTextJob );
#if 0
    foreach( AttachmentPart::Ptr part, attachmentParts ) {
      multipartJob->appendSubjob( new AttachmentJob( part ) );
    }
#endif
    mainJob = multipartJob;
  }
  q->addSubjob( mainJob );
  QObject::connect( mainJob, SIGNAL(finished(KJob*)), q, SLOT(contentJobFinished(KJob*)) );
  mainJob->start();
}

void ComposerPrivate::contentJobFinished( KJob *job )
{
  if( job->error() ) {
    return; // KCompositeJob takes care of the error.
  }

  Q_ASSERT( dynamic_cast<ContentJobBase*>( job ) );
  ContentJobBase *cjob = static_cast<ContentJobBase*>( job );
  resultContent = cjob->content();
  resultContent->assemble();

  composeStep3();
}

void ComposerPrivate::composeStep3()
{
  Q_Q( Composer );

  // Compose the final resulting message.
  QByteArray allData = skeletonMessage->head() + resultContent->encodedContent();
  resultMessage = Message::Ptr( new Message );
  resultMessage->setContent( allData );
  resultMessage->parse(); // Not strictly necessary.
  delete resultContent;
  resultContent = 0;
  kDebug() << "Finished composing the message.";
  finished = true;
  q->emitResult();
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

Message::Ptr Composer::resultMessage() const
{
  Q_D( const Composer );
  Q_ASSERT( d->finished );
  Q_ASSERT( !error() );
  return d->resultMessage;
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

void Composer::start()
{
  QTimer::singleShot( 0, this, SLOT(doStart()) );
}

#include "composer.moc"
