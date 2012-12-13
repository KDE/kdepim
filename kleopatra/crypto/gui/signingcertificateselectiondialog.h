/* -*- mode: c++; c-basic-offset:4 -*-
    crypto/gui/signingcertificateselectiondialog.h

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
#ifndef __KLEOPATRA_CRYPTO_GUI_SIGNINGCERTIFICATESELECTIONDIALOG_H__
#define __KLEOPATRA_CRYPTO_GUI_SIGNINGCERTIFICATESELECTIONDIALOG_H__

#include <KDialog>

#include <gpgme++/key.h>

#include <utils/pimpl_ptr.h>

template <typename K, typename U> class QMap;

namespace Kleo {
namespace Crypto {
namespace Gui {

    class SigningCertificateSelectionWidget;

    class SigningCertificateSelectionDialog : public KDialog {
        Q_OBJECT
    public:
        explicit SigningCertificateSelectionDialog( QWidget * parent=0, Qt::WindowFlags f=0 );
        ~SigningCertificateSelectionDialog();

        void setAllowedProtocols( const QVector<GpgME::Protocol>& allowedProtocols );
        void setSelectedCertificates( const QMap<GpgME::Protocol, GpgME::Key>& certificates );
        QMap<GpgME::Protocol, GpgME::Key> selectedCertificates() const;

        bool rememberAsDefault() const;

    private:
        SigningCertificateSelectionWidget * widget;
    };

}
}
}

#endif // __KLEOPATRA_CRYPTO_GUI_SIGNINGCERTIFICATESELECTIONDIALOG_H__

