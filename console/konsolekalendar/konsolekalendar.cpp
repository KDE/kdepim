/*******************************************************************************
 * konsolekalendar.cpp                                                         *
 *                                                                             *
 * KonsoleKalendar is a command line interface to KDE calendars                *
 * Copyright (C) 2002-2004  Tuukka Pasanen <illuusio@mailcity.com>             *
 * Copyright (C) 2003-2005  Allen Winter <winter@kde.org>
 *                                                                             *
 * This program is free software; you can redistribute it and/or modify        *
 * it under the terms of the GNU General Public License as published by        *
 * the Free Software Foundation; either version 2 of the License, or           *
 * (at your option) any later version.                                         *
 *                                                                             *
 * This program is distributed in the hope that it will be useful,             *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                *
 * GNU General Public License for more details.                                *
 *                                                                             *
 * You should have received a copy of the GNU General Public License           *
 * along with this program; if not, write to the Free Software                 *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA. *
 *                                                                             *
 * As a special exception, permission is given to link this program            *
 * with any edition of Qt, and distribute the resulting executable,            *
 * without including the source code for Qt in the source distribution.        *
 *                                                                             *
 ******************************************************************************/
/**
 * @file konsolekalendar.cpp
 * Provides the KonsoleKalendar class definition.
 * @author Tuukka Pasanen
 * @author Allen Winter
 */
#include "konsolekalendar.h"
#include "konsolekalendaradd.h"
#include "konsolekalendarchange.h"
#include "konsolekalendardelete.h"
#include "konsolekalendarexports.h"

#include <qdebug.h>
#include <klocale.h>
#include <ksystemtimezone.h>

#include <KCalCore/Event>
#include <KCalUtils/HtmlExport>
#include <KCalUtils/HTMLExportSettings>
#include <AkonadiCore/AgentManager>
#include <AkonadiCore/AgentInstanceCreateJob>
#include <AkonadiCore/CollectionFetchJob>
#include <AkonadiCore/Collection>
#include <AkonadiCore/CollectionFetchScope>

#include <QtCore/QDateTime>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QEventLoop>

#include <stdio.h>
#include <stdlib.h>
#include <iostream>

using namespace KCalCore;
using namespace std;

KonsoleKalendar::KonsoleKalendar( KonsoleKalendarVariables *variables )
{
  m_variables = variables;
}

KonsoleKalendar::~KonsoleKalendar()
{
}

bool KonsoleKalendar::importCalendar()
{
  KonsoleKalendarAdd add( m_variables );

  qDebug() << "konsolecalendar.cpp::importCalendar() | importing now!";
  return add.addImportedCalendar();
}

bool KonsoleKalendar::printCalendarList()
{
    Akonadi::CollectionFetchJob *job = new Akonadi::CollectionFetchJob(Akonadi::Collection::root(),
                                                                       Akonadi::CollectionFetchJob::Recursive);
    QStringList mimeTypes = QStringList() << QLatin1String("text/calendar")
                                          << KCalCore::Event::eventMimeType()
                                          << KCalCore::Todo::todoMimeType()
                                          << KCalCore::Journal::journalMimeType();
    job->fetchScope().setContentMimeTypes( mimeTypes );
    QEventLoop loop;
    QObject::connect(job, SIGNAL(result(KJob*)), &loop, SLOT(quit()));
    job->start();
    loop.exec();

    if (job->error() != 0)
        return false;

    Akonadi::Collection::List collections = job->collections();

    if (collections.isEmpty()) {
        cout << i18n("There are no calendars available.").toLocal8Bit().data() << endl;
    } else {
        cout << "--------------------------" << endl;
        QSet<QString> mimeTypeSet = mimeTypes.toSet();
        foreach(const Akonadi::Collection &collection, collections) {
            if (!mimeTypeSet.intersect(collection.contentMimeTypes().toSet()).isEmpty()) {
                QString colId = QString::number(collection.id()).leftJustified(6, QLatin1Char(' '));
                colId += QLatin1String("- ");

                bool readOnly = !( collection.rights() & Akonadi::Collection::CanCreateItem ||
                                   collection.rights() & Akonadi::Collection::CanChangeItem ||
                                   collection.rights() & Akonadi::Collection::CanDeleteItem );

                QString readOnlyString = readOnly ? i18n("(Read only)") + QLatin1Char(' ') : QString();

                cout << colId.toLocal8Bit().data() << readOnlyString.toLocal8Bit().constData() << collection.displayName().toLocal8Bit().data() << endl;
            }
        }
    }

    return true;
}

