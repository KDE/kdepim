/*
 * Copyright (c) 2014 Sandro Knauß <knauss@kolabsys.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * As a special exception, permission is given to link this program
 * with any edition of Qt, and distribute the resulting executable,
 * without including the source code for Qt in the source distribution.
 */

#ifndef INCIDENCEEDITOR_INDUVIDUALMAILJOBFACTORY_H
#define INCIDENCEEDITOR_INDUVIDUALMAILJOBFACTORY_H

#include "incidenceeditors_ng_export.h"

#include "opencomposerjob.h"

#include <mailtransport/messagequeuejob.h>
#include <Akonadi/Calendar/IncidenceChanger>
#include <KIdentityManagement/Identity>

namespace IncidenceEditorNG
{

class IndividualMailDialog;

class IndividualMessageQueueJob : public MailTransport::MessageQueueJob
{
    Q_OBJECT
public:
    explicit IndividualMessageQueueJob(const KIdentityManagement::Identity &identity, const KCalCore::Attendee::List &update, const KCalCore::Attendee::List &edit, QObject *parent);

    void start() Q_DECL_OVERRIDE;
private Q_SLOTS:
    void startQueueJob(const QStringList &to, const QStringList &cc);
    void startComposerJob(const QStringList &to, const QStringList &cc);
    void handleJobFinished(KJob *job);

private:
    KCalCore::Attendee::List mUpdate;
    KCalCore::Attendee::List mEdit;
    KIdentityManagement::Identity mIdentity;
    MailTransport::MessageQueueJob *mQueueJob;
    OpenComposerJob *mComposerJob;
};

class IndividualMailITIPHandlerDialogDelegate : public Akonadi::ITIPHandlerDialogDelegate
{
    Q_OBJECT
public:

    explicit IndividualMailITIPHandlerDialogDelegate(const KCalCore::Incidence::Ptr &incidence, KCalCore::iTIPMethod method, QWidget *parent);

    void openDialogIncidenceCreated(Recipient recipient,
                                    const QString &question,
                                    Action action = ActionAsk,
                                    const KGuiItem &buttonYes = KGuiItem(i18nc("@action:button dialog positive answer", "Send Email")),
                                    const KGuiItem &buttonNo = KGuiItem(i18nc("@action:button dialog negative answer", "Do Not Send"))) Q_DECL_OVERRIDE;

    void openDialogIncidenceModified(bool attendeeStatusChanged,
                                     Recipient recipient,
                                     const QString &question,
                                     Action action = ActionAsk,
                                     const KGuiItem &buttonYes = KGuiItem(i18nc("@action:button dialog positive answer", "Send Email")),
                                     const KGuiItem &buttonNo = KGuiItem(i18nc("@action:button dialog negative answer", "Do Not Send"))) Q_DECL_OVERRIDE;

    void openDialogIncidenceDeleted(Recipient recipient,
                                    const QString &question,
                                    Action action = ActionAsk,
                                    const KGuiItem &buttonYes = KGuiItem(i18nc("@action:button dialog positive answer", "Send Email")),
                                    const KGuiItem &buttonNo = KGuiItem(i18nc("@action:button dialog negative answer", "Do Not Send"))) Q_DECL_OVERRIDE;

Q_SIGNALS:
    void setEdit(const KCalCore::Incidence::Ptr &incidence, const KCalCore::Attendee::List &edit);
    void setUpdate(const KCalCore::Incidence::Ptr &incidence, const KCalCore::Attendee::List &update);

protected:
    void openDialog(const QString &question, const KCalCore::Attendee::List &attendees,
                    Action action,
                    const KGuiItem &buttonYes, const KGuiItem &buttonNo);

private Q_SLOTS:
    void onDialogClosed(int result);
private:
    IndividualMailDialog *mDialog;
};

class INCIDENCEEDITORS_NG_EXPORT IndividualMailComponentFactory : public Akonadi::ITIPHandlerComponentFactory
{
    Q_OBJECT
public:
    explicit IndividualMailComponentFactory(QObject *parent = Q_NULLPTR);
    MailTransport::MessageQueueJob *createMessageQueueJob(const KCalCore::IncidenceBase::Ptr &incidence,
            const KIdentityManagement::Identity &identity, QObject *parent) Q_DECL_OVERRIDE;

    Akonadi::ITIPHandlerDialogDelegate *createITIPHanderDialogDelegate(const KCalCore::Incidence::Ptr &incidence,
            KCalCore::iTIPMethod method, QWidget *parent) Q_DECL_OVERRIDE;

public Q_SLOTS:
    void onSetEdit(const KCalCore::Incidence::Ptr &incidence, const KCalCore::Attendee::List &edit);
    void onSetUpdate(const KCalCore::Incidence::Ptr &incidence, const KCalCore::Attendee::List &update);

private:
    QHash<QString, KCalCore::Attendee::List> mEdit;
    QHash<QString, KCalCore::Attendee::List> mUpdate;
};

}
#endif
