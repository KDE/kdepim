/*
    This file is part of libkcal.

    Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#include <qapplication.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qtextcodec.h>
#include <qregexp.h>

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <kcalendarsystem.h>

#include <libkcal/calendar.h>
#include <libkcal/event.h>
#include <libkcal/todo.h>

#ifndef KORG_NOKABC
 #include <kabc/stdaddressbook.h>
#endif
#include "htmlexport.h"
#include "htmlexportsettings.h"

using namespace KCal;

HtmlExport::HtmlExport( Calendar *calendar, HTMLExportSettings *settings ) :
  mCalendar( calendar ), mSettings( settings )
{
}

bool HtmlExport::save( const QString &fileName )
{
  QString fn( fileName );
  if ( fn.isEmpty() && mSettings ) {
    fn = mSettings->outputFile();
  }
  if ( !mSettings || fn.isEmpty() ) {
    return false;
  }
  QFile f( fileName );
  if ( !f.open(IO_WriteOnly)) {
    return false;
  }
  QTextStream ts(&f);
  bool success = save(&ts);
  f.close();
  return success;
}

bool HtmlExport::save(QTextStream *ts)
{
  if ( !mSettings ) return false;
  ts->setEncoding( QTextStream::UnicodeUTF8 );

  // Write HTML header
  *ts << "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" ";
  *ts << "\"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n";

  *ts << "<html><head>" << endl;
  *ts << "  <meta http-equiv=\"Content-Type\" content=\"text/html; charset=";
  *ts << "UTF-8\" />\n";
  if ( !mSettings->pageTitle().isEmpty())
    *ts << "  <title>" << mSettings->pageTitle() << "</title>\n";
  *ts << "  <style type=\"text/css\">\n";
  *ts << styleSheet();
  *ts << "  </style>\n";
  *ts << "</head><body>\n";

  // FIXME: Write header
  // (Heading, Calendar-Owner, Calendar-Date, ...)

  if ( mSettings->eventView() || mSettings->monthView() || mSettings->weekView() ) {
    if (!mSettings->eventTitle().isEmpty())
      *ts << "<h1>" << mSettings->eventTitle() << "</h1>\n";

    // Write Week View
    if ( mSettings->weekView() )
      createWeekView( ts );
    // Write Month View
    if ( mSettings->monthView() )
      createMonthView( ts );
    // Write Event List
    if ( mSettings->eventView() )
      createEventList( ts );
  }

  // Write Todo List
  if ( mSettings->todoView() ) {
    if ( !mSettings->todoListTitle().isEmpty())
      *ts << "<h1>" << mSettings->todoListTitle() << "</h1>\n";
    createTodoList(ts);
  }

  // Write Journals
  if ( mSettings->journalView() ) {
    if ( !mSettings->journalTitle().isEmpty())
      *ts << "<h1>" << mSettings->journalTitle() << "</h1>\n";
    createJournalView(ts);
  }

  // Write Free/Busy
  if ( mSettings->freeBusyView() ) {
    if ( !mSettings->freeBusyTitle().isEmpty())
      *ts << "<h1>" << mSettings->freeBusyTitle() << "</h1>\n";
    createFreeBusyView(ts);
  }

  createFooter( ts );
  
  // Write HTML trailer
  *ts << "</body></html>\n";

  return true;
}

void HtmlExport::createMonthView(QTextStream *ts)
{
  QDate start = fromDate();
  start.setYMD( start.year(), start.month(), 1 );  // go back to first day in month

  QDate end( start.year(), start.month(), start.daysInMonth() );

  int startmonth = start.month();
  int startyear = start.year();

  while ( start < toDate() ) {
    // Write header
    *ts << "<h2>" << (i18n("month_year","%1 %2").arg(KGlobal::locale()->calendar()->monthName(start))
        .arg(start.year())) << "</h2>\n";
    if ( KGlobal::locale()->weekStartDay() == 1 ) {
      start = start.addDays(1 - start.dayOfWeek());
    } else {
      if (start.dayOfWeek() != 7) {
        start = start.addDays(-start.dayOfWeek());
      }
    }
    *ts << "<table border=\"1\">\n";

    // Write table header
    *ts << "  <tr>";
    for(int i=0; i<7; ++i) {
      *ts << "<th>" << KGlobal::locale()->calendar()->weekDayName( start.addDays(i) ) << "</th>";
    }
    *ts << "</tr>\n";

    // Write days
    while (start <= end) {
      *ts << "  <tr>\n";
      for(int i=0;i<7;++i) {
        *ts << "    <td valign=\"top\"><table border=\"0\">";

        *ts << "<tr><td ";
        if (mHolidayMap.contains(start) || start.dayOfWeek() == 7) {
          *ts << "class=\"dateholiday\"";
        } else {
          *ts << "class=\"date\"";
        }
        *ts << ">" << QString::number(start.day());

        if (mHolidayMap.contains(start)) {
          *ts << " <em>" << mHolidayMap[start] << "</em>";
        }

        *ts << "</td></tr><tr><td valign=\"top\">";

        Event::List events = mCalendar->events(start,true);
        if (events.count()) {
          *ts << "<table>";
          Event::List::ConstIterator it;
          for( it = events.begin(); it != events.end(); ++it ) {
            if ( checkSecrecy( *it ) ) {
              createEvent( ts, *it, start, false );
            }
          }
          *ts << "</table>";
        } else {
          *ts << "&nbsp;";
        }

        *ts << "</td></tr></table></td>\n";
        start = start.addDays(1);
      }
      *ts << "  </tr>\n";
    }
    *ts << "</table>\n";
    startmonth += 1;
    if ( startmonth > 12 ) {
      startyear += 1;
      startmonth = 1;
    }
    start.setYMD( startyear, startmonth, 1 );
    end.setYMD(start.year(),start.month(),start.daysInMonth());
  }
}

void HtmlExport::createEventList (QTextStream *ts)
{
  int columns = 3;
  *ts << "<table border=\"0\" cellpadding=\"3\" cellspacing=\"3\">\n";
  *ts << "  <tr>\n";
  *ts << "    <th class=\"sum\">" << i18n("Start Time") << "</th>\n";
  *ts << "    <th>" << i18n("End Time") << "</th>\n";
  *ts << "    <th>" << i18n("Event") << "</th>\n";
  if ( mSettings->eventCategories() ) {
    *ts << "    <th>" << i18n("Categories") << "</th>\n";
    ++columns;
  }
  if ( mSettings->eventAttendees() ) {
    *ts << "    <th>" << i18n("Attendees") << "</th>\n";
    ++columns;
  }

  *ts << "  </tr>\n";
  
  for ( QDate dt = fromDate(); dt <= toDate(); dt = dt.addDays(1) ) {
    kdDebug(5850) << "Getting events for " << dt.toString() << endl;
    Event::List events = mCalendar->events(dt,true);
    if (events.count()) {
      *ts << "  <tr><td colspan=\"" << QString::number(columns)
          << "\" class=\"datehead\"><i>"
          << KGlobal::locale()->formatDate(dt)
          << "</i></td></tr>\n";

      Event::List::ConstIterator it;
      for( it = events.begin(); it != events.end(); ++it ) {
        if ( checkSecrecy( *it ) ) {
          createEvent( ts, *it, dt );
        }
      }
    }
  }

  *ts << "</table>\n";
}

void HtmlExport::createEvent (QTextStream *ts, Event *event,
                                       QDate date,bool withDescription)
{
  kdDebug(5850) << "HtmlExport::createEvent(): " << event->summary() << endl;
  *ts << "  <tr>\n";

  if (!event->doesFloat()) {
    if (event->isMultiDay() && (event->dtStart().date() != date)) {
      *ts << "    <td>&nbsp;</td>\n";
    } else {
      *ts << "    <td valign=\"top\">" << event->dtStartTimeStr() << "</td>\n";
    }
    if (event->isMultiDay() && (event->dtEnd().date() != date)) {
      *ts << "    <td>&nbsp;</td>\n";
    } else {
      *ts << "    <td valign=\"top\">" << event->dtEndTimeStr() << "</td>\n";
    }
  } else {
    *ts << "    <td>&nbsp;</td><td>&nbsp;</td>\n";
  }

  *ts << "    <td class=\"sum\">\n";
  *ts << "      <b>" << cleanChars(event->summary()) << "</b>\n";
  if ( withDescription && !event->description().isEmpty() ) {
    *ts << "      <p>" << breakString( cleanChars( event->description() ) ) << "</p>\n";
  }
  *ts << "    </td>\n";

  if ( mSettings->eventCategories() ) {
    *ts << "  <td>\n";
    formatCategories( ts, event );
    *ts << "  </td>\n";
  }

  if ( mSettings->eventAttendees() ) {
    *ts << "  <td>\n";
    formatAttendees( ts, event );
    *ts << "  </td>\n";
  }

  *ts << "  </tr>\n";
}

void HtmlExport::createTodoList ( QTextStream *ts )
{
  Todo::List rawTodoList = mCalendar->todos();

  Todo::List::Iterator it = rawTodoList.begin();
  while ( it != rawTodoList.end() ) {
    Todo *ev = *it;
    Todo *subev = ev;
    if ( ev->relatedTo() ) {
      if ( ev->relatedTo()->type()=="Todo" ) {
        if ( rawTodoList.find( static_cast<Todo *>( ev->relatedTo() ) ) ==
             rawTodoList.end() ) {
          rawTodoList.append( static_cast<Todo *>( ev->relatedTo() ) );
        }
      }
    }
    it = rawTodoList.find( subev );
    ++it;
  }

  // FIXME: Sort list by priorities. This is brute force and should be
  // replaced by a real sorting algorithm.
  Todo::List todoList;
  for ( int i = 1; i <= 9; ++i ) {
    for( it = rawTodoList.begin(); it != rawTodoList.end(); ++it ) {
      if ( (*it)->priority() == i && checkSecrecy( *it ) ) {
        todoList.append( *it );
      }
    }
  }
  for( it = rawTodoList.begin(); it != rawTodoList.end(); ++it ) {
    if ( (*it)->priority() == 0 && checkSecrecy( *it ) ) {
      todoList.append( *it );
    }
 }

  int columns = 3;
  *ts << "<table border=\"0\" cellpadding=\"3\" cellspacing=\"3\">\n";
  *ts << "  <tr>\n";
  *ts << "    <th class=\"sum\">" << i18n("Task") << "</th>\n";
  *ts << "    <th>" << i18n("Priority") << "</th>\n";
  *ts << "    <th>" << i18n("Completed") << "</th>\n";
  if ( mSettings->taskDueDate() ) {
    *ts << "    <th>" << i18n("Due Date") << "</th>\n";
    ++columns;
  }
  if ( mSettings->taskCategories() ) {
    *ts << "    <th>" << i18n("Categories") << "</th>\n";
    ++columns;
  }
  if ( mSettings->taskAttendees() ) {
    *ts << "    <th>" << i18n("Attendees") << "</th>\n";
    ++columns;
  }
  *ts << "  </tr>\n";

  // Create top-level list.
  for( it = todoList.begin(); it != todoList.end(); ++it ) {
    if ( !(*it)->relatedTo() ) createTodo( ts, *it );
  }

  // Create sub-level lists
  for( it = todoList.begin(); it != todoList.end(); ++it ) {
    Incidence::List relations = (*it)->relations();
    if (relations.count()) {
      // Generate sub-task list of event ev
      *ts << "  <tr>\n";
      *ts << "    <td class=\"subhead\" colspan=";
      *ts << "\"" << QString::number(columns) << "\"";
      *ts << "><a name=\"sub" << (*it)->uid() << "\"></a>"
          << i18n("Sub-Tasks of: ") << "<a href=\"#"
          << (*it)->uid() << "\"><b>" << cleanChars( (*it)->summary())
          << "</b></a></td>\n";
      *ts << "  </tr>\n";

      Todo::List sortedList;
      // FIXME: Sort list by priorities. This is brute force and should be
      // replaced by a real sorting algorithm.
      for ( int i = 1; i <= 9; ++i ) {
        Incidence::List::ConstIterator it2;
        for( it2 = relations.begin(); it2 != relations.end(); ++it2 ) {
          Todo *ev3 = dynamic_cast<Todo *>( *it2 );
          if ( ev3 && ev3->priority() == i ) sortedList.append( ev3 );
        }
      }
      Incidence::List::ConstIterator it2;
      for( it2 = relations.begin(); it2 != relations.end(); ++it2 ) {
        Todo *ev3 = dynamic_cast<Todo *>( *it2 );
        if ( ev3 && ev3->priority() == 0 ) sortedList.append( ev3 );
      }

      Todo::List::ConstIterator it3;
      for( it3 = sortedList.begin(); it3 != sortedList.end(); ++it3 ) {
        createTodo( ts, *it3 );
      }
    }
  }

  *ts << "</table>\n";
}

void HtmlExport::createTodo (QTextStream *ts,Todo *todo)
{
  kdDebug(5850) << "HtmlExport::createTodo()" << endl;

  bool completed = todo->isCompleted();
  Incidence::List relations = todo->relations();

  *ts << "<tr>\n";

  *ts << "  <td class=\"sum\"";
  if (completed) *ts << "done";
  *ts << ">\n";
  *ts << "    <a name=\"" << todo->uid() << "\"></a>\n";
  *ts << "    <b>" << cleanChars(todo->summary()) << "</b>\n";
  if (!todo->description().isEmpty()) {
    *ts << "    <p>" << breakString(cleanChars(todo->description())) << "</p>\n";
  }
  if (relations.count()) {
    *ts << "    <div align=\"right\"><a href=\"#sub" << todo->uid()
        << "\">" << i18n("Sub-Tasks") << "</a></div>\n";
  }

  *ts << "  </td";
  if (completed) *ts << " class=\"done\"";
  *ts << ">\n";

  *ts << "  <td";
  if (completed) *ts << " class=\"done\"";
  *ts << ">\n";
  *ts << "    " << todo->priority() << "\n";
  *ts << "  </td>\n";

  *ts << "  <td";
  if (completed) *ts << " class=\"done\"";
  *ts << ">\n";
  *ts << "    " << i18n("%1 %").arg(todo->percentComplete()) << "\n";
  *ts << "  </td>\n";

  if ( mSettings->taskDueDate() ) {
    *ts << "  <td";
    if (completed) *ts << " class=\"done\"";
    *ts << ">\n";
    if (todo->hasDueDate()) {
      *ts << "    " << todo->dtDueDateStr() << "\n";
    } else {
      *ts << "    &nbsp;\n";
    }
    *ts << "  </td>\n";
  }

  if ( mSettings->taskCategories() ) {
    *ts << "  <td";
    if (completed) *ts << " class=\"done\"";
    *ts << ">\n";
    formatCategories(ts,todo);
    *ts << "  </td>\n";
  }

  if ( mSettings->taskAttendees() ) {
    *ts << "  <td";
    if (completed) *ts << " class=\"done\"";
    *ts << ">\n";
    formatAttendees(ts,todo);
    *ts << "  </td>\n";
  }

  *ts << "</tr>\n";
}

void HtmlExport::createWeekView( QTextStream */*ts*/ )
{
  // FIXME: Implement this!
}

