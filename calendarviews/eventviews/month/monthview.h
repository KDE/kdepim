/*
  This file is part of KOrganizer.

  Copyright (c) 2008 Bruno Virlet <bruno.virlet@gmail.com>

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

#ifndef VIEW_H
#define VIEW_H

#include "../eventview.h"

#include <Akonadi/Item>

#include <QDate>
#include <QTimer>

class KOEventPopupMenu;

class QWheelEvent;
class QKeyEvent;
class QModelIndex;

namespace CalendarSupport {
  class CalendarSearch;
}

namespace EventViews {

class MonthGraphicsView;
class MonthScene;
class MonthViewPrivate;

/**
  New month view.
*/
class MonthView : public EventView
{
  Q_OBJECT
  public:
    explicit MonthView( QWidget *parent = 0 );
    ~MonthView();

    virtual int currentDateCount() const;
    int currentMonth() const;

    virtual Akonadi::Item::List selectedIncidences() const;

    /** Returns dates of the currently selected events */
    virtual KCalCore::DateList selectedIncidenceDates() const;

    virtual QDateTime selectionStart() const;

    virtual QDateTime selectionEnd() const;

    virtual void setDateRange( const KDateTime &start, const KDateTime &end );

    virtual bool eventDurationHint( QDateTime &startDt, QDateTime &endDt, bool &allDay ) const;

    /**
     * Returns the average date in the view
     */
    QDate averageDate() const;

    bool usesFullWindow();

    bool supportsDateRangeSelection() { return false; }

    // TODO: Where does this come from?
//     virtual CalPrinterBase::PrintType printType();

  Q_SIGNALS:
    void showIncidencePopup( const Akonadi::Item &item, const QDate &date );

  public slots:
    virtual void updateView();

    virtual void showIncidences( const Akonadi::Item::List &incidenceList, const QDate &date );

    void changeIncidenceDisplay( const Akonadi::Item &, int );

    /* reimp */void updateConfig();

  protected slots:
    void moveBackMonth();
    void moveBackWeek();
    void moveFwdWeek();
    void moveFwdMonth();
    /* reimp */void calendarReset();

  private Q_SLOTS:
    void dataChanged( const QModelIndex &topLeft, const QModelIndex &bottomRight );
    void rowsInserted( const QModelIndex &parent, int start, int end );
    void rowsAboutToBeRemoved( const QModelIndex &parent, int start, int end );

  protected:
    int maxDatesHint() const;

#ifndef QT_NO_WHEELEVENT
    virtual void wheelEvent( QWheelEvent *event );
#endif
    virtual void keyPressEvent( QKeyEvent *event );
    virtual void keyReleaseEvent( QKeyEvent *event );

    /* reimp */void incidencesAdded( const Akonadi::Item::List &incidences );
    /* reimp */void incidencesAboutToBeRemoved( const Akonadi::Item::List &incidences );
    /* reimp */void incidencesChanged( const Akonadi::Item::List &incidences );
    /* reimp */QPair<KDateTime,KDateTime> actualDateRange( const KDateTime &start,
                                                           const KDateTime &end ) const;

    /**
     * @deprecated
     */
    void showDates( const QDate &start, const QDate &end );

  private slots:
    // Compute and update the whole view
    void reloadIncidences();

  private:
    MonthGraphicsView *mView;
    MonthScene *mScene;
    Akonadi::Item::Id mSelectedItemId;
    QDate mSelectedItemDate;

    QTimer mReloadTimer;

  private:
    Q_DECLARE_PRIVATE( MonthView );

    friend class MonthScene;
    friend class MonthViewPrivate;
};

}

#endif
