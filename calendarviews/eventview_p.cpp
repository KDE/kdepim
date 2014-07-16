/*
  Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
  Author: Kevin Krammer, krake@kdab.com
  Author: Sergio Martins, sergio.martins@kdab.com

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

#include "eventview_p.h"
#include "prefs.h"

#include <calendarsupport/collectionselection.h>
#include <calendarsupport/kcalprefs.h>

#include <KCheckableProxyModel>

#include <QApplication>

using namespace EventViews;

EventViewPrivate::EventViewPrivate()
  : calendar( 0 ),
    customCollectionSelection( 0 ),
    collectionSelectionModel( 0 ),
    mReturnPressed( false ),
    mDateRangeSelectionEnabled( true ),
    mTypeAhead( false ),
    mTypeAheadReceiver( 0 ),
    mPrefs( new Prefs() ),
    mKCalPrefs( new CalendarSupport::KCalPrefs() ),
    mChanger( 0 ),
    mChanges( EventView::DatesChanged ),
    mCollectionId( -1 )
{ }

EventViewPrivate::~EventViewPrivate()
{
  delete collectionSelectionModel;
}

void EventViewPrivate::finishTypeAhead()
{
  if ( mTypeAheadReceiver ) {
    foreach ( QEvent *e, mTypeAheadEvents ) {
      QApplication::sendEvent( mTypeAheadReceiver, e );
    }
  }
  qDeleteAll( mTypeAheadEvents );
  mTypeAheadEvents.clear();
  mTypeAhead = false;
}

void EventViewPrivate::setUpModels()
{
  delete customCollectionSelection;
  customCollectionSelection = 0;
  if ( collectionSelectionModel ) {
    customCollectionSelection = new CalendarSupport::CollectionSelection( collectionSelectionModel->selectionModel() );
  }
}
