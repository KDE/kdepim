/*
  This file is part of libkdepim.

  Copyright (c) 2004 Bram Schoenmakers <bramschoenmakers@kde.nl>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include <tqdatetime.h>
#include <tqpopupmenu.h>

#include <klocale.h>

#include "kdatepickerpopup.h"

KDatePickerPopup::KDatePickerPopup( int items, const TQDate &date, TQWidget *parent,
                                    const char *name )
  : TQPopupMenu( parent, name )
{
  mItems = items;

  mDatePicker = new KDatePicker( this );
  mDatePicker->setCloseButton( false );

  connect( mDatePicker, TQT_SIGNAL( dateEntered( TQDate ) ),
           TQT_SLOT( slotDateChanged( TQDate ) ) );
  connect( mDatePicker, TQT_SIGNAL( dateSelected( TQDate ) ),
           TQT_SLOT( slotDateChanged( TQDate ) ) );

  mDatePicker->setDate( date );

  buildMenu();
}

void KDatePickerPopup::buildMenu()
{
  if ( isVisible() ) return;
  clear();

  if ( mItems & DatePicker ) {
    insertItem( mDatePicker );

    if ( ( mItems & NoDate ) || ( mItems & Words ) )
      insertSeparator();
  }

  if ( mItems & Words ) {
    insertItem( i18n("&Today"), this, TQT_SLOT( slotToday() ) );
    insertItem( i18n("To&morrow"), this, TQT_SLOT( slotTomorrow() ) );
    insertItem( i18n("Next &Week"), this, TQT_SLOT( slotNextWeek() ) );
    insertItem( i18n("Next M&onth"), this, TQT_SLOT( slotNextMonth() ) );

    if ( mItems & NoDate )
      insertSeparator();
  }

  if ( mItems & NoDate )
    insertItem( i18n("No Date"), this, TQT_SLOT( slotNoDate() ) );
}

KDatePicker *KDatePickerPopup::datePicker() const
{
  return mDatePicker;
}

void KDatePickerPopup::setDate( const TQDate &date )
{
  mDatePicker->setDate( date );
}

#if 0
void KDatePickerPopup::setItems( int items )
{
  mItems = items;
  buildMenu();
}
#endif

void KDatePickerPopup::slotDateChanged( TQDate date )
{
  emit dateChanged( date );
  hide();
}

void KDatePickerPopup::slotToday()
{
  emit dateChanged( TQDate::currentDate() );
}

void KDatePickerPopup::slotTomorrow()
{
  emit dateChanged( TQDate::currentDate().addDays( 1 ) );
}

void KDatePickerPopup::slotNoDate()
{
  emit dateChanged( TQDate() );
}

void KDatePickerPopup::slotNextWeek()
{
  emit dateChanged( TQDate::currentDate().addDays( 7 ) );
}

void KDatePickerPopup::slotNextMonth()
{
  emit dateChanged( TQDate::currentDate().addMonths( 1 ) );
}

#include "kdatepickerpopup.moc"
