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

#include "individualmailjobfactory.h"
#include "individualmaildialog.h"

#include <KMessageBox>

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusReply>


namespace IncidenceEditorNG {

// IndividualMessageQueueJob

IndividualMessageQueueJob::IndividualMessageQueueJob(QObject *parent, const KPIMIdentities::Identity &identity, const QStringList &update, const QStringList &edit)
   : MailTransport::MessageQueueJob(parent)
   , mUpdate(update)
   , mEdit(edit)
   ,mIdentity(identity)
{
}

void IndividualMessageQueueJob::start()
{
    QStringList attendeesSendTo,  attendeesSendCc;
    QStringList attendeesTo(addressAttribute().to()), attendeesCc(addressAttribute().cc());
    kDebug() << attendeesTo << attendeesCc;
    foreach(const QString &attendee, mUpdate) {
       if (attendeesTo.indexOf(attendee) > -1) {
           attendeesSendTo.append(attendee);
       }
       if (attendeesCc.indexOf(attendee) > -1) {
           attendeesSendCc.append(attendee);
       }
    }

    if (!attendeesSendTo.isEmpty() || !attendeesSendCc.isEmpty() || !addressAttribute().bcc().isEmpty()) {
        mJob = new MailTransport::MessageQueueJob(this);
        mJob->setMessage(message());
        mJob->transportAttribute().setTransportId(transportAttribute().transportId());
        mJob->sentBehaviourAttribute().setSentBehaviour(sentBehaviourAttribute().sentBehaviour());
        mJob->addressAttribute().setFrom(addressAttribute().from());
        mJob->addressAttribute().setTo(attendeesSendTo);
        mJob->addressAttribute().setCc(attendeesSendCc);
        mJob->addressAttribute().setBcc(addressAttribute().bcc());
        connect(mJob, SIGNAL(finished(KJob*)), SLOT(handleQueueJobFinished(KJob*)));
        mJob->start();
    }

    if ( QDBusConnection::sessionBus().interface()->isServiceRegistered( QLatin1String("org.kde.kmail")) ) {
        QDBusInterface kmailObj( QLatin1String("org.kde.kmail"), QLatin1String("/KMail"), QLatin1String("org.kde.kmail.kmail") );

        QStringList attendeesSendTo,  attendeesSendCc;
        foreach(const QString &attendee, mEdit) {
           if (attendeesTo.indexOf(attendee) > -1) {
               attendeesSendTo.append(attendee);
           }
           if (attendeesCc.indexOf(attendee) > -1) {
               attendeesSendCc.append(attendee);
           }
        }

        if (!attendeesSendTo.isEmpty() || !attendeesSendCc.isEmpty()) {
            QList<QVariant> messages;
            //TODO: incidence anhang
            messages << attendeesSendTo.join(QLatin1String(", ")) << attendeesSendCc.join(QLatin1String(", ")) << QString() << message()->subject()->asUnicodeString() << message()->body() << false ;//<< QString() << attachURLs << customHeaders;
            QDBusReply<int> composerDbusPath = kmailObj.callWithArgumentList(QDBus::AutoDetect, QLatin1String("openComposer"), messages);
            if ( !composerDbusPath.isValid() ) {
                setErrorText( i18n( "Cannot connect to email service." ) );
                setError(true);
                if (mJob) {
                    mJob->kill();
                    mJob = 0;
                }
                emit emitResult();
                return;
            }
        }
    }
}
void IndividualMessageQueueJob::handleQueueJobFinished(KJob *job)
{
    setError(job->error());
    setErrorText(job->errorString());
    mJob = 0;

    emit emitResult();
}

// IndividualMailAskDelegator

IndividualMailAskDelegator::IndividualMailAskDelegator(QWidget *parent, const KCalCore::Incidence::Ptr &incidence, KCalCore::iTIPMethod method)
    : Akonadi::AskDelegator(parent, incidence, method)
{
}

void IndividualMailAskDelegator::openDialog(const QString &question, const QStringList &attendees,
                                            bool ignoreDefaultAction,
                                            const KGuiItem &buttonYes, const KGuiItem &buttonNo)
{
    if (ignoreDefaultAction || mDefaultAction == ActionAsk) {
        mDialog = new IndividualMailDialog(mParent, question, attendees, buttonYes, buttonNo);
        kDebug() << "hihi";
        connect(mDialog, SIGNAL(finished(int)), SLOT(onDialogClosed(int)));
        mDialog->show();
    } else {
        switch (mDefaultAction) {
        case ActionSendMessage:
            emit dialogClosed(KMessageBox::Yes, mMethod, mIncidence);
            break;
        case ActionDontSendMessage:
            emit dialogClosed(KMessageBox::No, mMethod, mIncidence);
            break;
        default:
            Q_ASSERT(false);
            emit dialogClosed(0, mMethod, mIncidence);
            break;
        }
    }
}

void IndividualMailAskDelegator::openDialogIncidenceCreated(bool attendees,
                                                            const QString &question,
                                                            bool ignoreDefaultAction,
                                                            const KGuiItem &buttonYes, const KGuiItem &buttonNo)
{
    if (attendees) {
        QStringList attendees;
        foreach(const KCalCore::Attendee::Ptr &attendee, mIncidence->attendees()) {
            attendees << attendee->fullName();
        }
                    openDialog(question, attendees, ignoreDefaultAction, buttonYes, buttonNo);
    } else {
        openDialog(question, QStringList() << mIncidence->organizer()->fullName(), ignoreDefaultAction, buttonYes, buttonNo);
    }
}

void IndividualMailAskDelegator::openDialogIncidenceModified(bool /*attendeeStatusChanged*/,
                                                             bool attendees,
                                                             const QString &question,
                                                             bool ignoreDefaultAction,
                                                             const KGuiItem &buttonYes, const KGuiItem &buttonNo)
{
    if (attendees) {
        QStringList attendees;
        foreach(const KCalCore::Attendee::Ptr &attendee, mIncidence->attendees()) {
            attendees << attendee->fullName();
        }
                    openDialog(question, attendees, ignoreDefaultAction, buttonYes, buttonNo);
    } else {
        openDialog(question, QStringList() << mIncidence->organizer()->fullName(), ignoreDefaultAction, buttonYes, buttonNo);
    }
}

void IndividualMailAskDelegator::openDialogIncidenceDeleted(bool attendees,
                                                            const QString &question,
                                                            bool ignoreDefaultAction,
                                                            const KGuiItem &buttonYes, const KGuiItem &buttonNo)
{
    if (attendees) {
        QStringList attendees;
        foreach(const KCalCore::Attendee::Ptr &attendee, mIncidence->attendees()) {
            attendees << attendee->fullName();
        }
                    openDialog(question, attendees, ignoreDefaultAction, buttonYes, buttonNo);
    } else {
        openDialog(question, QStringList() << mIncidence->organizer()->fullName(), ignoreDefaultAction, buttonYes, buttonNo);
    }
}

void IndividualMailAskDelegator::openDialogSchedulerFinished(const QString &question,
                                                             bool ignoreDefaultAction,
                                                             const KGuiItem &buttonYes, const KGuiItem &buttonNo)
{
    openDialog(question, QStringList() << mIncidence->organizer()->fullName(), ignoreDefaultAction, buttonYes, buttonNo);
}

void IndividualMailAskDelegator::onDialogClosed(int result)
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

MailTransport::MessageQueueJob *IndividualMailJobFactory::createMessageQueueJob(QObject *parent,
                                                                                const KCalCore::IncidenceBase::Ptr &incidence,
                                                                                const KPIMIdentities::Identity &identity)
{
    kDebug() << incidence->uid();
    return new IndividualMessageQueueJob(parent, identity, mUpdate[incidence->uid()], mEdit[incidence->uid()]);
    /*TODO: delete mEdit/mUpdate entry - finish event?*/
}

Akonadi::AskDelegator *IndividualMailJobFactory::createAskDelegator(QWidget *parent,
                                                                    const KCalCore::Incidence::Ptr &incidence,
                                                                    KCalCore::iTIPMethod method)
{
    IndividualMailAskDelegator *askDelegator =  new IndividualMailAskDelegator(parent, incidence, method);
    connect(askDelegator,SIGNAL(setEdit(const KCalCore::Incidence::Ptr &, const QStringList &)),
            SLOT(onSetEdit(const KCalCore::Incidence::Ptr, const QStringList&)));
    connect(askDelegator,SIGNAL(setUpdate(const KCalCore::Incidence::Ptr &, const QStringList &)),
            SLOT(onSetUpdate(const KCalCore::Incidence::Ptr, const QStringList&)));
    connect(askDelegator,SIGNAL(dialogClosed(int,KCalCore::iTIPMethod,KCalCore::Incidence::Ptr)),
            SLOT(onDialogClosed(int,KCalCore::iTIPMethod,KCalCore::Incidence::Ptr)));
    kDebug() << askDelegator;

    return askDelegator;
}

void IndividualMailJobFactory::onSetEdit(const KCalCore::Incidence::Ptr &incidence, const QStringList &edit)
{
    kDebug() << incidence->uid() << edit;
    mEdit[incidence->uid()] = edit;
}

void IndividualMailJobFactory::onSetUpdate(const KCalCore::Incidence::Ptr &incidence, const QStringList &update)
{
    kDebug() << incidence->uid() << update;
    mUpdate[incidence->uid()] = update;
}

void IndividualMailJobFactory::onDialogClosed(int result,KCalCore::iTIPMethod,KCalCore::Incidence::Ptr)
{
    kDebug() << result;
}

} // namespace