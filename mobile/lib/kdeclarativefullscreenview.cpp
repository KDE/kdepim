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

#include <akonadi/control.h>

#include <KDebug>
#include <KGlobalSettings>
#include <KStandardDirs>
#include <KMessageBox>
#include <klocalizedstring.h>
#include <KAction>
#include <KActionCollection>
#include <KCmdLineArgs>

#include <QtCore/QDir>
#include <QtCore/QTimer>
#include <QtGui/QApplication>
#include <QtGui/QLabel>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QtDeclarative/QDeclarativeContext>
#include <QtDeclarative/QDeclarativeEngine>
#include <QtDeclarative/QDeclarativeError>

#include <boost/bind.hpp>
#include <algorithm>


KDeclarativeFullScreenView::KDeclarativeFullScreenView(const QString& qmlFileName, QWidget* parent) :
  QDeclarativeView( parent ),
  m_qmlFileName( qmlFileName )
{
#ifndef Q_OS_WIN
  // make MainView use OpenGL ES2 backend for better performance
  // right now, the best performance can be achieved with a GLWidget
  // and the use of the raster graphicssystem.
  QGLFormat format = QGLFormat::defaultFormat();
  format.setSampleBuffers(false);
  glWidget = new QGLWidget(format, this); // use OpenGL ES2 backend.
  glWidget->setAutoFillBackground(false);
  setViewport(glWidget);
  Akonadi::Control::widgetNeedsAkonadi( glWidget );
#else
  Akonadi::Control::widgetNeedsAkonadi( this );
#endif

  setResizeMode( QDeclarativeView::SizeRootObjectToView );
#if defined (Q_WS_MAEMO_5) || defined (Q_OS_WINCE)
  setWindowState( Qt::WindowFullScreen );
  // use the oxygen black on whilte palette instead of the native white on black maemo5 one
  setPalette( KGlobalSettings::createApplicationPalette( KGlobal::config() ) );
#else
  // on the desktop start with a nice size
  resize(800, 480);
#endif

  qApp->setStartDragDistance(40);

  m_splashScreen = new QLabel( this );
  QPixmap splashBackground;
  splashBackground.load( KStandardDirs::locate( "data", QLatin1String( "mobileui" ) + QDir::separator() + QLatin1String( "splashscreenstatic.png" ) ) );
  m_splashScreen->setPixmap( splashBackground );

  QMetaObject::invokeMethod( this, "delayedInit", Qt::QueuedConnection );
}

void KDeclarativeFullScreenView::delayedInit()
{
  kDebug();
  static const bool debugTiming = KCmdLineArgs::parsedArgs()->isSet("timeit");
  QTime t;
  if ( debugTiming ) {
    kWarning() << "Applying style" << t.elapsed() << &t;
  }
  StyleSheetLoader::applyStyle( this );

  if ( debugTiming ) {
    kWarning() << "Applying style done" << t.elapsed() << &t;
  }

  connect( this, SIGNAL(statusChanged(QDeclarativeView::Status)), SLOT(slotStatusChanged(QDeclarativeView::Status)) );

  engine()->rootContext()->setContextProperty( "window", QVariant::fromValue( static_cast<QObject*>( this ) ) );

  if ( debugTiming ) {
    kWarning() << "Adding QML import paths" << t.elapsed() << &t;
  }
  foreach ( const QString &importPath, KGlobal::dirs()->findDirs( "module", "imports" ) )
    engine()->addImportPath( importPath );
  QString qmlPath = KStandardDirs::locate( "appdata", m_qmlFileName + ".qml" );

  if ( debugTiming ) {
    kWarning() << "Applying style done" << t.elapsed() << &t;
  }

  if ( qmlPath.isEmpty() ) // Try harder
    qmlPath = KStandardDirs::locate( "data", QLatin1String( "mobileui" ) + QDir::separator() + m_qmlFileName + ".qml" );

  // call setSource() only once our derived classes have set up everything
  QMetaObject::invokeMethod( this, "setQmlFile", Qt::QueuedConnection, Q_ARG( QString, qmlPath ) );

  // TODO: Get this from a KXMLGUIClient?
  mActionCollection = new KActionCollection( this );

  KAction *action = KStandardAction::close( this, SLOT(close()), this );
  mActionCollection->addAction( QLatin1String( "close" ), action );
  action = new KAction( i18n( "Switch Windows" ), this );
  connect( action, SIGNAL(triggered()), SLOT(triggerTaskSwitcher()) );
  mActionCollection->addAction( QLatin1String( "wm_task_switch" ), action );

  if ( debugTiming ) {
    kWarning() << "KDeclarativeFullScreenView ctor done" << t.elapsed() << &t << QDateTime::currentDateTime();
  }
}

KDeclarativeFullScreenView::~KDeclarativeFullScreenView()
{
#ifndef Q_OS_WIN
  delete glWidget;
#endif
}

void KDeclarativeFullScreenView::setQmlFile(const QString& source)
{
  static const bool debugTiming = KCmdLineArgs::parsedArgs()->isSet("timeit");
  QTime t;
  if ( debugTiming ) {
    t.start();
    kWarning() << "start setSource" << &t << " - " << QDateTime::currentDateTime();
  }
  setSource( source );
  if ( debugTiming ) {
    kWarning() << "setSourceDone" << t.elapsed() << &t;
  }
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
  kWarning() << status << QDateTime::currentDateTime();

  if ( status == QDeclarativeView::Error ) {
    QStringList errorMessages;
    std::transform( errors().constBegin(), errors().constEnd(), std::back_inserter( errorMessages ), boost::bind( &QDeclarativeError::toString, _1 ) );
    KMessageBox::error( this, i18n( "Application loading failed: %1", errorMessages.join( QLatin1String( "\n" ) ) ) );
    QCoreApplication::instance()->exit( 1 );
  }

  if ( status == QDeclarativeView::Ready )
    m_splashScreen->deleteLater();
}

KActionCollection* KDeclarativeFullScreenView::actionCollection() const
{
  return mActionCollection;
}

QObject* KDeclarativeFullScreenView::getAction( const QString& name, const QString& argument ) const
{
  QAction * action = mActionCollection->action( name );
  if ( !argument.isEmpty() && action )
    action->setData( argument );
  return action;
}

QString KDeclarativeFullScreenView::getActionIconName( const QString& name ) const
{
  QAction * action = mActionCollection->action( name );
  if ( action )
    return action->icon().name();

  return QString();
}

void KDeclarativeFullScreenView::setActionTitle(const QString& name, const QString& title)
{
  QAction * action = mActionCollection->action( name );
  if ( !title.isEmpty() && action )
    action->setText( title );
}

#include "kdeclarativefullscreenview.moc"
