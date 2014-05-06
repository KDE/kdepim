/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Tobias Koenig <tokoe@kdab.com>

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

#include "messagelistsettings.h"

#include <kconfiggroup.h>
#include <kglobal.h>
#include <ksharedconfig.h>
#include <KSharedConfig>

MessageListSettings::MessageListSettings()
  : mSortingOption( SortByDateTimeMostRecent ),
    mSortDescending( false ),
    mGroupingOption( GroupByDate ),
    mUseThreading( true ),
    mUseGlobalSettings( true )
{
}

MessageListSettings::~MessageListSettings()
{
}

void MessageListSettings::setSortingOption( SortingOption option )
{
  mSortingOption = option;
}

MessageListSettings::SortingOption MessageListSettings::sortingOption() const
{
  return mSortingOption;
}

void MessageListSettings::setSortingOrder( Qt::SortOrder order )
{
  mSortDescending = (order == Qt::DescendingOrder);
}

Qt::SortOrder MessageListSettings::sortingOrder() const
{
  return (mSortDescending ? Qt::DescendingOrder : Qt::AscendingOrder);
}

void MessageListSettings::setGroupingOption( GroupingOption option )
{
  mGroupingOption = option;
}

MessageListSettings::GroupingOption MessageListSettings::groupingOption() const
{
  return mGroupingOption;
}

void MessageListSettings::setUseThreading( bool threading )
{
  mUseThreading = threading;
}

bool MessageListSettings::useThreading() const
{
  return mUseThreading;
}

void MessageListSettings::setUseGlobalSettings( bool value )
{
  mUseGlobalSettings = value;
}

bool MessageListSettings::useGlobalSettings() const
{
  return mUseGlobalSettings;
}

MessageListSettings MessageListSettings::fromConfig( qint64 collectionId )
{
  const QString groupName = QString::fromLatin1( "MessageListSettings-%1" ).arg( collectionId );

  MessageListSettings settings;

  if ( KSharedConfig::openConfig()->hasGroup( groupName ) ) { // use collection specific settings
    const KConfigGroup group( KSharedConfig::openConfig(), groupName );

    settings.mSortingOption = static_cast<SortingOption>( group.readEntry<int>( "SortingOption", SortByDateTimeMostRecent ) );
    settings.mSortDescending = group.readEntry<bool>( "SortDescending", false );
    settings.mGroupingOption = static_cast<GroupingOption>( group.readEntry<int>( "GroupingOption", GroupByDate ) );
    settings.mUseThreading = group.readEntry<bool>( "UseThreading", true );
    settings.mUseGlobalSettings = false;
  } else { // use default settings
    const KConfigGroup group( KSharedConfig::openConfig(), QLatin1String( "MessageListSettings-default" ) );

    settings.mSortingOption = static_cast<SortingOption>( group.readEntry<int>( "SortingOption", SortByDateTimeMostRecent ) );
    settings.mSortDescending = group.readEntry<bool>( "SortDescending", false );
    settings.mGroupingOption = static_cast<GroupingOption>( group.readEntry<int>( "GroupingOption", GroupByDate ) );
    settings.mUseThreading = group.readEntry<bool>( "UseThreading", true );
    settings.mUseGlobalSettings = true;
  }

  return settings;
}

void MessageListSettings::toConfig( qint64 collectionId, const MessageListSettings &settings )
{
  const QString groupName = QString::fromLatin1( "MessageListSettings-%1" ).arg( collectionId );

  if ( settings.useGlobalSettings() ) {
    KSharedConfig::openConfig()->deleteGroup( groupName );
  } else {
    KConfigGroup group( KSharedConfig::openConfig(), groupName );

    group.writeEntry( "SortingOption", static_cast<int>( settings.mSortingOption ) );
    group.writeEntry( "SortDescending", settings.mSortDescending );
    group.writeEntry( "GroupingOption", static_cast<int>( settings.mGroupingOption ) );
    group.writeEntry( "UseThreading", settings.mUseThreading );
  }

  KSharedConfig::openConfig()->sync();
}

MessageListSettings MessageListSettings::fromDefaultConfig()
{
  const KConfigGroup group( KSharedConfig::openConfig(), QLatin1String( "MessageListSettings-default" ) );

  MessageListSettings settings;
  settings.mSortingOption = static_cast<SortingOption>( group.readEntry<int>( "SortingOption", SortByDateTimeMostRecent ) );
  settings.mSortDescending = group.readEntry<bool>( "SortDescending", false );
  settings.mGroupingOption = static_cast<GroupingOption>( group.readEntry<int>( "GroupingOption", GroupByDate ) );
  settings.mUseThreading = group.readEntry<bool>( "UseThreading", true );
  settings.mUseGlobalSettings = true;

  return settings;
}

void MessageListSettings::toDefaultConfig( const MessageListSettings &settings )
{
  const QLatin1String groupName( "MessageListSettings-default" );

  KConfigGroup group( KSharedConfig::openConfig(), groupName );

  group.writeEntry( "SortingOption", static_cast<int>( settings.mSortingOption ) );
  group.writeEntry( "SortDescending", settings.mSortDescending );
  group.writeEntry( "GroupingOption", static_cast<int>( settings.mGroupingOption ) );
  group.writeEntry( "UseThreading", settings.mUseThreading );

  KSharedConfig::openConfig()->sync();
}
