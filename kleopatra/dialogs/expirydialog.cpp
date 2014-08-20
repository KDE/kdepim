/* -*- mode: c++; c-basic-offset:4 -*-
    dialogs/expirydialog.cpp

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

#include "expirydialog.h"

#include "ui_expirydialog.h"

#include <QDate>



#include <cassert>

using namespace Kleo;
using namespace Kleo::Dialogs;

enum Period {
    Days,
    Weeks,
    Months,
    Years,

    NumPeriods
};



static QDate date_by_amount_and_unit( int inAmount, int inUnit ) {
    const QDate current = QDate::currentDate();
    switch ( inUnit ) {
    case Days:   return current.addDays(   inAmount ); break;
    case Weeks:  return current.addDays( 7*inAmount ); break;
    case Months: return current.addMonths( inAmount ); break;
    case Years:  return current.addYears(  inAmount ); break;
    default:
        assert( !"Should not reach here" );
    }
    return QDate();
}

// these calculations should be precise enough for the forseeable future...
static const double DAYS_IN_GREGORIAN_YEAR = 365.2425;

static int monthsBetween( const QDate & d1, const QDate & d2 ) {
    const int days = d1.daysTo( d2 );
    return qRound( days / DAYS_IN_GREGORIAN_YEAR * 12 );
}

static int yearsBetween( const QDate & d1, const QDate & d2 ) {
    const int days = d1.daysTo( d2 );
    return qRound( days / DAYS_IN_GREGORIAN_YEAR );
}




class ExpiryDialog::Private {
    friend class ::Kleo::Dialogs::ExpiryDialog;
    ExpiryDialog * const q;
public:
    explicit Private( ExpiryDialog * qq )
        : q( qq ),
          inUnit( Days ),
          ui( q )
    {
        connect( ui.inSB, SIGNAL(valueChanged(int)),
                 q, SLOT(slotInAmountChanged()) );
        connect( ui.inCB, SIGNAL(currentIndexChanged(int)),
                 q, SLOT(slotInUnitChanged()) );
        connect( ui.onCW, SIGNAL(selectionChanged()),
                 q, SLOT(slotOnDateChanged()) );

        assert( ui.inCB->currentIndex() == inUnit );
    }

private:
    void slotInAmountChanged();
    void slotInUnitChanged();
    void slotOnDateChanged();

private:
    QDate inDate() const;
    int inAmountByDate( const QDate & date ) const;

private:
    int inUnit;

    struct UI : public Ui::ExpiryDialog {
        explicit UI( Dialogs::ExpiryDialog * qq )
            : Ui::ExpiryDialog()
        {
            setupUi( qq->mainWidget() );
            qq->setButtons( KDialog::Ok | KDialog::Cancel );

            assert( inCB->count() == NumPeriods );

            onCW->setMinimumDate( QDate::currentDate().addDays( 1 ) );
        }
    } ui;
};

ExpiryDialog::ExpiryDialog( QWidget * p, Qt::WindowFlags f )
    : KDialog( p, f ), d( new Private( this ) )
{

}

ExpiryDialog::~ExpiryDialog() {}


void ExpiryDialog::setDateOfExpiry( const QDate & date ) {
    const QDate current = QDate::currentDate();
    if ( date.isValid() ) {
        d->ui.onRB->setChecked( true );
        d->ui.onCW->setSelectedDate( qMax( date, current ) );
    } else {
        d->ui.neverRB->setChecked( true );
        d->ui.onCW->setSelectedDate( current );
        d->ui.inSB->setValue( 0 );
    }
}

QDate ExpiryDialog::dateOfExpiry() const {
    return
        d->ui.inRB->isChecked() ? d->inDate() :
        d->ui.onRB->isChecked() ? d->ui.onCW->selectedDate() :
        QDate() ;
}




void ExpiryDialog::Private::slotInUnitChanged() {
    const int oldInAmount = ui.inSB->value();
    const QDate targetDate = date_by_amount_and_unit( oldInAmount, inUnit );
    inUnit = ui.inCB->currentIndex();
    if ( targetDate.isValid() )
        ui.inSB->setValue( inAmountByDate( targetDate ) );
    else
        slotInAmountChanged();
}

void ExpiryDialog::Private::slotInAmountChanged() {
    // Only modify onCW when onCW is slave:
    if ( ui.inRB->isChecked() )
        ui.onCW->setSelectedDate( inDate() );
}

void ExpiryDialog::Private::slotOnDateChanged() {
    // Only modify inSB/inCB when onCW is master:
    if ( ui.onRB->isChecked() )
        ui.inSB->setValue( inAmountByDate( ui.onCW->selectedDate() ) );
}




QDate ExpiryDialog::Private::inDate() const {
    return date_by_amount_and_unit( ui.inSB->value(), ui.inCB->currentIndex() );
}

int ExpiryDialog::Private::inAmountByDate( const QDate & selected ) const {
    const QDate current  = QDate::currentDate();

    switch ( ui.inCB->currentIndex() ) {
    case Days:   return current.daysTo( selected );
    case Weeks:  return qRound( current.daysTo( selected ) / 7.0 );
    case Months: return monthsBetween( current, selected );
    case Years:  return yearsBetween( current, selected );
    };
    assert( !"Should not reach here" );
    return -1;
}

#include "moc_expirydialog.cpp"
