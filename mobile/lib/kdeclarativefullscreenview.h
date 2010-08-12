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

#ifndef Q_OS_WINCE
#include <QGLWidget>
#endif


#include "mobileui_export.h"

class KActionCollection;
class QLabel;

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
    virtual ~KDeclarativeFullScreenView();

  public slots:
    /** Triggers de-fullscreen/task switcher */
    void triggerTaskSwitcher();

    /** Get an action based on name. If the @param argument is not empty, it sets that
     * as the data member of the action, see QAction::setData().
     */
    QObject* getAction( const QString &name, const QString& argument ) const;

    void setActionTitle( const QString& name, const QString& title);
    
    KActionCollection* actionCollection() const;

  protected slots:
    /** Most initialization work should be done here instead of the ctor.
     * @note: Remember to call the base class implementation when overwriting this.
     */
    virtual void delayedInit();

  private slots:
    void setQmlFile( const QString &source );
    void slotStatusChanged ( QDeclarativeView::Status );

  private:
    KActionCollection *mActionCollection;
#ifndef Q_OS_WINCE
    QGLWidget *glWidget;
#endif
    QString m_qmlFileName;
    QLabel *m_splashScreen;
};

#endif
