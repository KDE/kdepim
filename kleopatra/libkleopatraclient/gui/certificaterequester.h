/* -*- mode: c++; c-basic-offset:4 -*-
    gui/certificaterequester.h

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

#ifndef __LIBKLEOPATRACLIENT_GUI_CERTIFICATEREQUESTER_H__
#define __LIBKLEOPATRACLIENT_GUI_CERTIFICATEREQUESTER_H__

#include "kleopatraclientgui_export.h"

#include <QWidget>

namespace KleopatraClientCopy {
namespace Gui {

    class KLEOPATRACLIENTGUI_EXPORT CertificateRequester : public QWidget {
        Q_OBJECT
        Q_PROPERTY( bool multipleCertificatesAllowed READ multipleCertificatesAllowed WRITE setMultipleCertificatesAllowed )
        Q_PROPERTY( bool onlySigningCertificatesAllowed READ onlySigningCertificatesAllowed WRITE setOnlySigningCertificatesAllowed )
        Q_PROPERTY( bool onlyEncryptionCertificatesAllowed READ onlyEncryptionCertificatesAllowed WRITE setOnlyEncryptionCertificatesAllowed )
        Q_PROPERTY( bool onlyOpenPGPCertificatesAllowed READ onlyOpenPGPCertificatesAllowed WRITE setOnlyOpenPGPCertificatesAllowed )
        Q_PROPERTY( bool onlyX509CertificatesAllowed READ onlyX509CertificatesAllowed WRITE setOnlyX509CertificatesAllowed )
        Q_PROPERTY( bool onlySecretKeysAllowed READ onlySecretKeysAllowed WRITE setOnlySecretKeysAllowed )
        Q_PROPERTY( QStringList selectedCertificates READ selectedCertificates WRITE setSelectedCertificates )
    public:
        explicit CertificateRequester( QWidget * parent=0, Qt::WindowFlags f=0 );
        ~CertificateRequester();

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

        void setSelectedCertificates( const QStringList & certs );
        QStringList selectedCertificates() const;

        void setSelectedCertificate( const QString & cert );
        QString selectedCertificate() const;

    Q_SIGNALS:
        void selectedCertificatesChanged( const QStringList & certs );

    private:
        class Private;
        Private * d;
        Q_PRIVATE_SLOT( d, void slotButtonClicked() )
        Q_PRIVATE_SLOT( d, void slotCommandFinished() )
    };

}
}

#endif /* __LIBKLEOPATRACLIENT_GUI_CERTIFICATEREQUESTER_H__ */