void HtmlExport::createJournalView( QTextStream */*ts*/ )
{
//   Journal::List rawJournalList = mCalendar->journals();
  // FIXME: Implement this!
}

void HtmlExport::createFreeBusyView( QTextStream */*ts*/ )
{
  // FIXME: Implement this!
}

bool HtmlExport::checkSecrecy( Incidence *incidence )
{
  int secrecy = incidence->secrecy();
  if ( secrecy == Incidence::SecrecyPublic ) {
    return true;
  }
  if ( secrecy == Incidence::SecrecyPrivate && !mSettings->excludePrivate() ) {
    return true;
  }
  if ( secrecy == Incidence::SecrecyConfidential &&
       !mSettings->excludeConfidential() ) {
    return true;
  }
  return false;
}

void HtmlExport::formatCategories (QTextStream *ts,Incidence *event)
{
  if (!event->categoriesStr().isEmpty()) {
    *ts << "    " << cleanChars(event->categoriesStr()) << "\n";
  } else {
    *ts << "    &nbsp;\n";
  }
}

void HtmlExport::formatAttendees( QTextStream *ts, Incidence *event )
{
  Attendee::List attendees = event->attendees();
  if (attendees.count()) {
    *ts << "<em>";
#ifndef KORG_NOKABC
    KABC::AddressBook *add_book = KABC::StdAddressBook::self();
    KABC::Addressee::List addressList;
    addressList = add_book->findByEmail(event->organizer().email());
    KABC::Addressee o = addressList.first();
    if (!o.isEmpty() && addressList.size()<2) {
      *ts << "<a href=\"mailto:" << event->organizer().email() << "\">";
      *ts << cleanChars(o.formattedName()) << "</a>\n";
    }
    else *ts << event->organizer().fullName();
#else
    *ts << event->organizer().fullName();
#endif
    *ts << "</em><br />";
    Attendee::List::ConstIterator it;
    for( it = attendees.begin(); it != attendees.end(); ++it ) {
      Attendee *a = *it;
      if (!a->email().isEmpty()) {
        *ts << "<a href=\"mailto:" << a->email();
        *ts << "\">" << cleanChars(a->name()) << "</a>";
      }
      else {
        *ts << "    " << cleanChars(a->name());
      }
      *ts << "<br />" << "\n";
    }
  } else {
    *ts << "    &nbsp;\n";
  }
}

