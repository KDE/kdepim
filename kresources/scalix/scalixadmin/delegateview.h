/*
 *   This file is part of ScalixAdmin.
 *
 *   Copyright (C) 2007 Trolltech ASA. All rights reserved.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef DELEGATEVIEW_H
#define DELEGATEVIEW_H

#include <QtCore/QAbstractTableModel>
#include <QtGui/QTableView>

namespace Scalix {
class Delegate;
}

class DelegateManager;

class DelegateModel : public QAbstractTableModel
{
  Q_OBJECT

  public:
    DelegateModel( DelegateManager *manager, QObject *parent = 0 );

    virtual int rowCount( const QModelIndex &parent ) const;
    virtual int columnCount( const QModelIndex &parent ) const;
    virtual QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;
    virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

  private Q_SLOTS:
    void delegateChanged();

  private:
    DelegateManager *mManager;
};

class DelegateView : public QTableView
{
  Q_OBJECT

  public:
    DelegateView( DelegateManager *manager, QWidget *parent = 0 );

    Scalix::Delegate selectedDelegate() const;

  Q_SIGNALS:
    void selectionChanged();

  private:
    DelegateManager *mManager;
    DelegateModel *mModel;
};

#endif
