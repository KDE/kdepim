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
#include <QTextDocument>

#include <KConfigGroup>
#include <KMenu>
#include <KIcon>
#include <KLocalizedString>
#include <nepomuk2/nmo.h>
#include <Nepomuk2/Resource>
#include <Nepomuk2/Variant>


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


  KConfigGroup storageModelAggregation( Settings::self()->config(),
                                        MessageList::Util::storageModelAggregationsGroup() );
  storageModelAggregation.deleteEntry( collectionId + setForStorageModelConfigName() );

}

QColor MessageList::Util::unreadDefaultMessageColor()
{
    return QColor( "blue" );
}

QColor MessageList::Util::importantDefaultMessageColor()
{
    return QColor( 0x98, 0x0, 0x0 );
}

QColor MessageList::Util::todoDefaultMessageColor()
{
    return QColor( 0x0, 0x98, 0x0 );
}

void MessageList::Util::fillViewMenu( KMenu * menu, QObject *receiver )
{
  KMenu* sortingMenu = new KMenu( i18n( "Sorting" ), menu );
  sortingMenu->setIcon( KIcon( QLatin1String( "view-sort-ascending" ) ) );
  menu->addMenu( sortingMenu );
  QObject::connect( sortingMenu, SIGNAL(aboutToShow()),
           receiver, SLOT(sortOrderMenuAboutToShow()) );

  KMenu* aggregationMenu = new KMenu( i18n( "Aggregation" ), menu );
  aggregationMenu->setIcon( KIcon( QLatin1String( "view-process-tree" ) ) );
  menu->addMenu( aggregationMenu );
  QObject::connect( aggregationMenu, SIGNAL(aboutToShow()),
           receiver, SLOT(aggregationMenuAboutToShow()) );

  KMenu* themeMenu = new KMenu( i18n( "Theme" ), menu );
  themeMenu->setIcon( KIcon( QLatin1String( "preferences-desktop-theme" ) ) );
  menu->addMenu( themeMenu );
  QObject::connect( themeMenu, SIGNAL(aboutToShow()),
           receiver, SLOT(themeMenuAboutToShow()) );
}

QString MessageList::Util::contentSummary( const KUrl& url )
{
  Nepomuk2::Resource mail( url );
  const QString content =
      mail.property( Nepomuk2::Vocabulary::NMO::plainTextMessageContent() ).toString();
  // Extract the first 5 non-empty, non-quoted lines from the content and return it
  int numLines = 0;
  const int maxLines = 5;
  const QStringList lines = content.split( QLatin1Char( '\n' ) );
  QString ret;
  foreach( const QString &line, lines ) {
    const QString lineTrimmed = line.trimmed();
    const bool isQuoted = lineTrimmed.startsWith( QLatin1Char( '>' ) ) || lineTrimmed.startsWith( QLatin1Char( '|' ) );
    if ( !isQuoted && !lineTrimmed.isEmpty() ) {
      ret += line + QLatin1Char( '\n' );
      numLines++;
      if ( numLines >= maxLines )
        break;
    }
  }
  return Qt::escape(ret);
}

