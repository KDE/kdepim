/*
  Copyright (c) 2007 Till Adam <adam@kde.org>
  Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
  Copyright (c) 2010 Andras Mantia <andras@kdab.com>

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
#ifndef EVENTVIEWS_TIMELINEVIEW_H
#define EVENTVIEWS_TIMELINEVIEW_H

#include "eventview.h"

#include <Akonadi/Item>

#include <QDateTime>

namespace EventViews {

/**
  This class provides a view ....
*/
class EVENTVIEWS_EXPORT TimelineView : public EventView
{
    Q_OBJECT
  public:
    explicit TimelineView( QWidget *parent = 0 );
    ~TimelineView();

    virtual Akonadi::Item::List selectedIncidences() const;
    virtual KCalCore::DateList selectedIncidenceDates() const;
    virtual int currentDateCount() const;

    // ensure start and end are valid before calling this.
    virtual void showDates( const QDate &, const QDate &, const QDate &preferredMonth = QDate() );

    // FIXME: we already have startDateTime() in the base class
    // why aren't we using it.
    QDate startDate() const;
    QDate endDate() const;

    virtual void showIncidences( const Akonadi::Item::List &incidenceList, const QDate &date );
    virtual void updateView();
    virtual void changeIncidenceDisplay( const Akonadi::Item &incidence, int mode );
    virtual bool eventDurationHint( QDateTime &startDt, QDateTime &endDt, bool &allDay ) const;

  Q_SIGNALS:
    void showNewEventPopupSignal();
    void showIncidencePopupSignal( Akonadi::Item, const QDate & );

  protected:
    virtual bool eventFilter( QObject *object, QEvent *event );

  private:
    class Private;
    Private *const d;

};

} // namespace EventViews

#endif
