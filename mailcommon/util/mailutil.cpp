/*******************************************************************************
**
** Filename   : util
** Created on : 03 April, 2005
** Copyright  : (c) 2005 Till Adam
** Email      : <adam@kde.org>
**
*******************************************************************************/

/*******************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
**   It is distributed in the hope that it will be useful, but
**   WITHOUT ANY WARRANTY; without even the implied warranty of
**   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
**   General Public License for more details.
**
**   You should have received a copy of the GNU General Public License
**   along with this program; if not, write to the Free Software
**   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
**
**   In addition, as a special exception, the copyright holders give
**   permission to link the code of this program with any edition of
**   the Qt library by Trolltech AS, Norway (or with modified versions
**   of Qt that use the same license as Qt), and distribute linked
**   combinations including the two.  You must obey the GNU General
**   Public License in all respects for all of the code used other than
**   Qt.  If you modify this file, you may extend this exception to
**   your version of the file, but you are not obligated to do so.  If
**   you do not wish to do so, delete this exception statement from
**   your version.
**
*******************************************************************************/

#include "mailutil.h"

#include "mailcommon_debug.h"
#include "calendarinterface.h"
#include "job/expirejob.h"
#include "foldercollection.h"
#include "pop3settings.h"
#include "kernel/mailkernel.h"
#include "filter/dialog/filteractionmissingargumentdialog.h"

#include "mailimporter/filterbalsa.h"
#include "mailimporter/filter_evolution.h"
#include "mailimporter/filter_evolution_v2.h"
#include "mailimporter/filter_evolution_v3.h"
#include "mailimporter/filter_clawsmail.h"
#include "mailimporter/filter_sylpheed.h"
#include "mailimporter/filter_thunderbird.h"
#include "mailimporter/filter_opera.h"
#include "mailimporter/filtericedove.h"
#include "mailimporter/othermailerutil.h"

#include <messagecore/utils/stringutil.h>
#include <messagecore/helpers/messagehelpers.h>

#include <messagecomposer/helper/messagehelper.h>

#include <AgentManager>
#include <entitymimetypefiltermodel.h>
#include <EntityTreeModel>
#include <ItemFetchJob>
#include <ItemFetchScope>
#include <Akonadi/KMime/MessageParts>
#include <AkonadiCore/NewMailNotifierAttribute>

#include <KMime/KMimeMessage>

#include <KColorScheme>
#include <KDBusServiceStarter>
#include <KJob>
#include <KIO/JobUiDelegate>
#include <collectionpage/expirecollectionattribute.h>

OrgKdeAkonadiPOP3SettingsInterface *MailCommon::Util::createPop3SettingsInterface(
    const QString &ident)
{
    return
        new OrgKdeAkonadiPOP3SettingsInterface(
            QLatin1String("org.freedesktop.Akonadi.Resource.") + ident, QLatin1String("/Settings"), QDBusConnection::sessionBus());
}

bool MailCommon::Util::isVirtualCollection(const Akonadi::Collection &collection)
{
    return MailCommon::Util::isVirtualCollection(collection.resource());
}

bool MailCommon::Util::isVirtualCollection(const QString &resource)
{
    return resource == QLatin1String("akonadi_search_resource");
}

bool MailCommon::Util::isLocalCollection(const QString &resource)
{
    return resource.contains(QLatin1String("akonadi_mbox_resource")) ||
           resource.contains(QLatin1String("akonadi_maildir_resource")) ||
           resource.contains(QLatin1String("akonadi_mixedmaildir_resource"));
}

QString MailCommon::Util::fullCollectionPath(const Akonadi::Collection &collection)
{
    QString fullPath;

    QModelIndex idx =
        Akonadi::EntityTreeModel::modelIndexForCollection(KernelIf->collectionModel(), collection);
    if (!idx.isValid()) {
        return fullPath;
    }

    fullPath = idx.data().toString();
    idx = idx.parent();
    while (idx != QModelIndex()) {
        fullPath = idx.data().toString() + QLatin1Char('/') + fullPath;
        idx = idx.parent();
    }
    return fullPath;
}

bool MailCommon::Util::showJobErrorMessage(KJob *job)
{
    if (job->error()) {
        if (static_cast<KIO::Job *>(job)->ui()) {
            static_cast<KIO::Job *>(job)->ui()->showErrorMessage();
        } else {
            qCDebug(MAILCOMMON_LOG) << " job->errorString() :" << job->errorString();
        }
        return true;
    }
    return false;
}

