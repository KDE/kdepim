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

#ifndef INCIDENCEEDITOR_INDUVIDUALMAILJOBFACTORY_H
#define INCIDENCEEDITOR_INDUVIDUALMAILJOBFACTORY_H

#include "incidenceeditors_ng_export.h"

#include "opencomposerjob.h"

#include <MailTransport/MessageQueueJob>
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

    virtual void start();
private slots:
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

    virtual void openDialogIncidenceCreated(Recipient recipient,
                                            const QString &question,
                                            Action action = ActionAsk,
                                            const KGuiItem &buttonYes = KGuiItem(i18nc("@action:button dialog positive answer", "Send Email")),
                                            const KGuiItem &buttonNo = KGuiItem(i18nc("@action:button dialog negative answer", "Do Not Send")));

    virtual void openDialogIncidenceModified(bool attendeeStatusChanged,
            Recipient recipient,
            const QString &question,
            Action action = ActionAsk,
            const KGuiItem &buttonYes = KGuiItem(i18nc("@action:button dialog positive answer", "Send Email")),
            const KGuiItem &buttonNo = KGuiItem(i18nc("@action:button dialog negative answer", "Do Not Send")));

    virtual void openDialogIncidenceDeleted(Recipient recipient,
                                            const QString &question,
                                            Action action = ActionAsk,
                                            const KGuiItem &buttonYes = KGuiItem(i18nc("@action:button dialog positive answer", "Send Email")),
                                            const KGuiItem &buttonNo = KGuiItem(i18nc("@action:button dialog negative answer", "Do Not Send")));

signals:
    void setEdit(const KCalCore::Incidence::Ptr &incidence, const KCalCore::Attendee::List &edit);
    void setUpdate(const KCalCore::Incidence::Ptr &incidence, const KCalCore::Attendee::List &update);

protected:
    void openDialog(const QString &question, const KCalCore::Attendee::List &attendees,
                    Action action,
                    const KGuiItem &buttonYes, const KGuiItem &buttonNo);

private slots:
    void onDialogClosed(int result);
private:
    IndividualMailDialog *mDialog;
};

class INCIDENCEEDITORS_NG_EXPORT IndividualMailComponentFactory : public Akonadi::ITIPHandlerComponentFactory
{
    Q_OBJECT
public:
    explicit IndividualMailComponentFactory(QObject *parent = 0);
    virtual MailTransport::MessageQueueJob *createMessageQueueJob(const KCalCore::IncidenceBase::Ptr &incidence,
            const KIdentityManagement::Identity &identity, QObject *parent);

    virtual Akonadi::ITIPHandlerDialogDelegate *createITIPHanderDialogDelegate(const KCalCore::Incidence::Ptr &incidence,
            KCalCore::iTIPMethod method, QWidget *parent);

public slots:
    void onSetEdit(const KCalCore::Incidence::Ptr &incidence, const KCalCore::Attendee::List &edit);
    void onSetUpdate(const KCalCore::Incidence::Ptr &incidence, const KCalCore::Attendee::List &update);

private:
    QHash<QString, KCalCore::Attendee::List> mEdit;
    QHash<QString, KCalCore::Attendee::List> mUpdate;
};

}
#endif
