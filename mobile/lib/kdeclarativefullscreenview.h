/*
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

#ifndef KDECLARATIVEFULLSCREENVIEW_H
#define KDECLARATIVEFULLSCREENVIEW_H

#include <QtDeclarative/QDeclarativeView>

#include "mobileui_export.h"

/**
 * Full screen view for mobile applications. This class is just to share code and therefore
 * should not be instantiated by itself.
 */
class MOBILEUI_EXPORT KDeclarativeFullScreenView : public QDeclarativeView
{
  Q_OBJECT;
  protected:
    /**
    * Creates a new full screen view for a mobile application.
    *
    * @param qmlFileName is used to find the QML file in ${APP_DATA_DIR}/qmlFileName.qml
    */
    KDeclarativeFullScreenView( const QString &qmlFileName, QWidget *parent = 0 );

  public slots:
    /** Triggers de-fullscreen/task switcher */
    void triggerTaskSwitcher();
    void slotStatusChanged ( QDeclarativeView::Status );

  private slots:
    void setQmlFile( const QString &source ) { setSource( source ); }
};

#endif
