/*
  This file is part of KOrganizer.

  Copyright (c) 1999 Preston Brown <pbrown@kde.org>
  Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
#ifndef EVENTVIEW_H
#define EVENTVIEW_H

//TODO_SPLIT: remove agenda
//#include "agenda.h"

#include "eventviews_export.h"

#include <akonadi/kcal/calendar.h>
#include <akonadi/kcal/calendarsearch.h>
#include <akonadi/kcal/collectionselection.h>
#include <akonadi/kcal/incidencechanger.h>

#include <KCal/Todo>

#include <KConfigGroup>

#include <QWidget>


namespace KCal {
  class Incidence;
}
namespace Akonadi {
  class Item;
  class Calendar;
}

namespace boost {
  template <typename T> class shared_ptr;
}

namespace KHolidays {
    class HolidayRegion;
    typedef boost::shared_ptr<HolidayRegion> HolidayRegionPtr;
}

using namespace KCal;

class QMenu;

namespace EventViews
{
  class Prefs;
  typedef boost::shared_ptr<Prefs> PrefsPtr;

/**
  EventView is the abstract base class from wich all other
  calendar views for event data are derived.  It provides methods for
  displaying
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
    /**
     * Constructs a view.
     * @param cal is a pointer to the calendar object from which events
     *        will be retrieved for display.
     * @param parent is the parent QWidget.
     */
    explicit EventView( QWidget *parent = 0 );

    /**
     * Destructor.  Views will do view-specific cleanups here.
     */
    virtual ~EventView();

    enum {
      // This value is passed to QColor's lighter(int factor) for selected events
      BRIGHTNESS_FACTOR = 125
    };

    virtual void setCalendar( Akonadi::Calendar *cal );

    /**
      Return calendar object of this view.
    */
    virtual Akonadi::Calendar *calendar() const;

    virtual void setPreferences( const PrefsPtr &preferences );

    PrefsPtr preferences() const;

    Akonadi::CalendarSearch* calendarSearch() const;

    /**
       A todo can have two pixmaps, one for completed and one for incomplete.
    */
    bool usesCompletedTodoPixmap( const Akonadi::Item &aItem, const QDate &date );

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
    virtual DateList selectedIncidenceDates() const = 0;

    /**
       Returns the start of the selection, or an invalid QDateTime if there is no selection
       or the view doesn't support selecting cells.
     */
    virtual QDateTime selectionStart() const { return QDateTime(); }

    /**
       Returns the end of the selection, or an invalid QDateTime if there is no selection
       or the view doesn't support selecting cells.
     */
    virtual QDateTime selectionEnd() const { return QDateTime(); }

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

    virtual void showConfigurationDialog( QWidget *parent );

    QByteArray identifier() const;
    void setIdentifier( const QByteArray &identifier );

    /**
     * reads the view configuration. View-specific configuration can be
     * restored via doRestoreConfig()
     *
     * @param configGroup the group to read settings from
     * @see doRestoreConfig()
     */
    void restoreConfig( const KConfigGroup &configGroup );

    /**
     * writes out the view configuration. View-specific configuration can be
     * saved via doSaveConfig()
     *
     * @param configGroup the group to store settings in
     * @see doSaveConfig()
     */
    void saveConfig( KConfigGroup &configGroup );

    //----------------------------------------------------------------------------
    // TODO_SPLIT: review these collection stuff
    Akonadi::CollectionSelectionProxyModel *takeCustomCollectionSelectionProxyModel();
    Akonadi::CollectionSelectionProxyModel *customCollectionSelectionProxyModel() const;
    void setCustomCollectionSelectionProxyModel( Akonadi::CollectionSelectionProxyModel* model );

    Akonadi::CollectionSelection *customCollectionSelection() const;

    static Akonadi::CollectionSelection* globalCollectionSelection();
    static void setGlobalCollectionSelection( Akonadi::CollectionSelection* selection );
    //----------------------------------------------------------------------------

    /**
     * returns the view at the given widget coordinate. This is usually the view itself,
     * except for composite views, where a subview will be returned. The default implementation returns @p this .
     */
    virtual EventView* viewAt( const QPoint &p );

    void setDateRange( const KDateTime &start, const KDateTime &end );

    KDateTime startDateTime() const;
    KDateTime endDateTime() const;

    KDateTime actualStartDateTime() const;
    KDateTime actualEndDateTime() const;

    int showMoveRecurDialog( const Akonadi::Item &inc, const QDate &date );

    /**
     * Handles key events, opens the new event dialog when enter is pressed, activates
     * type ahead.
     */
    bool processKeyEvent( QKeyEvent * );

    /*
     * Sets the QObject that will receive key events that were made
     * while the new event dialog was still being created.
     */
    void setTypeAheadReceiver( QObject *o );

    /**
     * returns the selection of collection to be used by this view (custom if set, or global otherwise)
     */
    Akonadi::CollectionSelection* collectionSelection() const;

  public slots:

    /**
      Shows given incidences. Depending on the actual view it might not
      be possible to show all given events.

      @param incidenceList a list of incidences to show.
      @param date is the QDate on which the incidences are being shown.
    */
    virtual void showIncidences( const Akonadi::Item::List &incidenceList,
                                 const QDate &date ) = 0;

    /**
      Updates the current display to reflect changes that may have happened
      in the calendar since the last display refresh.
    */
    virtual void updateView() = 0;
    virtual void dayPassed( const QDate & );

    /**
      Assign a new incidence change helper object.
     */
    virtual void setIncidenceChanger( Akonadi::IncidenceChanger *changer );

    /**
      Write all unsaved data back to calendar store.
    */
    virtual void flushView();

    /**
      Updates the current display to reflect the changes to one particular incidence.
    */
    virtual void changeIncidenceDisplay( const Akonadi::Item &, int ) = 0;

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
    virtual bool eventDurationHint( QDateTime &startDt, QDateTime &endDt, bool &allDay ) const;

    void focusChanged( QWidget*, QWidget* );

    /**
     Perform the default action for an incidence, e.g. open the event editor,
     when double-clicking an event in the agenda view.
    */
    void defaultAction( const Akonadi::Item &incidence );

    /**
       Set which holidays the user wants to use.
       @param holidayRegion a HolidayRegion object initialized with the desired locale.
    */
    void setHolidayRegion( const KHolidays::HolidayRegionPtr &holidayRegion );

  signals:
    /**
     * when the view changes the dates that are selected in one way or
     * another, this signal is emitted.  It should be connected back to
     * the KDateNavigator object so that it changes appropriately,
     * and any other objects that need to be aware that the list of
     * selected dates has changed.
     *   @param datelist the new list of selected dates
     */
    void datesSelected( const DateList datelist );

    /**
     * Emitted when an event is moved using the mouse in an agenda
     * view (week / month).
     */
    void shiftedEvent( const QDate &olddate, const QDate &ewdate );

    void incidenceSelected( const Akonadi::Item &, const QDate );

    /**
     * instructs the receiver to show the incidence in read-only mode.
     */
    void showIncidenceSignal( const Akonadi::Item & );

    /**
     * instructs the receiver to begin editing the incidence specified in
     * some manner.  Doesn't make sense to connect to more than one
     * receiver.
     */
    void editIncidenceSignal( const Akonadi::Item & );

    /**
     * instructs the receiver to delete the Incidence in some manner; some
     * possibilities include automatically, with a confirmation dialog
     * box, etc.  Doesn't make sense to connect to more than one receiver.
     */
    void deleteIncidenceSignal( const Akonadi::Item & );

    /**
    * instructs the receiver to cut the Incidence
    */
    void cutIncidenceSignal( const Akonadi::Item & );

    /**
    * instructs the receiver to copy the incidence
    */
    void copyIncidenceSignal( const Akonadi::Item & );

    /**
    * instructs the receiver to paste the incidence
    */
    void pasteIncidenceSignal();

    /**
     * instructs the receiver to toggle the alarms of the Incidence.
     */
    void toggleAlarmSignal( const Akonadi::Item & );

    /**
     * instructs the receiver to toggle the completion state of the Incidence
     * (which must be a Todo type).
     */
    void toggleTodoCompletedSignal( const Akonadi::Item & );

    /**
     * Copy the incidence to the specified resource.
     */
    void copyIncidenceToResourceSignal( const Akonadi::Item &, const QString & );

    /**
     * Move the incidence to the specified resource.
     */
    void moveIncidenceToResourceSignal( const Akonadi::Item &, const QString & );

    /** Dissociate from a recurring incidence the occurrence on the given
     *  date to a new incidence or dissociate all occurrences from the
     *  given date onwards.
     */
    void dissociateOccurrencesSignal( const Akonadi::Item &, const QDate & );

    void startMultiModify( const QString & );
    void endMultiModify();

    /**
     * instructs the receiver to create a new event in given collection. Doesn't make
     * sense to connect to more than one receiver.
     */
    void newEventSignal( const Akonadi::Collection::List & );
    /**
     * instructs the receiver to create a new event with the specified beginning
     * time. Doesn't make sense to connect to more than one receiver.
     */
    void newEventSignal( const Akonadi::Collection::List &, const QDate & );
    /**
     * instructs the receiver to create a new event with the specified beginning
     * time. Doesn't make sense to connect to more than one receiver.
     */
    void newEventSignal( const Akonadi::Collection::List &, const QDateTime & );
    /**
     * instructs the receiver to create a new event, with the specified
     * beginning end ending times.  Doesn't make sense to connect to more
     * than one receiver.
     */
    void newEventSignal( const Akonadi::Collection::List &, const QDateTime &, const QDateTime & );

    void newTodoSignal( const QDate & );
    void newSubTodoSignal( const Akonadi::Item & );

    void newJournalSignal( const QDate & );

  protected slots:
    virtual void collectionSelectionChanged();
    virtual void calendarReset();

  private slots:
    void backendErrorOccurred();
    void dataChanged( const QModelIndex &topLeft, const QModelIndex &bottomRight );
    void rowsInserted( const QModelIndex &parent, int start, int end );
    void rowsAboutToBeRemoved( const QModelIndex &parent, int start, int end );

  protected:
    Akonadi::Item mCurrentIncidence;  // Incidence selected e.g. for a context menu
    Akonadi::IncidenceChanger *mChanger;

   /**
     * reimplement to read view-specific settings
     */
    virtual void doRestoreConfig( const KConfigGroup &configGroup );

    /**
     * reimplement to write vie- specific settings
     */
    virtual void doSaveConfig( KConfigGroup &configGroup );

    /**
      @deprecated
     */
    virtual void showDates( const QDate& start, const QDate& end ) = 0;

    /**
     * from the requested date range (passed via setDateRange()), calculates the adjusted date range actually displayed by the view, depending
     * on the view's supported range (e.g., a month view always displays one month)
     * The default implementation returns the range unmodified
     */
    virtual QPair<KDateTime,KDateTime> actualDateRange( const KDateTime& start, const KDateTime& end ) const;

    virtual void incidencesAdded( const Akonadi::Item::List& incidences );
    virtual void incidencesAboutToBeRemoved( const Akonadi::Item::List& incidences );
    virtual void incidencesChanged( const Akonadi::Item::List& incidences );

    virtual void handleBackendError( const QString &error );

    bool isWorkDay( const QDate &date ) const;
    QStringList holidayNames( const QDate &date ) const;

  private:
    /*
     * This is called when the new event dialog is shown. It sends
     * all events in mTypeAheadEvents to the receiver.
     */
    void finishTypeAhead();

  private:

    /* When we receive a QEvent with a key_Return release
     * we will only show a new event dialog if we previously received a
     * key_Return press, otherwise a new event dialog appears when
     * you hit return in some yes/no dialog */
    bool mReturnPressed;

    bool mTypeAhead;
    QObject *mTypeAheadReceiver;
    QList<QEvent*> mTypeAheadEvents;
    static Akonadi::CollectionSelection* sGlobalCollectionSelection;

    class Private;
    Private *const d;
};

} // namespace EventViews

#endif