Akonadi::AgentInstance::List MailCommon::Util::agentInstances(bool excludeMailDispacher)
{
    Akonadi::AgentInstance::List relevantInstances;
    foreach (const Akonadi::AgentInstance &instance, Akonadi::AgentManager::self()->instances()) {
        const QStringList capabilities(instance.type().capabilities());
        if (instance.type().mimeTypes().contains(KMime::Message::mimeType())) {
            if (capabilities.contains(QLatin1String("Resource")) &&
                    !capabilities.contains(QLatin1String("Virtual")) &&
                    !capabilities.contains(QLatin1String("MailTransport"))) {
                relevantInstances << instance;
            } else if (!excludeMailDispacher &&
                       instance.identifier() == QLatin1String("akonadi_maildispatcher_agent")) {
                relevantInstances << instance;
            }
        }
    }
    return relevantInstances;
}

/* static */
bool MailCommon::Util::ensureKorganizerRunning(bool switchTo)
{
    // FIXME: this function should be inside a QObject, and async,
    //         and emit a signal when korg registered itself successfuly

    QString error;
    bool result = true;
    QString dbusService;

#if defined(Q_OS_WIN32)
    //Can't run the korganizer-mobile.sh through KDBusServiceStarter in these platforms.
    QDBusInterface *interface = new QDBusInterface(QLatin1String("org.kde.korganizer"), QLatin1String("/MainApplication"));
    if (!interface->isValid()) {
        qCDebug(MAILCOMMON_LOG) << "Starting korganizer...";

        QDBusServiceWatcher *watcher =
            new QDBusServiceWatcher(QLatin1String("org.kde.korganizer"), QDBusConnection::sessionBus(),
                                    QDBusServiceWatcher::WatchForRegistration);
        QEventLoop loop;
        watcher->connect(watcher, &QDBusServiceWatcher::serviceRegistered, &loop, &QEventLoop::quit);
        result = QProcess::startDetached(QLatin1String("korganizer-mobile"));
        if (result) {
            qCDebug(MAILCOMMON_LOG) << "Starting loop";
            loop.exec();
            qCDebug(MAILCOMMON_LOG) << "Korganizer finished starting";
        } else {
            qCWarning(MAILCOMMON_LOG) << "Failed to start korganizer with QProcess";
        }

        delete watcher;
    }
    delete interface;
#else
    QString constraint;

#ifdef KDEPIM_MOBILE_UI
    // start the mobile korg instead of the desktop one
    constraint = QLatin1String("'mobile' in Keywords");
#endif

    result = KDBusServiceStarter::self()->findServiceFor(QLatin1String("DBUS/Organizer"),
             constraint,
             &error, &dbusService) == 0;
#endif
    if (result) {
        // OK, so korganizer (or kontact) is running. Now ensure the object we want is loaded.
        QDBusInterface iface(QLatin1String("org.kde.korganizer"), QLatin1String("/MainApplication"),
                             QLatin1String("org.kde.KUniqueApplication"));
        if (iface.isValid()) {
            if (switchTo) {
                iface.call(QLatin1String("newInstance"));   // activate korganizer window
            }
#if 0 //Not exist
            QDBusInterface pimIface("org.kde.korganizer", "/korganizer_PimApplication",
                                    "org.kde.KUniqueApplication");
            QDBusReply<bool> r = pimIface.call("load");
            if (!r.isValid() || !r.value()) {
                qCWarning(MAILCOMMON_LOG) << "Loading korganizer failed: " << pimIface.lastError().message();
            }
#endif
        } else {
            qCWarning(MAILCOMMON_LOG) << "Couldn't obtain korganizer D-Bus interface" << iface.lastError().message();
        }

        // We don't do anything with it, we just need it to be running so that it handles
        // the incoming directory.
    } else {
        qCWarning(MAILCOMMON_LOG) << "Couldn't start DBUS/Organizer:" << dbusService << error;
    }
    return result;
}

uint MailCommon::Util::folderIdentity(const Akonadi::Item &item)
{
    uint id = 0;
    if (item.isValid() && item.parentCollection().isValid()) {
        Akonadi::Collection col = item.parentCollection();
        if (col.resource().isEmpty()) {
            col = parentCollectionFromItem(item);
        }
        const QSharedPointer<FolderCollection> fd =
            FolderCollection::forCollection(col, false);

        id = fd->identity();
    }
    return id;
}

