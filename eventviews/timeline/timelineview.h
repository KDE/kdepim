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

#include <Item>

#include <QDateTime>

namespace EventViews
{

/**
  This class provides a view ....
*/
class EVENTVIEWS_EXPORT TimelineView : public EventView
{
    Q_OBJECT
public:
    explicit TimelineView(QWidget *parent = Q_NULLPTR);
    ~TimelineView();

    Akonadi::Item::List selectedIncidences() const Q_DECL_OVERRIDE;
    KCalCore::DateList selectedIncidenceDates() const Q_DECL_OVERRIDE;
    int currentDateCount() const Q_DECL_OVERRIDE;

    // ensure start and end are valid before calling this.
    void showDates(const QDate &, const QDate &, const QDate &preferredMonth = QDate()) Q_DECL_OVERRIDE;

    // FIXME: we already have startDateTime() in the base class
    // why aren't we using it.
    QDate startDate() const;
    QDate endDate() const;

    void showIncidences(const Akonadi::Item::List &incidenceList, const QDate &date) Q_DECL_OVERRIDE;
    void updateView() Q_DECL_OVERRIDE;
    virtual void changeIncidenceDisplay(const Akonadi::Item &incidence, int mode);
    bool eventDurationHint(QDateTime &startDt, QDateTime &endDt, bool &allDay) const Q_DECL_OVERRIDE;

Q_SIGNALS:
    void showNewEventPopupSignal();
    void showIncidencePopupSignal(const Akonadi::Item &, const QDate &);

protected:
    bool eventFilter(QObject *object, QEvent *event) Q_DECL_OVERRIDE;

private:
    class Private;
    Private *const d;

};

} // namespace EventViews

#endif
