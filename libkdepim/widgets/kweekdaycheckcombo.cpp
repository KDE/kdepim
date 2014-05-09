/*
    Copyright (C) 2010 Casey Link <unnamedrambler@gmail.com>
    Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>
    Copyright (C) 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "kweekdaycheckcombo.h"

#include <KGlobal>
#include <KLocale>
#include <KCalendarSystem>
#include <QDebug>

using namespace KPIM;

KWeekdayCheckCombo::KWeekdayCheckCombo(QWidget* parent,bool first5Checked ): KCheckComboBox( parent )
{
    const KCalendarSystem *calSys = KLocale::global()->calendar();
    const int weekStart = KLocale::global()->weekStartDay();
    QStringList checkedItems;
    for ( int i = 0; i < 7; ++i ) {
        // i is the nr of the combobox, not the day of week!
        const int dayOfWeek = ( i + weekStart + 6 ) % 7;

        const QString weekDayName = calSys->weekDayName( dayOfWeek + 1, KCalendarSystem::ShortDayName );
        addItem( weekDayName );
        // by default Monday - Friday should be checked
        // which corresponds to index 0 - 4;
        if ( first5Checked && dayOfWeek < 5 ) {
            checkedItems << weekDayName;
        }
    }
    if ( first5Checked ) {
        setCheckedItems( checkedItems );
    }
}


KWeekdayCheckCombo::~KWeekdayCheckCombo()
{
}

QBitArray KWeekdayCheckCombo::days() const
{
    QBitArray days( 7 );
    const int weekStart = KLocale::global()->weekStartDay();

    for ( int i = 0; i < 7; ++i ) {
        // i is the nr of the combobox, not the day of week!
        const int index = ( 1 + i +  ( 7 - weekStart ) ) % 7;
        days.setBit( i, itemCheckState( index ) == Qt::Checked );
    }

    return days;
}

int KWeekdayCheckCombo::weekdayIndex( const QDate &date ) const
{
    if ( !date.isValid() )
        return -1;
    const int weekStart = KLocale::global()->weekStartDay();
    const KCalendarSystem *calSys = KLocale::global()->calendar();
    const int dayOfWeek = calSys->dayOfWeek( date ) - 1; // Values 1 - 7, we need 0 - 6

    // qDebug() << "dayOfWeek = " << dayOfWeek << " weekStart = " << weekStart
    // << "; result " << ( ( dayOfWeek + weekStart ) % 7 ) << "; date = " << date;
    return ( 1 + dayOfWeek +  ( 7 - weekStart ) ) % 7;
}

void KWeekdayCheckCombo::setDays( const QBitArray &days,  const QBitArray &disableDays )
{
    Q_ASSERT( count() == 7 ); // The combobox must be filled.

    QStringList checkedDays;
    const int weekStart = KLocale::global()->weekStartDay();
    for ( int i = 0; i < 7; ++i ) {
        // i is the nr of the combobox, not the day of week!
        const int index = ( 1 + i +  ( 7 - weekStart ) ) % 7;

        // qDebug() << "Checking for i = " << i << "; index = " << index << days.testBit( i );
        // qDebug() << "Disabling? for i = " << i << "; index = " << index << !disableDays.testBit( i );

        if ( days.testBit( i ) ) {
            checkedDays << itemText( index );
        }
        if( !disableDays.isEmpty() ) {
            setItemEnabled( index, !disableDays.testBit( i ) );
        }
    }
    setCheckedItems( checkedDays );
}



