/*
    This file is part of the KOrganizer interfaces.

    Copyright (c) 1999,2001,2003 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2004           Reinhold Kainhofer   <reinhold@kainhofer.com>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef KORG_BASEVIEW_H
#define KORG_BASEVIEW_H

#include <tqwidget.h>
#include <tqptrlist.h>
#include <tqvaluelist.h>

#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kdepimmacros.h>
#include "korganizer/incidencechangerbase.h"

#include "printplugin.h"

#include <libkcal/event.h>

using namespace KCal;

namespace KCal {
  class Calendar;
  class ResourceCalendar;
}

namespace KOrg {

/**
  This class provides an interface for all views being displayed within the
  main calendar view. It has functions to update the view, to specify date
  range and other display parameter and to return selected objects. An
  important class, which inherits KOBaseView is KOEventView, which provides
  the interface for all views of event data like the agenda or the month view.

  @short Base class for calendar views
  @author Preston Brown, Cornelius Schumacher
  @see KOTodoView, KOEventView, KOListView, KOAgendaView, KOMonthView
*/
class KDE_EXPORT BaseView : public QWidget
{
  Q_OBJECT
  public:
    /**
      Constructs a view.

      @param cal    Pointer to the calendar object from which events
                    will be retrieved for display.
      @param parent parent widget.
      @param name   name of this widget.
    */
    BaseView( Calendar *cal, TQWidget *parent = 0, const char *name = 0 )
      : TQWidget( parent, name ),
        mReadOnly( false ), mCalendar( cal ), mResource( 0 ), mChanger( 0 ) {}

    /**
      Destructor.  Views will do view-specific cleanups here.
    */
    virtual ~BaseView() {}

    /** Flag indicating if the view is read-only */
    void setReadOnly( bool readonly ) { mReadOnly = readonly; }
    bool readOnly() { return mReadOnly; }

    virtual void setCalendar( Calendar *cal ) { mCalendar = cal; }
    /**
      Return calendar object of this view.
    */
    virtual Calendar *calendar() { return mCalendar; }

    virtual void setResource( ResourceCalendar *res, const TQString &subResource )
    {
      mResource = res;
      mSubResource = subResource;
    }

    /**
      Return resourceCalendar of this view.
    */
    ResourceCalendar *resourceCalendar() { return mResource; }

    /**
      Return subResourceCalendar of this view.
    */
    TQString subResourceCalendar() const { return mSubResource; }

    /**
      @return a list of selected events.  Most views can probably only
      select a single event at a time, but some may be able to select
      more than one.
    */
    virtual Incidence::List selectedIncidences() = 0;

    /**
      @return a list of the dates of selected events.  Most views can probably only
      select a single event at a time, but some may be able to select
      more than one.
    */
    virtual DateList selectedIncidenceDates() = 0;

    /**
      Returns the start of the selection, or an invalid TQDateTime if there is no selection
      or the view doesn't support selecting cells.
    */
    virtual TQDateTime selectionStart() { return TQDateTime(); }

    /**
       Returns the end of the selection, or an invalid TQDateTime if there is no selection
       or the view doesn't support selecting cells.
     */
    virtual TQDateTime selectionEnd() { return TQDateTime(); }

    virtual CalPrinterBase::PrintType printType()
    {
      return CalPrinterBase::Month;
    }

    /**
      Return number of currently shown dates. A return value of 0 means no idea.
    */
    virtual int currentDateCount() = 0;

    /** Return if this view is a view for displaying events. */
    virtual bool isEventView() { return false; }

    /** Returns true if the view supports navigation through the date navigator
        ( selecting a date range, changing month, changing year, etc. )
     */
    virtual bool supportsDateNavigation() const { return false; }

  public slots:
    /**
      Show incidences for the given date range. The date range actually shown may be
      different from the requested range, depending on the particular requirements
      of the view.

      @param start Start of date range.
      @param end   End of date range.
    */
    virtual void showDates( const TQDate &start, const TQDate &end ) = 0;

    /**
      Show given incidences. Depending on the actual view it might not be possible to
      show all given events.

      @param incidenceList a list of incidences to show.
      @param date is the TQDate on which the incidences are being shown.
    */
    virtual void showIncidences( const Incidence::List &incidenceList, const TQDate &date ) = 0;

