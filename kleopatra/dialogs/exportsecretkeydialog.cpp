/* -*- mode: c++; c-basic-offset:4 -*-
    dialogs/exportsecretkeydialog.cpp

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

#include "exportsecretkeydialog.h"

#include "ui_exportsecretkeydialog.h"

#include <utils/formatting.h>

#include <gpgme++/key.h>


#include <KMessageBox>
#include <KLocalizedString>

#include <cassert>

using namespace Kleo;
using namespace Kleo::Dialogs;
using namespace GpgME;

// This comes from gnupg's sources, agent/minip12.c
// In fact, any charset supported by iconv would work, but we don't link to iconv directly...
static const char *charsets[] = {
    "utf8",
    "iso-8859-1",
    "iso-8859-15",
    "iso-8859-2",
    "iso-8859-3",
    "iso-8859-4",
    "iso-8859-5",
    "iso-8859-6",
    "iso-8859-7",
    "iso-8859-8",
    "iso-8859-9",
    "koi8-r",
    "ibm437",
    "ibm850",
    "euc-jp",
    "big5",
};
static const unsigned int numCharsets = sizeof charsets / sizeof *charsets;

class ExportSecretKeyDialog::Private {
    friend class ::Kleo::Dialogs::ExportSecretKeyDialog;
    ExportSecretKeyDialog * const q;
public:
    explicit Private( ExportSecretKeyDialog * qq )
        : q( qq ),
          ui( q )
    {

    }

private:
    void updateWidgets() {
        const bool x509 = key.protocol() == CMS;
        ui.charsetCB->setVisible( x509 );
        ui.charsetLB->setVisible( x509 );
    }

    void updateFileName() {
        const bool x509 = key.protocol() == CMS;
        const bool armor = q->useArmor();

        static const char * extensions[] = {
            ".gpg", ".asc", ".p12", ".pem"
        };
        const unsigned int idx = 2*x509+armor;
        const char * const extension = extensions[idx];

        const QString nf = i18n("Secret Key Files") + QString::fromLatin1("(*%1 *%2 *%3 *%4 *.pgp)")
            .arg( QLatin1String(extensions[idx]), QLatin1String(extensions[(idx+1)%4]), QLatin1String(extensions[(idx+2)%4]), QLatin1String(extensions[(idx+3)%4]) );
        ui.outputFileFR->setNameFilter( nf );

        QString fn = q->fileName();
        if ( fn.isEmpty() )
            return;

        bool found = false;
        for ( unsigned int i = 0 ; i < sizeof extensions / sizeof *extensions ; ++i )
            if ( fn.endsWith( QLatin1String(extensions[i]), Qt::CaseInsensitive ) ) {
                fn.chop( 4 );
                found = true;
                break;
            }
        if ( found )
            q->setFileName( fn + QLatin1String(extension) );
    }

    void updateLabel() {
        ui.descriptionLB->setText( i18nc("@info",
                                         "Please select export options for %1:",
                                         Formatting::formatForComboBox( key ) ) );
    }
private:
    Key key;

    struct UI : public Ui_ExportSecretKeyDialog {
        explicit UI( Dialogs::ExportSecretKeyDialog * qq )
            : Ui_ExportSecretKeyDialog()
        {
            setupUi( qq );

            outputFileFR->setExistingOnly( false );
            outputFileFR->setFilter( QDir::Files );
            outputFileFR->setNameFilter( i18n("Secret Key Files (*.pem *.p12 *.gpg *.asc *.pgp)") );

            for ( unsigned int i = 0 ; i < numCharsets ; ++i )
                charsetCB->addItem( QString::fromLatin1( charsets[i] ) );
            charsetCB->setCurrentIndex( 0 );
                           
        }
    } ui;
};

ExportSecretKeyDialog::ExportSecretKeyDialog( QWidget * p, Qt::WindowFlags f )
    : QDialog( p, f ), d( new Private( this ) )
{

}

ExportSecretKeyDialog::~ExportSecretKeyDialog() {}


void ExportSecretKeyDialog::setKey( const Key & key ) {
    if ( qstricmp( key.primaryFingerprint(), d->key.primaryFingerprint() ) == 0 )
        return;
    d->key = key;
    d->updateWidgets();
    d->updateLabel();
    d->updateFileName();
}

Key ExportSecretKeyDialog::key() const {
    return d->key;
}

void ExportSecretKeyDialog::setFileName( const QString & fileName ) {
    d->ui.outputFileFR->setFileName( fileName );
}

QString ExportSecretKeyDialog::fileName() const {
    return d->ui.outputFileFR->fileName();
}

void ExportSecretKeyDialog::setCharset( const QByteArray & charset ) {
    for ( unsigned int i = 0 ; i < sizeof charsets / sizeof *charsets ; ++i )
        if ( charset == charsets[i] ) {
            d->ui.charsetCB->setCurrentIndex( i );
            return;
        }
}

QByteArray ExportSecretKeyDialog::charset() const {
    if ( d->ui.charsetCB->isVisible() )
        return d->ui.charsetCB->currentText().toLatin1();
    else
        return QByteArray();
}

void ExportSecretKeyDialog::setUseArmor( bool on ) {
    d->ui.armorCB->setChecked( on );
}

bool ExportSecretKeyDialog::useArmor() const {
    return d->ui.armorCB->isChecked();
}

void ExportSecretKeyDialog::accept() {
    d->updateFileName();
    const QString fn = fileName();
    if ( fn.isEmpty() ) {
        KMessageBox::information( this, i18nc("@info",
                                              "You have to enter an output filename." ),
                                  i18nc("@title", "Incomplete data") );
        d->ui.outputFileFR->setFocus();
        return;
    }

    const QByteArray cs = charset();
    if ( d->key.protocol() == CMS && cs.isEmpty() ) {
        KMessageBox::information( this, i18nc("@info",
                                              "You have to choose a passphrase character set." ),
                                  i18nc("@title", "Incomplete data") );
        d->ui.charsetCB->setFocus();
        return;
    }

    QDialog::accept();
}

#include "moc_exportsecretkeydialog.cpp"
