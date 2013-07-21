/* -*- mode: c++; c-basic-offset:4 -*-
    controllers/keylistcontroller.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2007 Klarälvdalens Datakonsult AB

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

#include "keylistcontroller.h"
#include "tabwidget.h"

#include <models/keycache.h>
#include <models/keylistmodel.h>

#include <smartcard/readerstatus.h>

#include <utils/formatting.h>
#include <utils/action_data.h>

#include "tooltippreferences.h"

#include "commands/exportcertificatecommand.h"
#include "commands/exportopenpgpcertstoservercommand.h"
#include "commands/exportsecretkeycommand.h"
#include "commands/importcertificatefromfilecommand.h"
#include "commands/changepassphrasecommand.h"
#include "commands/lookupcertificatescommand.h"
#include "commands/reloadkeyscommand.h"
#include "commands/refreshx509certscommand.h"
#include "commands/refreshopenpgpcertscommand.h"
#include "commands/detailscommand.h"
#include "commands/deletecertificatescommand.h"
#include "commands/decryptverifyfilescommand.h"
#include "commands/signencryptfilescommand.h"
#include "commands/clearcrlcachecommand.h"
#include "commands/dumpcrlcachecommand.h"
#include "commands/dumpcertificatecommand.h"
#include "commands/importcrlcommand.h"
#include "commands/changeexpirycommand.h"
#include "commands/changeownertrustcommand.h"
#include "commands/changeroottrustcommand.h"
#include "commands/certifycertificatecommand.h"
#include "commands/adduseridcommand.h"
#include "commands/newcertificatecommand.h"
#include "commands/checksumverifyfilescommand.h"
#include "commands/checksumcreatefilescommand.h"

#include <kleo/stl_util.h>

#include <gpgme++/key.h>

#include <KActionCollection>
#include <KLocalizedString>

#include <QAbstractItemView>
#include <QTreeView>
#include <QTableView>
#include <QPointer>
#include <QItemSelectionModel>
#include <QAction>

#include <boost/bind.hpp>

#include <algorithm>
#include <cassert>

using namespace Kleo;
using namespace Kleo::Commands;
using namespace Kleo::SmartCard;
using namespace boost;
using namespace GpgME;

class KeyListController::Private {
    friend class ::Kleo::KeyListController;
    KeyListController * const q;
public:
    explicit Private( KeyListController * qq );
    ~Private();

    void connectView( QAbstractItemView * view );
    void connectCommand( Command * cmd );

    void connectTabWidget();
    void disconnectTabWidget();

    void addCommand( Command * cmd ) {
        connectCommand( cmd );
        commands.insert( std::lower_bound( commands.begin(), commands.end(), cmd ), cmd );
    }
    void addView( QAbstractItemView * view ) {
        connectView( view );
        views.insert( std::lower_bound( views.begin(), views.end(), view ), view );
    }
    void removeView( QAbstractItemView * view ) {
        view->disconnect( q );
        view->selectionModel()->disconnect( q );
        views.erase( std::remove( views.begin(), views.end(), view ), views.end() );
    }

public:
    void slotDestroyed( QObject * o ) {
        kDebug() << ( void* )o;
        views.erase( std::remove( views.begin(), views.end(), o ), views.end() );
        commands.erase( std::remove( commands.begin(), commands.end(), o ), commands.end() );
    }
    void slotDoubleClicked( const QModelIndex & idx );
    void slotActivated( const QModelIndex & idx );
    void slotSelectionChanged( const QItemSelection & old, const QItemSelection & new_ );
    void slotContextMenu( const QPoint & pos );
    void slotCommandFinished();
    void slotAddKey( const Key & key );
    void slotAboutToRemoveKey( const Key & key );
    void slotProgress( const QString & what, int current, int total ) {
        emit q->progress( current, total );
        if ( !what.isEmpty() )
            emit q->message( what );
    }
    void slotActionTriggered();
    void slotCurrentViewChanged( QAbstractItemView * view ) {
        if ( view && !kdtools::binary_search( views, view ) ) {
            kDebug() << "you need to register view" << view << "before trying to set it as the current view!";
            addView( view );
        }
        currentView = view;
        q->enableDisableActions( view ? view->selectionModel() : 0 );
    }

private:
    int toolTipOptions() const;

private:
    static Command::Restrictions calculateRestrictionsMask( const QItemSelectionModel * sm );

private:
    struct action_item {
        QPointer<QAction> action;
        Command::Restrictions restrictions;
        Command * (*createCommand)( QAbstractItemView *, KeyListController * );
    };
    std::vector<action_item> actions;
    std::vector<QAbstractItemView*> views;
    std::vector<Command*> commands;
    QPointer<QWidget> parentWidget;
    QPointer<TabWidget> tabWidget;
    QPointer<QAbstractItemView> currentView;
    QPointer<AbstractKeyListModel> flatModel, hierarchicalModel;
};


KeyListController::Private::Private( KeyListController * qq )
    : q( qq ),
      actions(),
      views(),
      commands(),
      parentWidget(),
      tabWidget(),
      flatModel(),
      hierarchicalModel()
{
    connect( KeyCache::mutableInstance().get(), SIGNAL(added(GpgME::Key)),
             q, SLOT(slotAddKey(GpgME::Key)) );
    connect( KeyCache::mutableInstance().get(), SIGNAL(aboutToRemove(GpgME::Key)),
             q, SLOT(slotAboutToRemoveKey(GpgME::Key)) );
}

KeyListController::Private::~Private() {}

KeyListController::KeyListController( QObject * p )
    : QObject( p ), d( new Private( this ) )
{

}

KeyListController::~KeyListController() {}



void KeyListController::Private::slotAddKey( const Key & key ) {
    // ### make model act on keycache directly...
    if ( flatModel )
        flatModel->addKey( key );
    if ( hierarchicalModel )
        hierarchicalModel->addKey( key );
}


void KeyListController::Private::slotAboutToRemoveKey( const Key & key ) {
    // ### make model act on keycache directly...
    if ( flatModel )
        flatModel->removeKey( key );
    if ( hierarchicalModel )
        hierarchicalModel->removeKey( key );
}

void KeyListController::addView( QAbstractItemView * view ) {
    if ( !view || kdtools::binary_search( d->views, view ) )
        return;
    d->addView( view );
}

void KeyListController::removeView( QAbstractItemView * view ) {
    if ( !view || !kdtools::binary_search( d->views, view ) )
        return;
    d->removeView( view );
}

void KeyListController::setCurrentView( QAbstractItemView * view ) {
    d->slotCurrentViewChanged( view );
}

std::vector<QAbstractItemView*> KeyListController::views() const {
    return d->views;
}

void KeyListController::setFlatModel( AbstractKeyListModel * model ) {
    if ( model == d->flatModel )
        return;

    d->flatModel = model;

    if ( model ) {
        model->clear();
        model->addKeys( KeyCache::instance()->keys() );
        model->setToolTipOptions( d->toolTipOptions() );
    }
}

void KeyListController::setHierarchicalModel( AbstractKeyListModel * model ) {
    if ( model == d->hierarchicalModel )
        return;

    d->hierarchicalModel = model;

    if ( model ) {
        model->clear();
        model->addKeys( KeyCache::instance()->keys() );
        model->setToolTipOptions( d->toolTipOptions() );
    }
}

void KeyListController::setTabWidget( TabWidget * tabWidget ) {
    if ( tabWidget == d->tabWidget )
        return;

    d->disconnectTabWidget();

    d->tabWidget = tabWidget;

    d->connectTabWidget();

    d->slotCurrentViewChanged( tabWidget ? tabWidget->currentView() : 0 );
}

void KeyListController::setParentWidget( QWidget * parent ) {
    d->parentWidget = parent;
}

QWidget * KeyListController::parentWidget() const {
    return d->parentWidget;
}

static const struct {
    const char * signal;
    const char * slot;
} tabs2controller[] = {
    { SIGNAL(viewAdded(QAbstractItemView*)),            SLOT(addView(QAbstractItemView*))                },
    { SIGNAL(viewAboutToBeRemoved(QAbstractItemView*)), SLOT(removeView(QAbstractItemView*))             },
    { SIGNAL(currentViewChanged(QAbstractItemView*)),   SLOT(slotCurrentViewChanged(QAbstractItemView*)) },
};
static const unsigned int numTabs2Controller = sizeof tabs2controller / sizeof *tabs2controller ;

void KeyListController::Private::connectTabWidget() {
    if ( !tabWidget )
        return;
    kdtools::for_each( tabWidget->views(), boost::bind( &Private::addView, this, _1 ) );
    for ( unsigned int i = 0 ; i < numTabs2Controller ; ++i )
        connect( tabWidget, tabs2controller[i].signal, q, tabs2controller[i].slot );
}

void KeyListController::Private::disconnectTabWidget() {
    if ( !tabWidget )
        return;
    for ( unsigned int i = 0 ; i < numTabs2Controller ; ++i )
        disconnect( tabWidget, tabs2controller[i].signal, q, tabs2controller[i].slot );
    kdtools::for_each( tabWidget->views(), boost::bind( &Private::removeView, this, _1 ) );
}

AbstractKeyListModel * KeyListController::flatModel() const {
    return d->flatModel;
}

AbstractKeyListModel * KeyListController::hierarchicalModel() const {
    return d->hierarchicalModel;
}

QAbstractItemView * KeyListController::currentView() const {
    return d->currentView;
}

TabWidget * KeyListController::tabWidget() const {
    return d->tabWidget;
}

void KeyListController::createActions( KActionCollection * coll ) {

    const action_data action_data[] = {
        // File menu
        { "file_new_certificate", i18n("New Certificate..."), QString(),
          "view-certificate-add", 0, 0, "Ctrl+N", false, true },
        { "file_export_certificates", i18n("Export Certificates..."), QString(),
          "view-certificate-export", 0, 0, "Ctrl+E", false, true },
        { "file_export_certificates_to_server", i18n("Export Certificates to Server..."), QString(),
          "view-certificate-export-server", 0, 0, "Ctrl+Shift+E", false, true },
        { "file_export_secret_keys", i18n("Export Secret Keys..."), QString(),
          "view-certificate-export-secret", 0, 0, QString(), false, true },
        { "file_lookup_certificates", i18n("Lookup Certificates on Server..."), QString(),
          "edit-find", 0, 0, "Shift+Ctrl+I", false, true },
        { "file_import_certificates", i18n("Import Certificates..."), QString(),
          "view-certificate-import", 0, 0, "Ctrl+I", false, true },
        { "file_decrypt_verify_files", i18n("Decrypt/Verify Files..."), QString(),
          "document-edit-decrypt-verify", 0, 0, QString(), false, true },
        { "file_sign_encrypt_files", i18n("Sign/Encrypt Files..."), QString(),
          "document-edit-sign-encrypt", 0, 0, QString(), false, true },
        { "file_checksum_create_files", i18n("Create Checksum Files..."), QString(),
          0/*"document-checksum-create"*/, 0, 0, QString(), false, true },
        { "file_checksum_verify_files", i18n("Verify Checksum Files..."), QString(),
          0/*"document-checksum-verify"*/, 0, 0, QString(), false, true },
        // View menu
        { "view_redisplay", i18n("Redisplay"), QString(),
          "view-refresh", 0, 0, "F5", false, true },
        { "view_stop_operations", i18n( "Stop Operation" ), QString(),
          "process-stop", this, SLOT(cancelCommands()), "Escape", false, false },
        { "view_certificate_details", i18n( "Certificate Details" ), QString(),
          "dialog-information", 0, 0, QString(), false, true },
        // Certificate menu
        { "certificates_delete", i18n("Delete" ), QString()/*i18n("Delete selected certificates")*/,
          "edit-delete", 0, 0, "Delete", false, true },
        { "certificates_certify_certificate", i18n("Certify Certificate..."), QString(),
          "view-certificate-sign", 0, 0, QString(), false, true },
        { "certificates_change_expiry", i18n("Change Expiry Date..."), QString(),
          0, 0, 0, QString(), false, true },
        { "certificates_change_owner_trust", i18n("Change Owner Trust..."), QString(),
          0, 0, 0, QString(), false, true },
        { "certificates_trust_root", i18n("Trust Root Certificate"), QString(),
          0, 0, 0, QString(), false, true },
        { "certificates_distrust_root", i18n("Distrust Root Certificate"), QString(),
          0, 0, 0, QString(), false, true },
        { "certificates_change_passphrase", i18n("Change Passphrase..."), QString(),
          0, 0, 0, QString(), false, true },
        { "certificates_add_userid", i18n("Add User-ID..."), QString(),
          0, 0, 0, QString(), false, true },
        { "certificates_dump_certificate", i18n("Dump Certificate"), QString(),
          0, 0, 0, QString(), false, true },
          // Tools menu
        { "tools_refresh_x509_certificates", i18n("Refresh X.509 Certificates"), QString(),
          "view-refresh", 0, 0, QString(), false, true },
        { "tools_refresh_openpgp_certificates", i18n("Refresh OpenPGP Certificates"), QString(),
          "view-refresh", 0, 0, QString(), false, true },