bool KonsoleKalendar::createAkonadiResource(const QString &icalFileName)
{
    Akonadi::AgentType type = Akonadi::AgentManager::self()->type(QLatin1String("akonadi_ical_resource"));
    Akonadi::AgentInstanceCreateJob *job = new Akonadi::AgentInstanceCreateJob(type);
    job->setProperty("path", icalFileName);
    QEventLoop loop;
    QObject::connect(job, SIGNAL(result(KJob*)), &loop, SLOT(quit()));
    job->start();
    loop.exec();
    return job->error() == 0;
}

bool KonsoleKalendar::createCalendar()
{
  bool status = false;

  const QString filename = m_variables->getCalendarFile();

  if ( m_variables->isDryRun() ) {
    cout << i18n( "Create Calendar &lt;Dry Run&gt;: %1", filename ).toLocal8Bit().data()
         << endl;
  } else {
    qDebug() << "konsolekalendar.cpp::createCalendar() |"
             << "Creating calendar file: "
             << filename.toLocal8Bit().data();

    if ( m_variables->isVerbose() ) {
      cout << i18n( "Create Calendar &lt;Verbose&gt;: %1", filename ).toLocal8Bit().data()
           << endl;
    }

    status = createAkonadiResource(filename);
  }

  return status;
}

bool KonsoleKalendar::showInstance()
{
  bool status = true;
  QFile f;
  QString title;
  Event::Ptr event;
  const KDateTime::Spec timeSpec = m_variables->getCalendar()->timeSpec();
  Akonadi::CalendarBase::Ptr calendar = m_variables->getCalendar();

  if ( m_variables->isDryRun() ) {
    cout << i18n( "View Events &lt;Dry Run&gt;:" ).toLocal8Bit().data()
         << endl;
    printSpecs();
  } else {
    qDebug() << "konsolekalendar.cpp::showInstance() |"
             << "open export file";

    if ( m_variables->isExportFile() ) {
      f.setFileName( m_variables->getExportFile() );
      if ( !f.open( QIODevice::WriteOnly ) ) {
        status = false;
        qDebug() << "konsolekalendar.cpp::showInstance() |"
                 << "unable to open export file"
                 << m_variables->getExportFile();
      }
    } else {
      f.open( stdout, QIODevice::WriteOnly );
    }

    if ( status ) {
      qDebug() << "konsolekalendar.cpp::showInstance() |"
               << "opened successful";

      if ( m_variables->isVerbose() ) {
        cout << i18n( "View Event &lt;Verbose&gt;:" ).toLocal8Bit().data()
             << endl;
        printSpecs();
      }

      QTextStream ts( &f );

      if ( m_variables->getExportType() != ExportTypeHTML &&
           m_variables->getExportType() != ExportTypeMonthHTML ) {

        if ( m_variables->getAll() ) {
          qDebug() << "konsolekalendar.cpp::showInstance() |"
                   << "view all events sorted list";

          Event::List sortedList = calendar->events( EventSortStartDate );
          qDebug() << "Found" << sortedList.count() << "events";
          if ( !sortedList.isEmpty() ) {
            // The code that was here before the akonadi port was really slow with 200 events
            // this is much faster:
            foreach( const KCalCore::Event::Ptr &event, sortedList ) {
              status &= printEvent( &ts, event, event->dtStart().date() );
            }
          }
        } else if ( m_variables->isUID() ) {
          qDebug() << "konsolekalendar.cpp::showInstance() |"
                   << "view events by uid list";
          //TODO: support a list of UIDs
          event = calendar->event( m_variables->getUID() );
          //If this UID represents a recurring Event,
          //only the first day of the Event will be printed
          status = printEvent ( &ts, event, event->dtStart().date() );

        } else if ( m_variables->isNext() ) {
          qDebug() << "konsolekalendar.cpp::showInstance() |"
                   << "Show next activity in calendar";

          QDateTime datetime = m_variables->getStartDateTime();
          datetime = datetime.addDays( 720 );

          QDate dt;
          for ( dt = m_variables->getStartDateTime().date();
                dt <= datetime.date();
                dt = dt.addDays( 1 ) ) {
            Event::List events = calendar->events( dt, timeSpec,
                                                   EventSortStartDate,
                                                   SortDirectionAscending );
            qDebug() << "2-Found" << events.count() << "events on date" << dt;
            // finished here when we get the next event
            if ( !events.isEmpty() ) {
              qDebug() << "konsolekalendar.cpp::showInstance() |"
                       << "Got the next event";
              printEvent( &ts, events.first(), dt );
              return true;
            }
          }
        } else {
          qDebug() << "konsolekalendar.cpp::showInstance() |"
                   << "view raw events within date range list";

          QDate dt;
          for ( dt = m_variables->getStartDateTime().date();
                dt <= m_variables->getEndDateTime().date() && status != false;
                dt = dt.addDays( 1 ) ) {
            Event::List events = calendar->events( dt, timeSpec,
                                                   EventSortStartDate,
                                                   SortDirectionAscending );
            qDebug() << "3-Found" << events.count() << "events on date: " << dt;
            status = printEventList( &ts, &events, dt );
          }
        }
      } else {
        QDate firstdate, lastdate;
        if ( m_variables->getAll() ) {
          qDebug() << "konsolekalendar.cpp::showInstance() |"
                   << "HTML view all events sorted list";
          // sort the events for this date by start date
          // in order to determine the date range.
          Event::List *events =
            new Event::List ( calendar->rawEvents( EventSortStartDate,
                                                   SortDirectionAscending ) );
          firstdate = events->first()->dtStart().date();
          lastdate = events->last()->dtStart().date();
        } else if ( m_variables->isUID() ) {
          // TODO
          qDebug() << "konsolekalendar.cpp::showInstance() |"
                   << "HTML view events by uid list";
          cout << i18n( "Sorry, export to HTML by UID is not supported yet" ).
            toLocal8Bit().data() << endl;
          return false;
        } else {
          qDebug() << "konsolekalendar.cpp::showInstance() |"
                   << "HTML view raw events within date range list";
          firstdate = m_variables->getStartDateTime().date();
          lastdate = m_variables->getEndDateTime().date();
        }

        KCalUtils::HTMLExportSettings htmlSettings( QLatin1String("Konsolekalendar") );

        //TODO: get progname and url from the values set in main
        htmlSettings.setCreditName( QLatin1String("KonsoleKalendar") );
        htmlSettings.setCreditURL(
          QLatin1String("http://pim.kde.org/components/konsolekalendar.php") );

        htmlSettings.setExcludePrivate( true );
        htmlSettings.setExcludeConfidential( true );

        htmlSettings.setEventView( false );
        htmlSettings.setMonthView( false );
        if ( m_variables->getExportType() == ExportTypeMonthHTML ) {
          title = i18n( "Events:" );
          htmlSettings.setMonthView( true );
        } else {
          if ( firstdate == lastdate ) {
            title = i18n( "Events: %1",
                          firstdate.toString( Qt::TextDate ) );
          } else {
            title = i18n( "Events: %1 - %2",
                          firstdate.toString( Qt::TextDate ),
                          lastdate.toString( Qt::TextDate ) );
          }
          htmlSettings.setEventView( true );
        }
        htmlSettings.setEventTitle( title );
        htmlSettings.setEventAttendees( true );
// Not supporting Todos yet
//         title = "To-Do List for " + firstdate.toString(Qt::TextDate);
//         if ( firstdate != lastdate ) {
//           title += " - " + lastdate.toString(Qt::TextDate);
//         }
        htmlSettings.setTodoListTitle( title );
        htmlSettings.setTodoView( false );
//         htmlSettings.setTaskCategories( false );
//         htmlSettings.setTaskAttendees( false );
//         htmlSettings.setTaskDueDate( true );

        htmlSettings.setDateStart( QDateTime( firstdate ) );
        htmlSettings.setDateEnd( QDateTime( lastdate ) ) ;

        KCalUtils::HtmlExport *exp = new KCalUtils::HtmlExport( calendar.data(), &htmlSettings );
        status = exp->save( &ts );
        delete exp;
      }
      f.close();
    }
  }
  return status;
}

