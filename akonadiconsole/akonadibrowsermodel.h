/*
    Copyright (c) 2008 Stephen Kelly <steveire@gmail.com>

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

#ifndef AKONADIBROWSERMODEL_H
#define AKONADIBROWSERMODEL_H

#include <AkonadiCore/entitytreemodel.h>
#include <AkonadiCore/changerecorder.h>

#include <QSortFilterProxyModel>

using namespace Akonadi;

class AkonadiBrowserModel : public EntityTreeModel
{
  Q_OBJECT
public:
  explicit AkonadiBrowserModel( ChangeRecorder* monitor, QObject* parent = 0 );
  ~AkonadiBrowserModel();

  enum ItemDisplayMode
  {
    GenericMode,
    MailMode,
    ContactsMode,
    CalendarMode
  };

  void setItemDisplayMode( ItemDisplayMode itemDisplayMode );
  ItemDisplayMode itemDisplayMode() const;

  virtual QVariant entityHeaderData( int section, Qt::Orientation orientation, int role, HeaderGroup headerGroup ) const;

  virtual QVariant entityData(const Item &item, int column, int role) const;
  virtual QVariant entityData(const Collection &collection, int column, int role) const;

  virtual int entityColumnCount( HeaderGroup headerGroup ) const;

  class State;

Q_SIGNALS:
  void columnsChanged();

private:
  State *m_currentState;
  State *m_genericState;
  State *m_mailState;
  State *m_contactsState;
  State *m_calendarState;

  ItemDisplayMode m_itemDisplayMode;
};

class AkonadiBrowserSortModel : public QSortFilterProxyModel
{
  Q_OBJECT
public:
  explicit AkonadiBrowserSortModel( AkonadiBrowserModel *browserModel, QObject *parent = 0 );
  ~AkonadiBrowserSortModel();

protected:
  bool lessThan( const QModelIndex &left, const QModelIndex &right ) const;

private:
  AkonadiBrowserModel *mBrowserModel;
};

#endif

