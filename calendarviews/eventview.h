/*
  Copyright (c) 1999 Preston Brown <pbrown@kde.org>
  Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>
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
#ifndef EVENTVIEWS_EVENTVIEW_H
#define EVENTVIEWS_EVENTVIEW_H

#include "eventviews_export.h"

#include <Collection>
#include <Item>
#include <Akonadi/Calendar/ETMCalendar>

#include <KCalCore/Incidence>
#include <KCalCore/Todo>

#include <QWidget>
#include <QSet>
#include <QByteArray>
#include <QDate>

namespace boost
{
template <typename T> class shared_ptr;
}

namespace KCalCore
{
template <typename T> class SortableList;
typedef SortableList<QDate> DateList;
}

namespace KHolidays
{
class HolidayRegion;
typedef boost::shared_ptr<HolidayRegion> HolidayRegionPtr;
}

namespace CalendarSupport
{
class CollectionSelection;
class KCalPrefs;
}

namespace Akonadi
{
class IncidenceChanger;
}

class KCheckableProxyModel;
class KConfigGroup;

namespace EventViews
{

enum {
    BUSY_BACKGROUND_ALPHA = 70
};

class EventViewPrivate;
class Prefs;
typedef boost::shared_ptr<Prefs> PrefsPtr;
typedef boost::shared_ptr<CalendarSupport::KCalPrefs> KCalPrefsPtr;

/**
  EventView is the abstract base class from which all other calendar views
  for event data are derived.  It provides methods for displaying
  appointments and events on one or more days.  The actual number of
  days that a view actually supports is not defined by this abstract class;
  that is up to the classes that inherit from it.  It also provides
  methods for updating the display, retrieving the currently selected
  event (or events), and the like.

  @short Abstract class from which all event views are derived.
  @author Preston Brown <pbrown@kde.org>
  @see KOListView, AgendaView, KOMonthView
*/
class EVENTVIEWS_EXPORT EventView : public QWidget
{
    Q_OBJECT
public:
    enum {
        // This value is passed to QColor's lighter(int factor) for selected events
        BRIGHTNESS_FACTOR = 125
    };

    enum ItemIcon {
        CalendarCustomIcon = 0,
        TaskIcon,
        JournalIcon,
        RecurringIcon,
        ReminderIcon,
        ReadOnlyIcon,
        ReplyIcon,
        AttendingIcon,
        TentativeIcon,
        OrganizerIcon,
        IconCount = 10 // Always keep at the end
    };

    enum Change {
        NothingChanged = 0,
        IncidencesAdded = 1,
        IncidencesEdited = 2,
        IncidencesDeleted = 4,
        DatesChanged = 8,
        FilterChanged = 16,
        ResourcesChanged = 32,
        ZoomChanged = 64,
        ConfigChanged = 128
    };
    Q_DECLARE_FLAGS(Changes, Change)

    /**
     * Constructs a view.
     * @param cal is a pointer to the calendar object from which events
     *        will be retrieved for display.
     * @param parent is the parent QWidget.
     */
    explicit EventView(QWidget *parent = 0);

    /**
     * Destructor. Views will do view-specific cleanups here.
     */
    ~EventView();

    virtual void setCalendar(const Akonadi::ETMCalendar::Ptr &cal);

    /**
      Return calendar object of this view.
    */
    virtual Akonadi::ETMCalendar::Ptr calendar() const;

    /*
      update config is called after prefs are set.
    */
    virtual void setPreferences(const PrefsPtr &preferences);
    PrefsPtr preferences() const;

    virtual void setKCalPreferences(const KCalPrefsPtr &preferences);
    KCalPrefsPtr kcalPreferences() const;

    /**
      @return a list of selected events. Most views can probably only
      select a single event at a time, but some may be able to select
      more than one.
    */
    virtual Akonadi::Item::List selectedIncidences() const = 0;

    /**
      Returns a list of the dates of selected events. Most views can
      probably only select a single event at a time, but some may be able
      to select more than one.
    */
    virtual KCalCore::DateList selectedIncidenceDates() const = 0;

    /**
       Returns the start of the selection, or an invalid QDateTime if there is no selection
       or the view doesn't support selecting cells.
     */
    virtual QDateTime selectionStart() const;

    /**
      Returns the end of the selection, or an invalid QDateTime if there is no selection
      or the view doesn't support selecting cells.
     */
    virtual QDateTime selectionEnd() const;

    /**
      Returns whether or not date range selection is enabled. This setting only
      applies to views that actually supports selecting cells.
      @see selectionStart()
      @see selectionEnd()
     */
    bool dateRangeSelectionEnabled() const;

    /**
      Enable or disable date range selection.
      @see dateRangeSelectionEnabled()
     */
    void setDateRangeSelectionEnabled(bool enable);

    /**
      Returns the number of currently shown dates.
      A return value of 0 means no idea.
    */
    virtual int currentDateCount() const = 0;

    /**
     * returns whether this view supports zoom.
     * Base implementation returns false.
     */
    virtual bool supportsZoom() const;

