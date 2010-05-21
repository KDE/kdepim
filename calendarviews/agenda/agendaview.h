/*
  This file is part of KOrganizer.

  Copyright (c) 2000,2001,2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
  Author: Kevin Krammer, krake@kdab.com

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

#ifndef AGENDAVIEW_H
#define AGENDAVIEW_H

#include "eventviews_export.h"

#include "eventview.h"

#include <KCal/Todo>

#include <QFrame>

class KConfig;
class KHBox;

class QBoxLayout;
class QGridLayout;
class QMenu;
class QPaintEvent;
class QSplitter;

namespace Akonadi {
  class CollectionSelection;
}

namespace EventViews
{

class TimeLabels;
class TimeLabelsZone;

class Agenda;
class AgendaItem;
class AgendaView;
class AlternateLabel;

class EVENTVIEWS_EXPORT EventIndicator : public QFrame
{
  Q_OBJECT
  public:
    enum Location {
      Top,
      Bottom
    };
    explicit EventIndicator( Location loc = Top, QWidget *parent = 0 );
    virtual ~EventIndicator();

    void changeColumns( int columns );

    void enableColumn( int column, bool enable );

  protected:
    void paintEvent( QPaintEvent *event );

  private:
    class Private;
    Private *const d;
};

/**
  AgendaView is the agenda-like view that displays events in a single
  or multi-day view.
*/
class EVENTVIEWS_EXPORT AgendaView : public EventView
{
  Q_OBJECT
  public:
    explicit AgendaView( QWidget *parent = 0, bool isSideBySide = false );
    virtual ~AgendaView();

    /** Returns number of currently shown dates. */
    virtual int currentDateCount() const;

    /** returns the currently selected events */
    virtual Akonadi::Item::List selectedIncidences() const;

    /** returns the currently selected incidence's dates */
    virtual KCal::DateList selectedIncidenceDates() const;

    /** return the default start/end date/time for new events   */
    virtual bool eventDurationHint( QDateTime &startDt, QDateTime &endDt, bool &allDay ) const;

    /** Remove all events from view */
    void clearView();

    /** start-datetime of selection */
    virtual QDateTime selectionStart() const;

    /** end-datetime of selection */
    virtual QDateTime selectionEnd() const;

    /** returns true if selection is for whole day */
    bool selectedIsAllDay() const;

    /** make selected start/end invalid */
    void deleteSelectedDateTime();

    /** returns if only a single cell is selected, or a range of cells */
    bool selectedIsSingleCell() const;

    /* reimp from EventView */
    virtual void setCalendar( Akonadi::Calendar *cal );

    /** Show only incidences from the given collection selection. */
//    void setCollectionSelection( CollectionSelection* selection );
    void setCollection( Akonadi::Collection::Id id );
    Akonadi::Collection::Id collection() const;

    Agenda *agenda() const;
    QSplitter *splitter() const;

    /** First shown day */
    QDate startDate() const;
    /** Last shown day */
    QDate endDate() const;

  public slots:
    virtual void updateView();
    virtual void updateConfig();
    virtual void showDates( const QDate &start, const QDate &end );
    virtual void showIncidences( const Akonadi::Item::List &incidenceList, const QDate &date );

    void insertIncidence( const Akonadi::Item &incidence, const QDate &curDate );
    void changeIncidenceDisplayAdded( const Akonadi::Item &incidence );
    void changeIncidenceDisplay( const Akonadi::Item &incidence, int mode );

    void clearSelection();

    void startDrag( const Akonadi::Item & );

    void readSettings();
    void readSettings( const KConfig * );
    void writeSettings( KConfig * );

    void setContentsPos( int y );

    /** reschedule the todo  to the given x- and y- coordinates.
        Third parameter determines all-day (no time specified) */
    void slotTodosDropped( const QList<KCal::Todo::Ptr> & todos, const QPoint &, bool );
    void slotTodosDropped( const QList<KUrl>& todos, const QPoint &, bool );

    void enableAgendaUpdate( bool enable );
    void setIncidenceChanger( Akonadi::IncidenceChanger *changer );

    void zoomInHorizontally( const QDate &date=QDate() );
    void zoomOutHorizontally( const QDate &date=QDate() );

    void zoomInVertically( );
    void zoomOutVertically( );

    void zoomView( const int delta, const QPoint &pos,
      const Qt::Orientation orient=Qt::Horizontal );

    void clearTimeSpanSelection();

    /** Notifies agenda that there are pending changes */
    void setUpdateNeeded();

    // Used by the timelabelszone
    void updateTimeBarWidth();
    /** Create labels for the selected dates. */
    void createDayLabels( bool force );

    void createTimeBarHeaders();

  Q_SIGNALS:
    void showNewEventPopupSignal();
    void showIncidencePopupSignal(Akonadi::Item,QDate);
    void zoomViewHorizontally( const QDate &, int count );

    void timeSpanSelectionChanged();

  protected:
    /** Fill agenda beginning with date startDate */
    void fillAgenda( const QDate &startDate );

    /** Fill agenda using the current set value for the start date */
    void fillAgenda();

    void connectAgenda( Agenda *agenda, Agenda *otherAgenda );

    /**
      Set the masks on the agenda widgets indicating, which days are holidays.
    */
    void setHolidayMasks();

    void removeIncidence( const Akonadi::Item & );
    /**
      Updates the event indicators after a certain incidence was modified or
      removed.
    */
    void updateEventIndicators();

    virtual void resizeEvent( QResizeEvent *resizeEvent );

  protected Q_SLOTS:
    /** Update event belonging to agenda item */
    void updateEventDates( AgendaItem *item );
    /** update just the display of the given incidence, called by a single-shot timer */
    void doUpdateItem();

    void updateEventIndicatorTop( int newY );
    void updateEventIndicatorBottom( int newY );

    /** Updates data for selected timespan */
    void newTimeSpanSelected( const QPoint &start, const QPoint &end );
    /** Updates data for selected timespan for all day event*/
    void newTimeSpanSelectedAllDay( const QPoint &start, const QPoint &end );

    void handleNewEventRequest();

    void updateDayLabelSizes();

  private:

    bool filterByCollectionSelection( const Akonadi::Item &incidence );
    void setupTimeLabel( TimeLabels *timeLabel );
    int timeLabelsWidth();
    void displayIncidence( const Akonadi::Item &incidence );
    void placeDecorationsFrame( KHBox *frame, bool decorationsFound, bool isTop );

  private:
    class Private;
    Private *const d;
};

}

#endif
// kate: space-indent on; indent-width 2; replace-tabs on;
