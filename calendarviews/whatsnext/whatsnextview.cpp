/*
  This file is part of KOrganizer.

  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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

#include "whatsnextview.h"

#include <Akonadi/Calendar/ETMCalendar>
#include <calendarsupport/kcalprefs.h>
#include <calendarsupport/utils.h>

#include <KCalUtils/IncidenceFormatter>

#include <KIconLoader>

#include <QBoxLayout>
#include <KLocale>

using namespace EventViews;

void WhatsNextTextBrowser::setSource( const QUrl &name )
{
  QString uri = name.toString();
  if ( uri.startsWith( QLatin1String( "event:" ) ) ) {
    emit showIncidence( uri );
  } else if ( uri.startsWith( QLatin1String( "todo:" ) ) ) {
    emit showIncidence( uri );
  } else {
    QTextBrowser::setSource( uri );
  }
}

WhatsNextView::WhatsNextView( QWidget *parent )
  : EventView( parent )
{
  mView = new WhatsNextTextBrowser( this );
  connect( mView, SIGNAL(showIncidence(QString)),
           SLOT(showIncidence(QString)) );

  QBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->addWidget(mView);
}

WhatsNextView::~WhatsNextView()
{
}

int WhatsNextView::currentDateCount() const
{
  return mStartDate.daysTo( mEndDate );
}

void WhatsNextView::updateView()
{
  KIconLoader kil( QLatin1String("korganizer") );
  QString ipath;
  kil.loadIcon( QLatin1String("office-calendar"), KIconLoader::NoGroup, 32,
                KIconLoader::DefaultState, QStringList(), &ipath );

  mText = QLatin1String("<table width=\"100%\">\n");
  mText += QLatin1String("<tr bgcolor=\"#3679AD\"><td><h1>");
  mText += QLatin1String("<img src=\"");
  mText += ipath;
  mText += QLatin1String("\">");
  mText += QLatin1String("<font color=\"white\"> ");
  mText += i18n( "What's Next?" ) + QLatin1String("</font></h1>");
  mText += QLatin1String("</td></tr>\n<tr><td>");

  mText += QLatin1String("<h2>");
  if ( mStartDate.daysTo( mEndDate ) < 1 ) {
    mText += KLocale::global()->formatDate( mStartDate );
  } else {
    mText += i18nc(
      "date from - to", "%1 - %2",
      KLocale::global()->formatDate( mStartDate ),
      KLocale::global()->formatDate( mEndDate ) );
  }
  mText+=QLatin1String("</h2>\n");

  KCalCore::Event::List events;
  KDateTime::Spec timeSpec = CalendarSupport::KCalPrefs::instance()->timeSpec();

  events = calendar()->events( mStartDate, mEndDate, timeSpec, false );
  events = calendar()->sortEvents( events, KCalCore::EventSortStartDate,
                                   KCalCore::SortDirectionAscending );

  if ( events.count() > 0 ) {
    mText += QLatin1String("<p></p>");
    kil.loadIcon( QLatin1String("view-calendar-day"), KIconLoader::NoGroup, 22,
                  KIconLoader::DefaultState, QStringList(), &ipath );
    mText += QLatin1String("<h2><img src=\"");
    mText += ipath;
    mText += QLatin1String("\">");
    mText += i18n( "Events:" ) + QLatin1String("</h2>\n");
    mText += QLatin1String("<table>\n");
    Q_FOREACH( const KCalCore::Event::Ptr &ev, events ) {
      if ( !ev->recurs() ) {
        appendEvent( ev );
      } else {
        KCalCore::Recurrence *recur = ev->recurrence();
        int duration = ev->dtStart().secsTo( ev->dtEnd() );
        KDateTime start = recur->getPreviousDateTime( KDateTime( mStartDate, QTime(), timeSpec ) );
        KDateTime end = start.addSecs( duration );
        KDateTime endDate( mEndDate, QTime( 23, 59, 59 ), timeSpec );
        if ( end.date() >= mStartDate ) {
          appendEvent( ev, start.dateTime(), end.dateTime() );
        }
        KCalCore::DateTimeList times = recur->timesInInterval( start, endDate );
        int count = times.count();
        if ( count > 0 ) {
          int i = 0;
          if ( times[0] == start ) {
            ++i;  // start has already been appended
          }
          if ( !times[count - 1].isValid() ) {
            --count;  // list overflow
          }
          for ( ;  i < count && times[i].date() <= mEndDate;  ++i ) {
            appendEvent( ev, times[i].dateTime() );
          }
        }
      }
    }
    mText += QLatin1String("</table>\n");
  }

  mTodos.clear();
  KCalCore::Todo::List todos = calendar()->todos( KCalCore::TodoSortDueDate,
                                                  KCalCore::SortDirectionAscending );
  if ( todos.count() > 0 ) {
    kil.loadIcon( QLatin1String("view-calendar-tasks"), KIconLoader::NoGroup, 22,
                  KIconLoader::DefaultState, QStringList(), &ipath );
    mText += QLatin1String("<h2><img src=\"");
    mText += ipath;
    mText += QLatin1String("\">");
    mText += i18n( "To-dos:" ) + QLatin1String("</h2>\n");
    mText += QLatin1String("<ul>\n");
    Q_FOREACH( const KCalCore::Todo::Ptr &todo, todos ) {
      if ( !todo->isCompleted() && todo->hasDueDate() && todo->dtDue().date() <= mEndDate ) {
        appendTodo( todo );
      }
    }
    bool gotone = false;
    int priority = 1;
    while ( !gotone && priority <= 9 ) {
      Q_FOREACH( const KCalCore::Todo::Ptr &todo, todos ) {
        if ( !todo->isCompleted() && ( todo->priority() == priority ) ) {
          appendTodo( todo );
          gotone = true;
        }
      }
      priority++;
    }
    mText += QLatin1String("</ul>\n");
  }

  QStringList myEmails( CalendarSupport::KCalPrefs::instance()->allEmails() );
  int replies = 0;
  events = calendar()->events( QDate::currentDate(), QDate( 2975, 12, 6 ), timeSpec );
  Q_FOREACH( const KCalCore::Event::Ptr &ev, events ) {
    KCalCore::Attendee::Ptr me = ev->attendeeByMails( myEmails );
    if ( me != 0 ) {
      if ( me->status() == KCalCore::Attendee::NeedsAction && me->RSVP() ) {
        if ( replies == 0 ) {
          mText += QLatin1String("<p></p>");
          kil.loadIcon( QLatin1String("mail-reply-sender"), KIconLoader::NoGroup, 22,
                        KIconLoader::DefaultState, QStringList(), &ipath );
          mText += QLatin1String("<h2><img src=\"");
          mText += ipath;
          mText += QLatin1String("\">");
          mText += i18n( "Events and to-dos that need a reply:" ) + QLatin1String("</h2>\n");
          mText += QLatin1String("<table>\n");
        }
        replies++;
        appendEvent( ev );
      }
    }
  }
  todos = calendar()->todos();
  Q_FOREACH( const KCalCore::Todo::Ptr &to, todos ) {
    KCalCore::Attendee::Ptr me = to->attendeeByMails( myEmails );
    if ( me != 0 ) {
      if ( me->status() == KCalCore::Attendee::NeedsAction && me->RSVP() ) {
        if ( replies == 0 ) {
          mText += QLatin1String("<p></p>");
          kil.loadIcon( QLatin1String("mail-reply-sender"), KIconLoader::NoGroup, 22,
                        KIconLoader::DefaultState, QStringList(), &ipath );
          mText += QLatin1String("<h2><img src=\"");
          mText += ipath;
          mText += QLatin1String("\">");
          mText += i18n( "Events and to-dos that need a reply:" ) + QLatin1String("</h2>\n");
          mText += QLatin1String("<table>\n");
        }
        replies++;
        appendEvent( to );
      }
    }
  }
  if ( replies > 0 ) {
    mText += QLatin1String("</table>\n");
  }

  mText += QLatin1String("</td></tr>\n</table>\n");

  mView->setText(mText);
}

void WhatsNextView::showDates( const QDate &start, const QDate &end, const QDate & )
{
  mStartDate = start;
  mEndDate = end;
  updateView();
}

void WhatsNextView::showIncidences( const Akonadi::Item::List &incidenceList, const QDate &date )
{
  Q_UNUSED( incidenceList );
  Q_UNUSED( date );
}

void WhatsNextView::changeIncidenceDisplay( const Akonadi::Item &,
                                             Akonadi::IncidenceChanger::ChangeType )
{
  updateView();
}

void WhatsNextView::appendEvent( const KCalCore::Incidence::Ptr &incidence, const QDateTime &start,
                                   const QDateTime &end )
{
  mText += QLatin1String("<tr><td><b>");
  if ( const KCalCore::Event::Ptr event = incidence.dynamicCast<KCalCore::Event>() ) {
    KDateTime::Spec timeSpec = CalendarSupport::KCalPrefs::instance()->timeSpec();
    KDateTime starttime( start, timeSpec );
    if ( !starttime.isValid() ) {
      starttime = event->dtStart();
    }
    KDateTime endtime( end, timeSpec );
    if ( !endtime.isValid() ) {
      endtime = starttime.addSecs( event->dtStart().secsTo( event->dtEnd() ) );
    }

    if ( starttime.date().daysTo( endtime.date() ) >= 1 ) {
      mText += i18nc(
        "date from - to", "%1 - %2",
        KLocale::global()->formatDateTime(
          starttime.toTimeSpec( CalendarSupport::KCalPrefs::instance()->timeSpec() ) ),
        KLocale::global()->formatDateTime(
          endtime.toTimeSpec( CalendarSupport::KCalPrefs::instance()->timeSpec() ) ) );
    } else {
      mText += i18nc(
        "date, from - to", "%1, %2 - %3",
        KLocale::global()->formatDate(
          starttime.toTimeSpec( CalendarSupport::KCalPrefs::instance()->timeSpec() ).date(),
          KLocale::ShortDate ),
        KLocale::global()->formatTime(
          starttime.toTimeSpec( CalendarSupport::KCalPrefs::instance()->timeSpec() ).time() ),
        KLocale::global()->formatTime(
          endtime.toTimeSpec( CalendarSupport::KCalPrefs::instance()->timeSpec() ).time() ) );
    }
  }
  mText += QLatin1String("</b></td><td><a ");
  if ( incidence->type() == KCalCore::Incidence::TypeEvent ) {
    mText += QLatin1String("href=\"event:");
  }
  if ( incidence->type() == KCalCore::Incidence::TypeTodo ) {
    mText += QLatin1String("href=\"todo:");
  }
  mText += incidence->uid() + QLatin1String("\">");
  mText += incidence->summary();
  mText += QLatin1String("</a></td></tr>\n");
}

void WhatsNextView::appendTodo( const KCalCore::Incidence::Ptr &incidence )
{
  Akonadi::Item aitem = calendar()->item( incidence );
  if ( mTodos.contains( aitem ) ) {
    return;
  }

  mTodos.append( aitem );
  mText += QLatin1String("<li><a href=\"todo:") + incidence->uid() + QLatin1String("\">");
  mText += incidence->summary();
  mText += QLatin1String("</a>");

  if ( const KCalCore::Todo::Ptr todo = CalendarSupport::todo( aitem ) ) {
    if ( todo->hasDueDate() ) {
      mText += i18nc( "to-do due date", "  (Due: %1)",
                      KCalUtils::IncidenceFormatter::dateTimeToString(
                        todo->dtDue(), todo->allDay() ) );
    }
  }
  mText += QLatin1String("</li>\n");
}

void WhatsNextView::showIncidence( const QString &uid )
{
  Akonadi::Item item;

  Akonadi::ETMCalendar::Ptr cal = calendar();
  if ( !cal ) {
    return;
  }

  if ( uid.startsWith( QLatin1String( "event:" ) ) ) {
    item = cal->item( uid.mid( 6 ) );
  } else if ( uid.startsWith( QLatin1String( "todo:" ) ) ) {
    item = cal->item( uid.mid( 5 ) );
  }

  if ( item.isValid() ) {
    emit showIncidenceSignal( item );
  }
}