    virtual bool hasConfigurationDialog() const;

    virtual void showConfigurationDialog(QWidget *parent);

    QByteArray identifier() const;
    void setIdentifier(const QByteArray &identifier);

    /**
     * reads the view configuration. View-specific configuration can be
     * restored via doRestoreConfig()
     *
     * @param configGroup the group to read settings from
     * @see doRestoreConfig()
     */
    void restoreConfig(const KConfigGroup &configGroup);

    /**
     * writes out the view configuration. View-specific configuration can be
     * saved via doSaveConfig()
     *
     * @param configGroup the group to store settings in
     * @see doSaveConfig()
     */
    void saveConfig(KConfigGroup &configGroup);

    /**
      Makes the eventview display only items of collection @p id.
      Useful for example in multi-agendaview (side-by-side) where
      each AgendaView displays only one collection.
    */
    void setCollectionId(Akonadi::Collection::Id id);
    Akonadi::Collection::Id collectionId() const;

    //----------------------------------------------------------------------------
    KCheckableProxyModel *takeCustomCollectionSelectionProxyModel();
    KCheckableProxyModel *customCollectionSelectionProxyModel() const;
    void setCustomCollectionSelectionProxyModel(KCheckableProxyModel *model);

    CalendarSupport::CollectionSelection *customCollectionSelection() const;

    static CalendarSupport::CollectionSelection *globalCollectionSelection();
    static void setGlobalCollectionSelection(CalendarSupport::CollectionSelection *selection);
    //----------------------------------------------------------------------------

    /**
     * returns the view at the given widget coordinate. This is usually the view
     * itself, except for composite views, where a subview will be returned.
     * The default implementation returns @p this .
     */
    virtual EventView *viewAt(const QPoint &p);

    /**
     * @param preferredMonth Used by month orientated views.  Contains the
     * month to show when the week crosses months.  It's a QDate instead
     * of uint so it can be easily fed to KCalendarSystem's functions.
     */
    virtual void setDateRange(const KDateTime &start, const KDateTime &end,
                              const QDate &preferredMonth = QDate());

    KDateTime startDateTime() const;
    KDateTime endDateTime() const;

    KDateTime actualStartDateTime() const;
    KDateTime actualEndDateTime() const;

    int showMoveRecurDialog(const KCalCore::Incidence::Ptr &incidence, const QDate &date);

    /**
      Handles key events, opens the new event dialog when enter is pressed, activates type ahead.
    */
    bool processKeyEvent(QKeyEvent *);

    /*
     * Sets the QObject that will receive key events that were made
     * while the new event dialog was still being created.
     */
    void setTypeAheadReceiver(QObject *o);

    /**
      Returns the selection of collection to be used by this view
      (custom if set, or global otherwise).
    */
    CalendarSupport::CollectionSelection *collectionSelection() const;

    /**
      Notifies the view that there are pending changes so a redraw is needed.
      @param needed if the update is needed or not.
    */
    virtual void setChanges(Changes changes);

    /**
      Returns if there are pending changes and a redraw is needed.
    */
    Changes changes() const;

    /**
     * Returns a variation of @p color that will be used for the border
     * of an agenda or month item.
     */
    static QColor itemFrameColor(const QColor &color, bool selected);

    QString iconForItem(const Akonadi::Item &);

public Q_SLOTS:
    /**
      Shows given incidences. Depending on the actual view it might not
      be possible to show all given events.

      @param incidenceList a list of incidences to show.
      @param date is the QDate on which the incidences are being shown.
    */
    virtual void showIncidences(const Akonadi::Item::List &incidenceList,
                                const QDate &date) = 0;

    /**
      Updates the current display to reflect changes that may have happened
      in the calendar since the last display refresh.
    */
    virtual void updateView() = 0;
    virtual void dayPassed(const QDate &);

    /**
      Assign a new incidence change helper object.
     */
    virtual void setIncidenceChanger(Akonadi::IncidenceChanger *changer);

    /**
      Write all unsaved data back to calendar store.
    */
    virtual void flushView();

    /**
      Re-reads the configuration and picks up relevant
      changes which are applicable to the view.
    */
    virtual void updateConfig();

    /**
      Clear selection. The incidenceSelected signal is not emitted.
    */
    virtual void clearSelection();

    /**
      Sets the default start/end date/time for new events.
      Return true if anything was changed
    */
    virtual bool eventDurationHint(QDateTime &startDt, QDateTime &endDt, bool &allDay) const;

    void focusChanged(QWidget *, QWidget *);

    /**
     Perform the default action for an incidence, e.g. open the event editor,
     when double-clicking an event in the agenda view.
    */
    void defaultAction(const Akonadi::Item &incidence);

