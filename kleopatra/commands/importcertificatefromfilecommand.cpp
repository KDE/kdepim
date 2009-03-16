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
#include "importcertificatescommand_p.h"

#include "utils/classify.h"

#include <kleo/cryptobackendfactory.h>
#include <kleo/importjob.h>

#include <gpgme++/global.h>
#include <gpgme++/importresult.h>

#include <KLocale>
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

class ImportCertificateFromFileCommand::Private : public ImportCertificatesCommand::Private {
    friend class ::ImportCertificateFromFileCommand;
    ImportCertificateFromFileCommand * q_func() const { return static_cast<ImportCertificateFromFileCommand*>( q ); }
public:
    explicit Private( ImportCertificateFromFileCommand * qq, KeyListController * c );
    ~Private();

    bool ensureHaveFile();
    GpgME::Protocol checkProtocol( const QByteArray& data, const QString& filename ) const;
    void importResult( const ImportResult & result );

private:
    QStringList files;
};

ImportCertificateFromFileCommand::Private * ImportCertificateFromFileCommand::d_func() { return static_cast<Private*>(d.get()); }
const ImportCertificateFromFileCommand::Private * ImportCertificateFromFileCommand::d_func() const { return static_cast<const Private*>(d.get()); }

ImportCertificateFromFileCommand::Private::Private( ImportCertificateFromFileCommand * qq, KeyListController * c )
    : ImportCertificatesCommand::Private( qq, c ),
      files()
{

}

ImportCertificateFromFileCommand::Private::~Private() {}


#define d d_func()
#define q q_func()


ImportCertificateFromFileCommand::ImportCertificateFromFileCommand( KeyListController * p )
    : ImportCertificatesCommand( new Private( this, p ) )
{

}

ImportCertificateFromFileCommand::ImportCertificateFromFileCommand( QAbstractItemView * v, KeyListController * p )
    : ImportCertificatesCommand( v, new Private( this, p ) )
{

}

ImportCertificateFromFileCommand::ImportCertificateFromFileCommand( const QStringList & files, KeyListController * p )
    : ImportCertificatesCommand( new Private( this, p ) )
{
    d->files = files;
}

ImportCertificateFromFileCommand::ImportCertificateFromFileCommand( const QStringList & files, QAbstractItemView * v, KeyListController * p )
    : ImportCertificatesCommand( v, new Private( this, p ) )
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
        d->information( i18n( "Importing more than one file at a time is not yet implemented." ),
                        i18n( "Too many files" ) );
        emit canceled();
        d->finished();
        return;
    }

    //TODO: use KIO here
    QFile in( d->files.front() );
    if ( !in.open( QIODevice::ReadOnly ) ) {
        d->error( i18n( "Could not open file %1 for reading", in.fileName() ), i18n( "Certificate Import Failed" ) );
        d->finished();
        return;
    }
    const GpgME::Protocol protocol = findProtocol( d->files.front() );
    if ( protocol == GpgME::UnknownProtocol ) { //TODO: might use exceptions here
        d->error( i18n( "Could not determine certificate type of %1.", d->files.front() ), i18n( "Certificate Import Failed" ) );
        d->finished();
        return;
    }
    d->startImport( protocol, in.readAll(), d->files.front() );
}

static QStringList get_file_name( QWidget * parent ) {
    const QString certificateFilter = i18n("Certificates") + " (*.asc *.cer *.cert *.crt *.der *.pem *.gpg *.p7c *.p12 *.pfx *.pgp)";
    const QString anyFilesFilter = i18n("Any files") + " (*)";
    QString previousDir;
    if ( const KSharedConfig::Ptr config = KGlobal::config() ) {
        const KConfigGroup group( config, "Import Certificate" );
        previousDir = group.readPathEntry( "last-open-file-directory", QDir::homePath() );
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
        files = get_file_name( parentWidgetOrView() );
    return !files.empty();
}

void ImportCertificateFromFileCommand::Private::importResult( const GpgME::ImportResult& result )
{
    if ( result.error().code() ) {
        setImportResultProxyModel( result );
        if ( result.error().isCanceled() )
            emit q->canceled();
        else
            showError( result.error(), files.front() );
    } else {
        showDetails( result, files.front() );
    }

    finished();
}


#undef d
#undef q

#include "moc_importcertificatefromfilecommand.cpp"


