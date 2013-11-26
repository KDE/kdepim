/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#ifndef CHECKEDCOLLECTIONWIDGET_H
#define CHECKEDCOLLECTIONWIDGET_H

#include <QWidget>
#include "pimcommon_export.h"

class QItemSelectionModel;
class KRecursiveFilterProxyModel;
class QTreeView;
class KCheckableProxyModel;

namespace Akonadi {
class EntityTreeModel;
}

namespace PimCommon {
class PIMCOMMON_EXPORT CheckedCollectionWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CheckedCollectionWidget(const QString &mimetype, QWidget *parent=0);
    ~CheckedCollectionWidget();

    Akonadi::EntityTreeModel *entityTreeModel() const;

    QTreeView *folderTreeView() const;

    QItemSelectionModel *selectionModel() const;

    KCheckableProxyModel *checkableProxy() const;

private Q_SLOTS:
    void slotSetCollectionFilter(const QString &filter);

private:
    QTreeView *mFolderView;
    QItemSelectionModel *mSelectionModel;
    KCheckableProxyModel *mCheckProxy;
    KRecursiveFilterProxyModel *mCollectionFilter;
    Akonadi::EntityTreeModel *mEntityTreeModel;
};
}

#endif // CHECKEDCOLLECTIONWIDGET_H
