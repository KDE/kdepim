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
#include "utils/filedialog.h"

#include <kleo/cryptobackendfactory.h>
#include <kleo/importjob.h>

#include <gpgme++/global.h>
#include <gpgme++/importresult.h>

#include <KLocalizedString>
#include <KConfigGroup>

#include <QFile>
#include <QString>
#include <QWidget>
#include <QFileInfo>
#include <QDir>

#include <KSharedConfig>

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

    //TODO: use KIO here
    d->setWaitForMoreJobs( true );
    Q_FOREACH( const QString & fn, d->files ) {
        QFile in( fn );
        if ( !in.open( QIODevice::ReadOnly ) ) {
            d->error( i18n( "Could not open file %1 for reading: %2", in.fileName(), in.errorString() ), i18n( "Certificate Import Failed" ) );
            d->importResult( ImportResult(), fn );
            continue;
        }
        const GpgME::Protocol protocol = findProtocol( fn );
        if ( protocol == GpgME::UnknownProtocol ) { //TODO: might use exceptions here
            d->error( i18n( "Could not determine certificate type of %1.", in.fileName() ), i18n( "Certificate Import Failed" ) );
            d->importResult( ImportResult(), fn );
            continue;
        }
        d->startImport( protocol, in.readAll(), fn );
    }
    d->setWaitForMoreJobs( false );
}

static QStringList get_file_name( QWidget * parent ) {
    const QString certificateFilter = i18n("Certificates") + QLatin1String(" (*.asc *.cer *.cert *.crt *.der *.pem *.gpg *.p7c *.p12 *.pfx *.pgp)");
    const QString anyFilesFilter = i18n("Any files") + QLatin1String(" (*)");
    QString previousDir;
    if ( const KSharedConfig::Ptr config = KGlobal::config() ) {
        const KConfigGroup group( config, "Import Certificate" );
        previousDir = group.readPathEntry( "last-open-file-directory", QDir::homePath() );
    }
    const QStringList files = Kleo::FileDialog::getOpenFileNames( parent, i18n( "Select Certificate File" ), previousDir, certificateFilter + QLatin1String(";;") + anyFilesFilter );
    if ( !files.empty() )
        if ( const KSharedConfig::Ptr config = KGlobal::config() ) {
            KConfigGroup group( config, "Import Certificate" );
            group.writePathEntry( "last-open-file-directory", QFileInfo( files.front() ).path() );
        }
    return files;
}

bool ImportCertificateFromFileCommand::Private::ensureHaveFile()
{
    if ( files.empty() )
        files = get_file_name( parentWidgetOrView() );
    return !files.empty();
}

#undef d
#undef q



