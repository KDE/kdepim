/*******************************************************************************
 * konsolekalendaradd.cpp                                                      *
 *                                                                             *
 * KonsoleKalendar is a command line interface to KDE calendars                *
 * Copyright (C) 2002-2004  Tuukka Pasanen <illuusio@mailcity.com>             *
 * Copyright (C) 2003-2005  Allen Winter <winter@kde.org>                      *
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
 * @file konsolekalendaradd.cpp
 * Provides the KonsoleKalendarAdd class definition.
 * @author Tuukka Pasanen
 * @author Allen Winter
 */
#include "konsolekalendaradd.h"

#include <calendarsupport/kcalprefs.h>

#include <qdebug.h>

#include <ksystemtimezone.h>
#include <klocale.h>

#include <KCalCore/Event>
#include <Akonadi/Calendar/IncidenceChanger>
#include <AkonadiCore/Collection>

#include <QtCore/QDateTime>
#include <QtCore/QObject>
#include <QEventLoop>
#include <QElapsedTimer>

#include <stdlib.h>
#include <iostream>
#include <QStandardPaths>

using namespace KCalCore;
using namespace std;

KonsoleKalendarAdd::KonsoleKalendarAdd( KonsoleKalendarVariables *vars )
{
  m_variables = vars;
}

KonsoleKalendarAdd::~KonsoleKalendarAdd()
{
}

/**
 * Adds event to Calendar
 */

bool KonsoleKalendarAdd::addEvent()
{
  bool status = true;

  qDebug() << "konsolekalendaradd.cpp::addEvent()";

  if ( m_variables->isDryRun() ) {
    cout << i18n( "Insert Event &lt;Dry Run&gt;:" ).toLocal8Bit().data()
         << endl;
    printSpecs();
  } else {
    if ( m_variables->isVerbose() ) {
      cout << i18n( "Insert Event &lt;Verbose&gt;:" ).toLocal8Bit().data()
           << endl;
      printSpecs();
    }

    Event::Ptr event = Event::Ptr( new Event() );

    KDateTime::Spec timeSpec = m_variables->getCalendar()->timeSpec();
    event->setDtStart( KDateTime( m_variables->getStartDateTime(), timeSpec ) );
    event->setDtEnd( KDateTime( m_variables->getEndDateTime(), timeSpec ) );
    event->setSummary( m_variables->getSummary() );
    event->setAllDay( m_variables->getFloating() );
    event->setDescription( m_variables->getDescription() );
    event->setLocation( m_variables->getLocation() );

    Akonadi::CalendarBase::Ptr calendar = m_variables->getCalendar();
    QEventLoop loop;
    QObject::connect(calendar.data(), SIGNAL(createFinished(bool,QString)),
                     &loop, SLOT(quit()));
    QElapsedTimer t;
    t.start();
    Q_ASSERT(calendar->incidence(event->uid()) == 0 ); // can't exist yet
    if (!m_variables->allowGui()) {
       Akonadi::IncidenceChanger *changer = calendar->incidenceChanger();
       changer->setShowDialogsOnError(false);
       Akonadi::Collection collection = m_variables->collectionId() != -1 ? Akonadi::Collection(m_variables->collectionId())
                                                                          : Akonadi::Collection(CalendarSupport::KCalPrefs::instance()->defaultCalendarId());

       if (!collection.isValid()) {
           cout << i18n("Calendar is invalid. Please specify one with --calendar").toLocal8Bit().data() << "\n";
       }

       changer->setDefaultCollection(collection);
       changer->setDestinationPolicy(Akonadi::IncidenceChanger::DestinationPolicyNeverAsk);
    }
    calendar->addEvent(event);
    loop.exec();
    qDebug() << "Creation took " << t.elapsed() << "ms.";
    status = calendar->incidence(event->uid()) != 0;
    if ( status ) {
      cout << i18n( "Success: \"%1\" inserted",
                    m_variables->getSummary() ).toLocal8Bit().data()
           << endl;

    } else {
      cout << i18n( "Failure: \"%1\" not inserted",
                    m_variables->getSummary() ).toLocal8Bit().data()
           << endl;
      status = false;
    }
  }

  qDebug() << "konsolekalendaradd.cpp::addEvent() | Done";
  return status;
}

bool KonsoleKalendarAdd::addImportedCalendar()
{

// If --file specified, then import into that file
// else, import into the standard calendar
/*
 * TODO_SERGIO
  QString fileName;
  if ( m_variables->getCalendarFile().isEmpty() ) {
    fileName = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + "korganizer/std.ics" ;
  } else {
    fileName = m_variables->getCalendarFile();
  }

  CalendarLocal *cal = new CalendarLocal( KSystemTimeZones::local() );
  if ( !cal->load( fileName ) ||
       !cal->load( m_variables->getImportFile() ) ||
       !cal->save( fileName ) ) {
    qDebug() << "konsolekalendaradd.cpp::importCalendar() |"
             << "Can't import file:"
             << m_variables->getImportFile();
    return false;
  }
  qDebug() << "konsolekalendaradd.cpp::importCalendar() |"
           << "Successfully imported file:"
           << m_variables->getImportFile();
           */
  return true;
}

void KonsoleKalendarAdd::printSpecs()
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
