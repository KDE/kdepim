/*
    Copyright (C) 2011  Martin Bednár <serafean@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "attachmentfromfolderjob.h"

#include "messagecore_debug.h"
#include <KLocalizedString>

#include <QBuffer>
#include <QDir>
#include <QScopedPointer>

static const mode_t archivePerms = S_IFREG | 0644;

using namespace MessageCore;

class AttachmentFromFolderJob::Private
{

public:
    Private(AttachmentFromFolderJob *qq);

    void compressFolder();
    void addEntity(const QFileInfoList &f, const QString &path);

    AttachmentFromFolderJob *const q;
    KZip::Compression mCompression;
    AttachmentPart::Ptr mCompressedFolder;
    QScopedPointer<KZip> mZip;
    QDateTime mArchiveTime;
};

AttachmentFromFolderJob::Private::Private(AttachmentFromFolderJob *qq) :
    q(qq),
    mCompression(KZip::DeflateCompression),
    mZip(Q_NULLPTR),
    mArchiveTime(QDateTime::currentDateTime())
{
}

void AttachmentFromFolderJob::Private::compressFolder()
{
    qCDebug(MESSAGECORE_LOG) << "starting compression";
    QString fileName = q->url().fileName();
    QByteArray array;
    QBuffer dev(&array);
    mZip.reset(new KZip(&dev));
    if (!mZip->open(QIODevice::WriteOnly)) {
        q->setError(KJob::UserDefinedError);
        q->setErrorText(i18n("Could not create compressed file."));
        q->emitResult();
        return;
    }
    mZip->setCompression(mCompression);
    mZip->writeDir(q->url().fileName(), QString(), QString(), 040755, mArchiveTime, mArchiveTime, mArchiveTime);
    qCDebug(MESSAGECORE_LOG) << "writing root directory : " << q->url().fileName();
    addEntity(QDir(q->url().path()).entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot |
              QDir::NoSymLinks | QDir::Files, QDir::DirsFirst), fileName + QLatin1Char('/'));
    mZip->close();

    Q_ASSERT(mCompressedFolder == 0);

    mCompressedFolder = AttachmentPart::Ptr(new AttachmentPart);
    const QString newName = fileName + QLatin1String(".zip");
    mCompressedFolder->setName(newName);
    mCompressedFolder->setFileName(newName);
    mCompressedFolder->setMimeType("application/zip");
    mCompressedFolder->setUrl(q->url());
    //     mCompressedFolder->setCompressed( true );
    mCompressedFolder->setData(array);
    //     mCompressedFolder->setCompressible(false);
    q->setAttachmentPart(mCompressedFolder);
    q->emitResult();

    //TODO:add allowCompression bool to AttachmentPart && modify GUI to disable decompressing.
    //  Or leave attachment as uncompressed and let it be compressed again?
}

void AttachmentFromFolderJob::Private::addEntity(const QFileInfoList &f, const QString &path)
{
    foreach (const QFileInfo &info, f) {
        qCDebug(MESSAGECORE_LOG) << q->maximumAllowedSize() << "Attachment size : " << mZip->device()->size();

        if (q->maximumAllowedSize() != -1 && mZip->device()->size() > q->maximumAllowedSize()) {
            q->setError(KJob::UserDefinedError);
            q->setErrorText(i18n
                            ("The resulting attachment would be larger than the maximum allowed size, aborting."));
            q->emitResult();
            return;
        }

        if (info.isDir()) {
            qCDebug(MESSAGECORE_LOG) << "adding directory " << info.fileName() << "to zip";
            if (!mZip->writeDir(path + info.fileName(), QString(), QString(), 040755, mArchiveTime, mArchiveTime, mArchiveTime)) {
                q->setError(KJob::UserDefinedError);
                q->setErrorText(i18n("Could not add %1 to the archive", info.fileName()));
                q->emitResult();
            }
            addEntity(QDir(info.filePath()).entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot |
                      QDir::NoSymLinks | QDir::Files, QDir::DirsFirst), path + info.fileName() + QLatin1Char('/'));
        }

        if (info.isFile()) {
            qCDebug(MESSAGECORE_LOG) << "Adding file " << path + info.fileName() << "to zip";
            QFile file(info.filePath());
            if (!file.open(QIODevice::ReadOnly)) {
                q->setError(KJob::UserDefinedError);
                q->setErrorText(i18n("Could not open %1 for reading.", file.fileName()));
                q->emitResult();
            }
            if (!mZip->writeFile(path + info.fileName(), QString(), QString(),
                                 file.readAll().constData(), file.size(), archivePerms, mArchiveTime, mArchiveTime, mArchiveTime)) {
                q->setError(KJob::UserDefinedError);
                q->setErrorText(i18n("Could not add %1 to the archive", file.fileName()));
                q->emitResult();
            }
            file.close();
        }
    }
}

AttachmentFromFolderJob::AttachmentFromFolderJob(const QUrl &url, QObject *parent) :
    AttachmentFromUrlBaseJob(url, parent),
    d(new Private(this))
{
}

AttachmentFromFolderJob::~AttachmentFromFolderJob()
{
    delete d;
}

void AttachmentFromFolderJob::setCompression(KZip::Compression compression)
{
    d->mCompression = compression;
}

KZip::Compression AttachmentFromFolderJob::compression() const
{
    return d->mCompression;
}

void AttachmentFromFolderJob::doStart()
{
    d->compressFolder();
}

