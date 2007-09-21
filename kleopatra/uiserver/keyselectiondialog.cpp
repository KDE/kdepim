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

using namespace Kleo;

struct KeySelectionDialog::Private {
  
    Private( KeySelectionDialog * _q );
    ~Private() {}
    KeySelectionDialog * const q;
    AbstractKeyListModel* m_model;
};


KeySelectionDialog::Private::Private( KeySelectionDialog * _q)
:q( _q )
{
    m_model = AbstractKeyListModel::createFlatKeyListModel( q );
}

KeySelectionDialog::KeySelectionDialog()
    :ui( new Ui::KeySelectionWidget() ), d( new KeySelectionDialog::Private( this ) )
{
    ui->setupUi( this );
    connect ( ui->buttonBox, SIGNAL( accepted() ), this, SLOT( accept() ) );
    connect ( ui->buttonBox, SIGNAL( rejected() ), this, SLOT( reject() ) );
    
    ui->listView->setModel( d->m_model );
    ui->listView->setSelectionMode( QAbstractItemView::MultiSelection );
}

KeySelectionDialog::~KeySelectionDialog()
{
}

std::vector<GpgME::Key> KeySelectionDialog::selectedKeys() const
{
    QItemSelectionModel * sm = ui->listView->selectionModel();
    return d->m_model->keys( sm->selectedIndexes() );
}

void KeySelectionDialog::addKeys(const std::vector<GpgME::Key> & keys)
{
    d->m_model->addKeys( keys );
}




