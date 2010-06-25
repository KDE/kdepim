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

#include <KCal/Todo>

using namespace IncidenceEditorsNG;

class IncidenceCompletionPriority::Private
{
  IncidenceCompletionPriority *const q;
  public:
    explicit Private( IncidenceCompletionPriority *parent ) : q( parent ), mUi( 0 ), mDirty( false )
    {
    }

  public:
    Ui::EventOrTodoDesktop *mUi;

    bool mDirty;

  public: // slots
    void comboValueChanged();
};

void IncidenceCompletionPriority::Private::comboValueChanged()
{
  mDirty = true;
  q->checkDirtyStatus();
}

IncidenceCompletionPriority::IncidenceCompletionPriority( Ui::EventOrTodoDesktop *ui )
  : IncidenceEditor()
  , d( new Private( this ) )
{
  Q_ASSERT( ui != 0 );
 
  d->mUi = ui;

  d->mUi->mCompletionPriorityWidget->hide();

  connect( d->mUi->mCompletionCombo, SIGNAL( currentIndexChanged( int ) ), SLOT( comboValueChanged() ) );
  connect( d->mUi->mPriorityCombo, SIGNAL( currentIndexChanged( int ) ), SLOT( comboValueChanged() ) );
}

IncidenceCompletionPriority::~IncidenceCompletionPriority()
{
  delete d;
}

void IncidenceCompletionPriority::load( KCal::Incidence::ConstPtr incidence )
{
  mLoadedIncidence = incidence;

  // TODO priority might be valid for other incidence types as well
  // only for Todos
  KCal::Todo::ConstPtr todo = IncidenceCompletionPriority::incidence<KCal::Todo>();
  if ( todo == 0 ) {
    return;
  }

  d->mUi->mCompletionPriorityWidget->show();

  d->mUi->mCompletionCombo->blockSignals( true );
  d->mUi->mCompletionCombo->setCurrentIndex( todo->percentComplete() % 10 );
  d->mUi->mCompletionCombo->blockSignals( false );
  
  d->mUi->mPriorityCombo->blockSignals( true );
  d->mUi->mPriorityCombo->setCurrentIndex( todo->priority() );
  d->mUi->mPriorityCombo->blockSignals( false );
  
  d->mDirty = false;
  mWasDirty = false;
}

void IncidenceCompletionPriority::save( KCal::Incidence::Ptr incidence )
{
  // TODO priority might be valid for other incidence types as well
  // only for Todos
  KCal::Todo::Ptr todo = IncidenceCompletionPriority::incidence<KCal::Todo>( incidence );
  if ( todo == 0 ) {
    return;
  }

  todo->setPercentComplete( d->mUi->mCompletionCombo->currentIndex() * 10 );
  todo->setPriority( d->mUi->mPriorityCombo->currentIndex() );

  // TODO reset dirty?
}

bool IncidenceCompletionPriority::isDirty() const
{
  return d->mDirty;
}

#include "incidencecompletionpriority.moc"
