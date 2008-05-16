/* -*- mode: c++; c-basic-offset:4 -*-
    dialogs/ownertrustdialog.cpp

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

#include "ownertrustdialog.h"

#include "ui_ownertrustdialog.h"

#include <KDebug>

#include <cassert>

using namespace GpgME;
using namespace Kleo;
using namespace Kleo::Dialogs;


class OwnerTrustDialog::Private {
    friend class ::Kleo::Dialogs::OwnerTrustDialog;
    OwnerTrustDialog * const q;
public:
    explicit Private( OwnerTrustDialog * qq )
        : q( qq ),
          ui( qq )
    {
#if 0
        connect( ui.inSB, SIGNAL(valueChanged(int)),
                 q, SLOT(slotInAmountChanged()) );
        connect( ui.inCB, SIGNAL(currentIndexChanged(int)),
                 q, SLOT(slotInUnitChanged()) );
        connect( ui.onCW, SIGNAL(selectionChanged()),
                 q, SLOT(slotOnDateChanged()) );

        assert( ui.inCB->currentIndex() == inUnit );
#endif
    }

private:
    void slotOwnerTrustChanged( Key::OwnerTrust );
    
private:

    struct UI : public Ui::OwnerTrustDialog {
        explicit UI( Dialogs::OwnerTrustDialog * qq )
            : Ui::OwnerTrustDialog()
        {
            setupUi( qq );
#define addTrustItem(item)  trustCB->addItem( Formatting::ownerTrustShort( item ), item );
            addTrustItem( Key::Unknown )
            addTrustItem( Key::Undefined )
            addTrustItem( Key::Never)
            addTrustItem( Key::Marginal )
            addTrustItem( Key::Full )
            addTrustItem( Key::Ultimate )
#undef addTrustItem
        }
    } ui;
};

OwnerTrustDialog::OwnerTrustDialog( QWidget * p, Qt::WindowFlags f )
    : QDialog( p, f ), d( new Private( this ) )
{

}

OwnerTrustDialog::~OwnerTrustDialog() {}


void OwnerTrustDialog::setOwnerTrust( Key::OwnerTrust trust ) {
    const int idx = d->ui.trustCB->findData( trust );
    assert( idx != -1 );
    d->ui.trustCB->setCurrentIndex( idx );
}

Key::OwnerTrust OwnerTrustDialog::ownerTrust() const {
    return static_cast<Key::OwnerTrust>( d->ui.trustCB->itemData( d->ui.trustCB->currentIndex() ).toUInt() );
}


void OwnerTrustDialog::Private::slotOwnerTrustChanged( Key::OwnerTrust trust ) {
    
}




#include "moc_ownertrustdialog.cpp"