bool KonsoleKalendar::printEventList( QTextStream *ts,
                                      Event::List *eventList, QDate date )
{
  bool status = true;

  qDebug() << eventList->count();
  if ( !eventList->isEmpty() ) {
    Event::Ptr singleEvent;
    Event::List::ConstIterator it;

    for ( it = eventList->constBegin();
          it != eventList->constEnd() && status != false;
          ++it ) {
      singleEvent = *it;

      status = printEvent( ts, singleEvent, date );
    }
  }

  return status;
}

bool KonsoleKalendar::printEvent( QTextStream *ts, const Event::Ptr &event, QDate dt )
{
  bool status = false;
  bool sameDay = true;
  KonsoleKalendarExports exports;

  if ( event ) {
    switch ( m_variables->getExportType() ) {

    case ExportTypeCSV:
      qDebug() << "konsolekalendar.cpp::printEvent() |"
               << "CSV export";
      status = exports.exportAsCSV( ts, event, dt );
      break;

    case ExportTypeTextShort:
      qDebug() << "konsolekalendar.cpp::printEvent() |"
               << "TEXT-SHORT export";
      if ( dt.daysTo( m_saveDate ) ) {
        sameDay = false;
        m_saveDate = dt;
      }
      status = exports.exportAsTxtShort( ts, event, dt, sameDay );
      break;

    case ExportTypeHTML:
      // this is handled separately for now
      break;

    default:// Default export-type is ExportTypeText
      qDebug() << "konsolekalendar.cpp::printEvent() |"
               << "TEXT export";
      status = exports.exportAsTxt( ts, event, dt );
      break;
    }
  }
  return status;
}

