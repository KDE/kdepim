/*
    Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>

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

#include "monthviewitem.h"
#include "mainview.h"

#include <Akonadi/Calendar/ETMCalendar>
#include <calendarviews/month/monthview.h>

#include <KGlobal>
#include <KDebug>
#include <KGlobalSettings>

using namespace EventViews;

MonthViewItem::MonthViewItem( QDeclarativeItem* parent )
  : DeclarativeAkonadiItem( parent )
  , mView( new MonthView( MonthView::Hidden ) )
{
  // start with the oxygen palette (which is not necessarily the default on all platforms)
  QPalette pal = KGlobalSettings::createApplicationPalette( KGlobal::config() );
  mView->setPalette( pal );
  setWidget( mView );

  connect( this, SIGNAL(previousItemRequest()),
           mView, SLOT(moveBackMonth()) );
  connect( this, SIGNAL(nextItemRequest()),
           mView, SLOT(moveFwdMonth()) );
  connect( mView, SIGNAL(newEventSignal()),
           SLOT(emitDateClicked()));
  connect( mView, SIGNAL(incidenceSelected(Akonadi::Item,QDate)),
           SLOT(emitItemSelected(Akonadi::Item,QDate)) );

  setPreferences( MainView::preferences() );
}

MonthViewItem::~MonthViewItem()
{
  mView->preferences()->writeConfig();
  delete mView;
}

void MonthViewItem::emitDateClicked()
{
  emit dateClicked( mView->selectionStart().date() );
}

void MonthViewItem::emitItemSelected( const Akonadi::Item &item, const QDate &activeDate )
{
  emit itemSelected( item.id(), activeDate );
}

QObject* MonthViewItem::calendar() const
{
  return mView->calendar().data();
}

void MonthViewItem::setCalendar( QObject* calendarObj )
{
  Akonadi::ETMCalendar* cal = qobject_cast<Akonadi::ETMCalendar*>( calendarObj );
  kDebug() << calendarObj << cal;
  if ( cal ) {
    mView->setCalendar( cal->weakPointer().toStrongRef().dynamicCast<Akonadi::ETMCalendar>() );
    mView->updateConfig();
  }
}

void MonthViewItem::showMonth( const QDate &date )
{
  const KDateTime start( QDate( date.year(), date.month(), 1 ) );
  const KDateTime end( QDate( date.year(), date.month(), date.daysInMonth() ) );
  mView->setDateRange( start, end );
}

void MonthViewItem::setPreferences( const PrefsPtr &preferences )
{
  mView->setPreferences( preferences );
}

PrefsPtr MonthViewItem::preferences() const
{
  return mView->preferences();
}

void MonthViewItem::updateConfig()
{
  mView->updateConfig();
}

