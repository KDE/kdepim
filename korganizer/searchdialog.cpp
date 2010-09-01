/*
    This file is part of KOrganizer.

    Copyright (c) 1998 Preston Brown <pbrown@kde.org>
    Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#include <tqlayout.h>
#include <tqcheckbox.h>
#include <tqgroupbox.h>
#include <tqhbuttongroup.h>
#include <tqlabel.h>
#include <tqlineedit.h>

#include <klocale.h>
#include <kmessagebox.h>

#include <libkcal/calendar.h>

#include <libkdepim/kdateedit.h>

#include "koglobals.h"
#include "koprefs.h"
#include "kolistview.h"

#include "searchdialog.h"
#include "searchdialog.moc"

SearchDialog::SearchDialog(Calendar *calendar,TQWidget *parent)
  : KDialogBase(Plain,i18n("Find Events"),User1|Close,User1,parent,0,false,false,
                KGuiItem( i18n("&Find"), "find") )
{
  mCalendar = calendar;

  TQFrame *topFrame = plainPage();
  TQVBoxLayout *layout = new TQVBoxLayout(topFrame,0,spacingHint());

  // Search expression
  TQHBoxLayout *subLayout = new TQHBoxLayout();
  layout->addLayout(subLayout);

  searchEdit = new TQLineEdit( "*", topFrame ); // Find all events by default
  searchLabel = new TQLabel( searchEdit, i18n("&Search for:"), topFrame );
  subLayout->addWidget( searchLabel );
  subLayout->addWidget( searchEdit );
  searchEdit->setFocus();
  connect( searchEdit, TQT_SIGNAL( textChanged( const TQString & ) ),
           this, TQT_SLOT( searchTextChanged( const TQString & ) ) );


  TQHButtonGroup *itemsGroup = new TQHButtonGroup( i18n("Search For"), topFrame );
  layout->addWidget( itemsGroup );
  mEventsCheck = new TQCheckBox( i18n("&Events"), itemsGroup );
  mTodosCheck = new TQCheckBox( i18n("To-&dos"), itemsGroup );
  mJournalsCheck = new TQCheckBox( i18n("&Journal entries"), itemsGroup );
  mEventsCheck->setChecked( true );
  mTodosCheck->setChecked( true );

  // Date range
  TQGroupBox *rangeGroup = new TQGroupBox( 1, Horizontal, i18n( "Date Range" ),
                                        topFrame );
  layout->addWidget( rangeGroup );

  TQWidget *rangeWidget = new TQWidget( rangeGroup );
  TQHBoxLayout *rangeLayout = new TQHBoxLayout( rangeWidget, 0, spacingHint() );

  mStartDate = new KDateEdit( rangeWidget );
  rangeLayout->addWidget( new TQLabel( mStartDate, i18n("Fr&om:"), rangeWidget ) );
  rangeLayout->addWidget( mStartDate );

  mEndDate = new KDateEdit( rangeWidget );
  rangeLayout->addWidget( new TQLabel( mEndDate, i18n("&To:"), rangeWidget ) );
  mEndDate->setDate( TQDate::currentDate().addDays( 365 ) );
  rangeLayout->addWidget( mEndDate );

  mInclusiveCheck = new TQCheckBox( i18n("E&vents have to be completely included"),
                                  rangeGroup );
  mInclusiveCheck->setChecked( false );
  mIncludeUndatedTodos = new TQCheckBox( i18n("Include to-dos &without due date"), rangeGroup );
  mIncludeUndatedTodos->setChecked( true );

  // Subjects to search
  TQHButtonGroup *subjectGroup = new TQHButtonGroup( i18n("Search In"), topFrame );
  layout->addWidget(subjectGroup);

  mSummaryCheck = new TQCheckBox( i18n("Su&mmaries"), subjectGroup );
  mSummaryCheck->setChecked( true );
  mDescriptionCheck = new TQCheckBox( i18n("Desc&riptions"), subjectGroup );
  mCategoryCheck = new TQCheckBox( i18n("Cate&gories"), subjectGroup );


  // Results list view
  listView = new KOListView( mCalendar, topFrame );
  listView->showDates();
  layout->addWidget( listView );

  if ( KOPrefs::instance()->mCompactDialogs ) {
    KOGlobals::fitDialogToScreen( this, true );
  }

  connect( this,TQT_SIGNAL(user1Clicked()),TQT_SLOT(doSearch()));

  // Propagate edit and delete event signals from event list view
  connect( listView, TQT_SIGNAL(showIncidenceSignal(Incidence *,const TQDate &)),
          TQT_SIGNAL(showIncidenceSignal(Incidence *,const TQDate &)) );
  connect( listView, TQT_SIGNAL(editIncidenceSignal(Incidence *,const TQDate &)),
           TQT_SIGNAL(editIncidenceSignal(Incidence *,const TQDate &)) );
  connect( listView, TQT_SIGNAL(deleteIncidenceSignal(Incidence *)),
           TQT_SIGNAL(deleteIncidenceSignal(Incidence *)) );
}

SearchDialog::~SearchDialog()
{
}

void SearchDialog::searchTextChanged( const TQString &_text )
{
  enableButton( KDialogBase::User1, !_text.isEmpty() );
}

void SearchDialog::doSearch()
{
  TQRegExp re;

  re.setWildcard( true ); // most people understand these better.
  re.setCaseSensitive( false );
  re.setPattern( searchEdit->text() );
  if ( !re.isValid() ) {
    KMessageBox::sorry( this,
                        i18n("Invalid search expression, cannot perform "
                            "the search. Please enter a search expression "
                            "using the wildcard characters '*' and '?' "
                            "where needed." ) );
    return;
  }

  search( re );

  listView->showIncidences( mMatchedEvents, TQDate() );

  if ( mMatchedEvents.count() == 0 ) {
    KMessageBox::information( this,
        i18n("No events were found matching your search expression."),
        "NoSearchResults" );
  }
}

void SearchDialog::updateView()
{
  TQRegExp re;
  re.setWildcard( true ); // most people understand these better.
  re.setCaseSensitive( false );
  re.setPattern( searchEdit->text() );
  if ( re.isValid() ) {
    search( re );
  } else {
    mMatchedEvents.clear();
  }

  listView->showIncidences( mMatchedEvents, TQDate() );
}

void SearchDialog::search( const TQRegExp &re )
{
  TQDate startDt = mStartDate->date();
  TQDate endDt = mEndDate->date();

  Event::List events;
  if (mEventsCheck->isChecked()) {
    events = mCalendar->events( startDt, endDt, mInclusiveCheck->isChecked() );
  }
  Todo::List todos;
  if (mTodosCheck->isChecked()) {
    if ( mIncludeUndatedTodos->isChecked() ) {
      Todo::List alltodos = mCalendar->todos();
      Todo::List::iterator it;
      Todo *todo;
      for (it=alltodos.begin(); it!=alltodos.end(); ++it) {
        todo = *it;
        if ( (!todo->hasStartDate() && !todo->hasDueDate() ) || // undated
             ( todo->hasStartDate() && (todo->dtStart()>=startDt) && (todo->dtStart()<=endDt) ) || // start dt in range
             ( todo->hasDueDate() && (todo->dtDue().date()>=startDt) && (todo->dtDue()<=endDt) ) || // due dt in range
             ( todo->hasCompletedDate() && (todo->completed().date()>=startDt) && (todo->completed()<=endDt) ) ) { // completed dt in range
          todos.append( todo );
        }
      }
    } else {
      TQDate dt = startDt;
      while ( dt <= endDt ) {
        todos += mCalendar->todos( dt );
        dt = dt.addDays( 1 );
      }
    }
  }

  Journal::List journals;
  if (mJournalsCheck->isChecked()) {
    TQDate dt = startDt;
    while ( dt <= endDt ) {
      journals += mCalendar->journals( dt );
      dt = dt.addDays( 1 );
    }
  }

  Incidence::List allIncidences = Calendar::mergeIncidenceList( events, todos, journals );

  mMatchedEvents.clear();
  Incidence::List::ConstIterator it;
  for( it = allIncidences.begin(); it != allIncidences.end(); ++it ) {
    Incidence *ev = *it;
    if ( mSummaryCheck->isChecked() ) {
#if QT_VERSION >= 300
      if ( re.search( ev->summary() ) != -1 ) {
#else
      if ( re.match( ev->summary() ) != -1 ) {
#endif
        mMatchedEvents.append( ev );
        continue;
      }
    }
    if ( mDescriptionCheck->isChecked() ) {
#if QT_VERSION >= 300
      if ( re.search( ev->description() ) != -1 ) {
#else
      if ( re.match( ev->description() ) != -1 ) {
#endif
        mMatchedEvents.append( ev );
        continue;
      }
    }
    if ( mCategoryCheck->isChecked() ) {
#if QT_VERSION >= 300
      if ( re.search( ev->categoriesStr() ) != -1 ) {
#else
      if ( re.match( ev->categoriesStr() ) != -1 ) {
#endif
        mMatchedEvents.append( ev );
        continue;
      }
    }
  }
}
