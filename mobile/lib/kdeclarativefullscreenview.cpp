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
#include <akonadi/servermanager.h>

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

#ifdef Q_OS_WINCE
#include <windows.h>
#endif

#ifdef KDEQMLPLUGIN_STATIC
#include "runtime/qml/kde/kdeintegration.h"
#include <QDeclarativeContext>
#endif


KDeclarativeFullScreenView::KDeclarativeFullScreenView(const QString& qmlFileName, QWidget* parent) :
  QDeclarativeView( parent ),
#ifndef Q_OS_WIN
  m_glWidget( 0 ),
#endif
  m_qmlFileName( qmlFileName )
{
#ifdef Q_OS_WINCE
  RotateTo270Degrees();
#endif
#ifndef Q_OS_WIN
  bool openGlEnabled = false; // off by default, seems to have random bad side-effects on the N900
  if ( KCmdLineArgs::parsedArgs()->isSet( "enable-opengl" ) )
    openGlEnabled = true;
  if ( KCmdLineArgs::parsedArgs()->isSet( "disable-opengl" ) )
     openGlEnabled = false;

  if ( openGlEnabled ) {
    // make MainView use OpenGL ES2 backend for better performance
    // right now, the best performance can be achieved with a GLWidget
    // and the use of the raster graphicssystem.
    QGLFormat format = QGLFormat::defaultFormat();
    format.setSampleBuffers(false);
    m_glWidget = new QGLWidget(format, this); // use OpenGL ES2 backend.
    m_glWidget->setAutoFillBackground(false);
    setViewport(m_glWidget);
    Akonadi::Control::widgetNeedsAkonadi( m_glWidget );
  } else {
    Akonadi::Control::widgetNeedsAkonadi( this );
  }
#else
  Akonadi::Control::widgetNeedsAkonadi( this );
#endif

#ifdef KDEQMLPLUGIN_STATIC  
  rootContext()->setContextProperty( QLatin1String("KDE"), new KDEIntegration( this ) );
#endif

  setResizeMode( QDeclarativeView::SizeRootObjectToView );
#if defined (Q_WS_MAEMO_5) || defined (Q_OS_WINCE)
  setWindowState( Qt::WindowFullScreen );
#ifndef Q_OS_WINCE
  // use the oxygen black on whilte palette instead of the native white on black maemo5 one
  setPalette( KGlobalSettings::createApplicationPalette( KGlobal::config() ) );
#endif
#else
  // on the desktop start with a nice size
  resize(800, 480);
#endif

  qApp->setStartDragDistance(40);

#ifndef Q_OS_WINCE
  m_splashScreen = new QLabel( this );
//Take out Splashscreen, because it is loaded each time a new window is opened
//This is too much for wince
  QPixmap splashBackground;
  splashBackground.load( KStandardDirs::locate( "data", QLatin1String( "mobileui" ) + QDir::separator() + QLatin1String( "splashscreenstatic.png" ) ) );
  m_splashScreen->setPixmap( splashBackground );
#endif

  QMetaObject::invokeMethod( this, "delayedInit", Qt::QueuedConnection );
}

