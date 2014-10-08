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
#include "freebusymodel/freeperiodmodel.h"
#ifndef KDEPIM_MOBILE_UI
#include "visualfreebusywidget.h"
#endif
#include <KCalUtils/Stringify>

#include <KCalendarSystem>

using namespace IncidenceEditorNG;

SchedulingDialog::SchedulingDialog( const QDate &startDate, const QTime &startTime, int duration,
                                    ConflictResolver *resolver, QWidget *parent )
  : KDialog( parent ), mResolver( resolver ), mPeriodModel( new FreePeriodModel( this ) )
{
  QWidget *w = new QWidget( this );
  setupUi( w );
  setMainWidget( w );
  fillCombos();

  Q_ASSERT( duration > 0 );
  mDuration = duration;

#ifndef KDEPIM_MOBILE_UI
  mVisualWidget = new VisualFreeBusyWidget( resolver->model(), 8, this );
  QVBoxLayout *ganttlayout = new QVBoxLayout( mGanttTab );

  mGanttTab->setLayout( ganttlayout );
  ganttlayout->addWidget( mVisualWidget );
#endif

  connect( mStartDate, SIGNAL(dateEdited(QDate)),
           mResolver, SLOT(setEarliestDate(QDate)) );
  connect( mStartTime, SIGNAL(timeEdited(QTime)),
           mResolver, SLOT(setEarliestTime(QTime)) );
  connect( mEndDate, SIGNAL(dateEdited(QDate)),
           mResolver, SLOT(setLatestDate(QDate)) );
  connect( mEndTime, SIGNAL(timeEdited(QTime)),
           mResolver, SLOT(setLatestTime(QTime)) );

  connect( mStartDate, SIGNAL(dateEdited(QDate)),
           this, SLOT(slotStartDateChanged(QDate)) );

  connect( mWeekdayCombo, SIGNAL(checkedItemsChanged(QStringList)),
           SLOT(slotWeekdaysChanged()) );
  connect( mWeekdayCombo, SIGNAL(checkedItemsChanged(QStringList)),
           SLOT(slotMandatoryRolesChanged()) );

  connect( mResolver, SIGNAL(freeSlotsAvailable(KCalCore::Period::List)),
           mPeriodModel, SLOT(slotNewFreePeriods(KCalCore::Period::List)) );
  connect( mMoveBeginTimeEdit, SIGNAL(timeEdited(QTime)),
           this, SLOT(slotSetEndTimeLabel(QTime)) );

  mTableView->setModel( mPeriodModel );
  connect( mTableView->selectionModel(),
           SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
           this,
           SLOT(slotRowSelectionChanged(QModelIndex,QModelIndex)) );

  mStartDate->setDate( startDate );
  mEndDate->setDate( mStartDate->date().addDays( 7 ) );
  mStartTime->setTime( startTime );
  mEndTime->setTime( startTime );

  mResolver->setEarliestDate( mStartDate->date() );
  mResolver->setEarliestTime( mStartTime->time() );
  mResolver->setLatestDate( mEndDate->date() );
  mResolver->setLatestTime( mEndTime->time() );

  mMoveApptGroupBox->hide();
}

SchedulingDialog::~SchedulingDialog()
{
}

void SchedulingDialog::slotUpdateIncidenceStartEnd( const KDateTime &startDateTime,
                                                    const KDateTime &endDateTime )
{
#ifdef KDEPIM_MOBILE_UI
  Q_UNUSED( startDateTime );
  Q_UNUSED( endDateTime );
#else
  mVisualWidget->slotUpdateIncidenceStartEnd( startDateTime, endDateTime );
#endif
}

