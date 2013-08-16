/* -*- mode: c++; c-basic-offset:4 -*-
    mainwindow_mobile.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2010 Klarälvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include <config-kleopatra.h>

#include "mainwindow_mobile.h"

#include "aboutdata.h"

#include "models/keylistmodel.h"
#include "models/keylistsortfilterproxymodel.h"

#include "view/searchbar.h"
#if 0
#include "view/tabwidget.h"
#endif
#include "view/keytreeview.h"
#include "view/keylistcontroller.h"

#include "commands/selftestcommand.h"
#include "commands/importcrlcommand.h"
#include "commands/importcertificatefromfilecommand.h"
#include "commands/decryptverifyfilescommand.h"
#include "commands/signencryptfilescommand.h"

#include "utils/detail_p.h"
#include "utils/gnupg-helper.h"
#include "utils/action_data.h"
#include "utils/classify.h"
#include "utils/filedialog.h"

// from libkdepim
#include "progresswidget/statusbarprogresswidget.h"
#include "progresswidget/progressdialog.h"

// from mobileui
#include "declarativewidgetbase.h"

#include <KActionCollection>
#include <KLocale>
#include <KTabWidget>
#include <KStatusBar>
#include <KStandardAction>
#include <KAction>
#include <KAboutData>
#include <KMessageBox>
#include <KStandardGuiItem>
#include <KStandardDirs>
#include <KLineEdit>
#if 0
#include <KShortcutsDialog>
#include <KEditToolBar>
#endif
#include <KAboutApplicationDialog>
#include <kdebug.h>

#include <QTreeView>
#include <QFile>
#include <QToolBar>
#include <QWidgetAction>
#include <QApplication>
#include <QCloseEvent>
#include <QMenu>
#include <QTimer>
#include <QProcess>
#include <QPointer>
#include <QDeclarativeItem>
#include <QDeclarativeEngine>
#include <QDeclarativeContext>
#include <QVariant>
#include <QHeaderView>

#include <kleo/stl_util.h>
#include <kleo/cryptobackendfactory.h>
#include <ui/cryptoconfigdialog.h>
#include <kleo/cryptoconfig.h>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

#include <vector>

#ifdef Q_OS_WIN32
static const bool OS_WIN = true;
#else
static const bool OS_WIN = false;
#endif

using namespace Kleo;
using namespace Kleo::Commands;
using namespace boost;
using namespace GpgME;

namespace {

    static const KAboutData * aboutGpg4WinData() {
        static const AboutGpg4WinData data;
        return &data;
    }

}

class MainWindow::KeyTreeViewItem : public DeclarativeWidgetBase<KeyTreeView,MainWindow,&MainWindow::registerKeyTreeView> {
    Q_OBJECT
public:
    explicit KeyTreeViewItem( QGraphicsItem * parent=0 )
        : DeclarativeWidgetBase<KeyTreeView,MainWindow,&MainWindow::registerKeyTreeView>( parent ) {}
    ~KeyTreeViewItem() {}
};

class MainWindow::SearchBarItem : public DeclarativeWidgetBase<SearchBar,MainWindow,&MainWindow::registerSearchBar> {
    Q_OBJECT
public:
    explicit SearchBarItem( QGraphicsItem * parent=0 )
        : DeclarativeWidgetBase<SearchBar,MainWindow,&MainWindow::registerSearchBar>( parent ) {}
    ~SearchBarItem() {}
};

static KGuiItem KStandardGuiItem_quit() {
    static const QString app = KGlobal::mainComponent().aboutData()->programName();
    KGuiItem item = KStandardGuiItem::quit();
    item.setText( i18nc( "Quit [ApplicationName]", "&Quit %1", app ) );
    return item;
}

static KGuiItem KStandardGuiItem_close() {
    KGuiItem item = KStandardGuiItem::close();
    item.setText( i18n("Only &Close Window" ) );
    return item;
}

class MainWindow::Private {
    friend class ::MainWindow;
    MainWindow * const q;

public:
    explicit Private( MainWindow * qq );
    ~Private();

    void start( Command * c ) {
        c->setParentWidget( q );
        c->start();
    }

    template <typename T>
    void createAndStart() {
        this->start( new T( this->currentView(), &this->controller ) );
    }
    template <typename T>
    void createAndStart( QAbstractItemView * view ) {
        start( new T( view, &this->controller ) );
    }
    template <typename T>
    void createAndStart( const QStringList & a ) {
        start( new T( a, this->currentView(), &this->controller ) );
    }
    template <typename T>
    void createAndStart( const QStringList & a, QAbstractItemView * view ) {
        start( new T( a, view, &this->controller ) );
    }

    void closeAndQuit() {
        qApp->quit();
    }

    void selfTest() {
        createAndStart<SelfTestCommand>();
    }
    void configureBackend();
    void showHandbook();

    void gnupgLogViewer() {
        if( !QProcess::startDetached( QLatin1String( "kwatchgnupg" ) ) )
            KMessageBox::error( q, i18n( "Could not start the GnuPG Log Viewer (kwatchgnupg). "
                                         "Please check your installation." ),
                                i18n( "Error Starting KWatchGnuPG" ) );
    }

    void gnupgAdministrativeConsole() {
        if( !QProcess::startDetached( QLatin1String( "kgpgconf" ) ) )
            KMessageBox::error( q, i18n( "Could not start the GnuPG Administrative Console (kgpgconf). "
                                         "Please check your installation." ),
                                i18n( "Error Starting KGpgConf" ) );
    }

    void slotConfigCommitted();
    void slotSearchBarTextChanged( const QString & );

    void aboutGpg4Win() {
        ( new KAboutApplicationDialog( aboutGpg4WinData(), KAboutApplicationDialog::HideKdeVersion|KAboutApplicationDialog::HideTranslators, q ) )->show();
    }

private:
    void setupActions();
    void tryToConnectSearchBarToKeyTreeView() {
        if ( searchBar && keyTreeView )
            keyTreeView->connectSearchBar( searchBar );
    }

    QAbstractItemView * currentView() const {
        return controller.currentView();
    }

private:
    QPointer<SearchBar> searchBar;
    QPointer<KeyTreeView> keyTreeView;
    Kleo::KeyListController controller;
    bool firstShow : 1;
};

MainWindow::Private::Private( MainWindow * qq )
    : q( qq ),
      searchBar(),
      keyTreeView(),
      controller( q ),
      firstShow( true )
{
    KDAB_SET_OBJECT_NAME( controller );
    
    AbstractKeyListModel * flatModel = AbstractKeyListModel::createFlatKeyListModel( q );
    AbstractKeyListModel * hierarchicalModel = AbstractKeyListModel::createHierarchicalKeyListModel( q );

    KDAB_SET_OBJECT_NAME( flatModel );
    KDAB_SET_OBJECT_NAME( hierarchicalModel );


    controller.setFlatModel( flatModel );
    controller.setHierarchicalModel( hierarchicalModel );
    controller.setParentWidget( q );

}

MainWindow::Private::~Private() {}

MainWindow::MainWindow( QWidget * parent )
    : KDeclarativeFullScreenView( QLatin1String("kleopatra-mobile"), parent ), d( new Private( this ) )
{
}

MainWindow::~MainWindow() {}


void MainWindow::Private::setupActions() {

    KActionCollection * const coll = q->actionCollection();

    const action_data action_data[] = {
        // Settings menu
        { "settings_self_test", i18n("Perform Self-Test"), QString(),
          0, q, SLOT(selfTest()), QString(), false, true },
    };

    make_actions_from_data( action_data, coll );

    KStandardAction::close( q, SLOT(close()), coll );
    KStandardAction::quit( q, SLOT(closeAndQuit()), coll );
    KStandardAction::preferences( qApp, SLOT(openOrRaiseConfigDialog()), coll );

    controller.createActions( coll );
}

void MainWindow::doDelayedInit() {
    qmlRegisterType<KeyTreeViewItem>( "org.kde.kleopatra", 2, 1, "KeyTreeView" );
    qmlRegisterType<SearchBarItem>  ( "org.kde.kleopatra", 2, 1, "SearchBar"   );
    d->setupActions();
    engine()->rootContext()->setContextProperty( QLatin1String( "application" ), QVariant::fromValue( static_cast<QObject*>( this ) ) );
}

void MainWindow::registerKeyTreeView( KeyTreeView * view ) {
    if ( !view )
        return;
    view->setFlatModel( d->controller.flatModel() );
    view->setHierarchicalModel( d->controller.hierarchicalModel() );
    QTreeView * const v = view->view();
    v->setItemsExpandable( false );
    v->header()->setResizeMode( QHeaderView::Stretch );
    v->header()->hide();
    d->controller.addView( v );
    d->controller.setCurrentView( v );
    d->keyTreeView = view;
    d->tryToConnectSearchBarToKeyTreeView();

    connect( v->model(), SIGNAL(rowsInserted(QModelIndex,int,int)),
             SIGNAL(certificatesAvailabilityChanged()) );
    connect( v->model(), SIGNAL(rowsRemoved(QModelIndex,int,int)),
             SIGNAL(certificatesAvailabilityChanged()) );
    connect( v->model(), SIGNAL(modelReset()),
             SIGNAL(certificatesAvailabilityChanged()) );

    emit certificatesAvailabilityChanged();
}

void MainWindow::registerSearchBar( SearchBar * bar ) {
    if ( !bar )
        return;
    d->searchBar = bar;
    bar->setFixedHeight( 0 );
    connect( bar,  SIGNAL(stringFilterChanged(QString)),
             this, SLOT(slotSearchBarTextChanged(QString)) );
    d->tryToConnectSearchBarToKeyTreeView();
}

void MainWindow::Private::slotConfigCommitted() {
    controller.updateConfig();
}

void MainWindow::closeEvent( QCloseEvent * e ) {

    d->closeAndQuit();

    // KMainWindow::closeEvent() insists on quitting the application,
    // so do not let it touch the event...
    kDebug();
    if ( d->controller.hasRunningCommands() ) {
        if ( d->controller.shutdownWarningRequired() ) {
            const int ret = KMessageBox::warningContinueCancel( this, i18n("There are still some background operations ongoing. "
                                                                           "These will be terminated when closing the window. "
                                                                           "Proceed?"),
                                                                i18n("Ongoing Background Tasks") );
            if ( ret != KMessageBox::Continue ) {
                e->ignore();
                return;
            }
        }
        d->controller.cancelCommands();
        if ( d->controller.hasRunningCommands() ) {
            // wait for them to be finished:
            setEnabled( false );
            QEventLoop ev;
            QTimer::singleShot( 100, &ev, SLOT(quit()) );
            connect( &d->controller, SIGNAL(commandsExecuting(bool)), &ev, SLOT(quit()) );
            ev.exec();
            kWarning( d->controller.hasRunningCommands() )
                << "controller still has commands running, this may crash now...";
            setEnabled( true );
        }
    }
    e->accept();
}

void MainWindow::keyPressEvent( QKeyEvent * e ) {
    static bool isSendingEvent = false;

    if ( !isSendingEvent && d->searchBar && !e->text().isEmpty() ) {
        const struct guard { guard() { isSendingEvent = true; } ~guard() { isSendingEvent = false; } } guard;
        QCoreApplication::sendEvent( d->searchBar->lineEdit(), e );
    } else {
        KDeclarativeFullScreenView::keyPressEvent( e );
    }
}

void MainWindow::importCertificatesFromFile( const QStringList & files ) {
    if ( !files.empty() )
        d->createAndStart<ImportCertificateFromFileCommand>( files );
}

void MainWindow::Private::slotSearchBarTextChanged( const QString & text ) {
    if ( text.isEmpty() && searchBar && searchBar->isVisible() ) {
        searchBar->setFixedHeight( 0 );
        searchBar->hide();
    } else if ( !text.isEmpty() && searchBar && !searchBar->isVisible() ) {
        searchBar->setFixedHeight( searchBar->minimumSizeHint().height() );
        searchBar->show();
        searchBar->setFocus();
    }
}

bool MainWindow::certificatesAvailable() const {
    return (d->keyTreeView && d->keyTreeView->view()->model()->rowCount());
}

#include "moc_mainwindow_mobile.cpp"
#include "mainwindow_mobile.moc"