QString HtmlExport::breakString(const QString &text)
{
  int number = text.contains("\n");
  if(number < 0) {
    return text;
  } else {
    QString out;
    QString tmpText = text;
    int pos = 0;
    QString tmp;
    for(int i=0;i<=number;i++) {
      pos = tmpText.find("\n");
      tmp = tmpText.left(pos);
      tmpText = tmpText.right(tmpText.length() - pos - 1);
      out += tmp + "<br />";
    }
    return out;
  }
}

void HtmlExport::createFooter( QTextStream *ts )
{
  // FIXME: Implement this in a translatable way!
  QString trailer = i18n("This page was created ");

/*  bool hasPerson = false;
  bool hasCredit = false;
  bool hasCreditURL = false;
  QString mail, name, credit, creditURL;*/
  if (!mSettings->eMail().isEmpty()) {
    if (!mSettings->name().isEmpty())
      trailer += i18n("by <a href=\"mailto:%1\">%2</a> ").arg( mSettings->eMail() ).arg( mSettings->name() );
    else
      trailer += i18n("by <a href=\"mailto:%1\">%2</a> ").arg( mSettings->eMail() ).arg( mSettings->eMail() );
  } else {
    if (!mSettings->name().isEmpty())
      trailer += i18n("by %1 ").arg( mSettings->name() );
  }
  if (!mSettings->creditName().isEmpty()) {
    if (!mSettings->creditURL().isEmpty())
      trailer += i18n("with <a href=\"%1\">%2</a>")
                     .arg( mSettings->creditURL() )
                     .arg( mSettings->creditName() );
    else
      trailer += i18n("with %1").arg( mSettings->creditName() );
  }
  *ts << "<p>" << trailer << "</p>\n";
}