    /**
      Updates the current display to reflect changes that may have happened
      in the calendar since the last display refresh.
    */
    virtual void updateView() = 0;
    virtual void dayPassed( const TQDate & ) { updateView(); }

    /**
      Assign a new incidence change helper object.
     */
    virtual void setIncidenceChanger( IncidenceChangerBase *changer ) { mChanger = changer; }

    /**
      Write all unsaved data back to calendar store.
    */
    virtual void flushView() {}

    /**
      Updates the current display to reflect the changes to one particular incidence.
    */
    virtual void changeIncidenceDisplay( Incidence *, int ) = 0;

    /**
      Re-reads the KOrganizer configuration and picks up relevant
      changes which are applicable to the view.
    */
    virtual void updateConfig() {}

    /**
      Clear selection. The incidenceSelected signal is not emitted.
    */
    virtual void clearSelection() {}

    /**
      Set the default start/end date/time for new events. Return true if anything was changed
    */
    virtual bool eventDurationHint(TQDateTime &/*startDt*/, TQDateTime &/*endDt*/, bool &/*allDay*/) { return false; }

  signals:
    void incidenceSelected( Incidence *, const TQDate & );

    /**
     * instructs the receiver to show the incidence in read-only mode.
     */
    void showIncidenceSignal( Incidence *, const TQDate & );

    /**
     * instructs the receiver to begin editing the incidence specified in
     * some manner.  Doesn't make sense to connect to more than one
     * receiver.
     */
    void editIncidenceSignal( Incidence *, const TQDate & );

    /**
     * instructs the receiver to delete the Incidence in some manner; some
     * possibilities include automatically, with a confirmation dialog
     * box, etc.  Doesn't make sense to connect to more than one receiver.
     */
    void deleteIncidenceSignal(Incidence *);

    /**
    * instructs the receiver to cut the Incidence
    */
    void cutIncidenceSignal(Incidence *);

    /**
    * instructs the receiver to copy the incidence
    */
    void copyIncidenceSignal(Incidence *);

    /**
    * instructs the receiver to paste the incidence
    */
    void pasteIncidenceSignal();

    /**
     * instructs the receiver to toggle the alarms of the Incidence.
     */
    void toggleAlarmSignal(Incidence *);
    /** Dissociate from a recurring incidence the occurrence on the given
        date to a new incidence */
    void dissociateOccurrenceSignal( Incidence *, const TQDate & );
    /** Dissociate from a recurring incidence all occurrences after the given
        date to a new incidence */
    void dissociateFutureOccurrenceSignal( Incidence *, const TQDate & );

    void startMultiModify( const TQString & );
    void endMultiModify();

    /**
     * instructs the receiver to create a new event.  Doesn't make
     * sense to connect to more than one receiver.
     */
    void newEventSignal( ResourceCalendar *res, const TQString &subResource );
    /**
     * instructs the receiver to create a new event with the specified beginning
     * time. Doesn't make sense to connect to more than one receiver.
     */
    void newEventSignal( ResourceCalendar *res, const TQString &subResource,
                         const TQDate & );
    /**
     * instructs the receiver to create a new event with the specified beginning
     * time. Doesn't make sense to connect to more than one receiver.
     */
    void newEventSignal( ResourceCalendar *res, const TQString &subResource,
                         const TQDateTime & );
    /**
     * instructs the receiver to create a new event, with the specified
     * beginning end ending times.  Doesn't make sense to connect to more
     * than one receiver.
     */
    void newEventSignal( ResourceCalendar *res, const TQString &subResource,
                         const TQDateTime &, const TQDateTime & );

    void newTodoSignal( ResourceCalendar *res,const TQString &subResource,
                        const TQDate & );
    void newSubTodoSignal( Todo * );

    void newJournalSignal( ResourceCalendar *res,const TQString &subResource,
                           const TQDate & );

  private:
    bool mReadOnly;
    Calendar *mCalendar;
    ResourceCalendar *mResource;
    TQString mSubResource;

  protected:
    IncidenceChangerBase *mChanger;
};

}

#endif