static QModelIndex indexBelow(QAbstractItemModel *model, const QModelIndex &current)
{
    // if we have children, return first child
    if (model->rowCount(current) > 0) {
        return model->index(0, 0, current);
    }

    // if we have siblings, return next sibling
    const QModelIndex parent = model->parent(current);
    const QModelIndex sibling = model->index(current.row() + 1, 0, parent);

    if (sibling.isValid()) {   // found valid sibling
        return sibling;
    }

    if (!parent.isValid()) {   // our parent is the tree root and we have no siblings
        return QModelIndex(); // we reached the bottom of the tree
    }

    // We are the last child, the next index to check is our uncle, parent's first sibling
    const QModelIndex parentsSibling = parent.sibling(parent.row() + 1, 0);
    if (parentsSibling.isValid()) {
        return parentsSibling;
    }

    // iterate over our parents back to root until we find a parent with a valid sibling
    QModelIndex currentParent = parent;
    QModelIndex grandParent = model->parent(currentParent);
    while (currentParent.isValid()) {
        // check if the parent has children except from us
        if (model->rowCount(grandParent) > currentParent.row() + 1) {
            const QModelIndex index =
                indexBelow(model, model->index(currentParent.row() + 1, 0, grandParent));
            if (index.isValid()) {
                return index;
            }
        }

        currentParent = grandParent;
        grandParent = model->parent(currentParent);
    }

    return QModelIndex(); // nothing found -> end of tree
}

static QModelIndex lastChildOfModel(QAbstractItemModel *model, const QModelIndex &current)
{
    if (model->rowCount(current) == 0) {
        return current;
    }

    return lastChildOfModel(model, model->index(model->rowCount(current) - 1, 0, current));
}

static QModelIndex indexAbove(QAbstractItemModel *model, const QModelIndex &current)
{
    const QModelIndex parent = model->parent(current);

    if (current.row() == 0) {
        // we have no previous siblings -> our parent is the next item above us
        return parent;
    }

    // find previous sibling
    const QModelIndex previousSibling = model->index(current.row() - 1, 0, parent);

    // the item above us is the last child (or grandchild, or grandgrandchild... etc)
    // of our previous sibling
    return lastChildOfModel(model, previousSibling);
}

QModelIndex MailCommon::Util::nextUnreadCollection(QAbstractItemModel *model,
        const QModelIndex &current,
        SearchDirection direction,
        bool (*ignoreCollectionCallback)(const Akonadi::Collection &collection))
{
    QModelIndex index = current;
    while (true) {
        if (direction == MailCommon::Util::ForwardSearch) {
            index = indexBelow(model, index);
        } else if (direction == MailCommon::Util::BackwardSearch) {
            index = indexAbove(model, index);
        }

        if (!index.isValid()) {   // reach end or top of the model
            return QModelIndex();
        }

        // check if the index is a collection
        const Akonadi::Collection collection =
            index.data(Akonadi::EntityTreeModel::CollectionRole).value<Akonadi::Collection>();

        if (collection.isValid()) {

            // check if it is unread
            if (collection.statistics().unreadCount() > 0) {
                if (ignoreCollectionCallback && ignoreCollectionCallback(collection)) {
                    continue;
                }
                if (!ignoreNewMailInFolder(collection)) {
                    return index; // we found the next unread collection
                }
            }
        }
    }

    return QModelIndex(); // no unread collection found
}

bool MailCommon::Util::ignoreNewMailInFolder(const Akonadi::Collection &collection)
{
    if (collection.hasAttribute<Akonadi::NewMailNotifierAttribute>()) {
        if (collection.attribute<Akonadi::NewMailNotifierAttribute>()->ignoreNewMail()) {
            return true;
        }
    }
    return false;
}

Akonadi::Collection MailCommon::Util::parentCollectionFromItem(const Akonadi::Item &item)
{
    return updatedCollection(item.parentCollection());
}

QString MailCommon::Util::realFolderPath(const QString &path)
{
    QString realPath(path);
    realPath.remove(QLatin1String(".directory"));
    realPath.replace(QLatin1String("/."), QLatin1String("/"));
    if (!realPath.isEmpty() && (realPath.at(0) == QLatin1Char('.'))) {
        realPath.remove(0, 1);   //remove first "."
    }
    return realPath;
}

QColor MailCommon::Util::defaultQuotaColor()
{
    KColorScheme scheme(QPalette::Active, KColorScheme::View);
    return scheme.foreground(KColorScheme::NegativeText).color();
}