bool KonsoleKalendar::addEvent()
{
  qDebug() << "konsolecalendar.cpp::addEvent() |"
           << "Create Adding";
  KonsoleKalendarAdd add( m_variables );
  qDebug() << "konsolecalendar.cpp::addEvent() |"
           << "Adding Event now!";
  return add.addEvent();
}

bool KonsoleKalendar::changeEvent()
{

  qDebug() << "konsolecalendar.cpp::changeEvent() |"
           << "Create Changing";
  KonsoleKalendarChange change( m_variables );
  qDebug() << "konsolecalendar.cpp::changeEvent() |"
           << "Changing Event now!";
  return change.changeEvent();
}

bool KonsoleKalendar::deleteEvent()
{
  qDebug() << "konsolecalendar.cpp::deleteEvent() |"
           << "Create Deleting";
  KonsoleKalendarDelete del( m_variables );
  qDebug() << "konsolecalendar.cpp::deleteEvent() |"
           << "Deleting Event now!";
  return del.deleteEvent();
}

bool KonsoleKalendar::isEvent( const QDateTime &startdate,
                               const QDateTime &enddate, const QString &summary )
{
  // Search for an event with specified start and end datetime stamp and summary

  Event::Ptr event;
  Event::List::ConstIterator it;

  bool found = false;

  KDateTime::Spec timeSpec = m_variables->getCalendar()->timeSpec();
  Event::List eventList( m_variables->getCalendar()->
                         rawEventsForDate( startdate.date(), timeSpec,
                                           EventSortStartDate,
                                           SortDirectionAscending ) );
  for ( it = eventList.constBegin(); it != eventList.constEnd(); ++it ) {
    event = *it;
    if ( event->dtEnd().toTimeSpec( timeSpec ).dateTime() == enddate &&
         event->summary() == summary ) {
      found = true;
      break;
    }
  }
  return found;
}

void KonsoleKalendar::printSpecs()
{
  cout << i18n( "  What:  %1",
                m_variables->getSummary() ).toLocal8Bit().data()
       << endl;

  cout << i18n( "  Begin: %1",
                m_variables->getStartDateTime().toString( Qt::TextDate ) ).toLocal8Bit().data()
       << endl;

  cout << i18n( "  End:   %1",
                m_variables->getEndDateTime().toString( Qt::TextDate ) ).toLocal8Bit().data()
       << endl;

  if ( m_variables->getFloating() == true ) {
    cout << i18n( "  No Time Associated with Event" ).toLocal8Bit().data()
         << endl;
  }

  cout << i18n( "  Desc:  %1",
                m_variables->getDescription() ).toLocal8Bit().data()
       << endl;

  cout << i18n( "  Location:  %1",
                m_variables->getLocation() ).toLocal8Bit().data()
       << endl;
}
