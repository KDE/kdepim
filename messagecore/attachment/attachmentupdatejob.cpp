/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include "attachmentfromfolderjob.h"
#include "attachmentfromurlbasejob.h"
#include "attachmentfromurljob.h"
#include "attachmentupdatejob.h"
#include <KUrl>
#include <KMimeType>
#include <QDebug>
#include <QTimer>
using namespace MessageCore;

class MessageCore::AttachmentUpdateJob::Private
{
public:
    Private( AttachmentUpdateJob *qq );

    void doStart(); // slot
    void loadJobResult(KJob*);

    MessageCore::AttachmentFromUrlBaseJob *createAttachmentJob(const KUrl &url);

    AttachmentUpdateJob *const q;
    AttachmentPart::Ptr mOriginalPart;
    AttachmentPart::Ptr mUpdatedPart;
};

AttachmentUpdateJob::Private::Private( AttachmentUpdateJob *qq )
    : q( qq )
{
}

void AttachmentUpdateJob::Private::doStart()
{
    Q_ASSERT( mOriginalPart );
    if (mOriginalPart->url().isEmpty()) {
        qDebug()<< " url is empty. We can't update file";
        q->setError( KJob::UserDefinedError );
        //q->setErrorText( job->errorString() );
        q->emitResult();
        return;
    }
    MessageCore::AttachmentFromUrlBaseJob *job = createAttachmentJob(mOriginalPart->url());
    connect( job, SIGNAL(result(KJob*)), q, SLOT(loadJobResult(KJob*)) );
    job->start();
}

void AttachmentUpdateJob::Private::loadJobResult(KJob *job)
{
    if( job->error() ) {
        q->setError( KJob::UserDefinedError );
        //q->setErrorText( i18n( "Could not initiate attachment compression." ) );
        q->emitResult();
        return;
    }

    Q_ASSERT( dynamic_cast<AttachmentLoadJob*>( job ) );
    AttachmentLoadJob *ajob = static_cast<AttachmentLoadJob*>( job );
    mUpdatedPart = ajob->attachmentPart();
    mUpdatedPart->setName(q->originalPart()->name());
    mUpdatedPart->setDescription(q->originalPart()->description());
    mUpdatedPart->setSigned(q->originalPart()->isSigned());
    mUpdatedPart->setEncrypted(q->originalPart()->isEncrypted());
    mUpdatedPart->setEncoding(q->originalPart()->encoding());
    mUpdatedPart->setMimeType(q->originalPart()->mimeType());
    q->emitResult(); // Success.
}

MessageCore::AttachmentFromUrlBaseJob *AttachmentUpdateJob::Private::createAttachmentJob(const KUrl &url)
{
    MessageCore::AttachmentFromUrlBaseJob *ajob = 0;
    if( KMimeType::findByUrl( url )->name() == QLatin1String( "inode/directory" ) ) {
        qDebug() << "Creating attachment from folder";
        ajob = new AttachmentFromFolderJob ( url, q );
    } else {
        ajob = new AttachmentFromUrlJob( url, q );
        qDebug() << "Creating attachment from file";
    }
    /*
    if( MessageComposer::MessageComposerSettings::maximumAttachmentSize() > 0 ) {
        ajob->setMaximumAllowedSize( MessageComposer::MessageComposerSettings::maximumAttachmentSize() );
    }
    */
    return ajob;
}


AttachmentUpdateJob::AttachmentUpdateJob(const AttachmentPart::Ptr &part, QObject *parent)
    : KJob(parent),
      d( new Private( this ) )
{
    d->mOriginalPart = part;
}

AttachmentUpdateJob::~AttachmentUpdateJob()
{
    delete d;
}

void AttachmentUpdateJob::start()
{
    QTimer::singleShot( 0, this, SLOT(doStart()) );
}

AttachmentPart::Ptr AttachmentUpdateJob::originalPart() const
{
    return d->mOriginalPart;
}

AttachmentPart::Ptr AttachmentUpdateJob::updatedPart() const
{
    return d->mUpdatedPart;
}


#include "moc_attachmentupdatejob.cpp"
