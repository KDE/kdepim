/*
  Copyright (c) 2010 Klarlvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    Author: Allen Winter <allen.winter@kdab.com>
  Copyright (C) 2014 Sergio Martins <iamsergio@gmail.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/
/**
  @file
  This file is part of the API for handling calendar data and provides
  static functions for dealing with calendar incidence attachments.

  @brief
  vCalendar/iCalendar attachment handling.

  @author Allen Winter \<winter@kde.org\>
*/
#include "attachmenthandler.h"
#include "calendarsupport/utils.h"

#include <ItemFetchJob>
#include <KFileDialog>
#include <KLocalizedString>
#include <KMessageBox>

#include <KRun>
#include <QTemporaryFile>
#include <KToolInvocation>
#include <KIO/NetAccess>
#include <KJob>
#include <QDebug>

#include <QFile>
#include <QPointer>
#include <QMimeDatabase>

using namespace KCalCore;
using namespace Akonadi;

namespace CalendarSupport
{

struct ReceivedInfo {
    QString uid;
    QString attachmentName;
};

class AttachmentHandler::Private
{
public:
    Private(QWidget *parent)
    {
        mParent = parent;
    }
    QMap<KJob *, ReceivedInfo> mJobToReceivedInfo;
    QPointer<QWidget> mParent;
};

AttachmentHandler::AttachmentHandler(QWidget *parent) : QObject(parent), d(new Private(parent))
{

}

AttachmentHandler::~AttachmentHandler()
{
    delete d;
}

Attachment::Ptr AttachmentHandler::find(const QString &attachmentName,
                                        const Incidence::Ptr &incidence)
{
    if (!incidence) {
        return Attachment::Ptr();
    }

    // get the attachment by name from the incidence
    const Attachment::List as = incidence->attachments();
    Attachment::Ptr a;
    if (!as.isEmpty()) {
        Attachment::List::ConstIterator it;
        Attachment::List::ConstIterator end(as.constEnd());

        for (it = as.constBegin(); it != end; ++it) {
            if ((*it)->label() == attachmentName) {
                a = *it;
                break;
            }
        }
    }

    if (!a) {
        KMessageBox::error(
            d->mParent,
            i18n("No attachment named \"%1\" found in the incidence.", attachmentName));
        return Attachment::Ptr();
    }

    if (a->isUri()) {
        if (!KIO::NetAccess::exists(a->uri(), KIO::NetAccess::SourceSide, d->mParent)) {
            KMessageBox::sorry(
                d->mParent,
                i18n("The attachment \"%1\" is a web link that is inaccessible from this computer. ",
                     QUrl::fromPercentEncoding(a->uri().toLatin1())));
            return Attachment::Ptr();
        }
    }
    return a;
}

Attachment::Ptr AttachmentHandler::find(const QString &attachmentName,
                                        const ScheduleMessage::Ptr &message)
{
    if (!message) {
        return Attachment::Ptr();
    }

    Incidence::Ptr incidence = message->event().dynamicCast<Incidence>();
    if (!incidence) {
        KMessageBox::error(
            d->mParent,
            i18n("The calendar invitation stored in this email message is broken in some way. "
                 "Unable to continue."));
        return Attachment::Ptr();
    }

    return find(attachmentName, incidence);
}

static QTemporaryFile *s_tempFile = 0;

static QUrl tempFileForAttachment(const Attachment::Ptr &attachment)
{
    QUrl url;

    QMimeDatabase db;
    QStringList patterns = db.mimeTypeForName(attachment->mimeType()).globPatterns();
    if (!patterns.empty()) {
        s_tempFile = new QTemporaryFile(QDir::tempPath() + QLatin1String("/attachementview_XXXXXX") + patterns.first().remove(QLatin1Char('*')));
    } else {
        s_tempFile = new QTemporaryFile();
    }
    s_tempFile->setAutoRemove(false);
    s_tempFile->open();
    s_tempFile->setPermissions(QFile::ReadUser);
    s_tempFile->write(QByteArray::fromBase64(attachment->data()));
    s_tempFile->close();
    QFile tf(s_tempFile->fileName());
    if (tf.size() != attachment->size()) {
        //whoops. failed to write the entire attachment. return an invalid URL.
        delete s_tempFile;
        s_tempFile = 0;
        return url;
    }

    url.setPath(s_tempFile->fileName());
    return url;
}

bool AttachmentHandler::view(const Attachment::Ptr &attachment)
{
    if (!attachment) {
        return false;
    }

    bool stat = true;
    if (attachment->isUri()) {
        KToolInvocation::invokeBrowser(attachment->uri());
    } else {
        // put the attachment in a temporary file and launch it
        QUrl tempUrl = tempFileForAttachment(attachment);
        if (tempUrl.isValid()) {
            stat = KRun::runUrl(tempUrl, attachment->mimeType(), 0, true);
        } else {
            stat = false;
            KMessageBox::error(
                d->mParent,
                i18n("Unable to create a temporary file for the attachment."));
        }
        delete s_tempFile;
        s_tempFile = 0;
    }
    return stat;
}

bool AttachmentHandler::view(const QString &attachmentName,
                             const Incidence::Ptr &incidence)
{
    return view(find(attachmentName, incidence));
}

void AttachmentHandler::view(const QString &attachmentName, const QString &uid)
{
    Item item;
    item.setGid(uid);
    ItemFetchJob *job = new ItemFetchJob(item);
    connect(job, &ItemFetchJob::result, this, &AttachmentHandler::slotFinishView);
    ReceivedInfo info;
    info.attachmentName = attachmentName;
    info.uid = uid;
    d->mJobToReceivedInfo[job] = info;
}

bool AttachmentHandler::view(const QString &attachmentName,
                             const ScheduleMessage::Ptr &message)
{
    return view(find(attachmentName, message));
}

bool AttachmentHandler::saveAs(const Attachment::Ptr &attachment)
{
    // get the saveas file name
    QString saveAsFile = KFileDialog::getSaveFileName(attachment->label(), QString(), d->mParent,
                         i18n("Save Attachment"));
    if (saveAsFile.isEmpty() ||
            (QFile(saveAsFile).exists() &&
             (KMessageBox::warningYesNo(
                  d->mParent,
                  i18n("%1 already exists. Do you want to overwrite it?",
                       saveAsFile)) == KMessageBox::No))) {
        return false;
    }

    bool stat = false;
    if (attachment->isUri()) {
        // save the attachment url
        stat = KIO::NetAccess::file_copy(attachment->uri(), QUrl::fromLocalFile(saveAsFile));
    } else {
        // put the attachment in a temporary file and save it
        QUrl tempUrl = tempFileForAttachment(attachment);
        if (tempUrl.isValid()) {
            stat = KIO::NetAccess::file_copy(tempUrl, QUrl::fromLocalFile(saveAsFile));
            if (!stat && KIO::NetAccess::lastError()) {
                KMessageBox::error(d->mParent, KIO::NetAccess::lastErrorString());
            }
        } else {
            stat = false;
            KMessageBox::error(
                d->mParent,
                i18n("Unable to create a temporary file for the attachment."));
        }
        delete s_tempFile;
        s_tempFile = 0;
    }
    return stat;
}

bool AttachmentHandler::saveAs(const QString &attachmentName,
                               const Incidence::Ptr &incidence)
{
    return saveAs(find(attachmentName, incidence));
}

void AttachmentHandler::saveAs(const QString &attachmentName, const QString &uid)
{
    Item item;
    item.setGid(uid);
    ItemFetchJob *job = new ItemFetchJob(item);
    connect(job, &ItemFetchJob::result, this, &AttachmentHandler::slotFinishView);

    ReceivedInfo info;
    info.attachmentName = attachmentName;
    info.uid = uid;
    d->mJobToReceivedInfo[job] = info;
}

bool AttachmentHandler::saveAs(const QString &attachmentName,
                               const ScheduleMessage::Ptr &message)
{
    return saveAs(find(attachmentName, message));
}

void AttachmentHandler::slotFinishSaveAs(KJob *job)
{
    ReceivedInfo info = d->mJobToReceivedInfo[job];
    bool success = false;

    if (job->error() != 0) {
        ItemFetchJob *fetchJob = qobject_cast<ItemFetchJob *>(job);
        const Item::List items = fetchJob->items();
        if (!items.isEmpty()) {
            Incidence::Ptr incidence = CalendarSupport::incidence(items.first());
            success = incidence && saveAs(info.attachmentName, incidence);
        } else {
            qWarning() << Q_FUNC_INFO << "No item found";
        }
    } else {
        qWarning() << Q_FUNC_INFO << "Job error:" << job->errorString();
    }

    emit saveAsFinished(info.uid, info.attachmentName, success);
    d->mJobToReceivedInfo.remove(job);
}

void AttachmentHandler::slotFinishView(KJob *job)
{
    ReceivedInfo info = d->mJobToReceivedInfo[job];
    bool success = false;

    if (job->error() != 0) {
        ItemFetchJob *fetchJob = qobject_cast<ItemFetchJob *>(job);
        const Item::List items = fetchJob->items();
        if (!items.isEmpty()) {
            Incidence::Ptr incidence = CalendarSupport::incidence(items.first());
            success = incidence && view(info.attachmentName, incidence);
        } else {
            qWarning() << Q_FUNC_INFO << "No item found";
        }
    } else {
        qWarning() << Q_FUNC_INFO << "Job error:" << job->errorString();
    }

    emit viewFinished(info.uid, info.attachmentName, success);
    d->mJobToReceivedInfo.remove(job);
}

} // namespace CalendarSupport