void KDeclarativeFullScreenView::delayedInit()
{
  kDebug();
  static const bool debugTiming = KCmdLineArgs::parsedArgs()->isSet("timeit");
  QTime t;

  connect( this, SIGNAL(statusChanged(QDeclarativeView::Status)), SLOT(slotStatusChanged(QDeclarativeView::Status)) );

  engine()->rootContext()->setContextProperty( "window", QVariant::fromValue( static_cast<QObject*>( this ) ) );

  if ( debugTiming ) {
    kWarning() << "Adding QML import paths" << t.elapsed() << &t;
  }
  foreach ( const QString &importPath, KGlobal::dirs()->findDirs( "module", "imports" ) )
    engine()->addImportPath( importPath );
  QString qmlPath = KStandardDirs::locate( "appdata", m_qmlFileName + ".qml" );

  if ( debugTiming ) {
    kWarning() << "Adding QML import paths done" << t.elapsed() << &t;
  }

  if ( qmlPath.isEmpty() ) // Try harder
    qmlPath = KStandardDirs::locate( "data", QLatin1String( "mobileui" ) + QDir::separator() + m_qmlFileName + ".qml" );

  // TODO: Get this from a KXMLGUIClient?
  mActionCollection = new KActionCollection( this );

  KAction *action = KStandardAction::close( this, SLOT(close()), this );
  mActionCollection->addAction( QLatin1String( "close" ), action );

  action = new KAction( "Akonadi " + KStandardGuiItem::quit().text(), this ); //FIXME: use proper i18n after string freeze
  connect( action, SIGNAL( triggered() ), SLOT( closeAkonadi() ) );
  mActionCollection->addAction( QLatin1String( "quit_akonadi" ), action );

  action = new KAction( i18n( "Minimize Window" ), this );
  connect( action, SIGNAL(triggered()), SLOT(triggerTaskSwitcher()) );
  mActionCollection->addAction( QLatin1String( "wm_task_switch" ), action );

  if ( debugTiming ) {
    kWarning() << "KDeclarativeFullScreenView ctor done" << t.elapsed() << &t << QDateTime::currentDateTime();
  }

  doDelayedInitInternal();
  doDelayedInit(); // let sub-classes do their init work

  // call setSource() only once our derived classes have set up everything
  QMetaObject::invokeMethod( this, "setQmlFile", Qt::QueuedConnection, Q_ARG( QString, qmlPath ) );
}

KDeclarativeFullScreenView::~KDeclarativeFullScreenView()
{
#ifndef Q_OS_WIN
  delete m_glWidget;
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
  qDebug() << "trying to load \"" +  source << "\"";
  setSource( source );
  if ( debugTiming ) {
    kWarning() << "setSourceDone" << t.elapsed() << &t;
  }
}

void KDeclarativeFullScreenView::closeAkonadi()
{
  //FIXME: use proper i18n after string freeze
  const QString message = QLatin1String( "Shutting down Akonadi will disable notifications\nabout new emails and upcoming events." );
  const int result = KMessageBox::warningContinueCancel( 0, message );

  if ( result == KMessageBox::Cancel )
    return;

  Akonadi::ServerManager::self()->stop();
  close();
}

#ifdef Q_OS_WINCE
bool KDeclarativeFullScreenView::RotateTo270Degrees()
{
  DEVMODE DevMode;

  memset(&DevMode, 0, sizeof (DevMode));
  DevMode.dmSize               = sizeof (DevMode);
  DevMode.dmFields             = DM_DISPLAYORIENTATION;
  DevMode.dmDisplayOrientation = DMDO_270;
  if (DISP_CHANGE_SUCCESSFUL != ChangeDisplaySettingsEx(NULL, &DevMode, NULL, 0, NULL)){
    //error cannot change to 270 degrees
    return false;
  }

  return true;
}

bool KDeclarativeFullScreenView::winEvent ( MSG * message, long * result )
{
  Q_UNUSED(result);
  if ( message->message == WM_SETTINGCHANGE ) {
    RotateTo270Degrees();
  }
  return false;
}
#endif

void KDeclarativeFullScreenView::triggerTaskSwitcher()
{
#ifdef Q_WS_MAEMO_5
  QDBusConnection::sessionBus().call( QDBusMessage::createSignal( QLatin1String( "/" ), QLatin1String( "com.nokia.hildon_desktop" ), QLatin1String( "exit_app_view" ) ), QDBus::NoBlock );
#elif defined(_WIN32_WCE)
  HWND hWnd = ::FindWindow( _T( "DesktopExplorerWindow" ), NULL );
  if (hWnd != NULL){
    ::ShowWindow( hWnd, SW_SHOW );
    ::SetForegroundWindow(hWnd);
  }
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

  if ( status == QDeclarativeView::Ready ) {
#ifndef _WIN32_WCE
    m_splashScreen->deleteLater();
#else
    show();
    HWND hWnd = ::FindWindow( _T( "SplashScreen" ), NULL );
    if (hWnd != NULL)
      ::ShowWindow( hWnd, SW_HIDE );
#endif
  }
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
