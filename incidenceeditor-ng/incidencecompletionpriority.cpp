/*
    Copyright (C) 2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    Author: Kevin Krammer <krake@kdab.com>

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

#include "incidencecompletionpriority.h"

#ifdef KDEPIM_MOBILE_UI
#include "ui_eventortodomobile.h"
#else
#include "ui_eventortododesktop.h"
#endif

#include <kcalcore/todo.h>

using namespace IncidenceEditorsNG;

class IncidenceCompletionPriority::Private
{
  IncidenceCompletionPriority *const q;
  public:
    explicit Private( IncidenceCompletionPriority *parent )
      : q( parent ), mUi( 0 ), mDirty( false ), mOrigPercentCompleted( -1 )
    {
    }

  public:
    Ui::EventOrTodoDesktop *mUi;

    bool mDirty;
    int mOrigPercentCompleted;

  public: // slots
    void sliderValueChanged( int );
    void slotSetDirty();
};

void IncidenceCompletionPriority::Private::sliderValueChanged( int value )
{
  if ( q->sender() == mUi->mCompletionSlider ) {
    mOrigPercentCompleted = -1;
  }

  mUi->mCompletedLabel->setText( i18n( "%1% completed", value ) );

  slotSetDirty();
}

void IncidenceCompletionPriority::Private::slotSetDirty()
{
    mDirty = true;
    q->checkDirtyStatus();
}


IncidenceCompletionPriority::IncidenceCompletionPriority( Ui::EventOrTodoDesktop *ui )
  : IncidenceEditor()
  , d( new Private( this ) )
{
  Q_ASSERT( ui != 0 );
  setObjectName( "IncidenceCompletionPriority" );
 
  d->mUi = ui;

  d->sliderValueChanged( d->mUi->mCompletionSlider->value() );
  d->mUi->mCompletionPriorityWidget->hide();
  d->mUi->mTaskLabel->hide();
#ifndef KDEPIM_MOBILE_UI
  d->mUi->mTaskSeparator->hide();
#endif

  connect( d->mUi->mCompletionSlider, SIGNAL( valueChanged( int ) ), SLOT( sliderValueChanged( int ) ) );
  connect( d->mUi->mPriorityCombo, SIGNAL( currentIndexChanged( int ) ), SLOT( slotSetDirty() ) );
}

IncidenceCompletionPriority::~IncidenceCompletionPriority()
{
  delete d;
}

void IncidenceCompletionPriority::load( const KCalCore::Incidence::ConstPtr &incidence )
{
  mLoadedIncidence = incidence;

  // TODO priority might be valid for other incidence types as well
  // only for Todos
  KCalCore::Todo::ConstPtr todo = IncidenceCompletionPriority::incidence<KCalCore::Todo>();
  if ( todo == 0 ) {
    d->mDirty = false;
    return;
  }

  d->mUi->mCompletionPriorityWidget->show();
  d->mUi->mTaskLabel->show();
#ifndef KDEPIM_MOBILE_UI
  d->mUi->mTaskSeparator->show();
#endif

  d->mOrigPercentCompleted = todo->percentComplete();
  d->mUi->mCompletionSlider->blockSignals( true );
  d->mUi->mCompletionSlider->setValue( todo->percentComplete() );
  d->sliderValueChanged( d->mUi->mCompletionSlider->value() );
  d->mUi->mCompletionSlider->blockSignals( false );
  
  d->mUi->mPriorityCombo->blockSignals( true );
  d->mUi->mPriorityCombo->setCurrentIndex( todo->priority() );
  d->mUi->mPriorityCombo->blockSignals( false );
  
  d->mDirty = false;
  mWasDirty = false;
}

void IncidenceCompletionPriority::save( const KCalCore::Incidence::Ptr &incidence )
{
  // TODO priority might be valid for other incidence types as well
  // only for Todos
  KCalCore::Todo::Ptr todo = IncidenceCompletionPriority::incidence<KCalCore::Todo>( incidence );
  if ( todo == 0 ) {
    return;
  }

  // we only have multiples of ten on our combo. If the combo did not change its value,
  // see if we have an original value to restore
  if ( d->mOrigPercentCompleted != -1 ) {
    todo->setPercentComplete( d->mOrigPercentCompleted );
  } else {
    todo->setPercentComplete( d->mUi->mCompletionSlider->value() );
  }
  todo->setPriority( d->mUi->mPriorityCombo->currentIndex() );

  // TODO reset dirty?
}

bool IncidenceCompletionPriority::isDirty() const
{
  return d->mDirty;
}

#include "incidencecompletionpriority.moc"
