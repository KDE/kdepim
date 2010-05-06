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

#include "kdeclarativefullscreenview.h"

#include <KDebug>
#include <KStandardDirs>

#include <QtCore/QTimer>
#include <QtDBus/qdbusconnection.h>
#include <QtDBus/qdbusmessage.h>
#include <QtDeclarative/QDeclarativeContext>
#include <QtDeclarative/QDeclarativeEngine>


KDeclarativeFullScreenView::KDeclarativeFullScreenView(const QString& qmlFileName, QWidget* parent) :
  QDeclarativeView( parent )
{
  setResizeMode( QDeclarativeView::SizeRootObjectToView );
#ifdef Q_WS_MAEMO_5
  setWindowState( Qt::WindowFullScreen );
#endif

  engine()->rootContext()->setContextProperty( "window", QVariant::fromValue( static_cast<QObject*>( this ) ) );

  foreach ( const QString &importPath, KGlobal::dirs()->findDirs( "module", "imports" ) )
    engine()->addImportPath( importPath );
  const QString qmlPath = KStandardDirs::locate( "appdata", qmlFileName + ".qml" );
  // call setSource() only once our derived classes have set up everything
  QMetaObject::invokeMethod( this, "setQmlFile", Qt::QueuedConnection, Q_ARG( QString, qmlPath ) );
}

void KDeclarativeFullScreenView::triggerTaskSwitcher()
{
#ifdef Q_WS_MAEMO_5
  QDBusConnection::sessionBus().call( QDBusMessage::createSignal( QLatin1String( "/" ), QLatin1String( "com.nokia.hildon_desktop" ), QLatin1String( "exit_app_view" ) ), QDBus::NoBlock );
#else
  kDebug() << "not implemented for this platform";
#endif
}


#include "kdeclarativefullscreenview.moc"