void MailCommon::Util::expireOldMessages(const Akonadi::Collection &collection, bool immediate)
{
    ScheduledExpireTask *task = new ScheduledExpireTask(collection, immediate);
    KernelIf->jobScheduler()->registerTask(task);
}

Akonadi::Collection MailCommon::Util::updatedCollection(const Akonadi::Collection &col)
{
    const QModelIndex idx = Akonadi::EntityTreeModel::modelIndexForCollection(KernelIf->collectionModel(), col);
    const Akonadi::Collection collection = idx.data(Akonadi::EntityTreeModel::CollectionRole).value<Akonadi::Collection>();
    return collection;
}

Akonadi::Collection::Id MailCommon::Util::convertFolderPathToCollectionId(const QString &folder)
{
    Akonadi::Collection::Id newFolderId = -1;
    bool exactPath = false;
    Akonadi::Collection::List lst = FilterActionMissingCollectionDialog::potentialCorrectFolders(folder, exactPath);
    if (lst.count() == 1 && exactPath) {
        newFolderId = lst.at(0).id();
    } else {
        QPointer<FilterActionMissingCollectionDialog> dlg = new FilterActionMissingCollectionDialog(lst, QString(), folder);
        if (dlg->exec()) {
            newFolderId = dlg->selectedCollection().id();
        }
        delete dlg;
    }
    return newFolderId;
}

QString MailCommon::Util::convertFolderPathToCollectionStr(const QString &folder)
{
    Akonadi::Collection::Id newFolderId = MailCommon::Util::convertFolderPathToCollectionId(folder);
    if (newFolderId == -1) {
        return QString();
    }
    return QString::number(newFolderId);
}

bool MailCommon::Util::foundMailer()
{
    QStringList lst;
    lst << MailImporter::FilterEvolution::defaultSettingsPath();
    lst << MailImporter::FilterEvolution_v2::defaultSettingsPath();
    lst << MailImporter::FilterEvolution_v3::defaultSettingsPath();
    lst << MailImporter::FilterBalsa::defaultSettingsPath();
    lst << MailImporter::FilterClawsMail::defaultSettingsPath();
    lst << MailImporter::FilterOpera::defaultSettingsPath();
    lst << MailImporter::FilterSylpheed::defaultSettingsPath();
    lst << MailImporter::FilterThunderbird::defaultSettingsPath();
    lst << MailImporter::OtherMailerUtil::trojitaDefaultPath();
    lst << MailImporter::FilterIcedove::defaultSettingsPath();

    Q_FOREACH (const QString &path, lst) {
        QDir directory(path);
        if (directory.exists()) {
            return true;
        }
    }
    return false;
}

MailCommon::ExpireCollectionAttribute *MailCommon::Util::expirationCollectionAttribute(const Akonadi::Collection &collection, bool &mustDeleteExpirationAttribute)
{
    MailCommon::ExpireCollectionAttribute *attr = 0;
    if (collection.hasAttribute<MailCommon::ExpireCollectionAttribute>()) {
        attr = collection.attribute<MailCommon::ExpireCollectionAttribute>();
        mustDeleteExpirationAttribute = false;
    } else {
        attr = new MailCommon::ExpireCollectionAttribute();
        KConfigGroup configGroup(KernelIf->config(),
                                 MailCommon::FolderCollection::configGroupName(collection));

        if (configGroup.hasKey("ExpireMessages")) {
            attr->setAutoExpire(configGroup.readEntry("ExpireMessages", false));
            attr->setReadExpireAge(configGroup.readEntry("ReadExpireAge", 3));
            attr->setReadExpireUnits((MailCommon::ExpireCollectionAttribute::ExpireUnits)configGroup.readEntry("ReadExpireUnits", (int)MailCommon::ExpireCollectionAttribute::ExpireMonths));
            attr->setUnreadExpireAge(configGroup.readEntry("UnreadExpireAge", 12));
            attr->setUnreadExpireUnits((MailCommon::ExpireCollectionAttribute::ExpireUnits)configGroup.readEntry("UnreadExpireUnits", (int)MailCommon::ExpireCollectionAttribute::ExpireNever));
            attr->setExpireAction(configGroup.readEntry("ExpireAction", "Delete") == QLatin1String("Move") ?
                                  MailCommon::ExpireCollectionAttribute::ExpireMove :
                                  MailCommon::ExpireCollectionAttribute::ExpireDelete);
            attr->setExpireToFolderId(configGroup.readEntry("ExpireToFolder", -1));
        }

        mustDeleteExpirationAttribute = true;
    }
    return attr;
}

