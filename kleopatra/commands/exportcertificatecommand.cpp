/* -*- mode: c++; c-basic-offset:4 -*-
    exportcertificatecommand.cpp

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

#include "exportcertificatecommand.h"

#include <kleo/cryptobackend.h>
#include <kleo/cryptobackendfactory.h>
#include <kleo/exportjob.h>

#include <gpgme++/key.h>

#include <KLocale>
#include <KMessageBox>
#include <KSaveFile>

#include <QDataStream>
#include <QFileDialog>
#include <QPointer>
#include <QTextStream>

#include <boost/bind.hpp>
#include <algorithm>
#include <vector>
#include <cassert>

using namespace boost;
using namespace GpgME;
using namespace Kleo;

class ExportCertificateCommand::Private {
    friend class ::ExportCertificateCommand;
    ExportCertificateCommand * const q;
public:
    explicit Private( ExportCertificateCommand * qq );
    ~Private();
    void startSingleProtocolExport( GpgME::Protocol protocol, const std::vector<Key>& keys );
    void exportResult( const GpgME::Error&, const QByteArray& );
    void showError( const GpgME::Error& error );

private:
    bool textArmor;
    QWidget* parent;
    QString outfile;
    QPointer<ExportJob> exportJob;
    std::vector<GpgME::Key> certificates;
};


ExportCertificateCommand::Private::Private( ExportCertificateCommand * qq )
    : q( qq ), textArmor( true ), parent( 0 ), exportJob( 0 )
{
    
}

ExportCertificateCommand::Private::~Private() 
{
    if ( exportJob )
        exportJob->slotCancel();
}


ExportCertificateCommand::ExportCertificateCommand( KeyListController* controller )
  : Command( controller ), d( new Private( this ) )
{
    
}

ExportCertificateCommand::~ExportCertificateCommand() {}

void ExportCertificateCommand::doStart()
{
    std::vector<Key> keys = d->certificates;
    if ( keys.empty() )
        return;

    const std::vector<Key>::iterator firstCms = std::partition( keys.begin(), keys.end(), bind( &GpgME::Key::protocol, _1 ) != CMS );

    std::vector<Key> openpgp, cms; 
    std::copy( keys.begin(), firstCms, std::back_inserter( openpgp ) ); 
    std::copy( firstCms, keys.end(), std::back_inserter( cms ) ); 
    assert( !openpgp.empty() || !cms.empty() );
    if ( cms.empty() || openpgp.empty() )
    {
        d->startSingleProtocolExport( keys.front().protocol(), keys );
    }
    else
    {
        //handle mixed protocol case
        KMessageBox::error( d->parent, i18n( "Exporting both OpenPGP and S/MIME certificates at the same time is not supported yet. Please select only certificates of one type." ), i18n( "Certificate Export Failed" ) );
        emit canceled();
    }
}

void ExportCertificateCommand::Private::startSingleProtocolExport( GpgME::Protocol protocol, const std::vector<Key>& keys )
{
    assert( protocol != GpgME::UnknownProtocol );

    const QString out = QFileDialog::getSaveFileName( parent, i18n( "Export Certificates" ), QString(), protocol == GpgME::OpenPGP ? i18n( "OpenPGP Certificates (.asc)" ) : i18n( "S/MIME Certificates (.pem)" ) );
    if ( out.isNull() ) {
        emit q->canceled();
        return;
    }
    outfile = out;
    const CryptoBackend::Protocol* const backend = CryptoBackendFactory::instance()->protocol( protocol );
    assert( backend );
    std::auto_ptr<ExportJob> job( backend->publicKeyExportJob( /*armor=*/true ) );
    assert( job.get() );

    connect( job.get(), SIGNAL( result( GpgME::Error, QByteArray ) ),
             q, SLOT( exportResult( GpgME::Error, QByteArray ) ) );

    connect( job.get(), SIGNAL( progress( QString, int, int ) ),
             q, SIGNAL( progress( QString, int, int ) ) );

    QStringList fingerprints;
    Q_FOREACH ( const Key& i, keys )
        fingerprints << i.primaryFingerprint();

  const GpgME::Error err = job->start( fingerprints );
  if ( err ) {
      showError( err );
      emit q->finished();
      return;
  }
  emit q->info( i18n( "Exporting certificates..." ) );
  exportJob = job.release();
}

void ExportCertificateCommand::Private::showError( const GpgME::Error& err )
{
    assert( err );
    const QString msg = i18n("<qt><p>An error occurred while trying to export "
                             "the certificate:</p>"
                             "<p><b>%1</b></p></qt>",
                             QString::fromLocal8Bit( err.asString() ) );
    KMessageBox::error( parent, msg, i18n("Certificate Export Failed") );
}

void ExportCertificateCommand::doCancel()
{
    if ( d->exportJob )
        d->exportJob->slotCancel();
}

void ExportCertificateCommand::Private::exportResult( const GpgME::Error& err, const QByteArray& data )
{
    if ( err ) {
        showError( err );
        emit q->finished();
        return;
    }
    assert( !outfile.isNull() );
    KSaveFile savefile( outfile );
    //TODO: use KIO
    const QString writeErrorMsg = i18n( "Could not write to file %1.", outfile );
    const QString errorCaption = i18n( "Certificate Export Failed" );
    if ( !savefile.open() )
    {
        KMessageBox::error( parent, writeErrorMsg, errorCaption );
        emit q->finished();
        return;
    }
    if ( textArmor )
    {
        QTextStream out( &savefile );
        out << data;
    }
    else
    {
        QDataStream out( &savefile );
        out << data;
    }

    if ( !savefile.finalize() )
        KMessageBox::error( parent, writeErrorMsg, errorCaption );
    emit q->finished();
}

void ExportCertificateCommand::setCertificates( const std::vector<GpgME::Key>& certificates )
{
    d->certificates = certificates;
}

void ExportCertificateCommand::setParentWidget( QWidget* parent )
{
    d->parent = parent;
}

#include "moc_exportcertificatecommand.cpp"
