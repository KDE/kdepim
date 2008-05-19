/* -*- mode: c++; c-basic-offset:4 -*-
    dialogs/signcertificatedialog.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2008 Klarälvdalens Datakonsult AB

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

#include <utils/formatting.h>

#include "signcertificatedialog.h"

#include "ui_signcertificatedialog.h"

#include <KDebug>

#include <cassert>

using namespace GpgME;
using namespace Kleo;
using namespace Kleo::Dialogs;


class SignCertificateDialog::Private {
    friend class ::Kleo::Dialogs::SignCertificateDialog;
    SignCertificateDialog * const q;
public:
    explicit Private( SignCertificateDialog * qq )
        : q( qq ),
          ui( qq )
    {
    }
    
private:

    struct UI : public Ui::SignCertificateDialog {
        explicit UI( Dialogs::SignCertificateDialog * qq )
            : Ui::SignCertificateDialog()
        {
            setupUi( qq );
        }
    } ui;
};

SignCertificateDialog::SignCertificateDialog( QWidget * p, Qt::WindowFlags f )
    : QDialog( p, f ), d( new Private( this ) )
{

}

SignCertificateDialog::~SignCertificateDialog() {}

void SignCertificateDialog::setSigningOption( SignKeyJob::SigningOption option ) {
    if ( option == SignKeyJob::LocalSignature )
        d->ui.localRB->setChecked( true );
    else
        d->ui.exportableRB->setChecked( true );
}

SignKeyJob::SigningOption SignCertificateDialog::signingOption() const {
    return d->ui.localRB->isChecked() ? SignKeyJob::LocalSignature : SignKeyJob::ExportableSignature;
}


#include "moc_signcertificatedialog.cpp"
