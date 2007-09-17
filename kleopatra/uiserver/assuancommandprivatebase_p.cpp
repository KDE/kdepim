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

QList<AssuanCommandPrivateBase::Input> AssuanCommandPrivateBase::analyzeInput( GpgME::Error& error, QString& errorDetails ) const
{
    error = GpgME::Error();
    errorDetails = QString();

    const int numSignatures = q->numBulkInputDevices( "INPUT" );
    const int numMessages = q->numBulkInputDevices( "MESSAGE" );

    if ( numSignatures == 0 )
    {
        error = GpgME::Error( GPG_ERR_ASS_NO_INPUT );
        errorDetails = "At least one signature must be provided";
        return QList<Input>();
    }

    if ( numMessages > 0 && numMessages != numSignatures )
    {
        error = GpgME::Error( GPG_ERR_ASS_NO_INPUT ); //TODO use better error code if possible
        errorDetails = "The number of MESSAGE inputs must be either equal to the number of signatures or zero";
        return QList<Input>();
    }

    QList<Input> inputs;

    if ( numMessages == numSignatures )
    {
        for ( int i = 0; i < numSignatures; ++i )
        {
            Input input;
            input.type = Input::Detached;
            input.signature = q->bulkInputDevice( "INPUT", i );
            input.signatureFileName = q->bulkInputDeviceFileName( "INPUT", i );
            input.setupMessage( q->bulkInputDevice( "MESSAGE", i ), q->bulkInputDeviceFileName( "MESSAGE", i ) );
            assert( input.message || !input.messageFileName.isEmpty() );
            assert( input.signature );
            inputs.append( input );
        }
        return inputs;
    }

    assert( numMessages == 0 );

    for ( int i = 0; i < numSignatures; ++i )
    {
        Input input;
        input.signature = q->bulkInputDevice( "INPUT", i );
        input.signatureFileName = q->bulkInputDeviceFileName( "INPUT", i );
        assert( input.signature );
        const QString fname = q->bulkInputDeviceFileName( "INPUT", i );
        if ( !fname.isEmpty() && fname.endsWith( ".sig", Qt::CaseInsensitive )
                || fname.endsWith( ".asc", Qt::CaseInsensitive ) )
        { //detached signature file
            const QString msgFileName = fname.left( fname.length() - 4 );
            // TODO: handle error if msg file does not exist
            input.type = Input::Detached;
            input.messageFileName = msgFileName;
        }
        else // opaque
        {
            input.type = Input::Opaque;
        }
        inputs.append( input );
    }
    return inputs;
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


#include "assuancommandprivatebase_p.moc"
