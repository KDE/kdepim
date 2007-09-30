/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/assuancommandprivatebase_p.h

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

#ifndef __KLEOPATRA_UISERVER_ASSUANCOMMANDPRIVATEBASE_H__
#define __KLEOPATRA_UISERVER_ASSUANCOMMANDPRIVATEBASE_H__

#include <QObject>
#include <QString>
#include <QList>

#include <gpgme++/error.h>
#include <gpgme++/key.h>
#include <gpgme++/verificationresult.h>

#include <kleo/cryptobackendfactory.h>

class QIODevice;

namespace Kleo {

    class AssuanCommand;

class AssuanCommandPrivateBase : public QObject
{
    Q_OBJECT
public:
    AssuanCommandPrivateBase();
    virtual ~AssuanCommandPrivateBase() = 0;

    struct Input
    {
        Input() : type( Detached ), message( 0 ), signature( 0 ), backend( 0 ) {}
        enum SignatureType {
            Detached=0,
            Opaque
        };
        bool isFileInputOnly() const
        {
            if ( signature ) {
                return !signatureFileName.isEmpty() && ( type == Opaque || !messageFileName.isEmpty() );
            } else {
                // only message is being used
                return !messageFileName.isEmpty();
            }

        }

        void setupMessage( QIODevice* message, const QString& fileName );
        SignatureType type;
        QIODevice* message;
        QString messageFileName;
        QString signatureFileName;
        QIODevice* signature;
        const CryptoBackend::Protocol* backend;
    };
    QList<Input> inputList;
    int determineInputsAndProtocols( QString& reason );
    void writeToOutputDeviceOrAskForFileName( int id,  const QByteArray& stuff, const QString& _filename );
protected:
    virtual AssuanCommand *get_q() const = 0;
};


// helper Mixin which implements get_q with a co-variant return type.
template <typename Derived, typename Command>
class AssuanCommandPrivateBaseMixin : public AssuanCommandPrivateBase {
    /* reimpl */
    Command* get_q() const {
        return static_cast<const Derived*>( this )->q;
    }
};

}

#endif // __KLEOPATRA_UISERVER_ASSUANCOMMANDPRIVATEBASE_H__
