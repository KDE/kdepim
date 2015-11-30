/*
  This file is part of KMail, the KDE mail client.
  Copyright (c) 2011-2015 Montel Laurent <montel@kde.org>

  KMail is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  KMail is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef MESSAGELISTUTIL_H
#define MESSAGELISTUTIL_H

#include <messagelist_export.h>
#include <QObject>
#include <QString>
#include <QColor>

namespace Akonadi
{
class Item;
}
class QMenu;

namespace MessageList
{
namespace Util
{
QString messageSortingConfigName();
QString messageSortDirectionConfigName();
QString groupSortingConfigName();
QString groupSortDirectionConfigName();
QString messageUniqueIdConfigName();
QString storageModelSortOrderGroup();
QString storageModelThemesGroup();
QString storageModelAggregationsGroup();
QString setForStorageModelConfigName();
QString storageModelSelectedMessageGroup();
MESSAGELIST_EXPORT void deleteConfig(const QString &collectionId);
MESSAGELIST_EXPORT QColor unreadDefaultMessageColor();
MESSAGELIST_EXPORT QColor importantDefaultMessageColor();
MESSAGELIST_EXPORT QColor todoDefaultMessageColor();
void fillViewMenu(QMenu *menu, QObject *receiver);

/// Returns the first few lines of the actual email text if available.
MESSAGELIST_EXPORT QString contentSummary(const Akonadi::Item &item);
}
}

#endif /* MESSAGELISTUTIL_H */