#ifndef KDEPIM_ONLY_KLEO
        { "crl_clear_crl_cache", i18n("Clear CRL Cache"), QString(),
          0, 0, 0, QString(), false, true },
        { "crl_dump_crl_cache", i18n("Dump CRL Cache"), QString(),
          0, 0, 0, QString(), false, true },
#endif // KDEPIM_ONLY_KLEO
        { "crl_import_crl", i18n("Import CRL From File..."), QString(),
          0, 0, 0, QString(), false, true },
        // Window menu
        // (come from TabWidget)
        // Help menu
        // (come from MainWindow)
    };


    make_actions_from_data( action_data, coll );

    if ( QAction * action = coll->action( "view_stop_operations" ) )
        connect( this, SIGNAL(commandsExecuting(bool)), action, SLOT(setEnabled(bool)) );

    // ### somehow make this better...
    registerActionForCommand<NewCertificateCommand>(     coll->action( "file_new_certificate" ) );
    //---
    registerActionForCommand<LookupCertificatesCommand>( coll->action( "file_lookup_certificates" ) );
    registerActionForCommand<ImportCertificateFromFileCommand>( coll->action( "file_import_certificates" ) );
    //---
    registerActionForCommand<ExportCertificateCommand>(  coll->action( "file_export_certificates" ) );
    registerActionForCommand<ExportSecretKeyCommand>(    coll->action( "file_export_secret_keys" ) );
    registerActionForCommand<ExportOpenPGPCertsToServerCommand>( coll->action( "file_export_certificates_to_server" ) );
    //---
    registerActionForCommand<DecryptVerifyFilesCommand>( coll->action( "file_decrypt_verify_files" ) );
    registerActionForCommand<SignEncryptFilesCommand>(   coll->action( "file_sign_encrypt_files" ) );
    //---
