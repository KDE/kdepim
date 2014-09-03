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
#ifndef MAILCOMMON_MAILUTIL_H
#define MAILCOMMON_MAILUTIL_H

#include "mailcommon_export.h"

#include <AgentInstance>
#include <Collection>

class OrgKdeAkonadiPOP3SettingsInterface;

namespace Akonadi
{
class Item;
}

class KJob;

class QAbstractItemModel;
class QModelIndex;
class QString;

namespace MailCommon
{

/**
 * The Util namespace contains a collection of helper functions use in
 * various places.
 */
namespace Util
{

MAILCOMMON_EXPORT OrgKdeAkonadiPOP3SettingsInterface *createPop3SettingsInterface(
    const QString &ident);

MAILCOMMON_EXPORT bool isVirtualCollection(const Akonadi::Collection &col);

MAILCOMMON_EXPORT bool isVirtualCollection(const QString &resource);

MAILCOMMON_EXPORT QString fullCollectionPath(const Akonadi::Collection &collection);

MAILCOMMON_EXPORT bool showJobErrorMessage(KJob *job);

MAILCOMMON_EXPORT Akonadi::AgentInstance::List agentInstances(bool excludeMailTransport = true);

MAILCOMMON_EXPORT bool ensureKorganizerRunning(bool switchTo);

/**
   * Returns the identity of the folder that contains the given Akonadi::Item.
   */
MAILCOMMON_EXPORT uint folderIdentity(const Akonadi::Item &item);

/**
   * Describes the direction for searching next unread collection.
   */
enum SearchDirection {
    ForwardSearch,
    BackwardSearch
};

/**
   * Returns the index of the next unread collection following a given index.
   *
   * @param model The item model to search in.
   * @param current The index of the collection where the search will start.
   * @param direction The direction of search.
   * @param ignoreCollectionCallback A callback method to ignore certain
   *        collections by returning @c true.
   */
MAILCOMMON_EXPORT QModelIndex nextUnreadCollection(QAbstractItemModel *model,
        const QModelIndex &current,
        SearchDirection direction,
        bool (*ignoreCollectionCallback)(const Akonadi::Collection &collection) = 0);

MAILCOMMON_EXPORT Akonadi::Collection parentCollectionFromItem(const Akonadi::Item &item);

MAILCOMMON_EXPORT QString realFolderPath(const QString &path);

MAILCOMMON_EXPORT QColor defaultQuotaColor();

MAILCOMMON_EXPORT void expireOldMessages(const Akonadi::Collection &collection,
        bool immediate);

MAILCOMMON_EXPORT Akonadi::Collection updatedCollection(const Akonadi::Collection &col);

MAILCOMMON_EXPORT Akonadi::Collection::Id convertFolderPathToCollectionId(const QString &folder);
MAILCOMMON_EXPORT QString convertFolderPathToCollectionStr(const QString &folder);

MAILCOMMON_EXPORT bool foundMailer();
MAILCOMMON_EXPORT bool isLocalCollection(const QString &resource);

MAILCOMMON_EXPORT bool ignoreNewMailInFolder(const Akonadi::Collection &collection);
}

}

#endif
