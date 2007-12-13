/* -*- mode: c++; c-basic-offset:4 -*-
    importcertificatecommand.cpp

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

#include "importcertificatecommand.h"
#include "uiserver/classify.h"

#include <kleo/cryptobackendfactory.h>
#include <kleo/importjob.h>

#include <gpgme++/global.h>
#include <gpgme++/importresult.h>

#include <KLocale>
#include <KMessageBox>
#include <KJob>

#include <QByteArray>
#include <QFile>
#include <QFileDialog>
#include <QPointer>
#include <QString>
#include <QWidget>

#include <memory>
#include <cassert>

using namespace GpgME;
using namespace Kleo;

class ImportCertificateCommand::Private {
    friend class ::ImportCertificateCommand;
    ImportCertificateCommand * const q;
public:
    explicit Private( ImportCertificateCommand * qq );
    ~Private();

    bool ensureHaveFile();
    void startImport( const QByteArray& data );
    void importResult( const GpgME::ImportResult& );
    GpgME::Protocol checkProtocol( const QByteArray& data, const QString& filename ) const;
    void showError( const GpgME::Error& error );
    void showDetails( const ImportResult& result );

private:
    QWidget* parent;
    QPointer<ImportJob> importJob;
    QString filename;
};


ImportCertificateCommand::Private::Private( ImportCertificateCommand * qq )
    : q( qq ), parent( 0 ), importJob( 0 )
{
    
}

ImportCertificateCommand::Private::~Private() 
{
    if ( importJob )
        importJob->slotCancel();
}



ImportCertificateCommand::ImportCertificateCommand( KeyListController * parent )
  : Command( parent ), d( new Private( this ) )
{
    
}

ImportCertificateCommand::~ImportCertificateCommand() {}

void ImportCertificateCommand::setParentWidget( QWidget* parent )
{
    d->parent = parent;
}

QString ImportCertificateCommand::fileName() const
{
    return d->filename;
}

void ImportCertificateCommand::setFileName( const QString& fileName )
{
    d->filename = fileName;
}

void ImportCertificateCommand::doStart()
{
    if ( !d->ensureHaveFile() )
    {
        emit canceled();
        return;
    }
    //TODO: use KIO here
    QFile in( d->filename );
    if ( !in.open( QIODevice::ReadOnly ) )
    {
        KMessageBox::error( d->parent, i18n( "Could not open file %1 for reading", d->filename ), i18n( "Certificate Import Failed" ) );
        
        emit finished(); //TODO: correct signal??
        return;
    }
    d->startImport( in.readAll() ); 
    emit info( i18n( "Importing certificate..." ) );
}

bool ImportCertificateCommand::Private::ensureHaveFile()
{
   if ( filename.isNull() )
        filename = QFileDialog::getOpenFileName( parent, i18n( "Select Certificate File" ), QString(), i18n( "Certificates (*.asc *.pem *.der *.p7c *.p12)" )  );
   return !filename.isNull();
}

void ImportCertificateCommand::Private::showDetails( const ImportResult& res )
{
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
        
        KMessageBox::information( parent,
                                  i18n( "<qt><p>Detailed results of importing %1:</p>"
                                        "<table>%2</table></qt>" ,
                                        filename, lines.join( QString() ) ),
                                  i18n( "Certificate Import Result" ) );
}

void ImportCertificateCommand::Private::showError( const GpgME::Error& err )
{
  assert( err );
  const QString msg = i18n( "<qt><p>An error occurred while trying "
			    "to import the certificate %1:</p>"
			    "<p><b>%2</b></p></qt>",
                            filename,
                            QString::fromLocal8Bit( err.asString() ) );
  KMessageBox::error( parent, msg, i18n( "Certificate Import Failed" ) );
}

void ImportCertificateCommand::Private::importResult( const GpgME::ImportResult& result )
{
    if ( result.error().isCanceled() ) {
        emit q->canceled();
        return;
    }

    if ( result.error() ) { 
        showError( result.error() );
        emit q->finished();
        return;
    }

    showDetails( result );
    emit q->finished();
}

void ImportCertificateCommand::Private::startImport( const QByteArray& data )
{
    const GpgME::Protocol protocol = findProtocol( filename );
    if ( protocol == GpgME::UnknownProtocol ) //TODO: might use exceptions here
    {
        KMessageBox::error( parent, i18n( "Could not determine certificate type.", filename ), i18n( "Certificate Import Failed" ) );
        emit q->canceled(); //TODO: use correct signal
        return;
    }

    std::auto_ptr<ImportJob> job( CryptoBackendFactory::instance()->protocol( protocol )->importJob() );
    assert( job.get() );
    connect( job.get(), SIGNAL(result( GpgME::ImportResult ) ),
             q, SLOT( importResult( GpgME::ImportResult ) ) );
    connect( job.get(), SIGNAL( process( QString, int, int ) ), 
             q, SIGNAL( process( QString, int, int ) ) );
    const GpgME::Error err = job->start( data );

    importJob = job.release();
}


void ImportCertificateCommand::doCancel()
{
    if ( d->importJob )
        d->importJob->slotCancel();
}

#include "moc_importcertificatecommand.cpp"

