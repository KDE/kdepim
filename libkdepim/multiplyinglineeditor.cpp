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

#include "multiplyinglineeditor.h"
#include "multiplyinglineview_p.h"

#include <KDebug>
#include <KDialog>

#include <QHBoxLayout>
using namespace KPIM;
MultiplyingLineEditor::MultiplyingLineEditor( MultiplyingLineFactory* factory, QWidget *parent )
  : QWidget( parent ), mMultiplyingLineFactory( factory ),  mModified( false )
{
  QBoxLayout *topLayout = new QHBoxLayout();
  topLayout->setSpacing( KDialog::spacingHint() );
  topLayout->setMargin( 0 );
  setLayout( topLayout );

  mView = new MultiplyingLineView( mMultiplyingLineFactory, this );
  topLayout->addWidget( mView );
  connect( mView, SIGNAL( focusUp() ), SIGNAL( focusUp() ) );
  connect( mView, SIGNAL( focusDown() ), SIGNAL( focusDown() ) );
  connect( mView, SIGNAL( completionModeChanged( KGlobalSettings::Completion ) ),
    SIGNAL( completionModeChanged( KGlobalSettings::Completion ) ) );
  connect( mView, SIGNAL( lineDeleted( int ) ), SIGNAL( lineDeleted( int ) ) );
  connect( mView, SIGNAL( lineAdded( KPIM::MultiplyingLine* ) ), SIGNAL( lineAdded( KPIM::MultiplyingLine* ) ) );
  connect( mView, SIGNAL( sizeHintChanged() ),
           SIGNAL( sizeHintChanged() ) );
}

MultiplyingLineEditor::~MultiplyingLineEditor()
{
}

void MultiplyingLineEditor::addData( const MultiplyingLineData::Ptr &data )
{
  MultiplyingLine* line = mView->emptyLine();
  if( !line )
    line = mView->addLine();
  if( data )
    line->setData( data );
}

void MultiplyingLineEditor::removeData( const MultiplyingLineData::Ptr &data )
{
  mView->removeData( data );
}

void MultiplyingLineEditor::clear()
{
}

bool MultiplyingLineEditor::isModified()
{
  return mModified || mView->isModified();
}

void MultiplyingLineEditor::clearModified()
{
  mModified = false;
  mView->clearModified();
}

void MultiplyingLineEditor::setFocus()
{
  mView->setFocus();
}

void MultiplyingLineEditor::setFocusTop()
{
  mView->setFocusTop();
}

void MultiplyingLineEditor::setFocusBottom()
{
  mView->setFocusBottom();
}

int MultiplyingLineEditor::setFirstColumnWidth( int w )
{
  return mView->setFirstColumnWidth( w );
}

void MultiplyingLineEditor::setCompletionMode( KGlobalSettings::Completion mode )
{
  mView->setCompletionMode( mode );
}

MultiplyingLineFactory* MultiplyingLineEditor::factory() const
{
  return mMultiplyingLineFactory;
}

QList< MultiplyingLineData::Ptr > MultiplyingLineEditor::allData() const
{
  return mView->allData();
}

MultiplyingLineData::Ptr MultiplyingLineEditor::activeData() const
{
  return mView->activeLine()->data();
}


QList< MultiplyingLine* > MultiplyingLineEditor::lines() const
{
  return mView->lines();
}

MultiplyingLine* MultiplyingLineEditor::activeLine() const
{
  return mView->activeLine();
}



#include "multiplyinglineeditor.moc"
