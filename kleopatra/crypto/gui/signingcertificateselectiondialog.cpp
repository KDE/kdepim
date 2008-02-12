/* -*- mode: c++; c-basic-offset:4 -*-
    crypto/gui/signingcertificateselectiondialog.cpp

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

#include "signingcertificateselectiondialog.h"

#include "ui_signingcertificateselectionwidget.h"

#include <models/keycache.h>

#include <utils/formatting.h>
#include <utils/stl_util.h>

#include <KLocalizedString>
#include <QByteArray>
#include <QMap>

#include <boost/bind.hpp>

#include <cassert>

using namespace Kleo;
using namespace Kleo::Crypto::Gui;

class SigningCertificateSelectionDialog::Private {
    friend class ::SigningCertificateSelectionDialog;
    SigningCertificateSelectionDialog * const q;
public:
    explicit Private( SigningCertificateSelectionDialog * qq );
    ~Private();
    std::vector<GpgME::Key> candidates( GpgME::Protocol prot ) const;
    void addCandidates( GpgME::Protocol prot, QComboBox* combo );

private:
    Ui::SigningCertificateSelectionWidget ui;
};


SigningCertificateSelectionDialog::Private::Private( SigningCertificateSelectionDialog * qq )
  : q( qq )
{
    q->setWindowTitle( i18n( "Select Signing Certificates" ) );
    QWidget* main = new QWidget( q );
    ui.setupUi( main );
    q->setMainWidget( main ); 
    addCandidates( GpgME::CMS, ui.cmsCombo );
    addCandidates( GpgME::OpenPGP, ui.pgpCombo );
    ui.rememberCO->setChecked( true );
}

SigningCertificateSelectionDialog::Private::~Private() {}



SigningCertificateSelectionDialog::SigningCertificateSelectionDialog( QWidget * parent, Qt::WFlags f )
  : KDialog( parent, f ), d( new Private( this ) )
{
}

SigningCertificateSelectionDialog::~SigningCertificateSelectionDialog() {}


void SigningCertificateSelectionDialog::setSelectedCertificates( const QMap<GpgME::Protocol, GpgME::Key>& certificates )
{
}

std::vector<GpgME::Key> SigningCertificateSelectionDialog::Private::candidates( GpgME::Protocol prot ) const
{
    assert( prot != GpgME::UnknownProtocol );
    std::vector<GpgME::Key> keys = SecretKeyCache::instance()->keys();
    std::vector<GpgME::Key>::iterator end = keys.end();

    end = std::remove_if( keys.begin(), end, bind( &GpgME::Key::protocol, _1 ) != prot );
    //end = std::remove_if( keys.begin(), end, !bind( &GpgME::Key::hasSecret, _1 ) );
    assert( kdtools::all( keys.begin(), end, bind( &GpgME::Key::hasSecret, _1 ) ) );
    end = std::remove_if( keys.begin(), end, !bind( &GpgME::Key::canSign, _1 ) );
    keys.erase( end, keys.end() );
    return keys;
}

void SigningCertificateSelectionDialog::Private::addCandidates( GpgME::Protocol prot, QComboBox* combo )
{
    const std::vector<GpgME::Key> keys = candidates( prot );
    Q_FOREACH( const GpgME::Key& i, keys )
        combo->addItem( Formatting::formatForComboBox( i ), 
                        QByteArray( i.primaryFingerprint() ) );
}


QMap<GpgME::Protocol, GpgME::Key> SigningCertificateSelectionDialog::selectedCertificates() const
{
    QMap<GpgME::Protocol, GpgME::Key> res;
    
    const QByteArray pgpfpr = d->ui.pgpCombo->itemData( d->ui.pgpCombo->currentIndex() ).toByteArray();
    res.insert( GpgME::OpenPGP, SecretKeyCache::instance()->findByFingerprint( pgpfpr.constData() ) );
    const QByteArray cmsfpr = d->ui.cmsCombo->itemData( d->ui.cmsCombo->currentIndex() ).toByteArray();
    res.insert( GpgME::CMS, SecretKeyCache::instance()->findByFingerprint( cmsfpr.constData() ) );
    return res;
}

bool SigningCertificateSelectionDialog::rememberAsDefault() const
{
    return d->ui.rememberCO->isChecked();
}
