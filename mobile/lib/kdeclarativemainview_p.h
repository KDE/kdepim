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
#ifndef KDECLARATIVEMAINVIEW_P_H
#define KDECLARATIVEMAINVIEW_P_H

#include "kdeclarativemainview.h"
#include "akonadibreadcrumbnavigationfactory.h"

#include <akonadi/agentfilterproxymodel.h>
#include <akonadi/changerecorder.h>
#include <akonadi/entitymimetypefiltermodel.h>
#include <akonadi/etmviewstatesaver.h>
#include <akonadi_next/kviewstatemaintainer.h>

#include <QtCore/QObject>
#include <QtGui/QItemSelectionModel>
#include <QtGui/QStringListModel>

class AgentStatusMonitor;
class KActionCollection;
class KLineEdit;

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
  QAbstractItemModel                 *mItemModel;
  QAbstractProxyModel                *mItemFilterModel;
  QItemSelectionModel                *mFavSelection;
  QStringListModel                   *mFavsListModel;
  Akonadi::AgentFilterProxyModel     *mAgentInstanceFilterModel;
  QItemSelectionModel                *mAgentInstanceSelectionModel;
  Akonadi::BreadcrumbNavigationFactory *mBnf;
  Akonadi::BreadcrumbNavigationFactory *mMultiBnf;
  QItemSelectionModel                *mItemNavigationSelectionModel;
  QItemSelectionModel                *mItemActionSelectionModel;
  QHash<QString, QStringList>        mPersistedSelections;
  Future::KViewStateMaintainer<Akonadi::ETMViewStateSaver> *mItemViewStateMaintainer;
  KLineEdit                          *mFilterLineEdit;
  KLineEdit                          *mBulkActionFilterLineEdit;
  AgentStatusMonitor                 *mAgentStatusMonitor;
  GuiStateManager                    *mGuiStateManager;

public: /// Methods
  KDeclarativeMainViewPrivate();

  QAbstractItemModel* getFavoritesListModel();
  QStringList getFavoritesList();

public slots:
  void saveState();
  void restoreState();
  void filterLineEditChanged( const QString &text );
  void bulkActionFilterLineEditChanged( const QString &text );
};

#endif // KDECLARATIVEMAINVIEW_P_H
