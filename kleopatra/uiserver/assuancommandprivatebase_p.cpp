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
#include <KFileDialog>
#include <KUrl>
#include <KLocale>

#include <gpg-error.h>

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

    const QString protocol = q->option( "protocol" ).toString();

    if ( protocol.isEmpty() )
    {
        Q_FOREACH ( const Input input, inputList )
        {
            if ( !input.isFileInputOnly() )
            {
                reason = "--protocol option is required when passing file descriptors";
                return GPG_ERR_GENERAL;
            }
        }
    }

    if ( !protocol.isEmpty() )
    {
        const Kleo::CryptoBackend::Protocol* backend = Kleo::CryptoBackendFactory::instance()->protocol( protocol.toAscii().data() );
        if ( !backend )
        {
            reason = QString( "Unknown protocol: %1" ).arg( protocol );
            return GPG_ERR_GENERAL;
        }

        for ( int i = 0; i < inputList.size(); ++i )
            inputList[i].backend = backend;
    }
    else // no protocol given
    {
        //TODO: kick off protocol detection for all files
        for ( int i = 0; i < inputList.size(); ++i )
            inputList[i].backend = Kleo::CryptoBackendFactory::instance()->protocol( "openpgp" );
    }
    return 0;
}

void AssuanCommandPrivateBase::writeToOutputDeviceOrAskForFileName( int id,  const QByteArray& stuff, const QString& _filename )
{
    QIODevice * outdevice = q->bulkOutputDevice( "OUTPUT", id );
    KSaveFile file;
    if ( !outdevice ) {
        QString filename = _filename;
        if ( filename.isEmpty() ) {
            // no output specified, and no filename given, ask the user
            const KUrl url = KUrl::fromPath( q->bulkInputDeviceFileName( "INPUT", id ) );
            filename = KFileDialog::getSaveFileName( url, QString(), 0, i18n("Please select a target file: %1").arg(url.prettyUrl() ) );
        }
        if ( filename.isEmpty() )
            return; // user canceled the dialog, let's just move on. FIXME warning?
                    // FIXME sanitize, percent-encode, etc
        file.setFileName( filename );
        if ( !file.open() )
            throw assuan_exception( q->makeError( GPG_ERR_ASS_WRITE_ERROR ), file.errorString().toStdString() ) ;
        
        outdevice = &file;
    }
    assert(outdevice);
    if ( const int bytesWritten = outdevice->write( stuff ) != stuff.size() )
        throw assuan_exception( q->makeError( GPG_ERR_ASS_WRITE_ERROR ), outdevice->errorString().toStdString() );
    if ( !file.fileName().isEmpty() && !file.finalize() )
        throw assuan_exception( q->makeError( GPG_ERR_ASS_WRITE_ERROR ), file.errorString().toStdString() ) ;
}


#include "assuancommandprivatebase_p.moc"
