/*
* This file is part of Akonadi
*
* Copyright (c) 2010 Volker Krause <vkrause@kde.org> 
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
* 02110-1301  USA
*/
#ifndef MAINVIEW_H
#define MAINVIEW_H

#include "kdeclarativemainview.h"

#include <QItemSelectionModel>
#include <QStringListModel>

#include <akonadi/collection.h>
#include <akonadi/entitytreemodel.h>

class MainView : public KDeclarativeMainView
{
  Q_OBJECT
public:
    explicit MainView( QWidget *parent = 0 );

public slots:
    void saveFavorite( const QString &name );
    void loadFavorite( const QString &name );

private:
  QAbstractItemModel* getFavoritesListModel();
  QStringList getFavoritesList();

private:
  Akonadi::EntityTreeModel *m_etm;
  QItemSelectionModel *m_favSelection;
  QStringListModel *m_favsListModel;
};

#endif // MAINVIEW_H
