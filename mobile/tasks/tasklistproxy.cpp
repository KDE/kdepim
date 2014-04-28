/*
    Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>

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

#include "tasklistproxy.h"

#include "settings.h"

#include <calendarsupport/kcalprefs.h>

#include <AkonadiCore/entitytreemodel.h>
#include <KCalCore/Todo>

using namespace Akonadi;

TaskListProxy::TaskListProxy( QObject* parent )
  : ListProxy( parent )
{
}

QVariant TaskListProxy::data( const QModelIndex &index, int role ) const
{
  const Akonadi::Item item = QSortFilterProxyModel::data( index, Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();

  if ( item.isValid() && item.hasPayload<KCalCore::Todo::Ptr>() ) {
    const KCalCore::Todo::Ptr incidence = item.payload<KCalCore::Todo::Ptr>();
    switch ( role ) {
      case SummaryRole:
        return incidence->summary();
      case DescriptionRole:
        return incidence->description();
      case PercentCompleteRole:
        return incidence->percentComplete();
      case BackgroundColorRole:
        if ( incidence->hasDueDate() ) {
          if ( incidence->dtDue().date() == QDate::currentDate() ) {
            return mViewPrefs->todoDueTodayColor();
          } else if ( incidence->isOverdue() ) {
            return mViewPrefs->todoOverdueColor();
          }
        }
        return QColor(Qt::transparent);
      case IsSubTaskRole:
        return !incidence->relatedTo( KCalCore::Todo::RelTypeParent ).isEmpty();
      case SingleLineDescriptionRole:
        return incidence->description().replace( QLatin1Char('\n'), QLatin1Char(' ') );
      case HasDescriptionRole:
        return !incidence->description().isEmpty();
    }
  }

  return QSortFilterProxyModel::data( index, role );
}

bool TaskListProxy::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( role == PercentCompleteRole ) {
    Akonadi::Item item = QSortFilterProxyModel::data( index, Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();

    if ( item.isValid() && item.hasPayload<KCalCore::Todo::Ptr>() ) {
      KCalCore::Todo::Ptr incidence = item.payload<KCalCore::Todo::Ptr>();
      if ( incidence->percentComplete() != value.toInt() ) {
        incidence->setPercentComplete( value.toInt() );
        item.setPayload( incidence );
        return QSortFilterProxyModel::setData( index, QVariant::fromValue( item ), EntityTreeModel::ItemRole );
      } else {
        return true;
      }
    }
  }

  return QSortFilterProxyModel::setData( index, value, role );
}


void TaskListProxy::setSourceModel( QAbstractItemModel *sourceModel )
{
  ListProxy::setSourceModel( sourceModel );

  QHash<int, QByteArray> names = roleNames();
  names.insert( EntityTreeModel::ItemIdRole, "itemId" );
  names.insert( SummaryRole, "summary" );
  names.insert( DescriptionRole, "description" );
  names.insert( PercentCompleteRole, "percentComplete" );
  names.insert( BackgroundColorRole, "backgroundColor" );
  names.insert( IsSubTaskRole, "isSubTask" );
  names.insert( SingleLineDescriptionRole, "singleLineDescription" );
  names.insert( HasDescriptionRole, "hasDescription" );

  setRoleNames( names );
}

void TaskListProxy::setPreferences( const EventViews::PrefsPtr &preferences )
{
  mViewPrefs = preferences;
}
