/*
    Copyright (C) 2010 Casey Link <unnamedrambler@gmail.com>
    Copyright (C) 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

    Refactored from earlier code by:
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>
    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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
#include "multiplyinglineview_p.h"

#include <KDebug>

#include <QApplication>
#include <QLayout>
#include <QVBoxLayout>
#include <QTimer>
#include <QScrollBar>
#include <QResizeEvent>

using namespace KPIM;
  
MultiplyingLineView::MultiplyingLineView( MultiplyingLineFactory* factory, MultiplyingLineEditor *parent )
  : QScrollArea( parent ), mCurDelLine( 0 ),
    mLineHeight( 0 ), mFirstColumnWidth( 0 ),
    mModified( false ), mCompletionMode( KGlobalSettings::completionMode() ),
    mPage( new QWidget( this ) ), mTopLayout( new QVBoxLayout( this ) ),
    mMultiplyingLineFactory( factory )
{
  setWidgetResizable( true );
  setFrameStyle( QFrame::NoFrame );

  mPage->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
  setWidget( mPage );

  mTopLayout->setMargin( 0 );
  mTopLayout->setSpacing( 0 );
  mPage->setLayout( mTopLayout );
}

MultiplyingLine* MultiplyingLineView::activeLine() const
{
  return mLines.last();
}

MultiplyingLine* MultiplyingLineView::emptyLine() const
{
  MultiplyingLine* line;
  foreach( line, mLines ) {
    if ( line->isEmpty() )
      return line;
  }
  return 0;
}

MultiplyingLine* MultiplyingLineView::addLine()
{
  MultiplyingLine* line = mMultiplyingLineFactory->newLine( widget() );

  mTopLayout->addWidget( line );
  line->setCompletionMode( mCompletionMode );
  line->show();
  connect( line, SIGNAL( returnPressed( MultiplyingLine * ) ),
    SLOT( slotReturnPressed( MultiplyingLine * ) ) );
  connect( line, SIGNAL( upPressed( MultiplyingLine * ) ),
    SLOT( slotUpPressed( MultiplyingLine * ) ) );
  connect( line, SIGNAL( downPressed( MultiplyingLine * ) ),
    SLOT( slotDownPressed( MultiplyingLine * ) ) );
  connect( line, SIGNAL( rightPressed() ), SIGNAL( focusRight() ) );
  connect( line, SIGNAL( deleteLine( MultiplyingLine * ) ),
    SLOT( slotDecideLineDeletion( MultiplyingLine * ) ) );
  connect( line, SIGNAL( completionModeChanged( KGlobalSettings::Completion ) ),
    SLOT( setCompletionMode( KGlobalSettings::Completion ) ) );

  if( !mLines.isEmpty() ) {
    line->fixTabOrder( mLines.last()->tabOut() );
  }
  mLines.append( line );
  mFirstColumnWidth = line->setColumnWidth( mFirstColumnWidth );
  mLineHeight = line->minimumSizeHint().height();
  line->resize( viewport()->width(), mLineHeight );
  resizeView();
  ensureVisible( 0, mLines.count() * mLineHeight, 0, 0 );

  QTimer::singleShot( 0, this, SLOT(moveScrollBarToEnd()) );

  emit lineAdded( line );
  return line;
}


void MultiplyingLineView::moveScrollBarToEnd()
{
// scroll to bottom
   verticalScrollBar()->triggerAction( QAbstractSlider::SliderToMaximum );
}


void MultiplyingLineView::slotReturnPressed( MultiplyingLine *line )
{
  if ( !line->data()->isEmpty() ) {
    MultiplyingLine *empty = emptyLine();
    if ( !empty ) empty = addLine();
    activateLine( empty );
  }
}


void MultiplyingLineView::slotDownPressed( MultiplyingLine *line )
{
  int pos = mLines.indexOf( line );
  if ( pos >= (int)mLines.count() - 1 ) {
    emit focusDown();
  } else if ( pos >= 0 ) {
    activateLine( mLines.at( pos + 1 ) );
  }
}


void MultiplyingLineView::slotUpPressed( MultiplyingLine *line )
{
  int pos = mLines.indexOf( line );
  if ( pos > 0 ) {
    activateLine( mLines.at( pos - 1 ) );
  } else {
    emit focusUp();
  }
}


void MultiplyingLineView::slotDecideLineDeletion( MultiplyingLine *line )
{
  if ( !line->isEmpty() )
    mModified = true;
  if ( mLines.count() == 1 ) {
    line->clear();
  } else if ( mLines.indexOf( line ) != mLines.count() - 1 ) {
    mCurDelLine = line;
    QTimer::singleShot( 0, this, SLOT( slotDeleteLine( ) ) );
  }
}


void MultiplyingLineView::slotDeleteLine()
{
  if ( !mCurDelLine )
    return;

  MultiplyingLine *line = mCurDelLine;
  int pos = mLines.indexOf( line );

  if ( mCurDelLine->isActive() ) {
    int newPos;
    if ( pos == 0 )
      newPos = pos + 1;
    else
      newPos = pos - 1;

    // if there is something left to activate, do so
    if ( mLines.at( newPos ) )
      mLines.at( newPos )->activate();
  }

  mLines.removeAll( line );
  line->setParent( 0 );
  delete line;

  emit lineDeleted( pos );

  resizeView();
}

void MultiplyingLineView::resizeView()
{
  if ( mLines.count() < 6 ) {
    setMinimumHeight( mLineHeight * mLines.count() );
  } else {
    setMinimumHeight( mLineHeight * 5 );
    setMaximumHeight( mLineHeight * mLines.count() );
  }

  parentWidget()->layout()->activate();
  emit sizeHintChanged();
  QTimer::singleShot( 0, this, SLOT( moveCompletionPopup() ) );
}

void MultiplyingLineView::activateLine( MultiplyingLine *line )
{
  line->activate();
  ensureWidgetVisible( line );
}

void MultiplyingLineView::resizeEvent ( QResizeEvent *ev )
{
  QScrollArea::resizeEvent(ev);
  for( int i = 0; i < mLines.count(); ++i ) {
    mLines.at( i )->resize( ev->size().width(), mLineHeight );
  }
  ensureVisible( 0, mLines.count() * mLineHeight, 0, 0 );
}

QSize MultiplyingLineView::sizeHint() const
{
  return QSize( 200, mLineHeight * mLines.count() );
}

QSize MultiplyingLineView::minimumSizeHint() const
{
  int height;
  int numLines = 5;
  if ( mLines.count() < numLines ) height = mLineHeight * mLines.count();
  else height = mLineHeight * numLines;
  return QSize( 200, height );
}

QList<MultiplyingLineData::Ptr> MultiplyingLineView::allData() const
{
  QList<MultiplyingLineData::Ptr> data;

  QListIterator<MultiplyingLine*> it( mLines );
  MultiplyingLine *line;
  while( it.hasNext()) {
    line = it.next();
    if ( !line->data()->isEmpty() ) {
      data.append( line->data() );
    }
  }

  return data;
}

void MultiplyingLineView::setCompletionMode ( KGlobalSettings::Completion mode )
{
  if ( mCompletionMode == mode )
    return;
  mCompletionMode = mode;

  QListIterator<MultiplyingLine*> it( mLines );
  while( it.hasNext() ) {
    MultiplyingLine *line = it.next();
    line->blockSignals( true );
    line->setCompletionMode( mode );
    line->blockSignals( false );
  }
  emit completionModeChanged( mode ); //report change to MultiplyingLineEditor
}

void MultiplyingLineView::removeData( const MultiplyingLineData::Ptr &data )
{
    // search a line which matches recipient and type
  QListIterator<MultiplyingLine*> it( mLines );
  MultiplyingLine *line = 0;
  while (it.hasNext()) {
    line = it.next();
    if ( line->data() == data)
      break;
  }
  if ( line )
    line->slotPropagateDeletion();
}

bool MultiplyingLineView::isModified() const
{
  if ( mModified )
    return true;

  QListIterator<MultiplyingLine*> it( mLines );
  MultiplyingLine *line;
  while( it.hasNext()) {
  line = it.next();
    if ( line->isModified() ) {
      return true;
    }
  }

  return false;
}

void MultiplyingLineView::clearModified()
{
  mModified = false;

  QListIterator<MultiplyingLine*> it( mLines );
  MultiplyingLine *line;
  while( it.hasNext() ) {
  line = it.next();
    line->clearModified();
  }
}

void MultiplyingLineView::setFocus()
{
  if ( !mLines.empty() && mLines.last()->isActive() )
    setFocusBottom();
  else
    setFocusTop();
}

void MultiplyingLineView::setFocusTop()
{
  if ( !mLines.empty() ) {
    MultiplyingLine *line = mLines.first();
    if ( line )
      line->activate();
    else
      kWarning() <<"No first";
  }
  else
    kWarning() <<"No first";
}

void MultiplyingLineView::setFocusBottom()
{
  MultiplyingLine *line = mLines.last();
  if ( line ) {
    ensureWidgetVisible( line );
    line->activate();
  }
  else
    kWarning() <<"No last";
}

int MultiplyingLineView::setFirstColumnWidth( int w )
{
  mFirstColumnWidth = w;

  QListIterator<MultiplyingLine*> it( mLines );
  MultiplyingLine *line;
  while(it.hasNext()) {
    line = it.next();
    mFirstColumnWidth = line->setColumnWidth( mFirstColumnWidth );
  }

  resizeView();
  return mFirstColumnWidth;
}

void MultiplyingLineView::moveCompletionPopup()
{
  foreach ( MultiplyingLine *const line, mLines )
    line->moveCompletionPopup();
}

QList< MultiplyingLine* > MultiplyingLineView::lines() const
{
  return mLines;
}


#include "multiplyinglineview_p.moc"
