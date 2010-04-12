/*
    Copyright (c) 2010 Stephen Kelly <steveire@gmail.com>
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>

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

#ifndef MAINVIEW_H
#define MAINVIEW_H

#include <QtDeclarative/QDeclarativeView>

class QItemSelectionModel;

class KNavigatingProxyModel;
class KSelectionProxyModel;

namespace Akonadi
{
class EntityMimeTypeFilterModel;
}

/** The new KMMainWidget ;-) */
class MainView : public QDeclarativeView
{
  Q_OBJECT
  public:
    explicit MainView(QWidget* parent = 0);

  public slots:
    void setSelectedChildCollectionRow( int row );
    void setSelectedBreadcrumbCollectionRow( int row );

    bool hasChildCollectionHasChildren( int row );

  private slots:
    void saveState();
    void restoreState();

  private:
    QItemSelectionModel *m_collectionSelection;
    QItemSelectionModel *m_childCollectionSelection;
    QItemSelectionModel *m_breadcrumbCollectionSelection;
    Akonadi::EntityMimeTypeFilterModel *m_childCollectionFilter;
    KNavigatingProxyModel *m_childEntitiesModel;
    Akonadi::EntityMimeTypeFilterModel *m_collectionFilter;
    KSelectionProxyModel *m_selectedSubTree;
};

#endif
