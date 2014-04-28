/******************************************************************************
 * konsolekalendarchange.cpp                                                  *
 *                                                                            *
 * KonsoleKalendar is a command line interface to KDE calendars               *
 * Copyright (C) 2002-2004  Tuukka Pasanen <illuusio@mailcity.com>            *
 * Copyright (C) 2003-2005  Allen Winter <winter@kde.org>                     *
 *                                                                            *
 * This program is free software; you can redistribute it and/or modify       *
 * it under the terms of the GNU General Public License as published by       *
 * the Free Software Foundation; either version 2 of the License, or          *
 * (at your option) any later version.                                        *
 *                                                                            *
 * This program is distributed in the hope that it will be useful,            *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the               *
 * GNU General Public License for more details.                               *
 *                                                                            *
 * You should have received a copy of the GNU General Public License along    *
 * with this program; if not, write to the Free Software Foundation, Inc.,    *
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.               *
 *                                                                            *
 * As a special exception, permission is given to link this program           *
 * with any edition of Qt, and distribute the resulting executable,           *
 * without including the source code for Qt in the source distribution.       *
 *                                                                            *
 *****************************************************************************/
/**
 * @file konsolekalendarchange.cpp
 * Provides the KonsoleKalendarChange class definition.
 * @author Tuukka Pasanen
 * @author Allen Winter
 */
#include "konsolekalendarchange.h"

#include <qdebug.h>
#include <klocale.h>

#include <QEventLoop>
#include <QElapsedTimer>

#include <stdlib.h>
#include <iostream>

using namespace KCalCore;
using namespace std;

KonsoleKalendarChange::KonsoleKalendarChange( KonsoleKalendarVariables *vars )
{
  m_variables = vars;
}

KonsoleKalendarChange::~KonsoleKalendarChange()
{
}

bool KonsoleKalendarChange::changeEvent()
{
  bool status = false;

  qDebug() << "konsolekalendarchange.cpp::changeEvent()";

  /*
   * Retrieve event on the basis of the unique string ID
   */
  Event::Ptr event = m_variables->getCalendar()->event( m_variables->getUID() );
  if ( event ) {
    if ( m_variables->isDryRun() ) {
      cout << i18n( "Change Event &lt;Dry Run&gt;:" ).toLocal8Bit().data()
           << endl;
      printSpecs( event );

      cout << i18n( "To Event &lt;Dry Run&gt;:" ).toLocal8Bit().data()
           << endl;
      printSpecs();
    } else {
      qDebug() << "konsolekalendarchange.cpp:changeEvent() :"
               << m_variables->getUID().toLocal8Bit().data();

      if ( m_variables->isVerbose() ) {
        cout << i18n( "Change Event &lt;Verbose&gt;:" ).toLocal8Bit().data()
             << endl;
        printSpecs( event );

        cout << i18n( "To Event &lt;Dry Run&gt;:" ).toLocal8Bit().data()
             << endl;
        printSpecs();
      }

      event->startUpdates();
      Akonadi::CalendarBase::Ptr calendar = m_variables->getCalendar();
      KDateTime::Spec timeSpec = calendar->timeSpec();
      if ( m_variables->isStartDateTime() ) {
        event->setDtStart( KDateTime( m_variables->getStartDateTime(), timeSpec ) );
      }

      if ( m_variables->isEndDateTime() ) {
        event->setDtEnd( KDateTime( m_variables->getEndDateTime(), timeSpec ) );
      }

      event->setAllDay( m_variables->getFloating() );

      if ( m_variables->isSummary() ) {
        event->setSummary( m_variables->getSummary() );
      }

      if ( m_variables->isDescription() ) {
        event->setDescription( m_variables->getDescription() );
      }

      if ( m_variables->isLocation() ) {
        event->setLocation( m_variables->getLocation() );
      }
      event->endUpdates();
      QEventLoop loop;
      QObject::connect(calendar.data(), SIGNAL(modifyFinished(bool,QString)),
                       &loop, SLOT(quit()));
      QElapsedTimer t;
      t.start();
      calendar->modifyIncidence(event);
      loop.exec();

      status = *event == *calendar->incidence(event->uid());

      if ( status ) {
        cout << i18n( "Success: \"%1\" changed", event->summary() ).toLocal8Bit().data()
             << endl;
      } else {
        cout << i18n( "Failure: \"%1\" not changed", event->summary() ).toLocal8Bit().data()
             << endl;
      }
    }
  }

  qDebug() << "konsolekalendarchange.cpp::changeEvent() | Done";
  return status;
}

void KonsoleKalendarChange::printSpecs( const Event::Ptr &event )
{
  cout << i18n( "  UID:   %1",
     event->uid() ).toLocal8Bit().data()
       << endl;

  cout << i18n( "  What:  %1",
     event->summary() ).toLocal8Bit().data()
       << endl;

  KDateTime::Spec timeSpec = m_variables->getCalendar()->timeSpec();
  cout << i18n( "  Begin: %1",
     event->dtStart().toTimeSpec( timeSpec ).
                dateTime().toString( Qt::TextDate ) ).toLocal8Bit().data()
       << endl;

  cout << i18n( "  End:   %1",
     event->dtEnd().toTimeSpec( timeSpec ).
                dateTime().toString( Qt::TextDate ) ).toLocal8Bit().data()
       << endl;

  cout << i18n( "  Desc:  %1",
     event->description() ).toLocal8Bit().data()
       << endl;

  cout << i18n( "  Location:  %1",
     event->location() ).toLocal8Bit().data()
       << endl;
}

void KonsoleKalendarChange::printSpecs()
{
  cout << i18n( "  UID:   %1",
                m_variables->getUID() ).toLocal8Bit().data()
       << endl;

  cout << i18n( "  What:  %1",
                m_variables->getSummary() ).toLocal8Bit().data()
       << endl;

  cout << i18n( "  Begin: %1",
                m_variables->getStartDateTime().toString( Qt::TextDate ) ).toLocal8Bit().data()
       << endl;

  cout << i18n( "  End:   %1",
                m_variables->getEndDateTime().toString( Qt::TextDate ) ).toLocal8Bit().data()
       << endl;

  cout << i18n( "  Desc:  %1",
                m_variables->getDescription() ).toLocal8Bit().data()
       << endl;

  cout << i18n( "  Location:  %1",
                m_variables->getLocation() ).toLocal8Bit().data()
       << endl;
}
