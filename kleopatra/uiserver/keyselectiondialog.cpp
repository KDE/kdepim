/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/keyselectiondialog.cpp

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

#include "keyselectiondialog.h"
#include "ui_keyselectiondialog.h"

#include "models/keylistmodel.h"

#include <gpgme++/key.h>

#include <QDialogButtonBox>
#include <QAbstractItemView>

#include <boost/bind.hpp>

#include <algorithm>
#include <cassert>

using namespace Kleo;

struct KeySelectionDialog::Private {
  
    Private( KeySelectionDialog * _q );
    ~Private() {}
    KeySelectionDialog * const q;
    Ui::KeySelectionWidget ui;
    AbstractKeyListModel* m_model;
    QString m_customText;
    KeySelectionDialog::SelectionMode m_mode;
    void setLabelText();
    GpgME::Protocol m_protocol;
};


KeySelectionDialog::Private::Private( KeySelectionDialog * _q)
    :q( _q ), ui(), m_model( AbstractKeyListModel::createFlatKeyListModel( q ) ), m_mode( MultiSelection ), m_protocol( GpgME::UnknownProtocol )
{
    ui.setupUi( q );
    connect ( ui.buttonBox, SIGNAL( accepted() ), q, SLOT( accept() ) );
    connect ( ui.buttonBox, SIGNAL( rejected() ), q, SLOT( reject() ) );
    ui.listView->setModel( m_model );
    ui.listView->setSelectionMode( QAbstractItemView::MultiSelection );
    setLabelText();
}

KeySelectionDialog::KeySelectionDialog( QWidget * parent, Qt::WindowFlags flags ) : QDialog( parent, flags ), d( new KeySelectionDialog::Private( this ) )
{
}

KeySelectionDialog::~KeySelectionDialog()
{
}

void KeySelectionDialog::setCustomText( const QString& text )
{
    d->m_customText = text;
    d->setLabelText();
}


QString KeySelectionDialog::customText() const
{
    return d->m_customText;
}

void KeySelectionDialog::Private::setLabelText()
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

std::vector<GpgME::Key> KeySelectionDialog::selectedKeys() const
{
    QItemSelectionModel * const sm = d->ui.listView->selectionModel();
    assert( sm );
    return d->m_model->keys( sm->selectedIndexes() );
}

void KeySelectionDialog::addKeys(const std::vector<GpgME::Key> & keys_ )
{
    std::vector<GpgME::Key> keys = keys_;

    //TODO: this really should be done via KeyFilters/a proxy model
    if ( d->m_protocol != GpgME::UnknownProtocol )
    {
       keys.erase( std::remove_if( keys.begin(), keys.end(),
                                     bind( &GpgME::Key::protocol, _1 ) != d->m_protocol ), keys.end() );
 
    }

    d->m_model->addKeys( keys );
}

void KeySelectionDialog::setSelectionMode( KeySelectionDialog::SelectionMode mode )
{
    d->m_mode = mode;
    d->ui.listView->setSelectionMode( mode == MultiSelection ? QAbstractItemView::MultiSelection : QAbstractItemView::SingleSelection );
    d->setLabelText();
}

KeySelectionDialog::SelectionMode KeySelectionDialog::selectionMode() const
{
    return d->m_mode;
}

void KeySelectionDialog::setProtocol( GpgME::Protocol protocol )
{
    d->m_protocol = protocol;
}

GpgME::Protocol KeySelectionDialog::protocol() const
{
    return d->m_protocol;
}


