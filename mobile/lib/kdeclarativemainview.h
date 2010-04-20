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

#include <QtDeclarative/QDeclarativeView>

#include "mobileui_export.h"

class ListProxy;
class KDeclarativeMainViewPrivate;

/**
 * Main view for mobile applications. This class is just to share code and therefore
 * should not be instantiated by itself.
 */
class MOBILEUI_EXPORT KDeclarativeMainView : public QDeclarativeView
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

public:
  virtual ~KDeclarativeMainView();

  /**
    * By default the view fetches the full payloads for the list. Use this method
    * to fetch only a specific part.
    */
  void setListPayloadPart( const QByteArray &payloadPart );

  /** Mime type of the items handled by this application. */
  void setMimeType( const QString &mimeType );

public slots:
  void setSelectedAccount( int row );
  void setSelectedChildCollectionRow( int row );
  void setSelectedBreadcrumbCollectionRow( int row );

  /** Returns wheter or not the child collection at row @param row has children. */
  bool childCollectionHasChildren( int row );

  /** Triggers de-fullscreen/task switcher */
  void triggerTaskSwitcher();

private:
  KDeclarativeMainViewPrivate * const d;
  Q_DISABLE_COPY( KDeclarativeMainView )
};

#endif // KDECLARATIVEMAINVIEW_H