    /**
       Set which holidays the user wants to use.
       @param holidayRegion a HolidayRegion object initialized with the desired locale.
    */
    void setHolidayRegion(const KHolidays::HolidayRegionPtr &holidayRegion);

Q_SIGNALS:
    /**
     * when the view changes the dates that are selected in one way or
     * another, this signal is emitted.  It should be connected back to
     * the KDateNavigator object so that it changes appropriately,
     * and any other objects that need to be aware that the list of
     * selected dates has changed.
     *   @param datelist the new list of selected dates
     */
    void datesSelected(const KCalCore::DateList &datelist);

    /**
     * Emitted when an event is moved using the mouse in an agenda
     * view (week / month).
     */
    void shiftedEvent(const QDate &olddate, const QDate &newdate);

    void incidenceSelected(const Akonadi::Item &, const QDate);

    /**
     * instructs the receiver to show the incidence in read-only mode.
     */
    void showIncidenceSignal(const Akonadi::Item &);

    /**
     * instructs the receiver to begin editing the incidence specified in
     * some manner.  Doesn't make sense to connect to more than one
     * receiver.
     */
    void editIncidenceSignal(const Akonadi::Item &);

    /**
     * instructs the receiver to delete the Incidence in some manner; some
     * possibilities include automatically, with a confirmation dialog
     * box, etc.  Doesn't make sense to connect to more than one receiver.
     */
    void deleteIncidenceSignal(const Akonadi::Item &);

    /**
    * instructs the receiver to cut the Incidence
    */
    void cutIncidenceSignal(const Akonadi::Item &);

    /**
    * instructs the receiver to copy the incidence
    */
    void copyIncidenceSignal(const Akonadi::Item &);

    /**
    * instructs the receiver to paste the incidence
    */
    void pasteIncidenceSignal();

    /**
     * instructs the receiver to toggle the alarms of the Incidence.
     */
    void toggleAlarmSignal(const Akonadi::Item &);

    /**
     * instructs the receiver to toggle the completion state of the Incidence
     * (which must be a Todo type).
     */
    void toggleTodoCompletedSignal(const Akonadi::Item &);

    /**
     * Copy the incidence to the specified resource.
     */
    void copyIncidenceToResourceSignal(const Akonadi::Item &, const QString &);

    /**
     * Move the incidence to the specified resource.
     */
    void moveIncidenceToResourceSignal(const Akonadi::Item &, const QString &);

    /** Dissociate from a recurring incidence the occurrence on the given
     *  date to a new incidence or dissociate all occurrences from the
     *  given date onwards.
     */
    void dissociateOccurrencesSignal(const Akonadi::Item &, const QDate &);

    /**
     * instructs the receiver to create a new event in given collection. Doesn't make
     * sense to connect to more than one receiver.
     */
    void newEventSignal();
    /**
     * instructs the receiver to create a new event with the specified beginning
     * time. Doesn't make sense to connect to more than one receiver.
     */
    void newEventSignal(const QDate &);
    /**
     * instructs the receiver to create a new event with the specified beginning
     * time. Doesn't make sense to connect to more than one receiver.
     */
    void newEventSignal(const QDateTime &);
    /**
     * instructs the receiver to create a new event, with the specified
     * beginning end ending times.  Doesn't make sense to connect to more
     * than one receiver.
     */
    void newEventSignal(const QDateTime &, const QDateTime &);

    void newTodoSignal(const QDate &);
    void newSubTodoSignal(const Akonadi::Item &);

    void newJournalSignal(const QDate &);

protected Q_SLOTS:
    virtual void calendarReset();

private Q_SLOTS:
    void onCollectionChanged(const Akonadi::Collection &, const QSet<QByteArray> &);

protected:
    bool makesWholeDayBusy(const KCalCore::Incidence::Ptr &incidence) const;
    Akonadi::IncidenceChanger *changer() const;

    /**
      * reimplement to read view-specific settings.
      */
    virtual void doRestoreConfig(const KConfigGroup &configGroup);

    /**
     * reimplement to write view-specific settings.
     */
    virtual void doSaveConfig(KConfigGroup &configGroup);

    /**
      @deprecated
     */
    virtual void showDates(const QDate &start, const QDate &end,
                           const QDate &preferredMonth = QDate()) = 0;

    /**
     * from the requested date range (passed via setDateRange()), calculates the
     * adjusted date range actually displayed by the view, depending on the
     * view's supported range (e.g., a month view always displays one month)
     * The default implementation returns the range unmodified
     *
     * @param preferredMonth Used by month orientated views. Contains the
     * month to show when the week crosses months.  It's a QDate instead of
     * uint so it can be easily fed to KCalendarSystem's functions.
     */
    virtual QPair<KDateTime, KDateTime> actualDateRange(
        const KDateTime &start, const KDateTime &end, const QDate &preferredMonth = QDate()) const;
    /*
    virtual void incidencesAdded( const Akonadi::Item::List &incidences );
    virtual void incidencesAboutToBeRemoved( const Akonadi::Item::List &incidences );
    virtual void incidencesChanged( const Akonadi::Item::List &incidences );
    */
    virtual void handleBackendError(const QString &error);

private:
    EventViewPrivate *const d_ptr;
    Q_DECLARE_PRIVATE(EventView)
};

}

#endif
