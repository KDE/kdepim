/* -*- mode: C++; c-file-style: "gnu" -*-
  This file is part of KMail, the KDE mail client.
  Copyright (c) 2011 Montel Laurent <montel@kde.org>

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

#include <messagelist/messagelist_export.h>
#include <QString>
#include <QColor>

namespace Akonadi {
  class Item;
}
class KMenu;

namespace MessageList
{
namespace Util {
  MESSAGELIST_EXPORT QString messageSortingConfigName();
  MESSAGELIST_EXPORT QString messageSortDirectionConfigName();
  MESSAGELIST_EXPORT QString groupSortingConfigName();
  MESSAGELIST_EXPORT QString groupSortDirectionConfigName();
  MESSAGELIST_EXPORT QString messageUniqueIdConfigName();
  MESSAGELIST_EXPORT QString storageModelSortOrderGroup();
  MESSAGELIST_EXPORT QString storageModelThemesGroup();
  MESSAGELIST_EXPORT QString storageModelAggregationsGroup();
  MESSAGELIST_EXPORT QString setForStorageModelConfigName();
  MESSAGELIST_EXPORT QString storageModelSelectedMessageGroup();
  MESSAGELIST_EXPORT void deleteConfig( const QString& collectionId );
  MESSAGELIST_EXPORT QColor unreadDefaultMessageColor();
  MESSAGELIST_EXPORT QColor importantDefaultMessageColor();
  MESSAGELIST_EXPORT QColor todoDefaultMessageColor();
  MESSAGELIST_EXPORT void fillViewMenu( KMenu * menu, QObject *receiver );

  /// Returns the first few lines of the actual email text if available.
  MESSAGELIST_EXPORT QString contentSummary( const Akonadi::Item &item );
}
}

#endif /* MESSAGELISTUTIL_H */

