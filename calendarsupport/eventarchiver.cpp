/*
  Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (c) 2004 David Faure <faure@kde.org>
  Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#include "eventarchiver.h"

#include "calendar.h"
#include "calendaradaptor.h"
#include "kcalprefs.h"
#include "utils.h"

#include <akonadi/calendar/incidencechanger.h>

#include <KCalCore/ICalFormat>
#include <KCalCore/FileStorage>
#include <KCalCore/MemoryCalendar>

#include <KCalUtils/Stringify>

#include <KDebug>
#include <KGlobal>
#include <KLocale>
#include <KMessageBox>
#include <KTemporaryFile>
#include <KIO/NetAccess>

using namespace KCalCore;
using namespace KCalUtils;
using namespace CalendarSupport;

EventArchiver::EventArchiver( QObject *parent )
 : QObject( parent )
{
}

EventArchiver::~EventArchiver()
{
}

void EventArchiver::runOnce( CalendarSupport::Calendar *calendar,
                             Akonadi::IncidenceChanger *changer,
                             const QDate &limitDate, QWidget *widget )
{
  run( calendar, changer, limitDate, widget, true, true );
}

void EventArchiver::runAuto( CalendarSupport::Calendar *calendar,
                             Akonadi::IncidenceChanger *changer,
                             QWidget *widget, bool withGUI )
{
  QDate limitDate( QDate::currentDate() );
  int expiryTime = KCalPrefs::instance()->mExpiryTime;
  switch ( KCalPrefs::instance()->mExpiryUnit ) {
  case KCalPrefs::UnitDays: // Days
    limitDate = limitDate.addDays( -expiryTime );
    break;
  case KCalPrefs::UnitWeeks: // Weeks
    limitDate = limitDate.addDays( -expiryTime * 7 );
    break;
  case KCalPrefs::UnitMonths: // Months
    limitDate = limitDate.addMonths( -expiryTime );
    break;
  default:
    return;
  }
  run( calendar, changer, limitDate, widget, withGUI, false );
}

void EventArchiver::run( CalendarSupport::Calendar *calendar,
                         Akonadi::IncidenceChanger *changer,
                         const QDate &limitDate, QWidget *widget,
                         bool withGUI, bool errorIfNone )
{
  // We need to use rawEvents, otherwise events hidden by filters will not be archived.
  Akonadi::Item::List events;
  Akonadi::Item::List todos;
  Akonadi::Item::List journals;

  if ( KCalPrefs::instance()->mArchiveEvents ) {
    events = calendar->rawEvents(
      QDate( 1769, 12, 1 ),
      // #29555, also advertised by the "limitDate not included" in the class docu
      limitDate.addDays( -1 ),
      CalendarSupport::KCalPrefs::instance()->timeSpec(),
      true );
  }
  if ( KCalPrefs::instance()->mArchiveTodos ) {
    Akonadi::Item::List t = calendar->rawTodos();
    Akonadi::Item::List::ConstIterator it;
    Akonadi::Item::List::ConstIterator end( t.constEnd() );

    for ( it = t.constBegin(); it != end; ++it ) {
      const Todo::Ptr todo = CalendarSupport::todo( *it );
      Q_ASSERT( todo );
      if ( isSubTreeComplete( calendar, todo, limitDate ) ) {
        todos.append( *it );
      }
    }
  }

  const Akonadi::Item::List incidences =
    CalendarSupport::Calendar::mergeIncidenceList( events, todos, journals );

  kDebug() << "archiving incidences before" << limitDate
           << " ->" << incidences.count() <<" incidences found.";
  if ( incidences.isEmpty() ) {
    if ( withGUI && errorIfNone ) {
      KMessageBox::information( widget,
                                i18n( "There are no items before %1",
                                      KGlobal::locale()->formatDate( limitDate ) ),
                                "ArchiverNoIncidences" );
    }
    return;
  }

  switch ( KCalPrefs::instance()->mArchiveAction ) {
  case KCalPrefs::actionDelete:
    deleteIncidences( changer, limitDate, widget, incidences, withGUI );
    break;
  case KCalPrefs::actionArchive:
    archiveIncidences( calendar, changer, limitDate, widget, incidences, withGUI );
    break;
  }
}

void EventArchiver::deleteIncidences( Akonadi::IncidenceChanger *changer,
                                      const QDate &limitDate, QWidget *widget,
                                      const Akonadi::Item::List &incidences, bool withGUI )
{
  QStringList incidenceStrs;
  Akonadi::Item::List::ConstIterator it;
  Akonadi::Item::List::ConstIterator end( incidences.constEnd() );
  for ( it = incidences.constBegin(); it != end; ++it ) {
    incidenceStrs.append( CalendarSupport::incidence( *it )->summary() );
  }

  if ( withGUI ) {
    int result = KMessageBox::warningContinueCancelList(
      widget,
      i18n( "Delete all items before %1 without saving?\n"
            "The following items will be deleted:",
            KGlobal::locale()->formatDate( limitDate ) ),
      incidenceStrs,
      i18n( "Delete Old Items" ), KStandardGuiItem::del() );
    if ( result != KMessageBox::Continue ) {
      return;
    }
  }

  for ( it = incidences.constBegin(); it != incidences.constEnd(); ++it ) {
    changer->deleteIncidence( *it, widget );
  } // TODO: emit only after hearing back from incidence changer
  emit eventsDeleted();
}

void EventArchiver::archiveIncidences( CalendarSupport::Calendar *calendar,
                                       Akonadi::IncidenceChanger *changer,
                                       const QDate &limitDate, QWidget *widget,
                                       const Akonadi::Item::List &incidences, bool withGUI )
{
  Q_UNUSED( limitDate );
  Q_UNUSED( withGUI );

  CalendarSupport::CalendarAdaptor::Ptr cal(
    new CalendarSupport::CalendarAdaptor( calendar, widget ) );
  FileStorage storage( cal );

  QString tmpFileName;
  // KSaveFile cannot be called with an open File Handle on Windows.
  // So we use KTemporaryFile only to generate a unique filename
  // and then close/delete the file again. This file must be deleted
  // here.
  {
    KTemporaryFile tmpFile;
    tmpFile.open();
    tmpFileName = tmpFile.fileName();
  }
  // Save current calendar to disk
  storage.setFileName( tmpFileName );
  if ( !storage.save() ) {
    kDebug() << "Can't save calendar to temp file";
    return;
  }

  // Duplicate current calendar by loading in new calendar object
  MemoryCalendar::Ptr archiveCalendar(
    new MemoryCalendar( CalendarSupport::KCalPrefs::instance()->timeSpec() ) );

  FileStorage archiveStore( archiveCalendar );
  archiveStore.setFileName( tmpFileName );
  ICalFormat *format = new ICalFormat();
  archiveStore.setSaveFormat( format );
  if ( !archiveStore.load() ) {
    kDebug() << "Can't load calendar from temp file";
    QFile::remove( tmpFileName );
    return;
  }

  // Strip active events from calendar so that only events to be archived
  // remain. This is not really efficient, but there is no other easy way.
  QStringList uids;
  Incidence::List allIncidences = archiveCalendar->rawIncidences();
  foreach ( const Akonadi::Item &item, incidences ) {
    uids.append( CalendarSupport::incidence(item)->uid() );
  }
  foreach ( const Incidence::Ptr inc, allIncidences ) {
    if ( !uids.contains( inc->uid() ) ) {
      archiveCalendar->deleteIncidence( inc );
    }
  }

  // Get or create the archive file
  KUrl archiveURL( KCalPrefs::instance()->mArchiveFile );
  QString archiveFile;

#ifndef Q_OS_WINCE
  // There is no KIO::NetAccess availabe for Windows CE
  if ( KIO::NetAccess::exists( archiveURL, KIO::NetAccess::SourceSide, widget ) ) {
    if( !KIO::NetAccess::download( archiveURL, archiveFile, widget ) ) {
      kDebug() << "Can't download archive file";
      QFile::remove( tmpFileName );
      return;
    }
    // Merge with events to be archived.
    archiveStore.setFileName( archiveFile );
    if ( !archiveStore.load() ) {
      kDebug() << "Can't merge with archive file";
      QFile::remove( tmpFileName );
      return;
    }
  } else {
    archiveFile = tmpFileName;
  }
#else
  archiveFile = archiveURL.toLocalFile();
  archiveStore.setFileName( archiveFile );
#endif // Q_OS_WINCE

  // Save archive calendar
  if ( !archiveStore.save() ) {
    QString errmess;
    if ( format->exception() ) {
      errmess = Stringify::errorMessage( *format->exception() );
    } else {
      errmess = i18nc( "save failure cause unknown", "Reason unknown" );
    }
    KMessageBox::error( widget, i18n( "Cannot write archive file %1. %2",
                                      archiveStore.fileName(), errmess ) );
    QFile::remove( tmpFileName );
    return;
  }

#ifndef Q_OS_WINCE
  // Upload if necessary
  KUrl srcUrl;
  srcUrl.setPath( archiveFile );
  if ( srcUrl != archiveURL ) {
    if ( !KIO::NetAccess::upload( archiveFile, archiveURL, widget ) ) {
      KMessageBox::error( widget, i18n( "Cannot write archive. %1",
                                        KIO::NetAccess::lastErrorString() ) );
      QFile::remove( tmpFileName );
      return;
    }
  }

  KIO::NetAccess::removeTempFile( archiveFile );
#endif // Q_OS_WINCE
  QFile::remove( tmpFileName );

  // We don't want it to ask to send invitations for each incidence.
  changer->startAtomicOperation( i18n( "Archiving events" ) );

  // Delete archived events from calendar
  foreach ( const Akonadi::Item &item, incidences ) {
    changer->deleteIncidence( item, widget );
  } // TODO: emit only after hearing back from incidence changer
  changer->endAtomicOperation();

  emit eventsDeleted();
}

bool EventArchiver::isSubTreeComplete( CalendarSupport::Calendar *calendar,
                                       const Todo::Ptr &todo,
                                       const QDate &limitDate,
                                       QStringList checkedUids ) const
{
  if ( !todo->isCompleted() || todo->completed().date() >= limitDate ) {
    return false;
  }

  // This QList is only to prevent infinit recursion
  if ( checkedUids.contains( todo->uid() ) ) {
    // Probably will never happen, calendar.cpp checks for this
    kWarning() << "To-do hierarchy loop detected!";
    return false;
  }

  checkedUids.append( todo->uid() );
  const Akonadi::Item item = calendar->itemForIncidenceUid( todo->uid() );
  Akonadi::Item::List relations = calendar->findChildren( item );
  foreach ( const Akonadi::Item &item, relations ) {
    if ( CalendarSupport::hasTodo( item ) ) {
      const Todo::Ptr t = CalendarSupport::todo( item );
      if ( !isSubTreeComplete( calendar, t, limitDate, checkedUids ) ) {
        return false;
      }
    }
  }

  return true;
}

#include "eventarchiver.moc"
