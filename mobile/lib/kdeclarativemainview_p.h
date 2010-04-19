/*
 * This file is part of Akonadi
 *
 * Copyright (c) 2010 Bertjan Broeksema <b.broeksema@home.nl>
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
#ifndef KDECLARATIVEMAINVIEW_P_H
#define KDECLARATIVEMAINVIEW_P_H

#include <QtCore/QObject>
#include <QtGui/QItemSelectionModel>

#include <akonadi/changerecorder.h>
#include <akonadi/entitymimetypefiltermodel.h>

#include "breadcrumbnavigation.h"

class KDeclarativeMainViewPrivate : public QObject
{
  Q_OBJECT

public: /// members
  QItemSelectionModel                *mBreadcrumbCollectionSelection; // Deleted by ~QObect
  Akonadi::ChangeRecorder            *mChangeRecorder;                // Deleted by ~QObect
  Akonadi::EntityMimeTypeFilterModel *mChildCollectionFilter;         // Deleted by ~QObect
  QItemSelectionModel                *mChildCollectionSelection;      // Deleted by ~QObect
  KNavigatingProxyModel              *mChildEntitiesModel;            // Deleted by ~QObect
  QItemSelectionModel                *mCollectionSelection;           // Deleted by ~QObect
  KSelectionProxyModel               *mSelectedSubTree;               // Deleted by ~QObect
  Akonadi::EntityMimeTypeFilterModel *mCollectionFilter;              // Deleted by ~QObect

public: /// Methods
  KDeclarativeMainViewPrivate();

public slots:
  void saveState();
  void restoreState();
};

#endif // KDECLARATIVEMAINVIEW_P_H
