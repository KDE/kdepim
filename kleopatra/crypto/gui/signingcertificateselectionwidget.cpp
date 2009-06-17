/* -*- mode: c++; c-basic-offset:4 -*-
    crypto/gui/signingcertificateselectionwidget.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2007, 2009 Klarälvdalens Datakonsult AB

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

#include "signingcertificateselectionwidget.h"

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

class SigningCertificateSelectionWidget::Private {
    friend class ::SigningCertificateSelectionWidget;
    SigningCertificateSelectionWidget * const q;
public:
    explicit Private( SigningCertificateSelectionWidget * qq );
    ~Private();
    static std::vector<GpgME::Key> candidates( GpgME::Protocol prot );
    static void addCandidates( GpgME::Protocol prot, QComboBox* combo );

private:
    Ui::SigningCertificateSelectionWidget ui;
};


SigningCertificateSelectionWidget::Private::Private( SigningCertificateSelectionWidget * qq )
    : q( qq ), ui()
{
    ui.setupUi( q );
    addCandidates( GpgME::CMS, ui.cmsCombo );
    addCandidates( GpgME::OpenPGP, ui.pgpCombo );
    ui.rememberCO->setChecked( true );
}

SigningCertificateSelectionWidget::Private::~Private() {}



SigningCertificateSelectionWidget::SigningCertificateSelectionWidget( QWidget * parent, Qt::WindowFlags f )
    : QWidget( parent, f ), d( new Private( this ) )
{

}

SigningCertificateSelectionWidget::~SigningCertificateSelectionWidget() {}


void SigningCertificateSelectionWidget::setSelectedCertificates( const QMap<GpgME::Protocol, GpgME::Key>& certificates )
{
}

std::vector<GpgME::Key> SigningCertificateSelectionWidget::Private::candidates( GpgME::Protocol prot )
{
    assert( prot != GpgME::UnknownProtocol );
    std::vector<GpgME::Key> keys = KeyCache::instance()->keys();
    std::vector<GpgME::Key>::iterator end = keys.end();

    end = std::remove_if( keys.begin(), end, bind( &GpgME::Key::protocol, _1 ) != prot );
    end = std::remove_if( keys.begin(), end, !bind( &GpgME::Key::hasSecret, _1 ) );
    assert( kdtools::all( keys.begin(), end, bind( &GpgME::Key::hasSecret, _1 ) ) );
    end = std::remove_if( keys.begin(), end, !bind( &GpgME::Key::canSign, _1 ) );
    end = std::remove_if( keys.begin(), end, bind( &GpgME::Key::isExpired, _1 ) );
    end = std::remove_if( keys.begin(), end, bind( &GpgME::Key::isRevoked, _1 ) );
    keys.erase( end, keys.end() );
    return keys;
}

void SigningCertificateSelectionWidget::Private::addCandidates( GpgME::Protocol prot, QComboBox* combo )
{
    const std::vector<GpgME::Key> keys = candidates( prot );
    Q_FOREACH( const GpgME::Key& i, keys )
        combo->addItem( Formatting::formatForComboBox( i ),
                        QVariant( QByteArray( i.primaryFingerprint() ) ) );
}


QMap<GpgME::Protocol, GpgME::Key> SigningCertificateSelectionWidget::selectedCertificates() const
{
    QMap<GpgME::Protocol, GpgME::Key> res;

    const QByteArray pgpfpr = d->ui.pgpCombo->itemData( d->ui.pgpCombo->currentIndex() ).toByteArray();
    res.insert( GpgME::OpenPGP, KeyCache::instance()->findByFingerprint( pgpfpr.constData() ) );
    const QByteArray cmsfpr = d->ui.cmsCombo->itemData( d->ui.cmsCombo->currentIndex() ).toByteArray();
    res.insert( GpgME::CMS, KeyCache::instance()->findByFingerprint( cmsfpr.constData() ) );
    return res;
}

bool SigningCertificateSelectionWidget::rememberAsDefault() const
{
    return d->ui.rememberCO->isChecked();
}

void SigningCertificateSelectionWidget::setAllowedProtocols( const QVector<GpgME::Protocol>& allowedProtocols )
{
    assert( allowedProtocols.size() >= 1 );
    const bool pgp = allowedProtocols.contains( GpgME::OpenPGP );
    const bool cms = allowedProtocols.contains( GpgME::CMS );

    d->ui.pgpLabel->setVisible( pgp );
    d->ui.pgpCombo->setVisible( pgp );

    d->ui.cmsLabel->setVisible( cms );
    d->ui.cmsCombo->setVisible( cms );
}

#include "moc_signingcertificateselectionwidget.cpp"
