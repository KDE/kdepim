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

#include "kcalprefs.h"
#include "utils.h"

#include <Akonadi/Calendar/IncidenceChanger>

#include <KCalCore/ICalFormat>
#include <KCalCore/FileStorage>
#include <KCalCore/MemoryCalendar>

#include <KCalUtils/Stringify>

#include <KDebug>
#include <KGlobal>
#include <KLocalizedString>
#include <KMessageBox>
#include <KTemporaryFile>
#include <KIO/NetAccess>

using namespace KCalCore;
using namespace KCalUtils;
using namespace CalendarSupport;

class GroupwareScoppedDisabler {
public:
  GroupwareScoppedDisabler(Akonadi::IncidenceChanger *changer) : m_changer(changer)
  {
    m_wasEnabled = m_changer->groupwareCommunication();
    m_changer->setGroupwareCommunication(false);
  }

  ~GroupwareScoppedDisabler()
  {
    m_changer->setGroupwareCommunication(m_wasEnabled);
  }

  bool m_wasEnabled;
  Akonadi::IncidenceChanger *m_changer;
};


EventArchiver::EventArchiver( QObject *parent )
 : QObject( parent )
{
}

EventArchiver::~EventArchiver()
{
}

void EventArchiver::runOnce( const Akonadi::ETMCalendar::Ptr &calendar,
                             Akonadi::IncidenceChanger *changer,
                             const QDate &limitDate, QWidget *widget )
{
  run( calendar, changer, limitDate, widget, true, true );
}

void EventArchiver::runAuto( const Akonadi::ETMCalendar::Ptr &calendar,
                             Akonadi::IncidenceChanger *changer,
                             QWidget *widget, bool withGUI )
{
  QDate limitDate( QDate::currentDate() );
  const int expiryTime = KCalPrefs::instance()->mExpiryTime;
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

void EventArchiver::run( const Akonadi::ETMCalendar::Ptr &calendar,
                         Akonadi::IncidenceChanger *changer,
                         const QDate &limitDate, QWidget *widget,
                         bool withGUI, bool errorIfNone )
{
  GroupwareScoppedDisabler disabler(changer); // Disables groupware communication temporarily

  // We need to use rawEvents, otherwise events hidden by filters will not be archived.
  KCalCore::Event::List events;
  KCalCore::Todo::List todos;
  KCalCore::Journal::List journals;

  if ( KCalPrefs::instance()->mArchiveEvents ) {
    events = calendar->rawEvents(
      QDate( 1769, 12, 1 ),
      // #29555, also advertised by the "limitDate not included" in the class docu
      limitDate.addDays( -1 ),
      CalendarSupport::KCalPrefs::instance()->timeSpec(),
      true );
  }
  if ( KCalPrefs::instance()->mArchiveTodos ) {
    KCalCore::Todo::List rawTodos = calendar->rawTodos();

    foreach( const KCalCore::Todo::Ptr &todo, rawTodos ) {
      Q_ASSERT( todo );
      if ( isSubTreeComplete( calendar, todo, limitDate ) ) {
        todos.append( todo );
      }
    }
  }

  const KCalCore::Incidence::List incidences = calendar->mergeIncidenceList( events, todos, journals );

  kDebug() << "archiving incidences before" << limitDate
           << " ->" << incidences.count() <<" incidences found.";
  if ( incidences.isEmpty() ) {
    if ( withGUI && errorIfNone ) {
      KMessageBox::information( widget,
                                i18n( "There are no items before %1",
                                      KGlobal::locale()->formatDate( limitDate ) ),
                                QLatin1String("ArchiverNoIncidences") );
    }
    return;
  }

  switch ( KCalPrefs::instance()->mArchiveAction ) {
  case KCalPrefs::actionDelete:
    deleteIncidences( changer, limitDate, widget, calendar->itemList( incidences ), withGUI );
    break;
  case KCalPrefs::actionArchive:
    archiveIncidences( calendar, changer, limitDate, widget, incidences, withGUI );
    break;
  }
}

void EventArchiver::deleteIncidences( Akonadi::IncidenceChanger *changer,
                                      const QDate &limitDate, QWidget *widget,
                                      const Akonadi::Item::List &items, bool withGUI )
{
  QStringList incidenceStrs;
  Akonadi::Item::List::ConstIterator it;
  Akonadi::Item::List::ConstIterator end( items.constEnd() );
  for ( it = items.constBegin(); it != end; ++it ) {
    incidenceStrs.append( CalendarSupport::incidence( *it )->summary() );
  }

  if ( withGUI ) {
    const int result = KMessageBox::warningContinueCancelList(
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

  changer->deleteIncidences( items, /**parent=*/widget );

  // TODO: emit only after hearing back from incidence changer
  emit eventsDeleted();
}

void EventArchiver::archiveIncidences( const Akonadi::ETMCalendar::Ptr &calendar,
                                       Akonadi::IncidenceChanger *changer,
                                       const QDate &limitDate, QWidget *widget,
                                       const KCalCore::Incidence::List &incidences, bool withGUI )
{
  Q_UNUSED( limitDate );
  Q_UNUSED( withGUI );

  FileStorage storage( calendar );

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
  foreach ( const KCalCore::Incidence::Ptr &incidence, incidences ) {
    uids.append( incidence->uid() );
  }
  foreach ( const KCalCore::Incidence::Ptr &incidence, allIncidences ) {
    if ( !uids.contains( incidence->uid() ) ) {
      archiveCalendar->deleteIncidence( incidence );
    }
  }

  // Get or create the archive file
  KUrl archiveURL( KCalPrefs::instance()->mArchiveFile );
  QString archiveFile;

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
  QFile::remove( tmpFileName );

  // We don't want it to ask to send invitations for each incidence.
  changer->startAtomicOperation( i18n( "Archiving events" ) );

  // Delete archived events from calendar
  Akonadi::Item::List items = calendar->itemList( incidences );
  foreach ( const Akonadi::Item &item, items ) {
    changer->deleteIncidence( item, widget );
  } // TODO: emit only after hearing back from incidence changer
  changer->endAtomicOperation();

  emit eventsDeleted();
}

bool EventArchiver::isSubTreeComplete( const Akonadi::ETMCalendar::Ptr &calendar,
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
  KCalCore::Incidence::List childs = calendar->childIncidences( todo->uid() );
  foreach ( const KCalCore::Incidence::Ptr &incidence, childs ) {
    const Todo::Ptr t = incidence.dynamicCast<KCalCore::Todo>();
    if ( t && !isSubTreeComplete( calendar, t, limitDate, checkedUids ) ) {
      return false;
    }
  }

  return true;
}

