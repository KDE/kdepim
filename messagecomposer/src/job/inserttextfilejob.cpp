/*
 * Copyright 2010 Thomas McGuire <mcguire@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */
#include "inserttextfilejob.h"

#include <QTextEdit>

#include <KCharsets>
#include "messagecomposer_debug.h"
#include <KIO/Job>

#include <QTextCodec>

using namespace MessageComposer;
class MessageComposer::InsertTextFileJobPrivate
{
public:
    InsertTextFileJobPrivate(QTextEdit *editor, const QUrl &url)
        : mEditor(editor), mUrl(url)
    {

    }
    QPointer<QTextEdit> mEditor;
    QUrl mUrl;
    QString mEncoding;
    QByteArray mFileData;
};

InsertTextFileJob::InsertTextFileJob(QTextEdit *editor, const QUrl &url)
    : KJob(editor), d(new MessageComposer::InsertTextFileJobPrivate(editor, url))
{
}

InsertTextFileJob::~InsertTextFileJob()
{
    delete d;
}

void InsertTextFileJob::slotFileData(KIO::Job *job, const QByteArray &data)
{
    Q_UNUSED(job);
    d->mFileData += data;
}

void InsertTextFileJob::slotGetJobFinished(KJob *job)
{
    if (job->error()) {
        qCWarning(MESSAGECOMPOSER_LOG) << job->errorString();
        setError(job->error());
        setErrorText(job->errorText());
        emitResult();
        return;
    }

    if (d->mEditor) {
        if (!d->mEncoding.isEmpty()) {
            const QTextCodec *fileCodec = KCharsets::charsets()->codecForName(d->mEncoding);
            if (fileCodec) {
                d->mEditor->textCursor().insertText(fileCodec->toUnicode(d->mFileData.data()));
            } else {
                d->mEditor->textCursor().insertText(QString::fromLocal8Bit(d->mFileData.data()));
            }
        } else {
            d->mEditor->textCursor().insertText(QString::fromLocal8Bit(d->mFileData.data()));
        }
    }

    emitResult();
}

void InsertTextFileJob::setEncoding(const QString &encoding)
{
    d->mEncoding = encoding;
}

void InsertTextFileJob::start()
{
    KIO::TransferJob *job = KIO::get(d->mUrl);
    connect(job, &KIO::TransferJob::result, this, &InsertTextFileJob::slotGetJobFinished);
    connect(job, &KIO::TransferJob::data, this, &InsertTextFileJob::slotFileData);
    job->start();
}