#ifndef _WIN32_WCE
    registerActionForCommand<ChecksumCreateFilesCommand>(coll->action( "file_checksum_create_files" ) );
    registerActionForCommand<ChecksumVerifyFilesCommand>(coll->action( "file_checksum_verify_files" ) );
#endif

    registerActionForCommand<ReloadKeysCommand>(         coll->action( "view_redisplay" ) );
    //coll->action( "view_stop_operations" ) <-- already dealt with in make_actions_from_data()
    registerActionForCommand<DetailsCommand>(            coll->action( "view_certificate_details" ) );

    registerActionForCommand<ChangeOwnerTrustCommand>(   coll->action( "certificates_change_owner_trust" ) );
    registerActionForCommand<TrustRootCommand>(          coll->action( "certificates_trust_root" ) );
    registerActionForCommand<DistrustRootCommand>(       coll->action( "certificates_distrust_root" ) );
    //---
    registerActionForCommand<CertifyCertificateCommand>( coll->action( "certificates_certify_certificate" ) );
    registerActionForCommand<ChangeExpiryCommand>(       coll->action( "certificates_change_expiry" ) );
    registerActionForCommand<ChangePassphraseCommand>(   coll->action( "certificates_change_passphrase" ) );
    registerActionForCommand<AddUserIDCommand>(          coll->action( "certificates_add_userid" ) );
    //---
    registerActionForCommand<DeleteCertificatesCommand>( coll->action( "certificates_delete" ) );
    //---
    registerActionForCommand<DumpCertificateCommand>(    coll->action( "certificates_dump_certificate" ) );

    registerActionForCommand<RefreshX509CertsCommand>(   coll->action( "tools_refresh_x509_certificates" ) );
    registerActionForCommand<RefreshOpenPGPCertsCommand>(coll->action( "tools_refresh_openpgp_certificates" ) );
    //---
    registerActionForCommand<ImportCrlCommand>(          coll->action( "crl_import_crl" ) );
    //---
    registerActionForCommand<ClearCrlCacheCommand>(      coll->action( "crl_clear_crl_cache" ) );
    registerActionForCommand<DumpCrlCacheCommand>(       coll->action( "crl_dump_crl_cache" ) );

    enableDisableActions( 0 );
}

