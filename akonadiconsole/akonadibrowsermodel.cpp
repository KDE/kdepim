/*
    Copyright (c) 2009 Stephen Kelly <steveire@gmail.com>

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


#include "akonadibrowsermodel.h"

#include <kmime/kmime_message.h>

#include <kcontacts/addressee.h>
#include <kcontacts/contactgroup.h>

#include <KCalCore/Incidence>
#include <KCalCore/Event>

#include <boost/shared_ptr.hpp>

typedef boost::shared_ptr<KMime::Message> MessagePtr;
typedef QSharedPointer<KCalCore::Incidence> IncidencePtr;

class AkonadiBrowserModel::State
{
public:
  virtual ~State() {}
  QStringList m_collectionHeaders;
  QStringList m_itemHeaders;

  virtual QVariant entityData( const Item &item, int column, int role ) const = 0;

};

class GenericState : public AkonadiBrowserModel::State
{
public:
  GenericState()
  {
    m_collectionHeaders << "Collection";
    m_itemHeaders << "Id" << "Remote Id" << "MimeType";
  }
  virtual ~GenericState() {
  }

  QVariant entityData( const Item &item, int column, int role ) const
  {
    if (Qt::DisplayRole != role)
      return QVariant();

    switch (column)
    {
    case 0:
      return item.id();
    case 1:
      return item.remoteId();
    case 2:
      return item.mimeType();
    }
    return QVariant();
  }


};

class MailState : public AkonadiBrowserModel::State
{
public:
  MailState()
  {
    m_collectionHeaders << "Collection";
    m_itemHeaders << "Subject" << "Sender" << "Date";
  }
  virtual ~MailState() {}

  QVariant entityData( const Item &item, int column, int role ) const
  {
    if (Qt::DisplayRole != role)
      return QVariant();

    if (!item.hasPayload<MessagePtr>())
    {
      return QVariant();
    }
    const MessagePtr mail = item.payload<MessagePtr>();

    switch (column)
    {
    case 0:
      return mail->subject()->asUnicodeString();
    case 1:
      return mail->from()->asUnicodeString();
    case 2:
      return mail->date()->asUnicodeString();
    }

    return QVariant();
  }

};

class ContactsState : public AkonadiBrowserModel::State
{
public:
  ContactsState()
  {
    m_collectionHeaders << "Collection";
    m_itemHeaders << "Given Name" << "Family Name" << "Email";
  }
  virtual ~ContactsState() {}

  QVariant entityData( const Item &item, int column, int role ) const
  {
    if (Qt::DisplayRole != role)
      return QVariant();

    if ( !item.hasPayload<KContacts::Addressee>() && !item.hasPayload<KContacts::ContactGroup>() )
    {
      return QVariant();
    }

    if ( item.hasPayload<KContacts::Addressee>() )
    {
      const KContacts::Addressee addr = item.payload<KContacts::Addressee>();

      switch (column)
      {
      case 0:
        return addr.givenName();
      case 1:
        return addr.familyName();
      case 2:
        return addr.preferredEmail();
      }
      return QVariant();
    }
    if ( item.hasPayload<KContacts::ContactGroup>() ) {

      switch (column)
      {
      case 0:
        const KContacts::ContactGroup group = item.payload<KContacts::ContactGroup>();
        return group.name();
      }
      return QVariant();
    }
    return QVariant();
  }
};

class CalendarState : public AkonadiBrowserModel::State
{
public:
  CalendarState()
  {
    m_collectionHeaders << "Collection";
    m_itemHeaders << "Summary" << "DateTime start" << "DateTime End" << "Type";
  }
  virtual ~CalendarState() {}

  QVariant entityData( const Item &item, int column, int role ) const
  {
    if (Qt::DisplayRole != role)
      return QVariant();

    if ( !item.hasPayload<IncidencePtr>() )
    {
      return QVariant();
    }
    const IncidencePtr incidence = item.payload<IncidencePtr>();
    switch (column)
    {
    case 0:
      return incidence->summary();
      break;
    case 1:
      return incidence->dtStart().toString();
      break;
    case 2:
      return incidence->dateTime( KCalCore::Incidence::RoleEnd ).toString();
      break;
    case 3:
      return incidence->typeStr();
      break;
    default:
      break;
    }
    return QVariant();
  }
};

AkonadiBrowserModel::AkonadiBrowserModel( ChangeRecorder* monitor, QObject* parent )
    : EntityTreeModel( monitor, parent ),
      m_itemDisplayMode( GenericMode )
{

  m_genericState = new GenericState();
  m_mailState = new MailState();
  m_contactsState = new ContactsState();
  m_calendarState = new CalendarState();

  m_currentState = m_genericState;
}

AkonadiBrowserModel::~AkonadiBrowserModel()
{
  delete m_genericState;
  delete m_mailState;
  delete m_contactsState;
  delete m_calendarState;
}

QVariant AkonadiBrowserModel::entityData( const Item &item, int column, int role ) const
{
  QVariant var = m_currentState->entityData( item, column, role );
  if ( !var.isValid() )
  {
    if ( column < 1 )
      return EntityTreeModel::entityData( item, column, role );
    return QString();
  }

  return var;
}

QVariant AkonadiBrowserModel::entityData(const Akonadi::Collection& collection, int column, int role) const
{
  return Akonadi::EntityTreeModel::entityData( collection, column, role );
}

int AkonadiBrowserModel::entityColumnCount( HeaderGroup headerGroup ) const
{
  if ( ItemListHeaders == headerGroup )
  {
    return m_currentState->m_itemHeaders.size();
  }

  if ( CollectionTreeHeaders == headerGroup )
  {
    return m_currentState->m_collectionHeaders.size();
  }
  // Practically, this should never happen.
  return EntityTreeModel::entityColumnCount( headerGroup );
}


QVariant AkonadiBrowserModel::entityHeaderData( int section, Qt::Orientation orientation, int role, HeaderGroup headerGroup ) const
{
  if ( section < 0 )
    return QVariant();

  if ( orientation == Qt::Vertical )
     return EntityTreeModel::entityHeaderData( section, orientation, role, headerGroup );

  if ( headerGroup == EntityTreeModel::CollectionTreeHeaders )
  {
    if ( role == Qt::DisplayRole )
    {
      if ( section >= m_currentState->m_collectionHeaders.size() )
        return QVariant();
      return m_currentState->m_collectionHeaders.at( section );
    }
  } else if ( headerGroup == EntityTreeModel::ItemListHeaders )
  {
    if ( role == Qt::DisplayRole )
    {
      if ( section >= m_currentState->m_itemHeaders.size() )
        return QVariant();
      return m_currentState->m_itemHeaders.at( section );
    }
  }
  return EntityTreeModel::entityHeaderData( section, orientation, role, headerGroup );
}

AkonadiBrowserModel::ItemDisplayMode AkonadiBrowserModel::itemDisplayMode() const
{
  return m_itemDisplayMode;
}

void AkonadiBrowserModel::setItemDisplayMode( AkonadiBrowserModel::ItemDisplayMode itemDisplayMode )
{
  const int oldColumnCount = columnCount();
  m_itemDisplayMode = itemDisplayMode;
  AkonadiBrowserModel::State* newState = 0;
  switch (itemDisplayMode)
  {
  case MailMode:
    newState = m_mailState;
    break;
  case ContactsMode:
    newState = m_contactsState;
    break;
  case CalendarMode:
    newState = m_calendarState;
    break;
  case GenericMode:
  default:
    newState = m_genericState;
    break;
  }
  const int newColumnCount = qMax(newState->m_collectionHeaders.count(), newState->m_itemHeaders.count());

  //kDebug() << "column count changed from" << oldColumnCount << "to" << newColumnCount;
  if ( newColumnCount > oldColumnCount ) {
    beginInsertColumns(QModelIndex(), oldColumnCount, newColumnCount - 1);
    m_currentState = newState;
    endInsertColumns();
  } else if ( newColumnCount < oldColumnCount ) {
    beginRemoveColumns(QModelIndex(), newColumnCount, oldColumnCount - 1);
    m_currentState = newState;
    endRemoveColumns();
  } else {
    m_currentState = newState;
  }
  headerDataChanged(Qt::Horizontal, 0, newColumnCount - 1);

  // The above is not enough to see the new headers, because EntityMimeTypeFilterModel gets column count and headers from our data,
  // and doesn't listen to dataChanged/headerDataChanged...
  columnsChanged();
}

AkonadiBrowserSortModel::AkonadiBrowserSortModel( AkonadiBrowserModel *model, QObject *parent )
  : QSortFilterProxyModel( parent )
  , mBrowserModel( model )
{
}

AkonadiBrowserSortModel::~AkonadiBrowserSortModel()
{
}

bool AkonadiBrowserSortModel::lessThan( const QModelIndex &left, const QModelIndex &right ) const
{
  if ( mBrowserModel->itemDisplayMode() == AkonadiBrowserModel::CalendarMode ) {
    if ( left.column() == 1 || left.column() == 2 ) {
      const Item leftItem = left.data( EntityTreeModel::ItemRole ).value<Item>();
      const Item rightItem = right.data( EntityTreeModel::ItemRole ).value<Item>();
      if ( !leftItem.hasPayload<IncidencePtr>() || !rightItem.hasPayload<IncidencePtr>() ) {
        return false;
      }
      const IncidencePtr leftInc = leftItem.payload<IncidencePtr>();
      const IncidencePtr rightInc = rightItem.payload<IncidencePtr>();

      if ( left.column() == 1 ) {
        return leftInc->dtStart() < rightInc->dtStart();
      } else if ( left.column() == 2 ) {
        return leftInc->dateTime( KCalCore::IncidenceBase::RoleEnd ) < rightInc->dateTime( KCalCore::IncidenceBase::RoleEnd );
      }
    }
  } else if ( mBrowserModel->itemDisplayMode() == AkonadiBrowserModel::MailMode ) {
    if ( left.column() == 2 ) {
      const Item leftItem = left.data( EntityTreeModel::ItemRole ).value<Item>();
      const Item rightItem = right.data( EntityTreeModel::ItemRole ).value<Item>();
      if ( !leftItem.hasPayload<MessagePtr>() || !rightItem.hasPayload<MessagePtr>() ) {
        return false;
      }
      const MessagePtr leftMail = leftItem.payload<MessagePtr>();
      const MessagePtr rightMail = rightItem.payload<MessagePtr>();

      return leftMail->date(false)->dateTime() < rightMail->date(false)->dateTime();
    }
  }

  return QSortFilterProxyModel::lessThan( left, right );
}
