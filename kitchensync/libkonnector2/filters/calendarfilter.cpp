/*
    This file is part of KitchenSync.

    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qheader.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qwhatsthis.h>
#include <qwidget.h>

#include <kdialog.h>
#include <kgenericfactory.h>
#include <klistview.h>
#include <klocale.h>
#include <libkdepim/kdateedit.h>
#include <libkdepim/kpimprefs.h>

#include "calendarsyncee.h"

#include "calendarfilter.h"

using namespace KSync;

K_EXPORT_KS_FILTER( libksfilter_calendar, CalendarFilter )

CalendarFilter::CalendarFilter( QObject *parent )
  : Filter( parent, "CalendarFilter" )
{
  setName( i18n( "Calendar Filter" ) );
}

CalendarFilter::~CalendarFilter()
{
}

bool CalendarFilter::supports( Syncee *syncee )
{
  return (dynamic_cast<CalendarSyncee*>( syncee ) != 0);
}

QWidget *CalendarFilter::configWidget( QWidget *parent )
{
  CalendarConfigWidget *wdg = new CalendarConfigWidget( parent, "CalendarConfigWidget" );

  KPimPrefs prefs;
  prefs.usrReadConfig();
  wdg->setCategories( prefs.mCustomCategories );
  wdg->setSelectedCategories( mSelectedCategories );
  wdg->setUseDate( mFilterByDate );
  wdg->setStartDate( mStartDate );
  wdg->setEndDate( mEndDate );

  return wdg;
}

void CalendarFilter::configWidgetClosed( QWidget *widget )
{
  CalendarConfigWidget *wdg = static_cast<CalendarConfigWidget*>( widget );
  mSelectedCategories = wdg->selectedCategories();
  mFilterByDate = wdg->useDate();
  mStartDate = wdg->startDate();
  mEndDate = wdg->endDate();
}

void CalendarFilter::convert( Syncee *syncee )
{
  filterSyncee( dynamic_cast<CalendarSyncee*>( syncee ), mSelectedCategories, mStartDate, mEndDate );
}

void CalendarFilter::reconvert( Syncee *syncee )
{
  unfilterSyncee( dynamic_cast<CalendarSyncee*>( syncee ) );
}

void CalendarFilter::doLoad()
{
  mSelectedCategories = config()->readListEntry( "SelectedCategories" );
  mFilterByDate = config()->readBoolEntry( "FilterByDate", false );
  mStartDate = config()->readDateTimeEntry( "StartDate" ).date();
  mEndDate = config()->readDateTimeEntry( "EndDate" ).date();
}

void CalendarFilter::doSave()
{
  config()->writeEntry( "SelectedCategories", mSelectedCategories );
  config()->writeEntry( "FilterByDate", mFilterByDate );
  config()->writeEntry( "StartDate", QDateTime( mStartDate ) );
  config()->writeEntry( "EndDate", QDateTime( mEndDate ) );
}

void CalendarFilter::filterSyncee( CalendarSyncee *syncee, const QStringList &categories,
                                   const QDate &startDate, const QDate &endDate )
{
  mFilteredEntries.clear();

  if ( categories.isEmpty() ) // do not filter
    return;

  QStringList::ConstIterator it;
  
  CalendarSyncEntry *entry;
  for ( entry = syncee->firstEntry(); entry; entry = syncee->nextEntry() ) {
    bool found = false;
    for ( it = categories.begin(); it != categories.end(); ++it )
      if ( entry->incidence()->categories().contains( *it ) ) {
        if ( mFilterByDate ) {
          if ( entry->incidence()->dtStart().date() >= startDate &&
               entry->incidence()->dtStart().date() <= endDate )
            found = true;
        } else
          found = true;
        break;
      }

    if ( !found )
      mFilteredEntries.append( entry );
  }

  QPtrListIterator<CalendarSyncEntry> entryIt( mFilteredEntries );
  while ( entryIt.current() ) {
    syncee->removeEntry( entryIt.current() );
    ++entryIt;
  }
}

void CalendarFilter::unfilterSyncee( CalendarSyncee *syncee )
{
  QPtrListIterator<CalendarSyncEntry> entryIt( mFilteredEntries );
  while ( entryIt.current() ) {
    syncee->addEntry( entryIt.current() );
    ++entryIt;
  }
}



CalendarConfigWidget::CalendarConfigWidget( QWidget *parent, const char *name )
  : QWidget( parent, name )
{
  QVBoxLayout *layout = new QVBoxLayout( this );

  QGroupBox *box = new QGroupBox( 2, Qt::Vertical, i18n( "Events and Todos" ), this );

  mView = new KListView( box );
  mView->addColumn( i18n( "Categories" ) );
  mView->setFullWidth( true );

  QWhatsThis::add( mView, i18n( "Select the categories for which the events and todos shall be synchronized. When no category is selected, all events and todos will be included." ) );

  QWidget *timeWidget = new QWidget( box );
  QGridLayout *wdgLayout = new QGridLayout( timeWidget, 3, 3, KDialog::marginHint(),
                                            KDialog::spacingHint() );

  mUseDate = new QCheckBox( i18n( "Use time range" ), timeWidget );
  wdgLayout->addMultiCellWidget( mUseDate, 0, 0, 0, 1 );

  QWhatsThis::add( mUseDate, i18n( "Synchronize only events and todos in a special time range." ) );

  mStartLabel = new QLabel( i18n( "start date", "From:" ), timeWidget );
  mStartDate = new KDateEdit( timeWidget );
  mStartLabel->setBuddy( mStartDate );

  mEndLabel = new QLabel( i18n( "end date", "Till:" ), timeWidget );
  mEndDate = new KDateEdit( timeWidget );
  mEndLabel->setBuddy( mEndDate );

  wdgLayout->addWidget( mStartLabel, 1, 0, Qt::AlignRight );
  wdgLayout->addWidget( mStartDate, 1, 1 );
  wdgLayout->addWidget( mEndLabel, 2, 0, Qt::AlignRight );
  wdgLayout->addWidget( mEndDate, 2, 1 );
  wdgLayout->setColStretch( 2, 10 );

  QWhatsThis::add( box, i18n( "Only the events and todos in the given date range will be synchronized." ) );

  layout->addWidget( box );

  connect( mUseDate, SIGNAL( toggled( bool ) ), this, SLOT( useDateChanged( bool ) ) );

}

void CalendarConfigWidget::setCategories( const QStringList &categories )
{
  mView->clear();

  QStringList::ConstIterator it;
  for ( it = categories.begin(); it != categories.end(); ++it )
    new QCheckListItem( mView, *it, QCheckListItem::CheckBox );
}

void CalendarConfigWidget::setSelectedCategories( const QStringList &categories )
{
  QListViewItemIterator itemIt( mView );
  QStringList::ConstIterator it;

  while ( itemIt.current() ) {
    bool found = false;
    for ( it = categories.begin(); it != categories.end(); ++it ) {
      if ( itemIt.current()->text( 0 ) == *it ) {
        found = true;
        break;
      }
    }

    QCheckListItem *item = static_cast<QCheckListItem*>( itemIt.current() );
    item->setOn( found );

    ++itemIt;
  }
}

QStringList CalendarConfigWidget::selectedCategories() const
{
  QStringList categories;

  QListViewItemIterator itemIt( mView, QListViewItemIterator::Checked );
  while ( itemIt.current() )
    categories.append( itemIt.current()->text( 0 ) );

  return categories;
}

void CalendarConfigWidget::setStartDate( const QDate &date )
{
  mStartDate->setDate( date );
}

QDate CalendarConfigWidget::startDate() const
{
  return mStartDate->date();
}

void CalendarConfigWidget::setEndDate( const QDate &date )
{
  mEndDate->setDate( date );
}

QDate CalendarConfigWidget::endDate() const
{
  return mEndDate->date();
}

void CalendarConfigWidget::setUseDate( bool value )
{
  mUseDate->setChecked( value );
  useDateChanged( value );
}

bool CalendarConfigWidget::useDate() const
{
  return mUseDate->isChecked();
}

void CalendarConfigWidget::useDateChanged( bool value )
{
  mStartLabel->setEnabled( value );
  mStartDate->setEnabled( value );
  mEndDate->setEnabled( value );
  mEndLabel->setEnabled( value );
}

#include "calendarfilter.moc"