void KeyListController::registerAction( QAction * action, Command::Restrictions restrictions, Command * (*create)( QAbstractItemView *, KeyListController * ) ) {
    if ( !action )
        return;
    assert( !action->isCheckable() ); // can be added later, for now, disallow

    const Private::action_item ai = {
        action, restrictions, create
    };
    connect( action, SIGNAL(triggered()), this, SLOT(slotActionTriggered()) );
    d->actions.push_back( ai );
}

void KeyListController::registerCommand( Command * cmd ) {
    if ( !cmd || kdtools::binary_search( d->commands, cmd ) )
        return;
    d->addCommand( cmd );
    kDebug() << ( void* )cmd;
    if ( d->commands.size() == 1 )
        emit commandsExecuting( true );
}

bool KeyListController::hasRunningCommands() const {
    return !d->commands.empty();
}

bool KeyListController::shutdownWarningRequired() const {
    return kdtools::any( d->commands, mem_fn( &Command::warnWhenRunningAtShutdown ) );
}

// slot
void KeyListController::cancelCommands() {
    kdtools::for_each( d->commands, mem_fn( &Command::cancel ) );
}

void KeyListController::Private::connectView( QAbstractItemView * view ) {

    connect( view, SIGNAL(destroyed(QObject*)),
             q, SLOT(slotDestroyed(QObject*)) );
    connect( view, SIGNAL(doubleClicked(QModelIndex)),
             q, SLOT(slotDoubleClicked(QModelIndex)) );
    connect( view, SIGNAL(activated(QModelIndex)),
             q, SLOT(slotActivated(QModelIndex)) );
    connect( view->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
             q, SLOT(slotSelectionChanged(QItemSelection,QItemSelection)) );

    view->setContextMenuPolicy( Qt::CustomContextMenu );
    connect( view, SIGNAL(customContextMenuRequested(QPoint)),
             q, SLOT(slotContextMenu(QPoint)) );
}

