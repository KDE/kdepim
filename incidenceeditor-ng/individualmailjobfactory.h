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

#include <mailtransport/messagequeuejob.h>
#include <Akonadi/Calendar/IncidenceChanger>
#include <KPIMIdentities/Identity>

namespace IncidenceEditorNG {

class IndividualMailDialog;

class IndividualMessageQueueJob : public MailTransport::MessageQueueJob
{
    Q_OBJECT
public:
    explicit IndividualMessageQueueJob(QObject *parent, const KPIMIdentities::Identity &identity, const QStringList &update, const QStringList &edit);
    virtual void start();
private:
    void handleQueueJobFinished(KJob* job);

    QStringList mUpdate;
    QStringList mEdit;
    MailTransport::MessageQueueJob *mJob;
    KPIMIdentities::Identity mIdentity;
};

class IndividualMailAskDelegator : public Akonadi::AskDelegator
{
    Q_OBJECT
public:
    enum Action {
        ActionAsk,
        ActionSendMessage,
        ActionDontSendMessage
    };

    explicit IndividualMailAskDelegator(QWidget *parent, const KCalCore::Incidence::Ptr &incidence, KCalCore::iTIPMethod method);

    void openDialog(const QString &question, const QStringList &attendees,
                    bool ignoreDefaultAction,
                    const KGuiItem &buttonYes, const KGuiItem &buttonNo);

    virtual void openDialogIncidenceCreated(bool attendees,
                                    const QString &question,
                                    bool ignoreDefaultAction = true,
                                    const KGuiItem &buttonYes = KGuiItem(i18n("Send Email")),
                                    const KGuiItem &buttonNo = KGuiItem(i18n("Do Not Send")));

    virtual void openDialogIncidenceModified(bool attendeeStatusChanged,
                                     bool attendees,
                                     const QString &question,
                                     bool ignoreDefaultAction = true,
                                     const KGuiItem &buttonYes = KGuiItem(i18n("Send Email")),
                                     const KGuiItem &buttonNo = KGuiItem(i18n("Do Not Send")));

    virtual void openDialogIncidenceDeleted(bool attendees,
                                    const QString &question,
                                    bool ignoreDefaultAction = true,
                                    const KGuiItem &buttonYes = KGuiItem(i18n("Send Email")),
                                    const KGuiItem &buttonNo = KGuiItem(i18n("Do Not Send")));

    virtual void openDialogSchedulerFinished(const QString &question,
                                     bool ignoreDefaultAction = true,
                                     const KGuiItem &buttonYes = KGuiItem(i18n("Send Email")),
                                     const KGuiItem &buttonNo = KGuiItem(i18n("Do Not Send")));

signals:
    void setEdit(const KCalCore::Incidence::Ptr &incidence, const QStringList &edit);
    void setUpdate(const KCalCore::Incidence::Ptr &incidence, const QStringList &update);

private slots:
    void onDialogClosed(int result);

private:
    IndividualMailDialog *mDialog;
};

class IndividualMailJobFactory : public Akonadi::MessageQueueJobFactory
{
    Q_OBJECT
public:
    virtual MailTransport::MessageQueueJob *createMessageQueueJob(QObject *parent,
                                                                  const KCalCore::IncidenceBase::Ptr &incidence,
                                                                  const KPIMIdentities::Identity &identity);

    virtual Akonadi::AskDelegator *createAskDelegator(QWidget *parent,
                                                      const KCalCore::Incidence::Ptr &incidence,
                                                      KCalCore::iTIPMethod method);

public slots:
    void onSetEdit(const KCalCore::Incidence::Ptr &incidence, const QStringList &edit);
    void onSetUpdate(const KCalCore::Incidence::Ptr &incidence, const QStringList &update);
    void onDialogClosed(int result,KCalCore::iTIPMethod,KCalCore::Incidence::Ptr);

private:
    QHash<QString, QStringList> mEdit;
    QHash<QString, QStringList> mUpdate;
};


}
#endif
