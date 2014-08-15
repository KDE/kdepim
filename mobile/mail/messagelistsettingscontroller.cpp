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

#include "messagelistsettingscontroller.h"

#include "ui_messagelistsettingseditor.h"

#include <kdialog.h>
#include <KLocalizedString>

#include <QAction>

MessageListSettingsController::MessageListSettingsController( QObject *parent )
  : QObject( parent )
{
  mEditAction = new QAction( this );
  mEditAction->setText( i18n( "Change Sorting/Grouping..." ) );
  mEditAction->setEnabled( false );

  connect( mEditAction, SIGNAL(triggered(bool)), SLOT(editSettings()) );
}

QString MessageListSettingsController::groupingRole() const
{
  switch ( mSettings.groupingOption() ) {
    case MessageListSettings::GroupByNone:
      return QLatin1String( "" );
      break;
    case MessageListSettings::GroupByDate:
      return QLatin1String( "dateGroup" );
      break;
    case MessageListSettings::GroupBySenderReceiver:
      return QLatin1String( "senderGroup" );
      break;
  }

  return QString();
}

QAction* MessageListSettingsController::editAction() const
{
  return mEditAction;
}

void MessageListSettingsController::setCollection( const Akonadi::Collection &collection )
{
  mEditAction->setEnabled( collection.isValid() );

  if ( !collection.isValid() )
    return;

  mCollectionId = collection.id();

  mSettings = MessageListSettings::fromConfig( mCollectionId );

  emit settingsChanged( mSettings );
}

void MessageListSettingsController::editSettings()
{
  Ui_MessageListSettingsEditor ui;

  KDialog dialog;
  ui.setupUi( dialog.mainWidget() );

  ui.mSortingOption->setCurrentIndex( static_cast<int>( mSettings.sortingOption() ) );
  ui.mSortingOrder->setCurrentIndex( mSettings.sortingOrder() == Qt::AscendingOrder ? 0 : 1 );
  ui.mGroupingOption->setCurrentIndex( static_cast<int>( mSettings.groupingOption() ) );
  ui.mUseThreading->setChecked( mSettings.useThreading() );
  ui.mUseGlobalSettings->setChecked( mSettings.useGlobalSettings() );

  if ( !dialog.exec() )
    return;

  mSettings.setSortingOption( static_cast<MessageListSettings::SortingOption>( ui.mSortingOption->currentIndex() ) );
  mSettings.setSortingOrder( ui.mSortingOrder->currentIndex() == 0 ? Qt::AscendingOrder : Qt::DescendingOrder );
  mSettings.setGroupingOption( static_cast<MessageListSettings::GroupingOption>( ui.mGroupingOption->currentIndex() ) );
  mSettings.setUseThreading( ui.mUseThreading->isChecked() );
  mSettings.setUseGlobalSettings( ui.mUseGlobalSettings->isChecked() );

  MessageListSettings::toConfig( mCollectionId, mSettings );

  emit settingsChanged( mSettings );
}
