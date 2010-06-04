/*
    Copyright (c) 2010 Bertjan Broeksema <b.broeksema@home.nl>

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
#ifndef KDECLARATIVEMAINVIEW_P_H
#define KDECLARATIVEMAINVIEW_P_H

#include <QtCore/QObject>
#include <QtGui/QItemSelectionModel>
#include <QtGui/QStringListModel>

#include <akonadi/changerecorder.h>
#include <akonadi/entitymimetypefiltermodel.h>

#include "akonadibreadcrumbnavigationfactory.h"

class KActionCollection;

static const char * const sFavoritePrefix = "Favorite_";
static const int sFavoritePrefixLength = 9;

class ListProxy;
class KDeclarativeMainViewPrivate : public QObject
{
  Q_OBJECT

public: /// members
  Akonadi::ChangeRecorder            *mChangeRecorder;                // Deleted by ~QObect
  QAbstractItemModel                 *mCollectionFilter;              // Deleted by ~QObect
  Akonadi::EntityTreeModel           *mEtm;
  ListProxy                          *mListProxy;
  Akonadi::EntityMimeTypeFilterModel *mItemFilter;
  QItemSelectionModel                *mFavSelection;
  QStringListModel                   *mFavsListModel;
  QAbstractItemModel                 *mFavSelectedChildItems;
  Akonadi::BreadcrumbNavigationFactory *mBnf;
  QItemSelectionModel                *mItemSelectionModel;
  QHash<QString, QStringList>        mPersistedSelections;

public: /// Methods
  KDeclarativeMainViewPrivate();

  QAbstractItemModel* getFavoritesListModel();
  QStringList getFavoritesList();

public slots:
  void saveState();
  void restoreState();
};

#endif // KDECLARATIVEMAINVIEW_P_H
