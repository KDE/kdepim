/*
    Copyright (C) 2010 Casey Link <unnamedrambler@gmail.com>
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

#include "schedulingdialog.h"

#include "conflictresolver.h"

#include <KCalendarSystem>
#include <KIconLoader>
#include <KLocale>
#include <KDebug>
#include <KCal/Attendee>

using namespace IncidenceEditorsNG;

SchedulingDialog::SchedulingDialog( ConflictResolver* resolver ) : KDialog(), mResolver( resolver )
{
    setupUi( this );
    fillCombos();

    connect( mStartDate, SIGNAL( dateChanged( QDate ) ), mResolver, SLOT( setEarliestStartDate( QDate ) ) );
    connect( mStartTime, SIGNAL( timeChanged( QTime ) ), mResolver, SLOT( setEarliestStartTime( QTime ) ) );
    connect( mEndDate, SIGNAL( dateChanged( QDate ) ), mResolver, SLOT( setLatestEndDate( QDate ) ) );
    connect( mEndTime, SIGNAL( timeChanged( QTime ) ), mResolver, SLOT( setLatestEndTime( QTime ) ) );

    connect( mStartDate, SIGNAL( dateChanged( QDate ) ), this, SLOT( slotStartDateChanged( QDate ) ) );

    connect( mWeekdayCombo, SIGNAL( checkedItemsChanged( QStringList ) ), SLOT( slotWeekdaysChanged( QStringList ) ) );
}


SchedulingDialog::~SchedulingDialog()
{

}

void SchedulingDialog::fillCombos()
{
#ifdef KDEPIM_MOBILE_UI
    mRolesCombo->addItem( DesktopIcon( "meeting-participant", 48 ),
                          KCal::Attendee::roleName( KCal::Attendee::ReqParticipant ) );
    mRolesCombo->addItem( DesktopIcon( "meeting-participant-optional", 48 ),
                          KCal::Attendee::roleName( KCal::Attendee::OptParticipant ) );
    mRolesCombo->addItem( DesktopIcon( "meeting-observer", 48 ),
                          KCal::Attendee::roleName( KCal::Attendee::NonParticipant ) );
    mRolesCombo->addItem( DesktopIcon( "meeting-chair", 48 ),
                          KCal::Attendee::roleName( KCal::Attendee::Chair ) );

#else
    mRolesCombo->addItem( SmallIcon( "meeting-participant" ),
                          KCal::Attendee::roleName( KCal::Attendee::ReqParticipant ) );
    mRolesCombo->addItem( SmallIcon( "meeting-participant-optional" ),
                          KCal::Attendee::roleName( KCal::Attendee::OptParticipant ) );
    mRolesCombo->addItem( SmallIcon( "meeting-observer" ),
                          KCal::Attendee::roleName( KCal::Attendee::NonParticipant ) );
    mRolesCombo->addItem( SmallIcon( "meeting-chair" ),
                          KCal::Attendee::roleName( KCal::Attendee::Chair ) );

#endif
}

void SchedulingDialog::slotStartDateChanged( const QDate& newDate )
{
    QDate oldDate = mStDate;
    mStDate = newDate;
    if ( newDate.isValid() && oldDate.isValid() )
        updateWeekDays( oldDate );
}

void SchedulingDialog::updateWeekDays( const QDate& oldDate )
{
    const int oldStartDayIndex = mWeekdayCombo->weekdayIndex( oldDate );
    const int newStartDayIndex = mWeekdayCombo->weekdayIndex( mStDate );

    mWeekdayCombo->setItemCheckState( oldStartDayIndex, Qt::Unchecked );
    mWeekdayCombo->setItemEnabled( oldStartDayIndex, true );
    mWeekdayCombo->setItemCheckState( newStartDayIndex, Qt::Checked );
    mWeekdayCombo->setItemEnabled( newStartDayIndex, false );
}

void SchedulingDialog::slotWeekdaysChanged( const QStringList& checkedItems )
{

}

