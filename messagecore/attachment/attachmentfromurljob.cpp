/*
  Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>

  Parts based on KMail code by various authors.

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

#include "attachmentfromurljob.h"

#include "messagecore_debug.h"
#include <KIO/Scheduler>
#include <KIO/TransferJob>
#include <KLocalizedString>
#include <KFormat>

#include <QFileInfo>
#include <QMimeDatabase>
#include <QMimeType>
#include <QUrlQuery>

using namespace MessageCore;

class Q_DECL_HIDDEN MessageCore::AttachmentFromUrlJob::Private
{
public:
    Private(AttachmentFromUrlJob *qq);

    void transferJobData(KIO::Job *job, const QByteArray &jobData);
    void transferJobResult(KJob *job);

    AttachmentFromUrlJob *const q;
    QByteArray mData;
};

AttachmentFromUrlJob::Private::Private(AttachmentFromUrlJob *qq)
    : q(qq)
{
}

void AttachmentFromUrlJob::Private::transferJobData(KIO::Job *job, const QByteArray &jobData)
{
    Q_UNUSED(job);
    mData += jobData;
}

void AttachmentFromUrlJob::Private::transferJobResult(KJob *job)
{
    if (job->error()) {
        // TODO this loses useful stuff from KIO, like detailed error descriptions, causes+solutions,
        // ... use UiDelegate somehow?
        q->setError(job->error());
        q->setErrorText(job->errorString());
        q->emitResult();
        return;
    }

    Q_ASSERT(dynamic_cast<KIO::TransferJob *>(job));
    KIO::TransferJob *transferJob = static_cast<KIO::TransferJob *>(job);

    // Determine the MIME type and filename of the attachment.
    const QString mimeTypeName = transferJob->mimetype();
    qCDebug(MESSAGECORE_LOG) << "Mimetype is" << mimeTypeName;

    QString fileName = q->url().fileName();
    if (fileName.isEmpty()) {
        QMimeDatabase db;
        const auto mimeType = db.mimeTypeForName(mimeTypeName);
        if (mimeType.isValid()) {
            fileName = i18nc("a file called 'unknown.ext'", "unknown%1",
                             mimeType.preferredSuffix());
        } else {
            fileName = i18nc("a file called 'unknown'", "unknown");
        }
    }

    // Create the AttachmentPart.
    Q_ASSERT(q->attachmentPart() == 0);   // Not created before.

    AttachmentPart::Ptr part = AttachmentPart::Ptr(new AttachmentPart);
    QUrlQuery query(q->url());
    const QString value = query.queryItemValue(QStringLiteral("charset"));
    part->setCharset(value.toLatin1());
    part->setMimeType(mimeTypeName.toLatin1());
    part->setName(fileName);
    part->setFileName(fileName);
    part->setData(mData);
    part->setUrl(q->url());
    q->setAttachmentPart(part);
    q->emitResult(); // Success.
}

AttachmentFromUrlJob::AttachmentFromUrlJob(const QUrl &url, QObject *parent)
    : AttachmentFromUrlBaseJob(url, parent),
      d(new Private(this))
{
}

AttachmentFromUrlJob::~AttachmentFromUrlJob()
{
    delete d;
}

void AttachmentFromUrlJob::doStart()
{
    if (!url().isValid()) {
        setError(KJob::UserDefinedError);
        setErrorText(i18n("\"%1\" not found. Please specify the full path.", url().toDisplayString()));
        emitResult();
        return;
    }

    if (maximumAllowedSize() != -1 && url().isLocalFile()) {
        const qint64 size = QFileInfo(url().toLocalFile()).size();
        if (size > maximumAllowedSize()) {
            setError(KJob::UserDefinedError);
            setErrorText(i18n("You may not attach files bigger than %1. Share it with storage service.",
                              KFormat().formatByteSize(maximumAllowedSize())));
            emitResult();
            return;
        }
    }

    Q_ASSERT(d->mData.isEmpty());   // Not started twice.

    KIO::TransferJob *job = KIO::get(url(), KIO::NoReload,
                                     (uiDelegate() ? KIO::DefaultFlags : KIO::HideProgressInfo));
    QObject::connect(job, SIGNAL(result(KJob*)),
                     this, SLOT(transferJobResult(KJob*)));
    QObject::connect(job, SIGNAL(data(KIO::Job*,QByteArray)),
                     this, SLOT(transferJobData(KIO::Job*,QByteArray)));
}

#include "moc_attachmentfromurljob.cpp"
