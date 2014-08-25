/*
  Copyright (c) 2014 Sandro Knauß <knauss@kolabsys.com>

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

#include "individualmailcomponentfactory.h"
#include "individualmaildialog.h"

#include <KPIMUtils/Email>

#include <KMessageBox>

#include <QDBusConnection>
#include <QDBusConnectionInterface>

using namespace IncidenceEditorNG;

// IndividualMessageQueueJob

IndividualMessageQueueJob::IndividualMessageQueueJob(const KPIMIdentities::Identity &identity,
                                                     const KCalCore::Attendee::List &update, const KCalCore::Attendee::List &edit,
                                                     QObject *parent)
    : MailTransport::MessageQueueJob(parent)
    , mUpdate(update)
    , mEdit(edit)
    , mIdentity(identity)
    , mQueueJob(0)
    , mComposerJob(0)
{
}

void IndividualMessageQueueJob::start()
{
    QSet<QString> attendeesTo(QSet<QString>::fromList(addressAttribute().to()));
    QSet<QString> attendeesCc(QSet<QString>::fromList(addressAttribute().cc()));

    QStringList attendeesAutoTo,  attendeesAutoCc;
    foreach (const KCalCore::Attendee::Ptr & attendee, mUpdate) {
        if (attendeesTo.contains(attendee->email())) {
            attendeesAutoTo.append(attendee->fullName());
        }
        if (attendeesCc.contains(attendee->email())) {
            attendeesAutoCc.append(attendee->fullName());
        }
    }
    if (!attendeesAutoTo.isEmpty() || !attendeesAutoCc.isEmpty() || !addressAttribute().bcc().isEmpty()) {
        startQueueJob(attendeesAutoTo, attendeesAutoCc);
    }

    QStringList attendeesComposerTo,  attendeesComposerCc;
    foreach (const KCalCore::Attendee::Ptr & attendee, mEdit) {
        if (attendeesTo.contains(attendee->email())) {
            attendeesComposerTo.append(attendee->fullName());
        }
        if (attendeesCc.contains(attendee->email())) {
            attendeesComposerCc.append(attendee->fullName());
        }
    }
    if (!attendeesComposerTo.isEmpty() || !attendeesComposerCc.isEmpty()) {
        startComposerJob(attendeesComposerTo, attendeesComposerCc);
    }

    // No subjob has been started
    if (!mQueueJob && !mComposerJob) {
        emitResult();
    }
}

void IndividualMessageQueueJob::startQueueJob(const QStringList &to, const QStringList &cc)
{
    KMime::Message::Ptr msg(message());
    msg->to()->fromUnicodeString(to.join(QLatin1String(", ")), "utf-8");
    msg->cc()->fromUnicodeString(cc.join(QLatin1String(", ")), "utf-8");
    msg->assemble();

    mQueueJob->setMessage(msg);
    mQueueJob->transportAttribute().setTransportId(transportAttribute().transportId());
    mQueueJob->sentBehaviourAttribute().setSentBehaviour(sentBehaviourAttribute().sentBehaviour());
    mQueueJob->addressAttribute().setFrom(addressAttribute().from());
    mQueueJob->addressAttribute().setTo(to);
    mQueueJob->addressAttribute().setCc(cc);
    mQueueJob->addressAttribute().setBcc(addressAttribute().bcc());

    mQueueJob = new MailTransport::MessageQueueJob(this);
    connect(mQueueJob, SIGNAL(finished(KJob*)), SLOT(handleJobFinished(KJob*)));
    mQueueJob->start();
}

void IndividualMessageQueueJob::startComposerJob(const QStringList &to, const QStringList &cc)
{
    mComposerJob = new OpenComposerJob(this, to.join(QLatin1String(", ")), cc.join(QLatin1String(", ")), QString(), message(), mIdentity);
    connect(mComposerJob, SIGNAL(finished(KJob*)), SLOT(handleJobFinished(KJob*)));
    mComposerJob->start();
}

void IndividualMessageQueueJob::handleJobFinished(KJob *job)
{
    if (job->error()) {
        if (job == mQueueJob && mComposerJob) {
            mComposerJob->kill();
            mComposerJob = 0;
        } else if (mComposerJob) {
            mQueueJob->kill();
            mQueueJob = 0;
        }
        setError(job->error());
        setErrorText(job->errorString());
        emitResult();
        return;
    }
    if (job == mQueueJob) {
        if (!mComposerJob) {
            emitResult();
        }
        mQueueJob = 0;
    } else {
        if (!mQueueJob) {
            emitResult();
        }
        mComposerJob = 0;
    }

}

// IndividualMailAskDelegator

IndividualMailITIPHandlerDialogDelegate::IndividualMailITIPHandlerDialogDelegate(const KCalCore::Incidence::Ptr &incidence,
                                                                                 KCalCore::iTIPMethod method, QWidget *parent)
    : Akonadi::ITIPHandlerDialogDelegate(incidence, method, parent)
{
}

void IndividualMailITIPHandlerDialogDelegate::openDialog(const QString &question, const KCalCore::Attendee::List &attendees,
        Action action,
        const KGuiItem &buttonYes, const KGuiItem &buttonNo)
{
    switch (action) {
    case ActionSendMessage:
        emit setUpdate(mIncidence, attendees);
        emit dialogClosed(KMessageBox::Yes, mMethod, mIncidence);
        break;
    case ActionDontSendMessage:
        emit dialogClosed(KMessageBox::No, mMethod, mIncidence);
        break;
    default:
        mDialog = new IndividualMailDialog(question, attendees, buttonYes, buttonNo, mParent);
        connect(mDialog, SIGNAL(finished(int)), SLOT(onDialogClosed(int)));
        mDialog->show();
        break;
    }
}

void IndividualMailITIPHandlerDialogDelegate::openDialogIncidenceCreated(Recipient recipient,
        const QString &question,
        Action action,
        const KGuiItem &buttonYes, const KGuiItem &buttonNo)
{
    if (recipient == Attendees) {
        openDialog(question, mIncidence->attendees(), action, buttonYes, buttonNo);
    } else {
        KCalCore::Attendee::Ptr organizer(new KCalCore::Attendee(mIncidence->organizer()->name(), mIncidence->organizer()->email()));
        openDialog(question, KCalCore::Attendee::List() << organizer, action, buttonYes, buttonNo);
    }
}

void IndividualMailITIPHandlerDialogDelegate::openDialogIncidenceModified(bool attendeeStatusChanged,
                                                                          Recipient recipient,
                                                                          const QString &question,
                                                                          Action action,
                                                                          const KGuiItem &buttonYes, const KGuiItem &buttonNo)
{
    Q_UNUSED(attendeeStatusChanged);
    if (recipient == Attendees) {
        openDialog(question, mIncidence->attendees(), action, buttonYes, buttonNo);
    } else {
        KCalCore::Attendee::Ptr organizer(new KCalCore::Attendee(mIncidence->organizer()->name(), mIncidence->organizer()->email()));
        openDialog(question, KCalCore::Attendee::List() << organizer, action, buttonYes, buttonNo);
    }
}

void IndividualMailITIPHandlerDialogDelegate::openDialogIncidenceDeleted(Recipient recipient,
        const QString &question,
        Action action,
        const KGuiItem &buttonYes, const KGuiItem &buttonNo)
{
    if (recipient == Attendees) {
        openDialog(question, mIncidence->attendees(), action, buttonYes, buttonNo);
    } else {
        KCalCore::Attendee::Ptr organizer(new KCalCore::Attendee(mIncidence->organizer()->name(), mIncidence->organizer()->email()));
        openDialog(question, KCalCore::Attendee::List() << organizer, action, buttonYes, buttonNo);
    }
}

void IndividualMailITIPHandlerDialogDelegate::onDialogClosed(int result)
{
    if (result == KDialog::Yes) {
        emit setEdit(mIncidence, mDialog->editAttendees());
        emit setUpdate(mIncidence, mDialog->updateAttendees());
        emit dialogClosed(KMessageBox::Yes, mMethod, mIncidence);
    } else {
        emit dialogClosed(KMessageBox::No, mMethod, mIncidence);
    }
}

// IndividualMailJobFactory
IndividualMailComponentFactory::IndividualMailComponentFactory(QObject *parent)
    : Akonadi::ITIPHandlerComponentFactory(parent)
{

}

MailTransport::MessageQueueJob *IndividualMailComponentFactory::createMessageQueueJob(const KCalCore::IncidenceBase::Ptr &incidence,
        const KPIMIdentities::Identity &identity, QObject *parent)
{
    return new IndividualMessageQueueJob(identity, mUpdate.take(incidence->uid()), mEdit.take(incidence->uid()), parent);
}

Akonadi::ITIPHandlerDialogDelegate *IndividualMailComponentFactory::createITIPHanderDialogDelegate(const KCalCore::Incidence::Ptr &incidence,
                                                                                             KCalCore::iTIPMethod method, QWidget *parent)
{
    IndividualMailITIPHandlerDialogDelegate *askDelegator =  new IndividualMailITIPHandlerDialogDelegate(incidence, method, parent);
    connect(askDelegator, SIGNAL(setEdit(KCalCore::Incidence::Ptr,KCalCore::Attendee::List)),
            SLOT(onSetEdit(KCalCore::Incidence::Ptr,KCalCore::Attendee::List)));
    connect(askDelegator, SIGNAL(setUpdate(KCalCore::Incidence::Ptr,KCalCore::Attendee::List)),
            SLOT(onSetUpdate(KCalCore::Incidence::Ptr,KCalCore::Attendee::List)));

    return askDelegator;
}

void IndividualMailComponentFactory::onSetEdit(const KCalCore::Incidence::Ptr &incidence, const KCalCore::Attendee::List &edit)
{
    mEdit[incidence->uid()] = edit;
}

void IndividualMailComponentFactory::onSetUpdate(const KCalCore::Incidence::Ptr &incidence, const KCalCore::Attendee::List &update)
{
    mUpdate[incidence->uid()] = update;
}
