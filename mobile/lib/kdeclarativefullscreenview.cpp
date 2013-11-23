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
#include <QApplication>
#include <QLabel>
#include <QResizeEvent>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QDeclarativeError>
#include <qplatformdefs.h>

#include <boost/bind.hpp>
#include <algorithm>
#include <iterator>

#ifdef KDEQMLPLUGIN_STATIC
#include "runtime/qml/kde/kdeintegration.h"
#include <QDeclarativeContext>
#endif

#ifdef MEEGO_EDITION_HARMATTAN
#include <QX11Info>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <fixx11h.h>
#endif

KDeclarativeFullScreenView::KDeclarativeFullScreenView(const QString& qmlFileName, QWidget* parent) :
  QDeclarativeView( parent ),
#ifndef Q_OS_WIN
  m_glWidget( 0 ),
#endif
  m_qmlFileName( qmlFileName ),
  m_splashScreen( 0 )
{

#ifdef MEEGO_EDITION_HARMATTAN
  blockSwipeRegion( 0, 0, 100, height() );
#endif

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

#ifdef KDEQMLPLUGIN_STATIC
  rootContext()->setContextProperty( QLatin1String("KDE"), new KDEIntegration( this ) );
#endif

  setResizeMode( QDeclarativeView::SizeRootObjectToView );

  resize(800, 480);

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

  connect( this, SIGNAL(statusChanged(QDeclarativeView::Status)), SLOT(slotStatusChanged(QDeclarativeView::Status)) );

  engine()->rootContext()->setContextProperty( QLatin1String("window"), QVariant::fromValue( static_cast<QObject*>( this ) ) );

  if ( debugTiming ) {
    kWarning() << "Adding QML import paths" << t.elapsed() << &t;
  }
  foreach ( const QString &importPath, KGlobal::dirs()->findDirs( "module", QLatin1String("imports") ) )
    engine()->addImportPath( importPath );
  QString qmlPath = KStandardDirs::locate( "appdata", m_qmlFileName + QLatin1String(".qml") );

  if ( debugTiming ) {
    kWarning() << "Adding QML import paths done" << t.elapsed() << &t;
  }

  if ( qmlPath.isEmpty() ) // Try harder
    qmlPath = KStandardDirs::locate( "data", QLatin1String( "mobileui" ) + QDir::separator() + m_qmlFileName + QLatin1String(".qml") );

  // TODO: Get this from a KXMLGUIClient?
  mActionCollection = new KActionCollection( this );

  KAction *action = KStandardAction::close( this, SLOT(close()), this );
  mActionCollection->addAction( QLatin1String( "close" ), action );

  action = new KAction( i18n( "Full Shutdown" ), this );
  connect( action, SIGNAL(triggered()), SLOT(closeAkonadi()) );
  mActionCollection->addAction( QLatin1String( "quit_akonadi" ), action );

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
  qDebug() << QLatin1String("trying to load \"") +  source << QLatin1String("\"");
  setSource( QUrl::fromLocalFile(source) );
  if ( debugTiming ) {
    kWarning() << "setSourceDone" << t.elapsed() << &t;
  }
}

void KDeclarativeFullScreenView::closeAkonadi()
{
  const QString message = i18n( "A full shutdown will disable notifications\nabout new emails and upcoming events." );
  const int result = KMessageBox::warningContinueCancel( 0, message );

  if ( result == KMessageBox::Cancel )
    return;

  Akonadi::ServerManager::self()->stop();
  close();
}

void KDeclarativeFullScreenView::closeAllFrontends(const QString &qmlFileName)
{
  QStringList applications;
  applications << QLatin1String( "notes-mobile" )
               << QLatin1String( "tasks-mobile" )
               << QLatin1String( "kmail-mobile" )
               << QLatin1String( "kaddressbook-mobile" )
               << QLatin1String( "korganizer-mobile" );
  if ( !applications.contains( qmlFileName + QLatin1String( "-mobile" ) ) &&
       !applications.contains( qmlFileName ) ){
    return;
  }
  foreach( const QString &app, applications ) {
    if ( app.startsWith( qmlFileName ) )
      continue;
    QDBusConnection::sessionBus().call( QDBusMessage::createMethodCall(
          QLatin1String( "org.kde." ) + app, QLatin1String( "/MainApplication" ),
          QLatin1String( "org.kde.KApplication" ), QLatin1String( "quit" ) ), QDBus::NoBlock );
  }
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
    if ( m_splashScreen ) {
      m_splashScreen->deleteLater();
      m_splashScreen = 0;
    }
#else
    show();
    HWND hWnd = ::FindWindow( _T( "SplashScreen" ), NULL );
    if (hWnd != NULL)
      ::ShowWindow( hWnd, SW_HIDE );
    SetCursor( LoadCursor( NULL, NULL ) );
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

void KDeclarativeFullScreenView::bringToFront()
{
  activateWindow();
  raise();
}

void KDeclarativeFullScreenView::resizeEvent(QResizeEvent* event)
{
  QDeclarativeView::resizeEvent(event);
  if ( m_splashScreen ) {
    m_splashScreen->move( (event->size().width() - m_splashScreen->sizeHint().width())/2,
                          (event->size().height() - m_splashScreen->sizeHint().height())/2 );
  }
}

#ifdef MEEGO_EDITION_HARMATTAN
void KDeclarativeFullScreenView::blockSwipeRegion( const int x, const int y, const int  w, const int h )
{
  Display *dpy = QX11Info::display();
  Atom blockedRegionAtom = XInternAtom( dpy, "_MEEGOTOUCH_CUSTOM_REGION", False );
  unsigned int blockedRegion[] = { x, y, w, h };

  XChangeProperty( dpy, this->winId(), blockedRegionAtom,
                XA_CARDINAL, 32, PropModeReplace,
                reinterpret_cast<unsigned char*>( &blockedRegion[0] ), 4 );
}
#endif // MEEGO_EDITION_HARMATTAN

