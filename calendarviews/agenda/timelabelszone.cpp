/*
  This file is part of KOrganizer.

  Copyright (c) 2007 Bruno Virlet <bruno@virlet.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "timelabelszone.h"
#include "prefs.h"
#include "agenda.h"
#include "agendaview.h"

#include <ksystemtimezone.h>

#include <QScrollBar>
#include <QScrollArea>
#include <QHBoxLayout>

using namespace EventViews;

TimeLabelsZone::TimeLabelsZone( QWidget *parent, EventView *eventView, Agenda *agenda )
  : QWidget( parent ), mAgenda( agenda ), mEventView( eventView ),
    mParent( dynamic_cast<AgendaView*>( parent ) )
{
  mTimeLabelsLayout = new QHBoxLayout( this );
  mTimeLabelsLayout->setMargin( 0 );
  mTimeLabelsLayout->setSpacing( 0 );

  init();
}

void TimeLabelsZone::reset()
{
  foreach ( QScrollArea *label, mTimeLabelsList ) {
    label->hide();
    label->deleteLater();
  }
  mTimeLabelsList.clear();

  init();

  // Update some related geometry from the agenda view
  updateAll();
  if ( mParent ) {
    mParent->createDayLabels();
    mParent->updateTimeBarWidth();
  }
}

void TimeLabelsZone::init()
{
  addTimeLabels( mEventView->preferences()->timeSpec() );

  foreach ( const QString &zoneStr, mEventView->preferences()->timeScaleTimezones() ) {
    KTimeZone zone = KSystemTimeZones::zone( zoneStr );
    if ( zone.isValid() ) {
      addTimeLabels( zone );
    }
  }
}

void TimeLabelsZone::addTimeLabels( const KDateTime::Spec &spec )
{
  QScrollArea *area = new QScrollArea( this );
  TimeLabels *labels = new TimeLabels( spec, 24, mEventView, this );
  mTimeLabelsList.prepend( area );
  area->setWidget( labels );
  area->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  area->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  area->setBackgroundRole( QPalette::Window );
  area->setFrameStyle( QFrame::NoFrame );
  area->show();
  mTimeLabelsLayout->insertWidget( 0, area );

  setupTimeLabel( area );
}

void TimeLabelsZone::setupTimeLabel( QScrollArea *area )
{
  if ( mAgenda && mAgenda->verticalScrollBar() ) {
    connect( mAgenda->verticalScrollBar(), SIGNAL(valueChanged(int)),
             area->verticalScrollBar(), SLOT(setValue(int)) );

    TimeLabels *timeLabels = static_cast<TimeLabels*>( area->widget() );
    timeLabels->setAgenda( mAgenda );
    area->verticalScrollBar()->setValue( mAgenda->verticalScrollBar()->value() );

  }
  if ( mParent ) {
    connect( area->verticalScrollBar(), SIGNAL(valueChanged(int)),
             mParent, SLOT(setContentsPos(int)) );
  }
}

int TimeLabelsZone::timeLabelsWidth() const
{
  if ( mTimeLabelsList.isEmpty() ) {
    return 0;
  } else {
    return mTimeLabelsList.first()->widget()->width() * mTimeLabelsList.count();
  }
}

void TimeLabelsZone::updateAll()
{
  foreach ( QScrollArea *area, mTimeLabelsList ) {
    TimeLabels *timeLabel = static_cast<TimeLabels*>( area->widget() );
    timeLabel->updateConfig();
  }
}

void TimeLabelsZone::setTimeLabelsWidth( int width )
{
  foreach ( QScrollArea *timeLabel, mTimeLabelsList ) {
    timeLabel->setFixedWidth( width / mTimeLabelsList.count() );
  }
}

QList<QScrollArea*> TimeLabelsZone::timeLabels() const
{
  return mTimeLabelsList;
}

void TimeLabelsZone::setAgendaView( AgendaView *agenda )
{
  mAgenda = agenda->agenda();
  mParent = agenda;
  foreach ( QScrollArea *timeLabel, mTimeLabelsList ) {
    setupTimeLabel( timeLabel );
  }
}

#include "timelabelszone.moc"
