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
#include "stylesheetloader.h"

#include <KDebug>
#include <KGlobalSettings>
#include <KStandardDirs>
#include <KMessageBox>
#include <klocalizedstring.h>

#include <QtCore/qcoreapplication.h>
#include <QtCore/QTimer>
#include <QtDBus/qdbusconnection.h>
#include <QtDBus/qdbusmessage.h>
#include <QtDeclarative/QDeclarativeContext>
#include <QtDeclarative/QDeclarativeEngine>
#include <QtDeclarative/QDeclarativeError>

#include <boost/bind.hpp>
#include <algorithm>


KDeclarativeFullScreenView::KDeclarativeFullScreenView(const QString& qmlFileName, QWidget* parent) :
  QDeclarativeView( parent )
{
  setResizeMode( QDeclarativeView::SizeRootObjectToView );
#ifdef Q_WS_MAEMO_5
  setWindowState( Qt::WindowFullScreen );
  // use the oxygen black on whilte palette instead of the native white on black maemo5 one
  setPalette( KGlobalSettings::createApplicationPalette( KGlobal::config() ) );
#endif
  StyleSheetLoader::applyStyle( this );

  connect( this, SIGNAL(statusChanged(QDeclarativeView::Status)), SLOT(slotStatusChanged(QDeclarativeView::Status)) );

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

void KDeclarativeFullScreenView::slotStatusChanged ( QDeclarativeView::Status status )
{
  if ( status == QDeclarativeView::Error ) {
    QStringList errorMessages;
    std::transform( errors().constBegin(), errors().constEnd(), std::back_inserter( errorMessages ), boost::bind( &QDeclarativeError::toString, _1 ) );
    KMessageBox::error( this, i18n( "Application loading failed: %1", errorMessages.join( QLatin1String( "\n" ) ) ) );
    QCoreApplication::instance()->exit( 1 );
  }
}

#include "kdeclarativefullscreenview.moc"
