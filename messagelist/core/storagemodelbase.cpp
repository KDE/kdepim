/******************************************************************************
 *
 *  Copyright 2008 Szymon Tomasz Stefanek <pragma@kvirc.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *******************************************************************************/

#include "core/storagemodelbase.h"

#include "core/settings.h"

using namespace MessageList::Core;

StorageModel::StorageModel( QObject * parent )
  : QAbstractItemModel( parent )
{
}

StorageModel::~StorageModel()
{
}

int StorageModel::initialUnreadRowCountGuess() const
{
  return rowCount( QModelIndex() );
}

unsigned long StorageModel::preSelectedMessage() const
{
  const QString storageModelId = id();
  Q_ASSERT( !storageModelId.isEmpty() );

  KConfigGroup conf( Settings::self()->config(),
                     MessageList::Util::storageModelSelectedMessageGroup() );

  // QVariant supports unsigned int OR unsigned long long int, NOT unsigned long int... doh...
  qulonglong defValue = 0;

  return conf.readEntry( MessageList::Util::messageUniqueIdConfigName().arg( storageModelId ), defValue );
}

void StorageModel::savePreSelectedMessage( unsigned long uniqueIdOfMessage )
{
  const QString storageModelId = id();
  Q_ASSERT( !storageModelId.isEmpty() );

  KConfigGroup conf( Settings::self()->config(),
                     MessageList::Util::storageModelSelectedMessageGroup() );

  if ( uniqueIdOfMessage )
  {
    // QVariant supports unsigned int OR unsigned long long int, NOT unsigned long int... doh...
    qulonglong val = uniqueIdOfMessage;

    conf.writeEntry( MessageList::Util::messageUniqueIdConfigName().arg( storageModelId ), val );
  } else
    conf.deleteEntry( MessageList::Util::messageUniqueIdConfigName().arg( storageModelId ) );
}


