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

#include "followupremindercreatejob.h"
#include "messagecomposer_debug.h"
#include "FollowupReminder/FollowUpReminderUtil"
#include <KCalCore/Todo>
#include <KLocalizedString>
#include <AkonadiCore/ItemCreateJob>

using namespace MessageComposer;
class MessageComposer::FollowupReminderCreateJobPrivate
{
public:
    FollowupReminderCreateJobPrivate()
        : mInfo(new FollowUpReminder::FollowUpReminderInfo)
    {

    }
    ~FollowupReminderCreateJobPrivate()
    {
        delete mInfo;
    }

    Akonadi::Collection mCollection;
    FollowUpReminder::FollowUpReminderInfo *mInfo;
};

FollowupReminderCreateJob::FollowupReminderCreateJob(QObject *parent)
    : KJob(parent),
      d(new MessageComposer::FollowupReminderCreateJobPrivate)
{

}

FollowupReminderCreateJob::~FollowupReminderCreateJob()
{
    delete d;
}

void FollowupReminderCreateJob::setFollowUpReminderDate(const QDate &date)
{
    d->mInfo->setFollowUpReminderDate(date);
}

void FollowupReminderCreateJob::setOriginalMessageItemId(Akonadi::Item::Id value)
{
    d->mInfo->setOriginalMessageItemId(value);
}

void FollowupReminderCreateJob::setMessageId(const QString &messageId)
{
    d->mInfo->setMessageId(messageId);
}

void FollowupReminderCreateJob::setTo(const QString &to)
{
    d->mInfo->setTo(to);
}

void FollowupReminderCreateJob::setSubject(const QString &subject)
{
    d->mInfo->setSubject(subject);
}

void FollowupReminderCreateJob::setCollectionToDo(const Akonadi::Collection &collection)
{
    d->mCollection = collection;
}

void FollowupReminderCreateJob::start()
{
    if (d->mInfo->isValid()) {
        if (d->mCollection.isValid()) {
            KCalCore::Todo::Ptr todo(new KCalCore::Todo);
            todo->setSummary(i18n("Wait answer from \"%1\" send to \"%2\"").arg(d->mInfo->subject()).arg(d->mInfo->to()));
            Akonadi::Item newTodoItem;
            newTodoItem.setMimeType(KCalCore::Todo::todoMimeType());
            newTodoItem.setPayload<KCalCore::Todo::Ptr>(todo);

            Akonadi::ItemCreateJob *createJob = new Akonadi::ItemCreateJob(newTodoItem, d->mCollection);
            connect(createJob, &Akonadi::ItemCreateJob::result, this, &FollowupReminderCreateJob::slotCreateNewTodo);
        } else {
            writeFollowupReminderInfo();
        }
    } else {
        qCDebug(MESSAGECOMPOSER_LOG) << "FollowupReminderCreateJob info not valid ";
        Q_EMIT emitResult();
        return;
    }
}

void FollowupReminderCreateJob::slotCreateNewTodo(KJob *job)
{
    if (job->error()) {
        qCDebug(MESSAGECOMPOSER_LOG) << "Error during create new Todo " << job->errorString();
    } else {
        Akonadi::ItemCreateJob *createJob = qobject_cast<Akonadi::ItemCreateJob *>(job);
        d->mInfo->setTodoId(createJob->item().id());
    }
    writeFollowupReminderInfo();
}

void FollowupReminderCreateJob::writeFollowupReminderInfo()
{
    FollowUpReminder::FollowUpReminderUtil::writeFollowupReminderInfo(FollowUpReminder::FollowUpReminderUtil::defaultConfig(), d->mInfo, true);
    Q_EMIT emitResult();
}
