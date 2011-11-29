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
#include "messagelistutil.h"
#include "core/settings.h"
#include <KConfigGroup>

using namespace MessageList::Core;

QString MessageList::Util::messageSortingConfigName()
{
  return QLatin1String( "MessageSorting" );
}

QString MessageList::Util::messageSortDirectionConfigName()
{
  return QLatin1String( "MessageSortDirection" );
}

QString MessageList::Util::groupSortingConfigName()
{
  return QLatin1String( "GroupSorting" );
}

QString MessageList::Util::groupSortDirectionConfigName()
{
  return QLatin1String( "GroupSortDirection" );
}

QString MessageList::Util::messageUniqueIdConfigName()
{
  return QString::fromLatin1( "MessageUniqueIdForStorageModel%1" );
}

QString MessageList::Util::storageModelSortOrderGroup()
{
  return QLatin1String( "MessageListView::StorageModelSortOrder" );
}

QString MessageList::Util::storageModelThemesGroup()
{
  return QLatin1String( "MessageListView::StorageModelThemes" );
}

QString MessageList::Util::storageModelAggregationsGroup()
{
  return QLatin1String( "MessageListView::StorageModelAggregations" );
}


QString MessageList::Util::setForStorageModelConfigName()
{
  return QString::fromLatin1( "SetForStorageModel%1" );
}

QString MessageList::Util::storageModelSelectedMessageGroup()
{
  return QLatin1String( "MessageListView::StorageModelSelectedMessages" );
}


void MessageList::Util::deleteConfig( const QString& collectionId )
{
  KConfigGroup confselectedMessage( Settings::self()->config(),
                                    MessageList::Util::storageModelSelectedMessageGroup() );
  confselectedMessage.deleteEntry( MessageList::Util::messageUniqueIdConfigName().arg( collectionId ) );

  KConfigGroup storageModelOrder( Settings::self()->config(),
                                  MessageList::Util::storageModelSortOrderGroup() );
  storageModelOrder.deleteEntry( collectionId + groupSortDirectionConfigName() );
  storageModelOrder.deleteEntry( collectionId + groupSortingConfigName() );
  storageModelOrder.deleteEntry( collectionId + messageSortDirectionConfigName() );
  storageModelOrder.deleteEntry( collectionId + messageSortingConfigName() );

  KConfigGroup storageModelTheme( Settings::self()->config(),
                                  MessageList::Util::storageModelThemesGroup() );
  storageModelTheme.deleteEntry( collectionId + setForStorageModelConfigName() );
  
}