void KeyListController::Private::connectCommand( Command * cmd ) {
    if ( !cmd )
        return;
    connect( cmd, SIGNAL(destroyed(QObject*)), q, SLOT(slotDestroyed(QObject*)) );
    connect( cmd, SIGNAL(finished()), q, SLOT(slotCommandFinished()) );
    //connect( cmd, SIGNAL(canceled()), q, SLOT(slotCommandCanceled()) );
    connect( cmd, SIGNAL(info(QString,int)), q, SIGNAL(message(QString,int)) );
    connect( cmd, SIGNAL(progress(QString,int,int)), q, SLOT(slotProgress(QString,int,int)) );
}


void KeyListController::Private::slotDoubleClicked( const QModelIndex & idx ) {
    QAbstractItemView * const view = qobject_cast<QAbstractItemView*>( q->sender() );
    if ( !view || !kdtools::binary_search( views, view ) )
        return;

    DetailsCommand * const c = new DetailsCommand( view, q );
    if ( parentWidget )
        c->setParentWidget( parentWidget );

    c->setIndex( idx );
    c->start();
}

void KeyListController::Private::slotActivated( const QModelIndex & idx )
{
    Q_UNUSED( idx );
    QAbstractItemView * const view = qobject_cast<QAbstractItemView*>( q->sender() );
    if ( !view || !kdtools::binary_search( views, view ) )
        return;

}

void KeyListController::Private::slotSelectionChanged( const QItemSelection & old, const QItemSelection & new_ )
{
    Q_UNUSED( old );
    Q_UNUSED( new_ );

    const QItemSelectionModel * const sm = qobject_cast<QItemSelectionModel*>( q->sender() );
    if ( !sm )
        return;
    q->enableDisableActions( sm );
}

void KeyListController::Private::slotContextMenu( const QPoint & p ) {
    QAbstractItemView * const view = qobject_cast<QAbstractItemView*>( q->sender() );
    if ( view && kdtools::binary_search( views, view ) )
        emit q->contextMenuRequested( view, view->viewport()->mapToGlobal( p ) );
    else
        kDebug() << "sender is not a QAbstractItemView*!";
}

void KeyListController::Private::slotCommandFinished() {
    Command * const cmd = qobject_cast<Command*>( q->sender() );
    if ( !cmd || !kdtools::binary_search( commands, cmd ) )
        return;
    kDebug() << ( void* )cmd;
    if ( commands.size() == 1 )
        emit q->commandsExecuting( false );
}

void KeyListController::enableDisableActions( const QItemSelectionModel * sm ) const {
    const Command::Restrictions restrictionsMask = d->calculateRestrictionsMask( sm );
    Q_FOREACH( const Private::action_item & ai, d->actions )
        if ( ai.action )
            ai.action->setEnabled( ai.restrictions == ( ai.restrictions & restrictionsMask ) );
}

