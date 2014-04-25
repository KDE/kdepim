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
#include "declarativewidgetbase.h"
#include "favoriteseditor.h"
#include "searchmanager.h"
#include "statemachinebuilder.h"

#include <akonadi/agentfilterproxymodel.h>
#include <AkonadiCore/changerecorder.h>
#include <AkonadiCore/entitymimetypefiltermodel.h>
#include <akonadi/etmviewstatesaver.h>
#include <kviewstatemaintainer.h>
#include <klineedit.h>

#include <QItemSelectionModel>
#include <QStringListModel>
#include <QPointer>

class AgentStatusMonitor;
class KActionCollection;

class ListProxy;
class KDeclarativeMainViewPrivate : public QObject
{
  Q_OBJECT

public: /// members
  KDeclarativeMainView               *q;
  Akonadi::ChangeRecorder            *mChangeRecorder;                // Deleted by ~QObject
  QAbstractItemModel                 *mCollectionFilter;              // Deleted by ~QObject
  Akonadi::EntityTreeModel           *mEtm;
  ListProxy                          *mListProxy;
  QAbstractItemModel                 *mItemModel;
  QAbstractProxyModel                *mItemFilterModel;
  Akonadi::AgentFilterProxyModel     *mAgentInstanceFilterModel;
  QItemSelectionModel                *mAgentInstanceSelectionModel;
  Akonadi::BreadcrumbNavigationFactory *mBnf;
  Akonadi::BreadcrumbNavigationFactory *mMultiBnf;
  QItemSelectionModel                *mItemNavigationSelectionModel;
  QItemSelectionModel                *mItemActionSelectionModel;
  QHash<QString, QStringList>        mPersistedSelections;
  KViewStateMaintainer<Akonadi::ETMViewStateSaver> *mItemViewStateMaintainer;
  QPointer<KLineEdit>                mFilterLineEdit;
  QPointer<KLineEdit>                mBulkActionFilterLineEdit;
  AgentStatusMonitor                 *mAgentStatusMonitor;
  GuiStateManager                    *mGuiStateManager;
  NotifyingStateMachine              *mStateMachine;
  SearchManager                      *mSearchManager;
  FavoritesEditor                    *mFavoritesEditor;

public: /// Methods
  explicit KDeclarativeMainViewPrivate( KDeclarativeMainView* );

  void openHtml( const QString &path );

public slots:
  void initializeStateSaver();
  void saveState();
  void restoreState();
  void filterLineEditChanged( const QString &text );
  void bulkActionFilterLineEditChanged( const QString &text );
  void searchStarted( const Akonadi::Collection& );
  void searchStopped();
  void guiStateChanged( int oldState, int newState );
  void configureAgentInstance();
};

class DeclarativeBulkActionFilterLineEdit :
#ifndef Q_MOC_RUN
public DeclarativeWidgetBase<KLineEdit, KDeclarativeMainView, &KDeclarativeMainView::setBulkActionFilterLineEdit>
#else
public QGraphicsProxyWidget
#endif
{
  Q_OBJECT

  public:
    explicit DeclarativeBulkActionFilterLineEdit( QGraphicsItem *parent = 0 );
    ~DeclarativeBulkActionFilterLineEdit();

  public Q_SLOTS:
    void clear();
};

#endif // KDECLARATIVEMAINVIEW_P_H
