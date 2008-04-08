/* -*- mode: c++; c-basic-offset:4 -*-
    importcertificatefromfilecommand.cpp

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

#include "importcertificatefromfilecommand.h"
#include "command_p.h"

#include "utils/classify.h"

#include <kleo/cryptobackendfactory.h>
#include <kleo/importjob.h>

#include <gpgme++/global.h>
#include <gpgme++/importresult.h>

#include <KLocale>
#include <KMessageBox>
#include <KConfigGroup>

#include <QByteArray>
#include <QFile>
#include <QFileDialog>
#include <QPointer>
#include <QString>
#include <QWidget>
#include <QFileInfo>

#include <memory>
#include <cassert>

using namespace GpgME;
using namespace Kleo;

class ImportCertificateFromFileCommand::Private : public Command::Private {
    friend class ::ImportCertificateFromFileCommand;
    ImportCertificateFromFileCommand * q_func() const { return static_cast<ImportCertificateFromFileCommand*>( q ); }
public:
    explicit Private( ImportCertificateFromFileCommand * qq, KeyListController * c );
    ~Private();

    bool ensureHaveFile();
    void startImport( const QByteArray& data );
    void importResult( const GpgME::ImportResult& );
    GpgME::Protocol checkProtocol( const QByteArray& data, const QString& filename ) const;
    void showError( const GpgME::Error& error );
    void showDetails( const ImportResult& result );

private:
    QPointer<ImportJob> importJob;
    QStringList files;
};

ImportCertificateFromFileCommand::Private * ImportCertificateFromFileCommand::d_func() { return static_cast<Private*>(d.get()); }
const ImportCertificateFromFileCommand::Private * ImportCertificateFromFileCommand::d_func() const { return static_cast<const Private*>(d.get()); }

ImportCertificateFromFileCommand::Private::Private( ImportCertificateFromFileCommand * qq, KeyListController * c )
    : Command::Private( qq, c ), importJob( 0 )
{
    
}

ImportCertificateFromFileCommand::Private::~Private() {}


#define d d_func()
#define q q_func()


ImportCertificateFromFileCommand::ImportCertificateFromFileCommand( KeyListController * p )
    : Command( new Private( this, p ) )
{
    
}

ImportCertificateFromFileCommand::ImportCertificateFromFileCommand( QAbstractItemView * v, KeyListController * p )
    : Command( v, new Private( this, p ) )
{
    
}

ImportCertificateFromFileCommand::ImportCertificateFromFileCommand( const QStringList & files, KeyListController * p )
    : Command( new Private( this, p ) )
{
    d->files = files;
}

ImportCertificateFromFileCommand::ImportCertificateFromFileCommand( const QStringList & files, QAbstractItemView * v, KeyListController * p )
    : Command( v, new Private( this, p ) )
{
    d->files = files;
}

ImportCertificateFromFileCommand::~ImportCertificateFromFileCommand() {}

void ImportCertificateFromFileCommand::setFiles( const QStringList & files )
{
    d->files = files;
}

void ImportCertificateFromFileCommand::doStart()
{
    if ( !d->ensureHaveFile() ) {
        emit canceled();
        d->finished();
        return;
    }
    // TODO: allow multiple files
    if ( d->files.size() > 1 ) {
        KMessageBox::information( d->view(), i18n( "Importing more than one file at a time is not yet implemented." ),
                                  i18n( "Too many files" ) );
        emit canceled();
        d->finished();
        return;
    }
        
    //TODO: use KIO here
    QFile in( d->files.front() );
    if ( !in.open( QIODevice::ReadOnly ) ) {
        KMessageBox::error( d->view(), i18n( "Could not open file %1 for reading", in.fileName() ), i18n( "Certificate Import Failed" ) );
        d->finished();
        return;
    }
    d->startImport( in.readAll() ); 
    emit info( i18n( "Importing certificate..." ) );
}

static QStringList get_file_name( QWidget * parent ) {
    const QString certificateFilter = i18n("Certificates (*.asc *.pem *.der *.p7c *.p12)");
    const QString anyFilesFilter = i18n("Any files (*)" );
    QString previousDir;
    if ( const KSharedConfig::Ptr config = KGlobal::config() ) {
        const KConfigGroup group( config, "Import Certificate" );
        previousDir = group.readPathEntry( "last-open-file-directory", QString() );
    }
    const QString fn = QFileDialog::getOpenFileName( parent, i18n( "Select Certificate File" ), previousDir, certificateFilter + ";;" + anyFilesFilter );
    if ( fn.isEmpty() )
        return QStringList();
    if ( const KSharedConfig::Ptr config = KGlobal::config() ) {
        KConfigGroup group( config, "Import Certificate" );
        group.writePathEntry( "last-open-file-directory", QFileInfo( fn ).path() );
    }
    return QStringList( fn );
}

bool ImportCertificateFromFileCommand::Private::ensureHaveFile()
{
    if ( files.empty() )
        files = get_file_name( view() );
    return !files.empty();
}

void ImportCertificateFromFileCommand::Private::showDetails( const ImportResult& res )
{
    // ### TODO: make a keylisting over Import::fingerprints(), then
    // ### highlight imported certificates in view(), or maybe in a new tab?

    const KLocalizedString normalLine = ki18n("<tr><td align=\"right\">%1</td><td>%2</td></tr>");
    const KLocalizedString boldLine = ki18n("<tr><td align=\"right\"><b>%1</b></td><td>%2</td></tr>");

    QStringList lines;
    lines.push_back( normalLine.subs( i18n("Total number processed:") )
                     .subs( res.numConsidered() ).toString() );
    lines.push_back( normalLine.subs( i18n("Imported:") )
                     .subs( res.numImported() ).toString() );
    if ( res.newSignatures() )
        lines.push_back( normalLine.subs( i18n("New signatures:") )
                         .subs( res.newSignatures() ).toString() );
    if ( res.newUserIDs() )
        lines.push_back( normalLine.subs( i18n("New user IDs:") )
                         .subs( res.newUserIDs() ).toString() );
    if ( res.numKeysWithoutUserID() )
        lines.push_back( normalLine.subs( i18n("Keys without user IDs:") )
                         .subs( res.numKeysWithoutUserID() ).toString() );
    if ( res.newSubkeys() )
        lines.push_back( normalLine.subs( i18n("New subkeys:") )
                         .subs( res.newSubkeys() ).toString() );
    if ( res.newRevocations() )
        lines.push_back( boldLine.subs( i18n("Newly revoked:") )
                         .subs( res.newRevocations() ).toString() );
    if ( res.notImported() )
        lines.push_back( boldLine.subs( i18n("Not imported:") )
                         .subs( res.notImported() ).toString() );
    if ( res.numUnchanged() )
        lines.push_back( normalLine.subs( i18n("Unchanged:") )
                         .subs( res.numUnchanged() ).toString() );
    if ( res.numSecretKeysConsidered() )
        lines.push_back( normalLine.subs( i18n("Secret keys processed:") )
                         .subs( res.numSecretKeysConsidered() ).toString() );
    if ( res.numSecretKeysImported() )
        lines.push_back( normalLine.subs( i18n("Secret keys imported:") )
                         .subs( res.numSecretKeysImported() ).toString() );
    if ( res.numSecretKeysConsidered() - res.numSecretKeysImported() - res.numSecretKeysUnchanged() > 0 )
        lines.push_back( boldLine.subs( i18n("Secret keys <em>not</em> imported:") )
                         .subs(  res.numSecretKeysConsidered()
                                 - res.numSecretKeysImported()
                                 - res.numSecretKeysUnchanged() ).toString() );
    if ( res.numSecretKeysUnchanged() )
        lines.push_back( normalLine.subs( i18n("Secret keys unchanged:") )
                         .subs( res.numSecretKeysUnchanged() ).toString() );
    
    KMessageBox::information( view(),
                              i18n( "<qt><p>Detailed results of importing %1:</p>"
                                    "<table>%2</table></qt>" ,
                                    files.front(), lines.join( QString() ) ),
                              i18n( "Certificate Import Result" ) );
}

void ImportCertificateFromFileCommand::Private::showError( const GpgME::Error& err )
{
    assert( err );
    assert( !err.isCanceled() );
    const QString msg = i18n( "<qt><p>An error occurred while trying "
                              "to import the certificate %1:</p>"
                              "<p><b>%2</b></p></qt>",
                              files.front(),
                              QString::fromLocal8Bit( err.asString() ) );
    KMessageBox::error( view(), msg, i18n( "Certificate Import Failed" ) );
}

void ImportCertificateFromFileCommand::Private::importResult( const GpgME::ImportResult& result )
{
    if ( result.error().code() ) {
        if ( result.error().isCanceled() )
            emit q->canceled();
        else
            showError( result.error() );
    } else {
        showDetails( result );
    }

    finished();
}

void ImportCertificateFromFileCommand::Private::startImport( const QByteArray& data )
{
    const GpgME::Protocol protocol = findProtocol( files.front() );
    if ( protocol == GpgME::UnknownProtocol ) { //TODO: might use exceptions here
        KMessageBox::error( view(), i18n( "Could not determine certificate type of %1.", files.front() ), i18n( "Certificate Import Failed" ) );
        finished();
        return;
    }

    std::auto_ptr<ImportJob> job( CryptoBackendFactory::instance()->protocol( protocol )->importJob() );
    assert( job.get() );
    connect( job.get(), SIGNAL(result(GpgME::ImportResult)),
             q, SLOT(importResult(GpgME::ImportResult)) );
    connect( job.get(), SIGNAL(progress(QString,int,int)), 
             q, SIGNAL(progress(QString,int,int)) );
    if ( const GpgME::Error err = job->start( data ) ) {
        showError( err );
        finished();
    } else if ( err.isCanceled() ) {
        emit q->canceled();
        finished();
    } else {
        importJob = job.release();
    }
}


void ImportCertificateFromFileCommand::doCancel()
{
    if ( d->importJob )
        d->importJob->slotCancel();
}

#undef d
#undef q

#include "moc_importcertificatefromfilecommand.cpp"

