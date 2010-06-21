#include "incidencerecurrence.h"

#include "ui_eventortododesktop.h"

using namespace IncidenceEditorsNG;

IncidenceRecurrence::IncidenceRecurrence( Ui::EventOrTodoDesktop *ui )
  : mUi( ui )
{
  toggleRecurrenceWidgets( false );

  connect( mUi->mTypeCombo, SIGNAL(currentIndexChanged(int)),
           SLOT(handleRecurrenceTypeChange(int)));
}

void IncidenceRecurrence::load( KCal::Incidence::ConstPtr incidence )
{

}

void IncidenceRecurrence::save( KCal::Incidence::Ptr incidence )
{

}

bool IncidenceRecurrence::isDirty() const
{
  return false;
}

bool IncidenceRecurrence::isValid()
{
  return true;
}

void IncidenceRecurrence::handleRecurrenceTypeChange( int currentIndex )
{
  toggleRecurrenceWidgets( currentIndex > 0 );
}

void IncidenceRecurrence::toggleRecurrenceWidgets( bool enable )
{
  mUi->mFrequencyLabel->setVisible( enable );
  mUi->mFrequencyEdit->setVisible( enable );
  mUi->mRecurrenceRuleLabel->setVisible( enable );
  mUi->mRepeatStack->setVisible( enable && mUi->mTypeCombo->currentIndex() > 1 );
  mUi->mRecurrenceEndLabel->setVisible( enable );
  mUi->mRecurrenceEndCombo->setVisible( enable );
  mUi->mEndDurationEdit->setVisible( enable );
  mUi->mRecurrenceEndStack->setVisible( enable );

  // Exceptions widgets
  mUi->mExceptionsLabel->setVisible( enable );
  mUi->mExceptionDateEdit->setVisible( enable );
  mUi->mExceptionAddButton->setVisible( enable );
  mUi->mExceptionList->setVisible( enable );
  mUi->mExceptionSeperator->setVisible( enable );
}
