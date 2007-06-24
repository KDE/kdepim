/*  This file is part of kdepim.
    Copyright (C) 2005,2007 Will Stephenson <wstephenson@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2
    as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QTimer>

#include <KAboutData>
#include <KApplication>
#include <KCmdLineArgs>
#include <KDebug>
#include <KLocale>

#include "service.h"
#include "serviceinterface.h"

QString toString( NetworkStatus::Status st )
{
  QString str;
  switch ( st ) {
    case NetworkStatus::NoNetworks:
      str = "NoNetworks";
      break;
    case NetworkStatus::Unreachable:
      str = "Unreachable";
      break;
    case NetworkStatus::OfflineDisconnected:
      str = "OfflineDisconnected";
      break;
    case NetworkStatus::OfflineFailed:
      str = "OfflineFailed";
      break;
    case NetworkStatus::TearingDown:
      str = "TearingDown";
      break;
    case NetworkStatus::Offline:
      str = "Offline";
      break;
    case NetworkStatus::Establishing:
      str = "Establishing";
      break;
    case NetworkStatus::Online:
      str = "Online";
      break;
  }
  return str;
}

TestService::TestService() : KMainWindow( 0 ),
    m_service( new OrgKdeSolidNetworkingServiceInterface( "org.kde.kded", "/modules/networkstatus", QDBusConnection::sessionBus(), this ) ),
    m_status ( NetworkStatus::Offline ),
    m_nextStatus( NetworkStatus::OfflineDisconnected ),
    m_view( new QWidget( this ) )
{
    QDBusConnection::sessionBus().registerService( "org.kde.Solid.Networking.TestService" );

    ui.setupUi( m_view );
    setCentralWidget( m_view );
    connect( ui.changeCombo, SIGNAL( activated( int ) ), SLOT( changeComboActivated( int ) ) );
    connect( ui.changeButton, SIGNAL( clicked() ), SLOT( changeButtonClicked() ) );

    connect( QDBusConnection::sessionBus().interface(), SIGNAL(serviceOwnerChanged(const QString&, const QString&, const QString & ) ), SLOT(serviceOwnerChanged(const QString&, const QString&, const QString & ) ) );

    ui.statusLabel->setText( toString( m_status ) );
    QPalette palette;
    palette.setColor( ui.statusLabel->backgroundRole(), toQColor( m_status ) );
    ui.statusLabel->setPalette( palette );
    setCaption( toString( m_status ) );

    registerService();
}

TestService::~TestService()
{
    delete m_service;
}

void TestService::registerService()
{
    m_service->registerNetwork( "test_net", m_status, "org.kde.Solid.Networking.TestService" );
}

void TestService::serviceOwnerChanged( const QString& service,const QString& oldOwner, const QString& newOwner )
{
    Q_UNUSED( oldOwner );
    if ( !newOwner.isEmpty() && service == "org.kde.kded" ) {
        kDebug() << "KDED restarted, trying to re-register service with it" << endl;
        registerService();
    }
}

int TestService::status( const QString & network )
{
    Q_UNUSED( network );
    return (int)m_status;
}

void TestService::changeComboActivated( int index )
{
  switch ( index ) {
    case 0 /*NetworkStatus::OfflineDisconnected*/:
      m_nextStatus = NetworkStatus::OfflineDisconnected;
      break;
    case 1 /*NetworkStatus::OfflineFailed*/:
      m_nextStatus = NetworkStatus::OfflineFailed;
      break;
    case 2 /*NetworkStatus::TearingDown*/:
      m_nextStatus = NetworkStatus::TearingDown;
      break;
    case 3 /*NetworkStatus::Offline*/:
      m_nextStatus = NetworkStatus::Offline;
      break;
    case 4 /*NetworkStatus::Establishing*/:
      m_nextStatus = NetworkStatus::Establishing;
      break;
    case 5 /*NetworkStatus::Online*/:
      m_nextStatus = NetworkStatus::Online;
      break;
    default:
      kDebug() << "Unrecognised status!" << endl;
      Q_ASSERT( false );
  }
  ui.changeButton->setEnabled( true );
}

void TestService::changeButtonClicked()
{
  ui.changeButton->setEnabled( false );
  m_status = m_nextStatus;
  m_service->setNetworkStatus( "test_net", ( int )m_status );
  ui.statusLabel->setText( toString( m_status ) );
  QPalette palette;
  palette.setColor( ui.statusLabel->backgroundRole(), toQColor( m_status ) );
  ui.statusLabel->setPalette( palette );
  setCaption( toString( m_status ) );
}
#if 0
int TestService::establish( const QString & network )
{
	Q_UNUSED( network );
	m_status = NetworkStatus::Establishing;
	m_service->setNetworkStatus( "test_net", (int)m_status );
	m_nextStatus = NetworkStatus::Online;
	QTimer::singleShot( 5000, this, SLOT( slotStatusChange() ) );
	return (int)NetworkStatus::RequestAccepted;
}

int TestService::shutdown( const QString & network )
{
	Q_UNUSED( network );
	m_status = NetworkStatus::TearingDown;
	m_service->setNetworkStatus( "test_net", (int)m_status );
	m_nextStatus = NetworkStatus::Offline;
	QTimer::singleShot( 5000, this, SLOT( slotStatusChange() ) );
	return (int)NetworkStatus::RequestAccepted;
}

void TestService::simulateFailure()
{
	m_status = NetworkStatus::OfflineFailed;
	m_service->setNetworkStatus( "test_net", (int)m_status );
}

void TestService::simulateDisconnect()
{
	m_status = NetworkStatus::OfflineDisconnected;
	m_service->setNetworkStatus( "test_net", (int)m_status );
}
#endif
void TestService::slotStatusChange()
{
	m_status = m_nextStatus;
	m_service->setNetworkStatus( "test_net", (int)m_status );
}

QColor TestService::toQColor( NetworkStatus::Status st )
{
    QColor col;
    switch ( st ) {
      case NetworkStatus::NoNetworks:
        col = Qt::darkGray;
        break;
      case NetworkStatus::Unreachable:
        col = Qt::darkMagenta;
        break;
      case NetworkStatus::OfflineDisconnected:
        col = Qt::blue;
        break;
      case NetworkStatus::OfflineFailed:
        col = Qt::darkRed;
        break;
      case NetworkStatus::TearingDown:
        col = Qt::darkYellow;
        break;
      case NetworkStatus::Offline:
        col = Qt::blue;
        break;
      case NetworkStatus::Establishing:
        col = Qt::yellow;
        break;
      case NetworkStatus::Online:
        col = Qt::green;
        break;
    }
    return col;
}

static const char description[] =
    I18N_NOOP("Test Service for Network Status kded module");

static const char version[] = "v0.1";

static KCmdLineOptions options[] =
{
    KCmdLineLastOption
};

int main( int argc, char** argv )
{
    KAboutData about("KNetworkStatusTestService", I18N_NOOP("knetworkstatustestservice"), version, description, KAboutData::License_GPL, "(C) 2007 Will Stephenson", 0, 0, "wstephenson@kde.org");
    about.addAuthor( "Will Stephenson", 0, "wstephenson@kde.org" );
    KCmdLineArgs::init(argc, argv, &about);
    KCmdLineArgs::addCmdLineOptions(options);
    KApplication app;

    TestService * test = new TestService;
    test->show();
    return app.exec();
}

#include "service.moc"
