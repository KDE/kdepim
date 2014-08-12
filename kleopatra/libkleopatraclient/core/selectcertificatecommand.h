/* -*- mode: c++; c-basic-offset:4 -*-
    core/selectcertificatecommand.h

    This file is part of KleopatraClient, the Kleopatra interface library
    Copyright (c) 2008 Klarälvdalens Datakonsult AB

    KleopatraClient is free software; you can redistribute it and/or modify
    it under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    KleopatraClient is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef __LIBKLEOPATRACLIENT_CORE_SELECTCERTIFICATECOMMAND_H__
#define __LIBKLEOPATRACLIENT_CORE_SELECTCERTIFICATECOMMAND_H__

#include <libkleopatraclient/core/command.h>

namespace KleopatraClientCopy {

    class KLEOPATRACLIENTCORE_EXPORT SelectCertificateCommand : public Command {
        Q_OBJECT
    public:
        explicit SelectCertificateCommand( QObject * parent=0 );
        ~SelectCertificateCommand();

        // Inputs

        void setMultipleCertificatesAllowed( bool allow );
        bool multipleCertificatesAllowed() const;

        void setOnlySigningCertificatesAllowed( bool allow );
        bool onlySigningCertificatesAllowed() const;

        void setOnlyEncryptionCertificatesAllowed( bool allow );
        bool onlyEncryptionCertificatesAllowed() const;

        void setOnlyOpenPGPCertificatesAllowed( bool allow );
        bool onlyOpenPGPCertificatesAllowed() const;

        void setOnlyX509CertificatesAllowed( bool allow );
        bool onlyX509CertificatesAllowed() const;

        void setOnlySecretKeysAllowed( bool allow );
        bool onlySecretKeysAllowed() const;

        // Input/Outputs

        void setSelectedCertificates( const QStringList & certs );
        QStringList selectedCertificates() const;

        void setSelectedCertificate( const QString & cert );
        QString selectedCertificate() const;

    };

}

#endif /* __LIBKLEOPATRACLIENT_CORE_SELECTCERTIFICATECOMMAND_H__ */
