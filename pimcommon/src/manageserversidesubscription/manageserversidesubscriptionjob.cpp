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

#include "manageserversidesubscriptionjob.h"
#include "util/pimutil.h"
#include "pimcommon_debug.h"
#include <KLocalizedString>
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QDBusPendingReply>
#include <kdbusconnectionpool.h>
#include <KMessageBox>

using namespace PimCommon;

class PimCommon::ManageServerSideSubscriptionJobPrivate
{
public:
    ManageServerSideSubscriptionJobPrivate()
        : mParentWidget(Q_NULLPTR)
    {

    }

    Akonadi::Collection mCurrentCollection;
    QWidget *mParentWidget;
};

ManageServerSideSubscriptionJob::ManageServerSideSubscriptionJob(QObject *parent)
    : QObject(parent),
      d(new PimCommon::ManageServerSideSubscriptionJobPrivate)
{

}

ManageServerSideSubscriptionJob::~ManageServerSideSubscriptionJob()
{
    delete d;
}

void ManageServerSideSubscriptionJob::start()
{
    if (!d->mCurrentCollection.isValid()) {
        qCDebug(PIMCOMMON_LOG) << " collection not defined";
        deleteLater();
        return;
    }
    bool isImapOnline = false;
    if (PimCommon::Util::isImapFolder(d->mCurrentCollection, isImapOnline)) {
        QDBusInterface iface(
            QLatin1String("org.freedesktop.Akonadi.Resource.") + d->mCurrentCollection.resource(),
            QStringLiteral("/"), QStringLiteral("org.kde.Akonadi.ImapResourceBase"),
            KDBusConnectionPool::threadConnection(), this);
        if (!iface.isValid()) {
            qCDebug(PIMCOMMON_LOG) << "Cannot create imap dbus interface";
            deleteLater();
            return;
        }
        QDBusPendingCall call = iface.asyncCall(QStringLiteral("configureSubscription"), (qlonglong)d->mParentWidget->winId());
        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this, &ManageServerSideSubscriptionJob::slotConfigureSubscriptionFinished);
    }
}

void ManageServerSideSubscriptionJob::slotConfigureSubscriptionFinished(QDBusPendingCallWatcher *watcher)
{
    QDBusPendingReply<int> reply = *watcher;
    if (reply.isValid()) {
        if (reply == -2) {
            KMessageBox::error(d->mParentWidget, i18n("IMAP server not configured yet. Please configure the server in the IMAP account before setting up server-side subscription."));
        } else if (reply == -1) {
            KMessageBox::error(d->mParentWidget, i18n("Log in failed, please configure the IMAP account before setting up server-side subscription."));
        }
    } else {
        qCDebug(PIMCOMMON_LOG) << "ManageServerSideSubscriptionJob return an invalid reply";
    }
    watcher->deleteLater();
    watcher = Q_NULLPTR;
    deleteLater();
}

void ManageServerSideSubscriptionJob::setParentWidget(QWidget *parentWidget)
{
    d->mParentWidget = parentWidget;
}

void ManageServerSideSubscriptionJob::setCurrentCollection(const Akonadi::Collection &col)
{
    d->mCurrentCollection = col;
}

