/* -*- mode: c++; c-basic-offset:4 -*-
    crypto/gui/certificateselectiondialog.cpp

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

#include "certificateselectiondialog.h"
#include "ui_certificateselectionwidget.h"

#include <models/keylistmodel.h>

#include <gpgme++/key.h>

#include <QDialogButtonBox>
#include <QAbstractItemView>

#include <boost/bind.hpp>

#include <algorithm>
#include <cassert>

using namespace Kleo;
using namespace Kleo::Crypto::Gui;

struct CertificateSelectionDialog::Private {
  
    explicit Private( CertificateSelectionDialog * _q );
    ~Private() {}
    CertificateSelectionDialog * const q;
    Ui::CertificateSelectionWidget ui;
    AbstractKeyListModel* m_model;
    QString m_customText;
    CertificateSelectionDialog::SelectionMode m_mode;
    void setLabelText();
    GpgME::Protocol m_protocol;
    KeyType m_allowedKeys;
};


CertificateSelectionDialog::Private::Private( CertificateSelectionDialog * _q)
    :q( _q ), ui(), m_model( AbstractKeyListModel::createFlatKeyListModel( q ) ), m_mode( MultiSelection ), m_protocol( GpgME::UnknownProtocol ), m_allowedKeys( Any )
{
    ui.setupUi( q );
    connect ( ui.buttonBox, SIGNAL( accepted() ), q, SLOT( accept() ) );
    connect ( ui.buttonBox, SIGNAL( rejected() ), q, SLOT( reject() ) );
    ui.listView->setModel( m_model );
    ui.listView->setSelectionMode( QAbstractItemView::MultiSelection );
    setLabelText();
}

CertificateSelectionDialog::CertificateSelectionDialog( QWidget * parent, Qt::WindowFlags flags ) : QDialog( parent, flags ), d( new CertificateSelectionDialog::Private( this ) )
{
}

CertificateSelectionDialog::~CertificateSelectionDialog()
{
}

void CertificateSelectionDialog::setCustomText( const QString& text )
{
    d->m_customText = text;
    d->setLabelText();
}


QString CertificateSelectionDialog::customText() const
{
    return d->m_customText;
}

void CertificateSelectionDialog::Private::setLabelText()
{

    if ( m_customText.isNull() ) 
    {
        ui.instructionLabel->setText( m_mode == SingleSelection ? 
                                      i18n( "Please select a certificate from the list below:" ) : 
                                      i18n( "Please select one or more certificates from the list below:" ) );
    }
    else
    {
        ui.instructionLabel->setText( m_customText );
    }
}        

std::vector<GpgME::Key> CertificateSelectionDialog::selectedKeys() const
{
    QItemSelectionModel * const sm = d->ui.listView->selectionModel();
    assert( sm );
    return d->m_model->keys( sm->selectedIndexes() );
}

void CertificateSelectionDialog::addKeys(const std::vector<GpgME::Key> & keys_ )
{
    std::vector<GpgME::Key> keys = keys_;

    std::vector<GpgME::Key>::iterator end = keys.end();

    if ( d->m_protocol != GpgME::UnknownProtocol )
    {
        end = std::remove_if( keys.begin(), end, bind( &GpgME::Key::protocol, _1 ) != d->m_protocol );
    }

    switch ( d->m_allowedKeys )
    { 
    case EncryptOnly:
        end = std::remove_if( keys.begin(), end, !bind( &GpgME::Key::canEncrypt, _1 ) );
        break;
    case SignOnly:
        end = std::remove_if( keys.begin(), end, !bind( &GpgME::Key::hasSecret, _1 ) );
        end = std::remove_if( keys.begin(), end, !bind( &GpgME::Key::canSign, _1 ) );
        break;
    case Any:
    default:
        break;
    }

    keys.erase( end, keys.end() );

    d->m_model->addKeys( keys );
}

void CertificateSelectionDialog::setSelectionMode( CertificateSelectionDialog::SelectionMode mode )
{
    d->m_mode = mode;
    d->ui.listView->setSelectionMode( mode == MultiSelection ? QAbstractItemView::MultiSelection : QAbstractItemView::SingleSelection );
    d->setLabelText();
}

CertificateSelectionDialog::SelectionMode CertificateSelectionDialog::selectionMode() const
{
    return d->m_mode;
}

void CertificateSelectionDialog::setProtocol( GpgME::Protocol protocol )
{
    d->m_protocol = protocol;
}

GpgME::Protocol CertificateSelectionDialog::protocol() const
{
    return d->m_protocol;
}

void CertificateSelectionDialog::setAllowedKeys( KeyType type )
{
    d->m_allowedKeys = type;
}

CertificateSelectionDialog::KeyType CertificateSelectionDialog::allowedKeys() const
{
    return d->m_allowedKeys;
}
