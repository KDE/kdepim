/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/assuancommandprivatebase_p.cpp

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

#include "assuancommandprivatebase_p.h"
#include "assuancommand.h"

#include "kleo-assuan.h"

#include <KSaveFile>

#ifndef KLEO_ONLY_UISERVER
#include <KFileDialog>
#endif
#include <KUrl>
#include <KLocale>
#include <KMessageBox>

#include <gpg-error.h>

#ifdef KLEO_ONLY_UISERVER
#include <QFileDialog>
#endif
#include <QVariant>

using namespace Kleo;

#define q get_q()

AssuanCommandPrivateBase::AssuanCommandPrivateBase()
{
}

AssuanCommandPrivateBase::~AssuanCommandPrivateBase()
{
}

int AssuanCommandPrivateBase::determineInputsAndProtocols( QString& reason )
{
    reason = QString();

    try {

        const GpgME::Protocol protocol = q->checkProtocol();

        if ( protocol != GpgME::UnknownProtocol ) {
            const Kleo::CryptoBackend::Protocol* backend = Kleo::CryptoBackendFactory::instance()->protocol( protocol == GpgME::OpenPGP ? "openpgp" : "smime" );
            if ( !backend )
                throw assuan_exception( q->makeError( GPG_ERR_INV_ARG ), i18n( "unsupported protocol" ) );

            for ( int i = 0; i < inputList.size(); ++i )
                inputList[i].backend = backend;
        } else { // no protocol given
            //TODO: kick off protocol detection for all files
            for ( int i = 0; i < inputList.size(); ++i )
                inputList[i].backend = Kleo::CryptoBackendFactory::instance()->protocol( "openpgp" );
        }
        return 0;

    } catch ( const assuan_exception & e ) {
        reason = QString::fromLocal8Bit( e.message().c_str() );
        return e.error_code();
    }
}

void AssuanCommandPrivateBase::writeToOutputDeviceOrAskForFileName( int id,  const QByteArray& stuff, const QString& _filename )
{
    QIODevice * outdevice = q->bulkOutputDevice( "OUTPUT", id );
#if 0 // KSaveFile seems broken on Windows, revert this when it's fixed (errors when writing to temp file), TODO, KDAB_PENDING
    KSaveFile file;
#endif
    QFile file;
    if ( !outdevice ) {
        QString filename = _filename;
        if ( !filename.isEmpty() && QFileInfo( filename ).isRelative() ) {
            // prepend the path to the input file
            filename.prepend( QFileInfo(q->bulkInputDeviceFileName( "INPUT", id )).absolutePath() + "/" );
            if ( QFileInfo(filename).exists() ) {
                const QString text = i18n("The target file: <br><b>%1</b><br> seems to already exist. Do you want to overwrite it?", filename );
                const QString caption  = i18n("Overwrite existing file?");
                if ( KMessageBox::questionYesNo( 0, text, caption ) == KMessageBox::No )
                    filename = QString();
            }
        }
        if ( filename.isEmpty() ) {
	    const QString inputFilename = q->bulkInputDeviceFileName( "INPUT", id );
            // no output specified, and no filename given, ask the user
#ifndef KLEO_ONLY_UISERVER
	    filename = KFileDialog::getSaveFileName( KUrl::fromPath( inputFilename ), QString(), 0, i18n("Please select a target file: %1",inputFilename ) );
#else
	    filename = QFileDialog::getSaveFileName( 0, QString(), inputFilename );
#endif
        }
        if ( filename.isEmpty() )
            throw assuan_exception( q->makeError( GPG_ERR_ASS_WRITE_ERROR ), "Output file selection canceled" ) ;
        // FIXME sanitize, percent-encode, etc. Needed with KSaveFile?
        file.setFileName( filename );
#if 0
	if ( !file.open() )
#endif
	if ( !file.open( QIODevice::WriteOnly ) )
            throw assuan_exception( q->makeError( GPG_ERR_ASS_WRITE_ERROR ), file.errorString().toStdString() ) ;

        outdevice = &file;
    }
    assert(outdevice);
    qint64 totalBytesWritten = 0;
    do {
        const qint64 bytesWritten = outdevice->write( stuff.constData() + totalBytesWritten, stuff.size() - totalBytesWritten );
        if ( bytesWritten < 0 )
            throw assuan_exception( q->makeError( GPG_ERR_ASS_WRITE_ERROR ), outdevice->errorString().toStdString() );
        totalBytesWritten += bytesWritten;
    } while ( totalBytesWritten < stuff.size() );
#if 0
    if ( !file.fileName().isEmpty() && !file.finalize() )
        throw assuan_exception( q->makeError( GPG_ERR_ASS_WRITE_ERROR ), file.errorString().toStdString() ) ;
#endif
    file.close();
}


#include "assuancommandprivatebase_p.moc"
