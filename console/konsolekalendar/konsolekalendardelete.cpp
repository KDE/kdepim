/*******************************************************************************
 * konsolekalendardelete.cpp                                                   *
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
 * @file konsolekalendardelete.cpp
 * Provides the KonsoleKalendarDelete class definition.
 * @author Tuukka Pasanen
 * @author Allen Winter
 */
#include "konsolekalendardelete.h"

#include <stdlib.h>
#include <iostream>

#include <qdebug.h>
#include <KLocalizedString>
#include <QEventLoop>

using namespace KCalCore;
using namespace std;

KonsoleKalendarDelete::KonsoleKalendarDelete( KonsoleKalendarVariables *vars )
{
  m_variables = vars;
}

KonsoleKalendarDelete::~KonsoleKalendarDelete()
{
}

bool KonsoleKalendarDelete::deleteEvent()
{
  bool status = false;

  qDebug() << "konsolekalendardelete.cpp::deleteEvent()";

  /*
   * Retrieve event on the basis of the unique string ID
   */
  Event::Ptr event = m_variables->getCalendar()->event( m_variables->getUID() );
  if ( event ) {
    if ( m_variables->isDryRun() ) {
      cout << i18n( "Delete Event <Dry Run>:" ).data()
           << endl;
      printSpecs( event );
    } else {
      qDebug() << "konsolekalendardelete.cpp:deleteEvent() :"
               << m_variables->getUID().data();

      if ( m_variables->isVerbose() ) {
        cout << i18n( "Delete Event <Verbose>:" ).data()
             << endl;
        printSpecs( event );
      }

      QEventLoop loop;
      Akonadi::CalendarBase::Ptr calendar = m_variables->getCalendar();
      QObject::connect(calendar.data(), SIGNAL(deleteFinished(bool,QString)), &loop, SLOT(quit()));
      calendar->deleteEvent( event );
      loop.exec();
      qDebug() << "Finished deleting";
      status = calendar->incidence(event->uid()) == 0;

      if (status) {
          cout << i18n( "Success: \"%1\" deleted", event->summary() ).data() << endl;
      } else {
          cout << i18n( "Error deleting: \"%1\"", event->summary() ).data() << endl;
      }
    }
  }

  qDebug() << "konsolekalendardelete.cpp::deleteEvent() | Done";
  return status;
}

void KonsoleKalendarDelete::printSpecs( const Event::Ptr &event )
{
  cout << i18n( "  UID:   %1",  m_variables->getUID() ).data()
       << endl;

  cout << i18n( "  What:  %1", event->summary() ).data()
       << endl;

  KDateTime::Spec timeSpec = m_variables->getCalendar()->timeSpec();
  cout << i18n( "  Begin: %1",
     event->dtStart().toTimeSpec( timeSpec ).dateTime().toString( Qt::TextDate ) ).data()
       << endl;

  cout << i18n( "  End:   %1",
     event->dtEnd().toTimeSpec( timeSpec ).dateTime().toString( Qt::TextDate ) ).data()
       << endl;

  cout << i18n( "  Desc:  %1",
     event->description() ).data()
       << endl;

  cout << i18n( "  Location:  %1",
     event->location() ).data()
       << endl;
}
