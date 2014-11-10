/*
  Copyright (c) 2000,2001,2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
  Author: Kevin Krammer, krake@kdab.com
  Author: Sergio Martins, sergio.martins@kdab.com

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
#ifndef EVENTVIEWS_AGENDAVIEW_H
#define EVENTVIEWS_AGENDAVIEW_H

#include "eventviews_export.h"
#include "eventview.h"

#include <KCalCore/Todo>

#include <QFrame>

class KConfig;
class KHBox;

class QSplitter;

namespace EventViews
{

#ifndef EVENTVIEWS_NODECOS
namespace CalendarDecoration
{
class Decoration;
}
#endif

class TimeLabels;
class TimeLabelsZone;

class Agenda;
class AgendaItem;
class AgendaView;

class EVENTVIEWS_EXPORT EventIndicator : public QFrame
{
    Q_OBJECT
public:
    enum Location {
        Top,
        Bottom
    };
    explicit EventIndicator(Location loc = Top, QWidget *parent = 0);
    virtual ~EventIndicator();

    void changeColumns(int columns);

    void enableColumn(int column, bool enable);

protected:
    void paintEvent(QPaintEvent *event);
    bool eventFilter(QObject *, QEvent *);

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
    explicit AgendaView(const PrefsPtr &preferences,
                        const QDate &start,
                        const QDate &end,
                        bool isInteractive,
                        bool isSideBySide = false,
                        QWidget *parent = 0);

    explicit AgendaView(const QDate &start,
                        const QDate &end,
                        bool isInteractive,
                        bool isSideBySide = false,
                        QWidget *parent = 0);

    virtual ~AgendaView();

    enum {
        MAX_DAY_COUNT = 42 // ( 6 * 7)
    };

    /** Returns number of currently shown dates. */
    virtual int currentDateCount() const;

    /** returns the currently selected events */
    virtual Akonadi::Item::List selectedIncidences() const;

    /** returns the currently selected incidence's dates */
    virtual KCalCore::DateList selectedIncidenceDates() const;

    /** return the default start/end date/time for new events   */
    virtual bool eventDurationHint(QDateTime &startDt,
                                   QDateTime &endDt,
                                   bool &allDay) const;

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
    virtual void setCalendar(const Akonadi::ETMCalendar::Ptr &cal);

    QSplitter *splitter() const;

    // FIXME: we already have startDateTime() and endDateTime() in the base class

    /** First shown day */
    QDate startDate() const;
    /** Last shown day */
    QDate endDate() const;

    /** Update event belonging to agenda item
        If the incidence is multi-day, item is the first one
    */
    void updateEventDates(AgendaItem *item, bool addIncidence,
                          Akonadi::Collection::Id collectionId);

    QVector<bool> busyDayMask() const;

public slots:
    virtual void updateView();
    virtual void updateConfig();

    virtual void showDates(const QDate &start, const QDate &end,
                           const QDate &preferredMonth = QDate());

    virtual void showIncidences(const Akonadi::Item::List &incidenceList, const QDate &date);

    void clearSelection();

    void startDrag(const Akonadi::Item &);

    void readSettings();
    void readSettings(const KConfig *);
    void writeSettings(KConfig *);

    /** reschedule the todo  to the given x- and y- coordinates.
        Third parameter determines all-day (no time specified) */
    void slotIncidencesDropped(const KCalCore::Incidence::List &incidences,
                               const QPoint &, bool);
    void slotIncidencesDropped(const QList<QUrl> &incidences, const QPoint &, bool);

    void enableAgendaUpdate(bool enable);
    void setIncidenceChanger(Akonadi::IncidenceChanger *changer);

    void zoomInHorizontally(const QDate &date = QDate());
    void zoomOutHorizontally(const QDate &date = QDate());

    void zoomInVertically();
    void zoomOutVertically();

    void zoomView(const int delta, const QPoint &pos,
                  const Qt::Orientation orient = Qt::Horizontal);

    void clearTimeSpanSelection();

    // Used by the timelabelszone
    void updateTimeBarWidth();
    /** Create labels for the selected dates. */
    void createDayLabels(bool force);

    void createTimeBarHeaders();

    void setChanges(EventView::Changes);

Q_SIGNALS:
    void showNewEventPopupSignal();
    void showIncidencePopupSignal(Akonadi::Item, QDate);
    void zoomViewHorizontally(const QDate &, int count);

    void timeSpanSelectionChanged();

protected:
    /** Fill agenda using the current set value for the start date */
    void fillAgenda();

    void connectAgenda(Agenda *agenda, Agenda *otherAgenda);

    /**
      Set the masks on the agenda widgets indicating, which days are holidays.
    */
    void setHolidayMasks();

    void removeIncidence(const KCalCore::Incidence::Ptr &inc);

    virtual void resizeEvent(QResizeEvent *resizeEvent);

protected Q_SLOTS:
    void updateEventIndicatorTop(int newY);
    void updateEventIndicatorBottom(int newY);

    /** Updates data for selected timespan */
    void newTimeSpanSelected(const QPoint &start, const QPoint &end);
    /** Updates data for selected timespan for all day event*/
    void newTimeSpanSelectedAllDay(const QPoint &start, const QPoint &end);
    /**
      Updates the event indicators after a certain incidence was modified or
      removed.
    */
    void updateEventIndicators();
    void scheduleUpdateEventIndicators();
    void updateDayLabelSizes();

    void alignAgendas();

private:
    void init(const QDate &start, const QDate &end);
    bool filterByCollectionSelection(const Akonadi::Item &incidence);
    void setupTimeLabel(TimeLabels *timeLabel);
    bool displayIncidence(const Akonadi::Item &incidence, bool createSelected);

#ifndef EVENTVIEWS_NODECOS
    typedef QList<EventViews::CalendarDecoration::Decoration *> DecorationList;
    bool loadDecorations(const QStringList &decorations, DecorationList &decoList);
    void placeDecorationsFrame(KHBox *frame, bool decorationsFound, bool isTop);
    void placeDecorations(DecorationList &decoList, const QDate &date,
                          KHBox *labelBox, bool forWeek);
#endif

    friend class TimeLabelsZone;
    friend class MultiAgendaView;
    Agenda *agenda() const;
    Agenda *allDayAgenda() const;
private:
    class Private;
    Private *const d;
};

}

#endif
