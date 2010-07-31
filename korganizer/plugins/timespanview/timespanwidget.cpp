/*
    This file is part of KOrganizer.

    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <tqsplitter.h>
#include <tqlistview.h>
#include <tqlayout.h>
#include <tqheader.h>
#include <tqpushbutton.h>

#include <klocale.h>
#include <kdebug.h>

#include <libkcal/event.h>

#include "lineview.h"
#include "timeline.h"

#include "timespanwidget.h"
#include "timespanwidget.moc"

TimeSpanWidget::TimeSpanWidget( TQWidget *parent, const char *name ) :
  TQWidget( parent, name )
{
  TQBoxLayout *topLayout = new TQVBoxLayout( this );

  mSplitter = new TQSplitter( this );
  topLayout->addWidget( mSplitter );

  mList = new TQListView( mSplitter );
  mList->addColumn( i18n("Summary") );
  
  TQWidget *rightPane = new TQWidget( mSplitter );
  TQBoxLayout *rightPaneLayout = new TQVBoxLayout( rightPane );

  mTimeLine = new TimeLine( rightPane );
  mTimeLine->setFixedHeight( mList->header()->height() );
  rightPaneLayout->addWidget( mTimeLine );
  
  mLineView = new LineView( rightPane );
  rightPaneLayout->addWidget( mLineView );

  TQBoxLayout *buttonLayout = new TQHBoxLayout( rightPaneLayout );
  
  TQPushButton *zoomInButton = new TQPushButton( i18n("Zoom In"), rightPane );
  connect( zoomInButton, TQT_SIGNAL( clicked() ), TQT_SLOT( zoomIn() ) );
  buttonLayout->addWidget( zoomInButton );
  
  TQPushButton *zoomOutButton = new TQPushButton( i18n("Zoom Out"), rightPane );
  connect( zoomOutButton, TQT_SIGNAL( clicked() ), TQT_SLOT( zoomOut() ) );
  buttonLayout->addWidget( zoomOutButton );
  
  TQPushButton *centerButton = new TQPushButton( i18n("Center View"), rightPane );
  connect( centerButton, TQT_SIGNAL( clicked() ), TQT_SLOT( centerView() ) );
  buttonLayout->addWidget( centerButton );

  connect(mLineView->horizontalScrollBar(),TQT_SIGNAL(valueChanged(int)),
          mTimeLine,TQT_SLOT(setContentsPos(int)));
}

TimeSpanWidget::~TimeSpanWidget()
{
}

TQValueList<int> TimeSpanWidget::splitterSizes()
{
  return mSplitter->sizes();
}

void TimeSpanWidget::setSplitterSizes( TQValueList<int> sizes )
{
  mSplitter->setSizes( sizes );
}

void TimeSpanWidget::addItem( KCal::Event *event )
{
  new TQListViewItem( mList, event->summary() );
  
  TQDateTime startDt = event->dtStart();
  TQDateTime endDt = event->dtEnd();

//  kdDebug(5850) << "TimeSpanWidget::addItem(): start: " << startDt.toString()
//            << "  end: " << endDt.toString() << endl;

//  int startSecs = mStartDate.secsTo( startDt );
//  int durationSecs = startDt.secsTo( endDt );
  
//  kdDebug(5850) << "--- startSecs: " << startSecs << "  dur: " << durationSecs << endl;

  int startX = mStartDate.secsTo( startDt ) / mSecsPerPixel;
  int endX = startX + startDt.secsTo( endDt ) / mSecsPerPixel;
  
//  kdDebug(5850) << "TimeSpanWidget::addItem(): s: " << startX << "  e: " << endX << endl;
  
  mLineView->addLine( startX, endX );
}

void TimeSpanWidget::clear()
{
  mList->clear();
  mLineView->clear();
}

void TimeSpanWidget::updateView()
{
#if QT_VERSION >= 300
  mLineView->updateContents();
  mTimeLine->updateContents();
#else
#endif
}

void TimeSpanWidget::setDateRange( const TQDateTime &start, const TQDateTime &end )
{
  mStartDate = start;
  mEndDate = end;
  
  mTimeLine->setDateRange( start, end );

  mSecsPerPixel = mStartDate.secsTo( mEndDate ) / mLineView->pixelWidth();
}

TQDateTime TimeSpanWidget::startDateTime()
{
  return mStartDate;
}

TQDateTime TimeSpanWidget::endDateTime()
{
  return mEndDate;
}

void TimeSpanWidget::zoomIn()
{
  int span = mStartDate.daysTo( mEndDate );
  setDateRange( mStartDate.addDays( span / 4 ), mEndDate.addDays( span / -4 ) );

  emit dateRangeChanged();
}

void TimeSpanWidget::zoomOut()
{
  int span = mStartDate.daysTo( mEndDate );
  setDateRange( mStartDate.addDays( span / -4 ), mEndDate.addDays( span / 4 ) );

  emit dateRangeChanged();
}

void TimeSpanWidget::centerView()
{
  TQScrollBar *scrollBar = mLineView->horizontalScrollBar();
  int min = scrollBar->minValue();
  int max = scrollBar->maxValue();
  scrollBar->setValue( min + (max-min) / 2 );
}
