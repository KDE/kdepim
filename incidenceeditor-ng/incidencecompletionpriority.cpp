/*
  Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
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
#include "ui_dialogmobile.h"
#include "ui_dialogmoremobile.h"
#else
#include "ui_dialogdesktop.h"
#endif

#include <KCalCore/Todo>

using namespace IncidenceEditorNG;

class IncidenceCompletionPriority::Private
{
  IncidenceCompletionPriority *const q;
  public:
    explicit Private( IncidenceCompletionPriority *parent )
      : q( parent ), mUi( 0 ), mOrigPercentCompleted( -1 )
    {
    }

  public:
    Ui::EventOrTodoDesktop *mUi;
    int mOrigPercentCompleted;

  public: // slots
    void sliderValueChanged( int );
};

void IncidenceCompletionPriority::Private::sliderValueChanged( int value )
{
  if ( q->sender() == mUi->mCompletionSlider ) {
    mOrigPercentCompleted = -1;
  }

  mUi->mCompletedLabel->setText( QString( "%1%" ).arg( value ) );
  q->checkDirtyStatus();
}

IncidenceCompletionPriority::IncidenceCompletionPriority( Ui::EventOrTodoDesktop *ui )
  : IncidenceEditor(), d( new Private( this ) )
{
  Q_ASSERT( ui != 0 );
  setObjectName( "IncidenceCompletionPriority" );

  d->mUi = ui;

  d->sliderValueChanged( d->mUi->mCompletionSlider->value() );
  d->mUi->mCompletionPriorityWidget->hide();
  d->mUi->mTaskLabel->hide();
  const QFontMetrics metrics( d->mUi->mCompletedLabel->font() );
  d->mUi->mCompletedLabel->setMinimumWidth( metrics.width( QLatin1String( "100%" ) ) );
#ifndef KDEPIM_MOBILE_UI
  d->mUi->mTaskSeparator->hide();
#endif

#ifdef Q_WS_MAEMO_5
  // The default looks really bad in the editor.
  QPalette palette = d->mUi->mCompletionSlider->palette();
  palette.setColor( QPalette::Window, Qt::white );
  d->mUi->mCompletionSlider->setPalette( palette );
#endif

  connect( d->mUi->mCompletionSlider, SIGNAL(valueChanged(int)), SLOT(sliderValueChanged(int)) );
  connect( d->mUi->mPriorityCombo, SIGNAL(currentIndexChanged(int)), SLOT(checkDirtyStatus()) );
}

IncidenceCompletionPriority::~IncidenceCompletionPriority()
{
  delete d;
}

void IncidenceCompletionPriority::load( const KCalCore::Incidence::Ptr &incidence )
{
  mLoadedIncidence = incidence;

  // TODO priority might be valid for other incidence types as well
  // only for Todos
  KCalCore::Todo::Ptr todo = IncidenceCompletionPriority::incidence<KCalCore::Todo>();
  if ( todo == 0 ) {
    mWasDirty = false;
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
}

bool IncidenceCompletionPriority::isDirty() const
{
  KCalCore::Todo::Ptr todo = IncidenceCompletionPriority::incidence<KCalCore::Todo>();

  if ( !todo ) {
    return false;
  }

  if ( d->mUi->mCompletionSlider->value() != todo->percentComplete() ) {
    return true;
  }

  if ( d->mUi->mPriorityCombo->currentIndex() != todo->priority() ) {
    return true;
  }

  return false;
}

#include "incidencecompletionpriority.moc"
