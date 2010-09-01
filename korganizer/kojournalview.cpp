/*
    This file is part of KOrganizer.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
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

//
// View of Journal entries

#include <tqlayout.h>
#include <tqpopupmenu.h>
#include <tqvbox.h>
#include <tqlabel.h>
#include <tqscrollview.h>

#include <klocale.h>
#include <kdebug.h>

#include <libkcal/calendar.h>

#include "journalentry.h"

#include "kojournalview.h"
#include "koglobals.h"
using namespace KOrg;

KOJournalView::KOJournalView(Calendar *calendar, TQWidget *parent,
                       const char *name)
  : KOrg::BaseView(calendar, parent, name)
{
  TQVBoxLayout*topLayout = new TQVBoxLayout( this );
  topLayout->setAutoAdd(true);
  mSV = new TQScrollView( this, "JournalScrollView" );
  mVBox = new TQVBox( mSV->viewport() );
  mSV->setVScrollBarMode( TQScrollView::Auto );
  mSV->setHScrollBarMode( TQScrollView::AlwaysOff );
  mSV->setResizePolicy( TQScrollView::AutoOneFit );
  mSV->addChild( mVBox );
//  mVBox->setSpacing( 10 );
}

KOJournalView::~KOJournalView()
{
}

void KOJournalView::appendJournal( Journal*journal, const TQDate &dt)
{
  JournalDateEntry *entry = 0;
  if ( mEntries.contains( dt ) ) {
    entry = mEntries[dt];
  } else {
    entry = new JournalDateEntry( calendar(), mVBox );
    entry->setDate( dt );
    entry->setIncidenceChanger( mChanger );
    entry->show();
    connect( this, TQT_SIGNAL(flushEntries()),
             entry, TQT_SIGNAL(flushEntries()) );

    connect( this, TQT_SIGNAL(setIncidenceChangerSignal(IncidenceChangerBase *)),
             entry, TQT_SLOT(setIncidenceChanger( IncidenceChangerBase *)) );

    connect( this, TQT_SIGNAL(journalEdited(Journal *)),
             entry, TQT_SLOT(journalEdited(Journal *)) );
    connect( this, TQT_SIGNAL(journalDeleted(Journal *)),
             entry, TQT_SLOT(journalDeleted(Journal *)) );

    connect( entry, TQT_SIGNAL(editIncidence(Incidence *,const TQDate &)),
             this, TQT_SIGNAL(editIncidenceSignal(Incidence *,const TQDate &)) );
    connect( entry, TQT_SIGNAL(deleteIncidence(Incidence *)),
             this, TQT_SIGNAL(deleteIncidenceSignal(Incidence *)) );

    connect( entry, TQT_SIGNAL(newJournal(ResourceCalendar *,const TQString &,const TQDate &)),
             this, TQT_SIGNAL(newJournalSignal(ResourceCalendar *,const TQString &,const TQDate &)) );
    mEntries.insert( dt, entry );
  }

  if ( entry && journal ) {
    entry->addJournal( journal );
  }
}

int KOJournalView::currentDateCount()
{
  return mEntries.size();
}

Incidence::List KOJournalView::selectedIncidences()
{
  // We don't have a selection in the journal view.
  // FIXME: The currently edited journal is the selected incidence...
  Incidence::List eventList;
  return eventList;
}

void KOJournalView::clearEntries()
{
//  kdDebug(5850)<<"KOJournalView::clearEntries()"<<endl;
  TQMap<TQDate, JournalDateEntry*>::Iterator it;
  for ( it = mEntries.begin(); it != mEntries.end(); ++it ) {
    delete (it.data());
  }
  mEntries.clear();
}
void KOJournalView::updateView()
{
  TQMap<TQDate, JournalDateEntry*>::Iterator it;
  for ( it = mEntries.begin(); it != mEntries.end(); ++it ) {
    it.data()->clear();
    Journal::List journals = calendar()->journals( it.key() );
    Journal::List::Iterator it1;
    for ( it1 = journals.begin(); it1 != journals.end(); ++it1 ) {
      it.data()->addJournal( *it1 );
    }
  }
}

void KOJournalView::flushView()
{
//  kdDebug(5850) << "KOJournalView::flushView(): "<< endl;
  emit flushEntries();
}

void KOJournalView::showDates( const TQDate &start, const TQDate &end )
{
//  kdDebug(5850) << "KOJournalView::showDates(): "<<start.toString().latin1()<<" - "<<end.toString().latin1() << endl;
  clearEntries();
  if ( end < start ) {
    return;
  }

  Journal::List::ConstIterator it;
  Journal::List jnls;
  TQDate d = start;
  for ( TQDate d = start; d <= end; d = d.addDays( 1 ) ) {
    jnls = calendar()->journals( d );
    for ( it = jnls.begin(); it != jnls.end(); ++it ) {
      appendJournal( *it, d );
    }
    if ( jnls.count() < 1 ) {
      // create an empty dateentry widget
      appendJournal( 0, d );
    }
  }
}

void KOJournalView::showIncidences( const Incidence::List &incidences, const TQDate & )
{
//  kdDebug(5850) << "KOJournalView::showIncidences(): "<< endl;
  clearEntries();
  Incidence::List::const_iterator it;
  for ( it = incidences.constBegin(); it != incidences.constEnd(); ++it ) {
    if ( (*it) && ( (*it)->type() == "Journal" ) ) {
      Journal *j = static_cast<Journal*>(*it);
      if ( j ) {
	appendJournal( j, j->dtStart().date() );
      }
    }
  }
}

CalPrinterBase::PrintType KOJournalView::printType()
{
  return CalPrinterBase::Journallist;
}

void KOJournalView::changeIncidenceDisplay(Incidence *incidence, int action)
{
//  kdDebug(5850) << "KOJournalView::changeIncidenceDisplay(): "<< endl;
  Journal *journal = dynamic_cast<Journal*>(incidence);
  if (journal) {
    switch(action) {
      case KOGlobals::INCIDENCEADDED:
        appendJournal( journal, journal->dtStart().date() );
        break;
      case KOGlobals::INCIDENCEEDITED:
        emit journalEdited( journal );
        break;
      case KOGlobals::INCIDENCEDELETED:
        emit journalDeleted( journal );
        break;
      default:
        kdDebug(5850) << "KOListView::changeIncidenceDisplay(): Illegal action " << action << endl;
    }
  }
}

void KOJournalView::setIncidenceChanger( IncidenceChangerBase *changer )
{
  mChanger = changer;
  emit setIncidenceChangerSignal( changer );
}

void KOJournalView::newJournal()
{
  emit newJournalSignal( 0/*ResourceCalendar*/, TQString()/*subResource*/,
                         TQDate::currentDate() );
}

#include "kojournalview.moc"
