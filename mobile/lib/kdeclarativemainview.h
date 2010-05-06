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
#ifndef KDECLARATIVEMAINVIEW_H
#define KDECLARATIVEMAINVIEW_H

#include "kdeclarativefullscreenview.h"
#include <QItemSelectionModel>

class QAbstractItemModel;
namespace Akonadi {
class EntityTreeModel;
}

class ListProxy;
class KDeclarativeMainViewPrivate;

/**
 * Main view for mobile applications. This class is just to share code and therefore
 * should not be instantiated by itself.
 */
class MOBILEUI_EXPORT KDeclarativeMainView : public KDeclarativeFullScreenView
{
  Q_OBJECT;

protected:
  /**
   * Creates a new main view for a mobile application.
   *
   * @param appName is used to find the QML file in ${DATA_DIR}/mobile/appname.qml
   * @param listProxy proxy for the list view of the application. KDeclarativeMainView
                      takes ownwership over the pointer.
   */
  KDeclarativeMainView( const QString &appName, ListProxy *listProxy, QWidget *parent = 0 );

  /** Returns the central ETM. */
  Akonadi::EntityTreeModel* entityTreeModel() const;
  /** Returns the filtered and QML-adapted item model. */
  QAbstractItemModel* itemModel() const;

public:
  virtual ~KDeclarativeMainView();

  /**
    * By default the view fetches the full payloads for the list. Use this method
    * to fetch only a specific part.
    */
  void setListPayloadPart( const QByteArray &payloadPart );

  /** Mime type of the items handled by this application. */
  void addMimeType( const QString &mimeType );

  QStringList mimeTypes() const;

public slots:
  void setSelectedAccount( int row );
  void setSelectedChildCollectionRow( int row );
  void setSelectedBreadcrumbCollectionRow( int row );

  void setListSelectedRow( int row );

  /** Returns wheter or not the child collection at row @param row has children. */
  bool childCollectionHasChildren( int row );

  QString pathToItem( qint64 id );

  void launchAccountWizard();

  void saveFavorite( const QString &name );
  void loadFavorite( const QString &name );

  QObject* getAction( const QString &name ) const;

protected:
  QItemSelectionModel* regularSelectionModel() const;
  QItemSelectionModel* favoriteSelectionModel() const;
  QAbstractItemModel *regularSelectedItems() const;
  QAbstractItemModel *favoriteSelectedItems() const;

private:
  KDeclarativeMainViewPrivate * const d;
  Q_DISABLE_COPY( KDeclarativeMainView )
};

#endif // KDECLARATIVEMAINVIEW_H