QString HtmlExport::cleanChars(const QString &text)
{
  QString txt = text;
  txt = txt.replace( "&", "&amp;" );
  txt = txt.replace( "<", "&lt;" );
  txt = txt.replace( ">", "&gt;" );
  txt = txt.replace( "\"", "&quot;" );
  txt = txt.replace( "ä", "&auml;" );
  txt = txt.replace( "Ä", "&Auml;" );
  txt = txt.replace( "ö", "&ouml;" );
  txt = txt.replace( "Ö", "&Ouml;" );
  txt = txt.replace( "ü", "&uuml;" );
  txt = txt.replace( "Ü", "&Uuml;" );
  txt = txt.replace( "ß", "&szlig;" );
  txt = txt.replace( "¤", "&euro;" );
  txt = txt.replace( "é", "&eacute;" );

  return txt;
}

QString HtmlExport::styleSheet() const
{
  if ( !mSettings->styleSheet().isEmpty() ) 
    return mSettings->styleSheet();

  QString css;

  if ( QApplication::reverseLayout() ) {
    css += "    body { background-color:white; color:black; direction: rtl }\n";
    css += "    td { text-align:center; background-color:#eee }\n";
    css += "    th { text-align:center; background-color:#228; color:white }\n";
    css += "    td.sumdone { background-color:#ccc }\n";
    css += "    td.done { background-color:#ccc }\n";
    css += "    td.subhead { text-align:center; background-color:#ccf }\n";
    css += "    td.datehead { text-align:center; background-color:#ccf }\n";
    css += "    td.space { background-color:white }\n";
    css += "    td.dateholiday { color:red }\n";
  } else {
    css += "    body { background-color:white; color:black }\n";
    css += "    td { text-align:center; background-color:#eee }\n";
    css += "    th { text-align:center; background-color:#228; color:white }\n";
    css += "    td.sum { text-align:left }\n";
    css += "    td.sumdone { text-align:left; background-color:#ccc }\n";
    css += "    td.done { background-color:#ccc }\n";
    css += "    td.subhead { text-align:center; background-color:#ccf }\n";
    css += "    td.datehead { text-align:center; background-color:#ccf }\n";
    css += "    td.space { background-color:white }\n";
    css += "    td.date { text-align:left }\n";
    css += "    td.dateholiday { text-align:left; color:red }\n";
  }

  return css;
}


void HtmlExport::addHoliday( const QDate &date, const QString &name)
{
  mHolidayMap[date] = name;
}

QDate HtmlExport::fromDate() const
{
  return mSettings->dateStart().date();
}

QDate HtmlExport::toDate() const
{
  return mSettings->dateEnd().date();
}
