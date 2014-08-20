/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/selectcertificatecommand.cpp

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

#include "selectcertificatecommand.h"

#include <dialogs/certificateselectiondialog.h>

#include <models/keycache.h>

#include <boost/mem_fn.hpp>

#include <kleo/stl_util.h>
#include <kleo/exception.h>

#include <gpgme++/key.h>

#include <gpg-error.h>

#include <QDebug>
#include <KLocalizedString>

#include <QByteArray>
#include <QPointer>

#include <string>
#include <algorithm>

using namespace Kleo;
using namespace Kleo::Dialogs;
using namespace GpgME;
using namespace boost;

static const char option_prefix[] = "prefix";

class SelectCertificateCommand::Private {
    friend class ::Kleo::SelectCertificateCommand;
    SelectCertificateCommand * const q;
public:
    Private( SelectCertificateCommand * qq ) :
        q( qq ),
        dialog()
    {

    }

private:
    void slotDialogAccepted();
    void slotDialogRejected();
    void slotSelectedCertificates( int, const QByteArray & );

private:
    void ensureDialogCreated() {
        if ( dialog )
            return;
        dialog = new CertificateSelectionDialog;
        q->applyWindowID( dialog );
        dialog->setAttribute( Qt::WA_DeleteOnClose );
        //dialog->setWindowTitle( i18nc( "@title", "Certificate Selection" ) );
        connect( dialog, SIGNAL(accepted()), q, SLOT(slotDialogAccepted()) );
        connect( dialog, SIGNAL(rejected()), q, SLOT(slotDialogRejected()) );
    }
    void ensureDialogShown() {
        ensureDialogCreated();
        if ( dialog->isVisible() )
            dialog->raise();
        else
            dialog->show();
    }

private:
    QPointer<CertificateSelectionDialog> dialog;
};

SelectCertificateCommand::SelectCertificateCommand()
    : QObject(), AssuanCommandMixin<SelectCertificateCommand>(), d( new Private( this ) ) {}

SelectCertificateCommand::~SelectCertificateCommand() {}

static const struct {
    const char * name;
    CertificateSelectionDialog::Option option;
} option_table[] = {
    { "multi",        CertificateSelectionDialog::MultiSelection },
    { "sign-only",    CertificateSelectionDialog::SignOnly       },
    { "encrypt-only", CertificateSelectionDialog::EncryptOnly    },
    { "openpgp-only", CertificateSelectionDialog::OpenPGPFormat  },
    { "x509-only",    CertificateSelectionDialog::CMSFormat      },
    { "secret-only",  CertificateSelectionDialog::SecretKeys     },
};

int SelectCertificateCommand::doStart() {

    d->ensureDialogCreated();

    CertificateSelectionDialog::Options opts;
    for ( unsigned int i = 0 ; i < sizeof option_table / sizeof *option_table ; ++i )
        if ( hasOption( option_table[i].name ) )
             opts |= option_table[i].option;

    d->dialog->setOptions( opts );

    if ( const int err = inquire( "SELECTED_CERTIFICATES",
                                  this, SLOT(slotSelectedCertificates(int,QByteArray)) ) )
        return err;

    d->ensureDialogShown();

    return 0;
}

void SelectCertificateCommand::Private::slotSelectedCertificates( int err, const QByteArray & data ) {
    qDebug() << err << ", " << data.constData();
    if ( err )
        return;
    const std::vector<std::string> fprs = kdtools::transform< std::vector<std::string> >( data.split( '\n' ), mem_fn( &QByteArray::constData ) );
    const std::vector<Key> keys = KeyCache::instance()->findByKeyIDOrFingerprint( fprs );
    Q_FOREACH( const Key & key, keys )
        qDebug() << "found key " << key.userID(0).id();
    if ( dialog )
        dialog->selectCertificates( keys );
    else
        qWarning() << "dialog == NULL in slotSelectedCertificates";
}

void SelectCertificateCommand::doCanceled() {
    if ( d->dialog )
        d->dialog->close();
}

void SelectCertificateCommand::Private::slotDialogAccepted() {
    try {
        QByteArray data;
        Q_FOREACH( const Key & key, dialog->selectedCertificates() ) {
            data += key.primaryFingerprint();
            data += '\n';
        }
        q->sendData( data );
        q->done();
    } catch ( const Exception & e ) {
        q->done( e.error(), e.message() );
    } catch ( const std::exception & e ) {
        q->done( makeError( GPG_ERR_UNEXPECTED ),
                 i18n("Caught unexpected exception in SelectCertificateCommand::Private::slotDialogAccepted: %1",
                      QString::fromLocal8Bit( e.what() ) ) );
    } catch ( ... ) {
        q->done( makeError( GPG_ERR_UNEXPECTED ),
                 i18n("Caught unknown exception in SelectCertificateCommand::Private::slotDialogAccepted") );
    }
}

void SelectCertificateCommand::Private::slotDialogRejected() {
    dialog = 0;
    q->done( makeError( GPG_ERR_CANCELED ) );
}

#include "moc_selectcertificatecommand.cpp"
