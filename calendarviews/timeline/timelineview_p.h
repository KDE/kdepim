/*
  Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
  Copyright (c) 2010 Sérgio Martins <sergio.martins@kdab.com>

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
#ifndef EVENTVIEWS_TIMELINEVIEW_P_H
#define EVENTVIEWS_TIMELINEVIEW_P_H

#include "timelineview.h"

#include <Collection>
#include <AkonadiCore/Item>


#include <QMap>
#include <QModelIndex>
#include <QObject>

class QStandardItem;
class QTreeWidget;

namespace KDGantt {
  class GraphicsView;
}

namespace EventViews {

class TimelineItem;
class RowController;

class TimelineView::Private : public QObject
{
  Q_OBJECT
  public:
    explicit Private( TimelineView *parent = 0 );
    ~Private();

    TimelineItem *calendarItemForIncidence( const Akonadi::Item &incidence );
    void insertIncidence( const Akonadi::Item &incidence );
    void insertIncidence( const Akonadi::Item &incidence, const QDate &day );
    void removeIncidence( const Akonadi::Item &incidence );

  public Q_SLOTS:
    // void overscale( KDGantt::View::Scale scale );
    void itemSelected( const QModelIndex &index );
    void itemDoubleClicked( const QModelIndex &index );
    void itemChanged( QStandardItem *item );
    void contextMenuRequested( const QPoint &point );
    void newEventWithHint( const QDateTime & );
    void splitterMoved();

  public:
    Akonadi::Item::List mSelectedItemList;
    KDGantt::GraphicsView *mGantt;
    QTreeWidget *mLeftView;
    RowController *mRowController;
    QMap<Akonadi::Collection::Id, TimelineItem*> mCalendarItemMap;
    QDate mStartDate, mEndDate;
    QDateTime mHintDate;

  private:
    TimelineView *const q;
};

} // namespace EventViews

#endif