void SchedulingDialog::fillCombos()
{
// Note: we depend on the following order
#ifdef KDEPIM_MOBILE_UI
  mRolesCombo->addItem( DesktopIcon( "meeting-participant", 48 ),
                        KCalUtils::Stringify::attendeeRole( KCalCore::Attendee::ReqParticipant ) );
  mRolesCombo->addItem( DesktopIcon( "meeting-participant-optional", 48 ),
                        KCalUtils::Stringify::attendeeRole( KCalCore::Attendee::OptParticipant ) );
  mRolesCombo->addItem( DesktopIcon( "meeting-observer", 48 ),
                        KCalUtils::Stringify::attendeeRole( KCalCore::Attendee::NonParticipant ) );
  mRolesCombo->addItem( DesktopIcon( "meeting-chair", 48 ),
                        KCalUtils::Stringify::attendeeRole( KCalCore::Attendee::Chair ) );

#else
  mRolesCombo->addItem( SmallIcon( "meeting-participant" ),
                        KCalUtils::Stringify::attendeeRole( KCalCore::Attendee::ReqParticipant ) );
  mRolesCombo->addItem( SmallIcon( "meeting-participant-optional" ),
                        KCalUtils::Stringify::attendeeRole( KCalCore::Attendee::OptParticipant ) );
  mRolesCombo->addItem( SmallIcon( "meeting-observer" ),
                        KCalUtils::Stringify::attendeeRole( KCalCore::Attendee::NonParticipant ) );
  mRolesCombo->addItem( SmallIcon( "meeting-chair" ),
                        KCalUtils::Stringify::attendeeRole( KCalCore::Attendee::Chair ) );

#endif
  mRolesCombo->setWhatsThis( i18nc( "@info:whatsthis",
                                    "Edits the role of the attendee." ) );

  mRolesCombo->setItemCheckState( 0, Qt::Checked );
  mRolesCombo->setItemCheckState( 1, Qt::Checked );
  mRolesCombo->setItemCheckState( 2, Qt::Checked );
  mRolesCombo->setItemCheckState( 3, Qt::Checked );

  QBitArray days( 7 );
  days.setBit( 0 ); //Monday
  days.setBit( 1 ); //Tuesday
  days.setBit( 2 ); //Wednesday
  days.setBit( 3 ); //Thursday
  days.setBit( 4 ); //Friday.. surprise!

  mWeekdayCombo->setDays( days );
  mResolver->setAllowedWeekdays( days );
}

void SchedulingDialog::slotStartDateChanged( const QDate &newDate )
{
  QDate oldDate = mStDate;
  mStDate = newDate;
  if ( newDate.isValid() && oldDate.isValid() ) {
    updateWeekDays( oldDate );
  }
}

void SchedulingDialog::updateWeekDays( const QDate &oldDate )
{
  const int oldStartDayIndex = mWeekdayCombo->weekdayIndex( oldDate );
  const int newStartDayIndex = mWeekdayCombo->weekdayIndex( mStDate );

  mWeekdayCombo->setItemCheckState( oldStartDayIndex, Qt::Unchecked );
  mWeekdayCombo->setItemEnabled( oldStartDayIndex, true );
  mWeekdayCombo->setItemCheckState( newStartDayIndex, Qt::Checked );
  mWeekdayCombo->setItemEnabled( newStartDayIndex, false );
}

void SchedulingDialog::slotWeekdaysChanged()
{
  // notify the resolver
  mResolver->setAllowedWeekdays( mWeekdayCombo->days() );
}

void SchedulingDialog::slotMandatoryRolesChanged()
{
  QSet<KCalCore::Attendee::Role> roles;
  for ( int i = 0; i < mRolesCombo->count(); ++i ) {
    if ( mRolesCombo->itemCheckState( i ) == Qt::Checked ) {
      roles << KCalCore::Attendee::Role( i );
    }
  }
  mResolver->setMandatoryRoles( roles );
}

void SchedulingDialog::slotRowSelectionChanged( const QModelIndex &current,
                                                const QModelIndex &previous )
{
  Q_UNUSED( previous );
  if ( !current.isValid() ) {
    mMoveApptGroupBox->hide();
    return;
  }
  KCalCore::Period period = current.data( FreePeriodModel::PeriodRole ).value<KCalCore::Period>();
  const QDate startDate = period.start().date();

  const KCalendarSystem *calSys = KGlobal::locale()->calendar();
  const int dayOfWeek = calSys->dayOfWeek( startDate );
  const QString dayLabel =
    ki18nc( "@label Day of week followed by day of the month, then the month. "
            "Example: Monday, 12 June",
            "%1, %2 %3" ).
    subs( calSys->weekDayName( dayOfWeek, KCalendarSystem::LongDayName ) ).
    subs( startDate.day() ).
    subs( calSys->monthName( startDate ) ).toString();

  mMoveDayLabel->setText( dayLabel );
  mMoveBeginTimeEdit->setTimeRange( period.start().time(),
                                    period.end().addSecs( -mDuration ).time() );
  mMoveBeginTimeEdit->setTime( period.start().time() );
  slotSetEndTimeLabel( period.start().time() );
  mMoveApptGroupBox->show();

  mSelectedDate = startDate;
}

void SchedulingDialog::slotSetEndTimeLabel( const QTime &startTime )
{
  const QTime endTime = startTime.addSecs( mDuration );
  const QString endTimeLabel =
    ki18nc( "@label This is a suffix following a time selecting widget. "
            "Example: [timeedit] to 10:00am",
            "to %1" ).subs( KGlobal::locale()->formatTime( endTime ) ).toString();

  mMoveEndTimeLabel->setText( endTimeLabel );
  mSelectedTime = startTime;
}

QDate SchedulingDialog::selectedStartDate() const
{
  return mSelectedDate;
}

QTime SchedulingDialog::selectedStartTime() const
{
  return mSelectedTime;
}
