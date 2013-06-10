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

#include "kdatepickerpopup.h"

#include <KDatePicker>
#include <KLocale>

#include <QtCore/QDateTime>
#include <QWidgetAction>

using namespace KPIM;

class KDatePickerAction : public QWidgetAction
{
  public:
    KDatePickerAction( KDatePicker *widget, QObject *parent )
      : QWidgetAction( parent ),
        mDatePicker( widget ), mOriginalParent( widget->parentWidget() )
    {
    }

  protected:
    QWidget *createWidget( QWidget *parent )
    {
      mDatePicker->setParent( parent );
      return mDatePicker;
    }

    void deleteWidget( QWidget *widget )
    {
      if ( widget != mDatePicker ) {
        return;
      }

      mDatePicker->setParent( mOriginalParent );
    }

  private:
    KDatePicker *mDatePicker;
    QWidget *mOriginalParent;
};

class KDatePickerPopup::Private
{
  public:
    Private( KDatePickerPopup *qq )
      : q( qq ), mDatePicker( 0 )
    {
    }

    void buildMenu();

    void slotDateChanged( const QDate& );
    void slotToday();
    void slotTomorrow();
    void slotNextWeek();
    void slotNextMonth();
    void slotNoDate();

    KDatePickerPopup *q;
    KDatePicker *mDatePicker;
    Modes mModes;
};

void KDatePickerPopup::Private::buildMenu()
{
  if ( q->isVisible() )
    return;

  q->clear();

  if ( mModes & DatePicker ) {
    q->addAction( new KDatePickerAction( mDatePicker, q ) );

    if ( (mModes & NoDate) || (mModes & Words) )
      q->addSeparator();
  }

  if ( mModes & Words ) {
    q->addAction( i18nc( "@option today", "&Today" ), q, SLOT(slotToday()) );
    q->addAction( i18nc( "@option tomorrow", "To&morrow" ), q, SLOT(slotTomorrow()) );
    q->addAction( i18nc( "@option next week", "Next &Week" ), q, SLOT(slotNextWeek()) );
    q->addAction( i18nc( "@option next month", "Next M&onth" ), q, SLOT(slotNextMonth()) );

    if ( mModes & NoDate )
      q->addSeparator();
  }

  if ( mModes & NoDate )
    q->addAction( i18nc( "@option do not specify a date", "No Date" ), q, SLOT(slotNoDate()) );
}

void KDatePickerPopup::Private::slotDateChanged( const QDate &date )
{
  emit q->dateChanged( date );
  q->hide();
}

void KDatePickerPopup::Private::slotToday()
{
  emit q->dateChanged( QDate::currentDate() );
}

void KDatePickerPopup::Private::slotTomorrow()
{
  emit q->dateChanged( QDate::currentDate().addDays( 1 ) );
}

void KDatePickerPopup::Private::slotNoDate()
{
  emit q->dateChanged( QDate() );
}

void KDatePickerPopup::Private::slotNextWeek()
{
  emit q->dateChanged( QDate::currentDate().addDays( 7 ) );
}

void KDatePickerPopup::Private::slotNextMonth()
{
  emit q->dateChanged( QDate::currentDate().addMonths( 1 ) );
}


KDatePickerPopup::KDatePickerPopup( Modes modes, const QDate &date, QWidget *parent )
  : QMenu( parent ), d( new Private( this ) )
{
  d->mModes = modes;

  d->mDatePicker = new KDatePicker( this );
  d->mDatePicker->setCloseButton( false );

  connect( d->mDatePicker, SIGNAL(dateEntered(QDate)),
           SLOT(slotDateChanged(QDate)) );
  connect( d->mDatePicker, SIGNAL(dateSelected(QDate)),
           SLOT(slotDateChanged(QDate)) );

  d->mDatePicker->setDate( date );

  d->buildMenu();
}

KDatePickerPopup::~KDatePickerPopup()
{
  delete d;
}

KDatePicker *KDatePickerPopup::datePicker() const
{
  return d->mDatePicker;
}

void KDatePickerPopup::setDate( const QDate &date )
{
  d->mDatePicker->setDate( date );
}

#include "kdatepickerpopup.moc"
