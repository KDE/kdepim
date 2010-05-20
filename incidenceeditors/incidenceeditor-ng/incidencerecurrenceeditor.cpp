/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/

#include "incidencerecurrenceeditor.h"

#include "ui_incidencerecurrenceeditor.h"

using namespace IncidenceEditorsNG;

IncidenceRecurrenceEditor::IncidenceRecurrenceEditor( QWidget *parent )
  : IncidenceEditor( parent )
  , mUi( new Ui::IncidenceRecurrenceEditor )
{
  mUi->setupUi( this );

  QButtonGroup *timeGroup = new QButtonGroup( this );
  timeGroup->addButton( mUi->mDailyButton, 0 );
  timeGroup->addButton( mUi->mWeeklyButton, 1 );
  timeGroup->addButton( mUi->mMonthlyButton, 2 );
  timeGroup->addButton( mUi->mYearlyButton, 3 );

  connect( timeGroup, SIGNAL(buttonClicked(int)),
           mUi->mRuleStack, SLOT(setCurrentIndex(int)) );
  connect( timeGroup, SIGNAL(buttonClicked(int)),
           SLOT(updateRecerrenceLabel(int)) );
}

void IncidenceRecurrenceEditor::load( KCal::Incidence::ConstPtr incidence )
{

}

void IncidenceRecurrenceEditor::save( KCal::Incidence::Ptr incidence )
{

}

bool IncidenceRecurrenceEditor::isDirty() const
{
  return false;
}

/// Private slots

void IncidenceRecurrenceEditor::updateRecerrenceLabel( int recurrenceRadioIndex )
{
  switch ( recurrenceRadioIndex ) {
    case 0:
      mUi->mRecurrenceRuleLabel->setText( i18nc( "@label recurrence expressed in days", "day(s)" ) );
      break;
    case 1:
      mUi->mRecurrenceRuleLabel->setText( i18nc( "@label", "week(s) on:" ) );
      break;
    case 2:
      mUi->mRecurrenceRuleLabel->setText( i18nc( "@label", "month(s)" ) );
      break;
    case 3:
      mUi->mRecurrenceRuleLabel->setText( i18nc( "@label", "year(s)" ) );
      break;
    default:
      Q_ASSERT( false );
  }
}

