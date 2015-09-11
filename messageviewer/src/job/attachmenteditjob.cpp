/*
  Copyright (c) 2014-2015 Montel Laurent <montel@kde.org>

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

#include "attachmenteditjob.h"
#include "messageviewer_debug.h"
#include "viewer/editorwatcher.h"
#include <KMessageBox>
#include <KLocalizedString>
#include <QTemporaryFile>
#include <QUrl>
#include <AkonadiCore/ItemModifyJob>
#include <KMime/Content>

using namespace MessageViewer;

AttachmentEditJob::AttachmentEditJob(QObject *parent)
    : QObject(parent),
      mShowWarning(true),
      mMainWindow(0)
{

}

AttachmentEditJob::~AttachmentEditJob()
{
    qCDebug(MESSAGEVIEWER_LOG) << " AttachmentEditJob::~AttachmentEditJob()";
}

bool AttachmentEditJob::addAttachment(KMime::Content *node, bool showWarning)
{
    if (showWarning && KMessageBox::warningContinueCancel(mMainWindow,
            i18n("Modifying an attachment might invalidate any digital signature on this message."),
            i18n("Edit Attachment"), KGuiItem(i18n("Edit"), QStringLiteral("document-properties")), KStandardGuiItem::cancel(),
            QStringLiteral("EditAttachmentSignatureWarning"))
            != KMessageBox::Continue) {
        return false;
    }

    QTemporaryFile file;
    file.setAutoRemove(false);
    if (!file.open()) {
        qCWarning(MESSAGEVIEWER_LOG) << "Edit Attachment: Unable to open temp file.";
        return true;
    }
    file.write(node->decodedContent());
    file.flush();

    EditorWatcher *watcher =
        new EditorWatcher(QUrl::fromLocalFile(file.fileName()), QLatin1String(node->contentType()->mimeType()),
                          MessageViewer::EditorWatcher::NoOpenWithDialog, this, mMainWindow);
    mEditorWatchers[ watcher ] = node;

    connect(watcher, &EditorWatcher::editDone, this, &AttachmentEditJob::slotAttachmentEditDone);

    if ((watcher->start() != EditorWatcher::NoError)) {
        removeEditorWatcher(watcher, file.fileName());
    }
    return true;
}

void AttachmentEditJob::setMainWindow(QWidget *mainWindow)
{
    mMainWindow = mainWindow;
}

void AttachmentEditJob::slotAttachmentEditDone(MessageViewer::EditorWatcher *editorWatcher)
{
    const QString name = editorWatcher->url().path();
    if (editorWatcher->fileChanged()) {
        QFile file(name);
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray data = file.readAll();
            KMime::Content *node = mEditorWatchers[editorWatcher];
            node->setBody(data);
            file.close();

            mMessageItem.setPayloadFromData(mMessage->encodedContent());
            Akonadi::ItemModifyJob *job = new Akonadi::ItemModifyJob(mMessageItem);
            connect(job, &KJob::result, this, &AttachmentEditJob::slotItemModifiedResult);
            removeEditorWatcher(editorWatcher, name);
        }
    } else {
        removeEditorWatcher(editorWatcher, name);
        canDeleteJob();
    }
}

void AttachmentEditJob::setMessageItem(const Akonadi::Item &messageItem)
{
    mMessageItem = messageItem;
}

void AttachmentEditJob::slotItemModifiedResult(KJob *job)
{
    if (job->error()) {
        qCDebug(MESSAGEVIEWER_LOG) << "Item update failed:" << job->errorString();
    } else {
        Q_EMIT refreshMessage(mMessageItem);
    }
    canDeleteJob();
}

void AttachmentEditJob::canDeleteJob()
{
    if (mEditorWatchers.isEmpty()) {
        deleteLater();
    }
}

void AttachmentEditJob::removeEditorWatcher(MessageViewer::EditorWatcher *editorWatcher, const QString &name)
{
    mEditorWatchers.remove(editorWatcher);
    QFile::remove(name);
}

void AttachmentEditJob::setMessage(const KMime::Message::Ptr &message)
{
    mMessage = message;
}