static bool all_secret_are_not_owner_trust_ultimate( const std::vector<Key> & keys ) {
    Q_FOREACH( const Key & key, keys )
        if ( key.hasSecret() && key.ownerTrust() == Key::Ultimate )
            return false;
    return true;
}

Command::Restrictions find_root_restrictions( const std::vector<Key> & keys ) {
    bool trusted = false, untrusted = false;
    Q_FOREACH( const Key & key, keys )
        if ( key.isRoot() )
            if ( key.userID(0).validity() == UserID::Ultimate )
                trusted = true;
            else
                untrusted = true;
        else
            return Command::NoRestriction;
    if ( trusted )
        if ( untrusted )
            return Command::NoRestriction;
        else
            return Command::MustBeTrustedRoot;
    else
        if ( untrusted )
            return Command::MustBeUntrustedRoot;
        else
            return Command::NoRestriction;
}

Command::Restrictions KeyListController::Private::calculateRestrictionsMask( const QItemSelectionModel * sm ) {
    if ( !sm )
        return 0;

    const KeyListModelInterface * const m = dynamic_cast<const KeyListModelInterface*>( sm->model() );
    if ( !m )
        return 0;

    const std::vector<Key> keys = m->keys( sm->selectedRows() );
    if ( keys.empty() )
        return 0;

    Command::Restrictions result = Command::NeedSelection;

    if ( keys.size() == 1 )
        result |= Command::OnlyOneKey;

    if ( kdtools::all( keys.begin(), keys.end(), boost::bind( &Key::hasSecret, _1 ) ) )
        result |= Command::NeedSecretKey;
    else if ( !kdtools::any( keys.begin(), keys.end(), boost::bind( &Key::hasSecret, _1 ) ) )
        result |= Command::MustNotBeSecretKey;

    if ( kdtools::all( keys.begin(), keys.end(), boost::bind( &Key::protocol, _1 ) == OpenPGP ) )
        result |= Command::MustBeOpenPGP;
    else if ( kdtools::all( keys.begin(), keys.end(), boost::bind( &Key::protocol, _1 ) == CMS ) )
        result |= Command::MustBeCMS;

    if ( all_secret_are_not_owner_trust_ultimate( keys ) )
        result |= Command::MayOnlyBeSecretKeyIfOwnerTrustIsNotYetUltimate;

    result |= find_root_restrictions( keys );

#ifndef _WIN32_WCE
    if ( const ReaderStatus * rs = ReaderStatus::instance() ) {
        if ( rs->anyCardHasNullPin() )
            result |= Command::AnyCardHasNullPin;
        if ( rs->anyCardCanLearnKeys() )
            result |= Command::AnyCardCanLearnKeys;
    }
#endif

    return result;
}

void KeyListController::Private::slotActionTriggered() {
    if ( const QObject * const s = q->sender() ) {
        const std::vector<action_item>::const_iterator it
            = kdtools::find_if( actions, boost::bind( &action_item::action, _1 ) == q->sender() );
        if ( it != actions.end() )
            if ( Command * const c = it->createCommand( this->currentView, q ) )
            {
                if ( parentWidget )
                    c->setParentWidget( parentWidget );
                c->start();
            }
            else
                kDebug() << "createCommand() == NULL for action(?) \""
                         << qPrintable( s->objectName() ) << "\"";
        else
            kDebug() << "I don't know anything about action(?) \"%s\"", qPrintable( s->objectName() );
    } else {
        kDebug() << "not called through a signal/slot connection (sender() == NULL)";
    }
}

int KeyListController::Private::toolTipOptions() const
{
    using namespace Kleo::Formatting;
    static const int validityFlags = Validity|Issuer|ExpiryDates|CertificateUsage;
    static const int ownerFlags = Subject|UserIDs|OwnerTrust;
    static const int detailsFlags = StorageLocation|CertificateType|SerialNumber|Fingerprint;

    const TooltipPreferences prefs;

    int flags = KeyID;
    flags |= prefs.showValidity() ? validityFlags : 0;
    flags |= prefs.showOwnerInformation() ? ownerFlags : 0;
    flags |= prefs.showCertificateDetails() ? detailsFlags : 0;
    return flags;
}

void KeyListController::updateConfig()
{
    const int opts = d->toolTipOptions();
    if ( d->flatModel )
        d->flatModel->setToolTipOptions( opts );
    if ( d->hierarchicalModel )
        d->hierarchicalModel->setToolTipOptions( opts );
}

#include "moc_keylistcontroller.cpp"
