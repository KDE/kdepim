/*
    This file is part of libkcal.

    Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>

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

using namespace KCal;

HtmlExport::HtmlExport( Calendar *calendar ) :
  mCalendar( calendar ),
  mMonthViewEnabled( true ), mEventsEnabled( false ), mTodosEnabled( true ),
  mCategoriesTodoEnabled( false ), mAttendeesTodoEnabled( false ),
  mCategoriesEventEnabled( false ), mAttendeesEventEnabled( false ),
  mDueDateEnabled( false ),
  mExcludePrivateTodoEnabled( false ),
  mExcludeConfidentialTodoEnabled( false ),
  mExcludePrivateEventEnabled( false ),
  mExcludeConfidentialEventEnabled( false )
{
  mTitle = I18N_NOOP("Calendar");
  mTitleTodo = I18N_NOOP("To-do List");
  mCreditName = "";
  mCreditURL = "";
}

bool HtmlExport::save(const QString &fileName)
{
  QFile f(fileName);
  if (!f.open(IO_WriteOnly)) {
    return false;
  }
  QTextStream ts(&f);
  bool success = save(&ts);
  f.close();
  return success;
}

bool HtmlExport::save(QTextStream *ts)
{
  ts->setEncoding(QTextStream::UnicodeUTF8);

  // Write HTML header
  *ts << "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" ";
  *ts << "\"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n";

  *ts << "<html><head>" << endl;
  *ts << "  <meta http-equiv=\"Content-Type\" content=\"text/html; charset=";
  *ts << "UTF-8\" />\n";
  if (!mTitle.isEmpty())
    *ts << "  <title>" << mTitle << "</title>\n";
  *ts << "  <style type=\"text/css\">\n";
  *ts << styleSheet();
  *ts << "  </style>\n";
  *ts << "</head><body>\n";

  // TO DO: Write header
  // (Heading, Calendar-Owner, Calendar-Date, ...)

  if (eventsEnabled() || monthViewEnabled()) {
    if (!mTitle.isEmpty())
      *ts << "<h1>" << mTitle << "</h1>\n";
  }

  // Write Month View
  if (monthViewEnabled()) {
    createHtmlMonthView(ts);
  }

  // Write Event List
  if (eventsEnabled()) {
    // Write HTML page content
    createHtmlEventList(ts);
  }

  // Write Todo List
  if (todosEnabled()) {
    if (!mTitleTodo.isEmpty())
      *ts << "<h1>" << mTitleTodo << "</h1>\n";

    // Write HTML page content
    createHtmlTodoList(ts);
  }

  // Write trailer
  QString trailer = i18n("This page was created ");

  if (!mEmail.isEmpty()) {
    if (!mName.isEmpty())
      trailer += i18n("by <a href=\"mailto:%1\">%2</a> ").arg( mEmail ).arg( mName );
    else
      trailer += i18n("by <a href=\"mailto:%1\">%2</a> ").arg( mEmail ).arg( mEmail );
  } else {
    if (!mName.isEmpty())
      trailer += i18n("by %1 ").arg( mName );
  }
  if (!mCreditName.isEmpty()) {
    if (!mCreditURL.isEmpty())
      trailer += i18n("with <a href=\"%1\">%2</a>").arg( mCreditURL ).arg( mCreditName );
    else
      trailer += i18n("with %1").arg( mCreditName );
  }
  *ts << "<p>" << trailer << "</p>\n";

  // Write HTML trailer
  *ts << "</body></html>\n";

  return true;
}

void HtmlExport::createHtmlMonthView(QTextStream *ts)
{
  QDate start = fromDate();
  start.setYMD(start.year(),start.month(),1);  // go back to first day in month

  QDate end(start.year(),start.month(),start.daysInMonth());

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
              createHtmlEvent( ts, *it, start, false );
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

void HtmlExport::createHtmlEventList (QTextStream *ts)
{
  *ts << "<table border=\"0\" cellpadding=\"3\" cellspacing=\"3\">\n";
  *ts << "  <tr>\n";
  *ts << "    <th class=\"sum\">" << i18n("Start Time") << "</th>\n";
  *ts << "    <th>" << i18n("End Time") << "</th>\n";
  *ts << "    <th>" << i18n("Event") << "</th>\n";
  if (categoriesEventEnabled()) {
    *ts << "    <th>" << i18n("Categories") << "</th>\n";
  }
  if (attendeesEventEnabled()) {
    *ts << "    <th>" << i18n("Attendees") << "</th>\n";
  }

  *ts << "  </tr>\n";

  int columns = 3;
  if (categoriesEventEnabled()) ++columns;
  if (attendeesEventEnabled()) ++columns;

  for (QDate dt = fromDate(); dt <= toDate(); dt = dt.addDays(1)) {
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
	  createHtmlEvent( ts, *it, dt );
	}
      }
    }
  }

  *ts << "</table>\n";
}

void HtmlExport::createHtmlEvent (QTextStream *ts, Event *event,
                                       QDate date,bool withDescription)
{
  kdDebug(5850) << "HtmlExport::createHtmlEvent(): " << event->summary() << endl;
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
  if (withDescription && !event->description().isEmpty()) {
    *ts << "      <p>" << breakString(cleanChars(event->description())) << "</p>\n";
  }
  *ts << "    </td>\n";

  if (categoriesEventEnabled()) {
    *ts << "  <td>\n";
    formatHtmlCategories(ts,event);
    *ts << "  </td>\n";
  }

  if (attendeesEventEnabled()) {
    *ts << "  <td>\n";
    formatHtmlAttendees(ts,event);
    *ts << "  </td>\n";
  }

  *ts << "  </tr>\n";
}

void HtmlExport::createHtmlTodoList ( QTextStream *ts )
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

  // Sort list by priorities. This is brute force and should be
  // replaced by a real sorting algorithm.
  Todo::List todoList;
  for ( int i = 1; i <= 5; ++i ) {
    for( it = rawTodoList.begin(); it != rawTodoList.end(); ++it ) {
      if ( (*it)->priority() == i && checkSecrecy( *it ) ) {
        todoList.append( *it );
      }
    }
  }

  *ts << "<table border=\"0\" cellpadding=\"3\" cellspacing=\"3\">\n";
  *ts << "  <tr>\n";
  *ts << "    <th class=\"sum\">" << i18n("Task") << "</th>\n";
  *ts << "    <th>" << i18n("Priority") << "</th>\n";
  *ts << "    <th>" << i18n("Completed") << "</th>\n";
  if (dueDateEnabled()) {
    *ts << "    <th>" << i18n("Due Date") << "</th>\n";
  }
  if (categoriesTodoEnabled()) {
    *ts << "    <th>" << i18n("Categories") << "</th>\n";
  }
  if (attendeesTodoEnabled()) {
    *ts << "    <th>" << i18n("Attendees") << "</th>\n";
  }
  *ts << "  </tr>\n";

  // Create top-level list.
  for( it = todoList.begin(); it != todoList.end(); ++it ) {
    if ( !(*it)->relatedTo() ) createHtmlTodo( ts, *it );
  }

  // Create sub-level lists
  for( it = todoList.begin(); it != todoList.end(); ++it ) {
    Incidence::List relations = (*it)->relations();
    if (relations.count()) {
      // Generate sub-task list of event ev
      *ts << "  <tr>\n";
      *ts << "    <td class=\"subhead\" colspan=";
      int columns = 3;
      if (dueDateEnabled()) ++columns;
      if (categoriesTodoEnabled()) ++columns;
      if (attendeesTodoEnabled()) ++columns;
      *ts << "\"" << QString::number(columns) << "\"";
      *ts << "><a name=\"sub" << (*it)->uid() << "\"></a>"
          << i18n("Sub-Tasks of: ") << "<a href=\"#"
          << (*it)->uid() << "\"><b>" << cleanChars( (*it)->summary())
          << "</b></a></td>\n";
      *ts << "  </tr>\n";

      Todo::List sortedList;
      // Sort list by priorities. This is brute force and should be
      // replaced by a real sorting algorithm.
      for ( int i = 1; i <= 5; ++i ) {
        Incidence::List::ConstIterator it2;
        for( it2 = relations.begin(); it2 != relations.end(); ++it2 ) {
          Todo *ev3 = dynamic_cast<Todo *>( *it2 );
          if ( ev3 && ev3->priority() == i ) sortedList.append( ev3 );
        }
      }

      Todo::List::ConstIterator it3;
      for( it3 = sortedList.begin(); it3 != sortedList.end(); ++it3 ) {
        createHtmlTodo( ts, *it3 );
      }
    }
  }

  *ts << "</table>\n";
}

void HtmlExport::createHtmlTodo (QTextStream *ts,Todo *todo)
{
  kdDebug(5850) << "HtmlExport::createHtmlTodo()" << endl;

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

  if (dueDateEnabled()) {
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

  if (categoriesTodoEnabled()) {
    *ts << "  <td";
    if (completed) *ts << " class=\"done\"";
    *ts << ">\n";
    formatHtmlCategories(ts,todo);
    *ts << "  </td>\n";
  }

  if (attendeesTodoEnabled()) {
    *ts << "  <td";
    if (completed) *ts << " class=\"done\"";
    *ts << ">\n";
    formatHtmlAttendees(ts,todo);
    *ts << "  </td>\n";
  }

  *ts << "</tr>\n";
}

bool HtmlExport::checkSecrecy( Incidence *incidence )
{
  int secrecy = incidence->secrecy();
  if ( secrecy == Incidence::SecrecyPublic ) {
    return true;
  }
  if ( secrecy == Incidence::SecrecyPrivate && !excludePrivateEventEnabled() ) {
    return true;
  }
  if ( secrecy == Incidence::SecrecyConfidential &&
       !excludeConfidentialEventEnabled() ) {
    return true;
  }
  return false;
}

void HtmlExport::formatHtmlCategories (QTextStream *ts,Incidence *event)
{
  if (!event->categoriesStr().isEmpty()) {
    *ts << "    " << cleanChars(event->categoriesStr()) << "\n";
  } else {
    *ts << "    &nbsp;\n";
  }
}

void HtmlExport::formatHtmlAttendees (QTextStream *ts,Incidence *event)
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

void HtmlExport::setStyleSheet( const QString &styleSheet )
{
  mStyleSheet = styleSheet;
}

QString HtmlExport::styleSheet()
{
  if ( !mStyleSheet.isEmpty() ) return mStyleSheet;

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


void HtmlExport::addHoliday( QDate date, QString name)
{
  mHolidayMap[date] = name;
}

