/* -*- mode: c++; c-basic-offset:4 -*-
    commands/lookupcertificatescommand.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2008 Klarälvdalens Datakonsult AB

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

#include "lookupcertificatescommand.h"

#include "importcertificatescommand_p.h"

#include "detailscommand.h"

#include <dialogs/lookupcertificatesdialog.h>

#include <utils/stl_util.h>
#include <utils/formatting.h>

#include <kleo/downloadjob.h>
#include <kleo/importjob.h>
#include <kleo/keylistjob.h>
#include <kleo/cryptobackendfactory.h>
#include <kleo/cryptobackend.h>
#include <kleo/cryptoconfig.h>

#include <gpgme++/key.h>
#include <gpgme++/keylistresult.h>

#include <KLocale>
#include <KMessageBox>
#include <kdebug.h>

#include <QBuffer>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

#include <vector>
#include <map>
#include <algorithm>
#include <cassert>

using namespace Kleo;
using namespace Kleo::Commands;
using namespace Kleo::Dialogs;
using namespace GpgME;
using namespace boost;

class LookupCertificatesCommand::Private : public ImportCertificatesCommand::Private {
    friend class ::Kleo::Commands::LookupCertificatesCommand;
    LookupCertificatesCommand * q_func() const { return static_cast<LookupCertificatesCommand*>( q ); }
public:
    explicit Private( LookupCertificatesCommand * qq, KeyListController * c );
    ~Private();

    void init();

private:
    void slotSearchTextChanged( const QString & str );
    void slotNextKey( const Key & key ) {
        keyListing.keys.push_back( key );
    }
    void slotKeyListResult( const KeyListResult & result );
    void slotImportRequested( const std::vector<Key> & keys );
    void slotOpenPGPDownloadResult( const Error & err, const QByteArray & data );
    void slotCMSDownloadResult( const Error & err, const QByteArray & data );
    void slotDetailsRequested( const Key & key );
    void slotSaveAsRequested( const std::vector<Key> & keys );
    void slotDialogRejected() {
        canceled();
    }

private:
    using ImportCertificatesCommand::Private::showError;
    void showError( QWidget * parent, const KeyListResult & result );
    void showResult( QWidget * parent, const KeyListResult & result );
    void createDialog();
    KeyListJob * createKeyListJob( GpgME::Protocol proto ) const {
        const CryptoBackend::Protocol * const cbp = CryptoBackendFactory::instance()->protocol( proto );
        return cbp ? cbp->keyListJob( true ) : 0 ;
    }
    DownloadJob * createDownloadJob( GpgME::Protocol proto ) const {
        const CryptoBackend::Protocol * const cbp = CryptoBackendFactory::instance()->protocol( proto );
        return cbp ? cbp->downloadJob() : 0 ;
    }
    void startKeyListJob( GpgME::Protocol proto, const QString & str );
    void startDownloadJob( const Key & key );
    void checkForDownloadFinished();
    bool checkConfig() const;

    QWidget * dialogOrParentWidgetOrView() const { if ( dialog ) return dialog; else return parentWidgetOrView();
}

private:
    QPointer<LookupCertificatesDialog> dialog;
    struct KeyListingVariables {
        QPointer<KeyListJob> cms, openpgp;
        KeyListResult result;
        std::vector<Key> keys;

        void reset() { *this = KeyListingVariables(); }
    } keyListing;
    struct DownloadVariables {
        Key key;
        QPointer<DownloadJob> job;
        Error error;
    };
    std::vector<DownloadVariables> downloads;
    QByteArray openPGPDownloadData, cmsDownloadData;
};


LookupCertificatesCommand::Private * LookupCertificatesCommand::d_func() { return static_cast<Private*>( d.get() ); }
const LookupCertificatesCommand::Private * LookupCertificatesCommand::d_func() const { return static_cast<const Private*>( d.get() ); }

#define d d_func()
#define q q_func()

LookupCertificatesCommand::Private::Private( LookupCertificatesCommand * qq, KeyListController * c )
    : ImportCertificatesCommand::Private( qq, c ),
      dialog()
{

}

LookupCertificatesCommand::Private::~Private() {
    kDebug();
    delete dialog;
}

LookupCertificatesCommand::LookupCertificatesCommand( KeyListController * c )
    : ImportCertificatesCommand( new Private( this, c ) )
{
    d->init();
}

LookupCertificatesCommand::LookupCertificatesCommand( QAbstractItemView * v, KeyListController * c )
    : ImportCertificatesCommand( v, new Private( this, c ) )
{
    d->init();
}

void LookupCertificatesCommand::Private::init() {

}

LookupCertificatesCommand::~LookupCertificatesCommand() { kDebug(); }


void LookupCertificatesCommand::doStart() {

    if ( !d->checkConfig() ) {
        d->finished();
        return;
    }

    d->createDialog();
    assert( d->dialog );

    d->dialog->setPassive( false );
    d->dialog->show();

}

void LookupCertificatesCommand::Private::createDialog() {
    if ( dialog )
        return;
    dialog = new LookupCertificatesDialog( parentWidgetOrView() );
    dialog->setAttribute( Qt::WA_DeleteOnClose );
    connect( dialog, SIGNAL(searchTextChanged(QString)),
             q, SLOT(slotSearchTextChanged(QString)) );
    connect( dialog, SIGNAL(saveAsRequested(std::vector<GpgME::Key>)),
             q, SLOT(slotSaveAsRequested(std::vector<GpgME::Key>)) );
    connect( dialog, SIGNAL(importRequested(std::vector<GpgME::Key>)),
             q, SLOT(slotImportRequested(std::vector<GpgME::Key>)) );
    connect( dialog, SIGNAL(detailsRequested(GpgME::Key)),
             q, SLOT(slotDetailsRequested(GpgME::Key)) );
    connect( dialog, SIGNAL(rejected()),
             q, SLOT(slotDialogRejected()) );
}

void LookupCertificatesCommand::Private::slotSearchTextChanged( const QString & str ) {
    // pressing return might trigger both search and dialog destruction (search focused and default key set)
    // On Windows, the dialog is then destroyed before this slot is called
    if ( dialog ) { //thus test
        dialog->setPassive( true );
        dialog->setCertificates( std::vector<Key>() );
    }

    startKeyListJob( CMS,     str );
    startKeyListJob( OpenPGP, str );

}

void LookupCertificatesCommand::Private::startKeyListJob( GpgME::Protocol proto, const QString & str ) {
    KeyListJob * const klj = createKeyListJob( proto );
    if ( !klj )
        return;
    connect( klj, SIGNAL(result(GpgME::KeyListResult)),
             q, SLOT(slotKeyListResult(GpgME::KeyListResult)) );
    connect( klj, SIGNAL(nextKey(GpgME::Key)),
             q, SLOT(slotNextKey(GpgME::Key)) );
    if ( const Error err = klj->start( QStringList( str ) ) )
        keyListing.result.mergeWith( KeyListResult( err ) );
    else
        if ( proto == CMS )
            keyListing.cms     = klj;
        else
            keyListing.openpgp = klj;
}

void LookupCertificatesCommand::Private::slotKeyListResult( const KeyListResult & r ) {

    if ( q->sender() == keyListing.cms )
        keyListing.cms = 0;
    else if ( q->sender() == keyListing.openpgp )
        keyListing.openpgp = 0;
    else
        kDebug() << "unknown sender()" << q->sender();

    keyListing.result.mergeWith( r );
    if ( keyListing.cms || keyListing.openpgp ) // still waiting for jobs to complete
        return;

    if ( keyListing.result.error() && !keyListing.result.error().isCanceled() )
        showError( dialog, keyListing.result );

    if ( keyListing.result.isTruncated() )
        showResult( dialog, keyListing.result );

    if ( dialog ) {
        dialog->setCertificates( keyListing.keys );
        dialog->setPassive( false );
    } else {
        finished();
    }


    keyListing.reset();
}

void LookupCertificatesCommand::Private::slotImportRequested( const std::vector<Key> & keys ) {
    dialog = 0;

    assert( !keys.empty() );
    kdtools::for_each( keys, bind( &Private::startDownloadJob, this, _1 ) );
}

static shared_ptr<QBuffer> make_open_qbuffer() {
    const shared_ptr<QBuffer> buffer( new QBuffer );
    if ( !buffer->open( QIODevice::WriteOnly ) )
        kFatal() << "QBuffer::open failed";
    return buffer;
}

void LookupCertificatesCommand::Private::startDownloadJob( const Key & key ) {
    if ( key.isNull() )
        return;

    QStringList fprs;
    const char * const fpr  = key.primaryFingerprint();
    const char * const kid  = key.keyID();
    if ( fpr && *fpr )
        fprs.push_back( QString::fromLatin1( fpr ) );
    else if ( kid && *kid )
        fprs.push_back( QString::fromLatin1( kid ) );
    else
        return;

    DownloadJob * const dlj = createDownloadJob( key.protocol() );
    if ( !dlj )
        return;
    connect( dlj, SIGNAL(result(GpgME::Error,QByteArray)),
             q, key.protocol() == CMS
             ? SLOT(slotCMSDownloadResult(GpgME::Error,QByteArray))
             : SLOT(slotOpenPGPDownloadResult(GpgME::Error,QByteArray)) );
    DownloadVariables var;
    var.key = key;

    if ( const Error err = dlj->start( fprs ) )
        var.error = err;
    else
        var.job = dlj;
    downloads.push_back( var );
}

namespace {
    template <typename T>
    QStringList filter_and_format_successful_downloads( const T & t ) {
        QStringList result;

        return result;
    }

    template <typename T>
    QByteArray combined_nonfailed_data( GpgME::Protocol proto, const T & t ) {
        QByteArray result;
        Q_FOREACH( const typename T::value_type & v, t )
            if ( !v.error.code() && v.data && v.key.protocol() == proto )
                result.append( v.data->data() );
        return result;
    }
}

void LookupCertificatesCommand::Private::slotOpenPGPDownloadResult( const GpgME::Error & err, const QByteArray & keyData ) {
    const std::vector<DownloadVariables>::iterator it =
        std::find_if( downloads.begin(), downloads.end(),
                      bind( &DownloadVariables::job, _1 ) == q->sender() );
    assert( it != downloads.end() );

    openPGPDownloadData.append( keyData );

    it->job = 0;
    it->error = err;

    checkForDownloadFinished();
}

void LookupCertificatesCommand::Private::slotCMSDownloadResult( const GpgME::Error & err, const QByteArray & keyData ) {
    const std::vector<DownloadVariables>::iterator it =
        std::find_if( downloads.begin(), downloads.end(),
                      bind( &DownloadVariables::job, _1 ) == q->sender() );
    assert( it != downloads.end() );

    cmsDownloadData.append( keyData );

    it->job = 0;
    it->error = err;

    checkForDownloadFinished();
}

void LookupCertificatesCommand::Private::checkForDownloadFinished() {

    if ( kdtools::any( downloads, mem_fn( &DownloadVariables::job ) ) )
        return; // still jobs to end

    if ( kdtools::all( downloads, mem_fn( &DownloadVariables::error ) ) ) {
        KMessageBox::information( dialogOrParentWidgetOrView(),
                                  downloads.size() == 1 ?
                                  i18n( "Download of certificate %1 failed. Error message: %2",
                                        Formatting::formatForComboBox( downloads.front().key ),
                                        QString::fromLocal8Bit( downloads.front().error.asString() ) ) :
                                  i18n( "All certificate downloads failed. Sample error message: %1",
                                        QString::fromLocal8Bit( downloads.front().error.asString() ) ),
                                  i18n( "Certificate Download Failed" ) );
        finished();
        return;
    } else if ( kdtools::any( downloads, mem_fn( &DownloadVariables::error ) ) &&
                KMessageBox::questionYesNoList( dialogOrParentWidgetOrView(),
                                                i18n( "Some certificates failed to download. "
                                                      "Do you want to proceed with importing the following successful downloads?" ),
                                                filter_and_format_successful_downloads( downloads ),
                                                i18n( "Certificate Download Failed" ) )
                == KMessageBox::No )
    {
        finished();
        return;
    }

    const QByteArray cms = cmsDownloadData;//combined_nonfailed_data( CMS,     downloads );
    const QByteArray pgp = openPGPDownloadData;//combined_nonfailed_data( OpenPGP, downloads );

    if ( !cms.isEmpty() )
        startImport( CMS,     cms );
    if ( !pgp.isEmpty() )
        startImport( OpenPGP, pgp );
}

void LookupCertificatesCommand::Private::slotSaveAsRequested( const std::vector<Key> & keys ) {
    kDebug() << "not implemented";
}

void LookupCertificatesCommand::Private::slotDetailsRequested( const Key & key ) {
    Command * const cmd = new DetailsCommand( key, view(), controller() );
    cmd->setParentWidget( dialogOrParentWidgetOrView() );
    cmd->start();
}

void LookupCertificatesCommand::doCancel() {
    ImportCertificatesCommand::doCancel();
    if ( QDialog * const dlg = d->dialog ) {
        d->dialog = 0;
        dlg->close();
    }
}

void LookupCertificatesCommand::Private::showError( QWidget * parent, const KeyListResult & result ) {
    if ( !result.error() )
        return;
    KMessageBox::information( parent, i18n( "Failed to search on keyserver. The error returned was:\n%1",
                                            QString::fromLocal8Bit( result.error().asString() ) ) );
}

void LookupCertificatesCommand::Private::showResult( QWidget * parent, const KeyListResult & result ) {
    if ( result.isTruncated() )
        KMessageBox::information( parent,
                                  i18nc("@info",
                                        "<para>The query result has been truncated.</para>"
                                        "<para>Either the local or a remote limit on "
                                        "the maximum number of returned hits has "
                                        "been exceeded.</para>"
                                        "<para>You can try to increase the local limit "
                                        "in the configuration dialog, but if one "
                                        "of the configured servers is the limiting "
                                        "factor, you have to refine your search.</para>"),
                                  i18nc("@title", "Result Truncated"),
                                  "lookup-certificates-truncated-result" );
}

static bool haveOpenPGPKeyserverConfigured() {
    const Kleo::CryptoConfig * const config = Kleo::CryptoBackendFactory::instance()->config();
    if ( !config )
        return false;
    const Kleo::CryptoConfigEntry * const entry = config->entry( "gpg", "Keyserver", "keyserver" );
    return entry && !entry->stringValue().isEmpty();
}


static bool haveX509DirectoryServerConfigured() {
    const Kleo::CryptoConfig * const config = Kleo::CryptoBackendFactory::instance()->config();
    if ( !config )
        return false;
    const Kleo::CryptoConfigEntry * entry = config->entry( "dirmngr", "LDAP", "LDAP Server" );
    bool entriesExist = entry && !entry->urlValueList().empty();
    entry = config->entry( "gpgsm", "Configuration", "keyserver" );
    entriesExist |= entry && !entry->stringValueList().empty();
    return entriesExist;
}


bool LookupCertificatesCommand::Private::checkConfig() const {
    const bool ok = haveOpenPGPKeyserverConfigured() || haveX509DirectoryServerConfigured();
    if ( !ok )
        KMessageBox::information( parentWidgetOrView(), i18nc("@info",
                                           "<para>You do not have any directory servers configured.</para>"
                                           "<para>You need to configure at least one directory server to "
                                           "search on one.</para>"
                                           "<para>You can configure directory servers here: "
                                           "<interface>Settings->Configure Kleopatra</interface>.</para>"),
                                  i18nc("@title", "No Directory Servers Configured") );
    return ok;
}

#undef d
#undef q

#include "moc_lookupcertificatescommand.cpp"
