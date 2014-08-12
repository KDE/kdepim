/*
  Copyright (c) 2007 Volker Krause <vkrause@kde.org>
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
*/

#ifndef EVENTVIEWS_TIMELINEITEM_H
#define EVENTVIEWS_TIMELINEITEM_H

#include <KCalCore/Incidence>

#include <Akonadi/Calendar/ETMCalendar>
#include <AkonadiCore/Item>

#include <KDateTime>

#include <QMap>
#include <QList>
#include <QStandardItemModel>


namespace EventViews {

class TimelineSubItem;

class TimelineItem : public QObject
{
  Q_OBJECT
  public:
    TimelineItem( const Akonadi::ETMCalendar::Ptr &calendar, uint index, QStandardItemModel *model,
                  QObject *parent );

    void insertIncidence( const Akonadi::Item &incidence,
                          const KDateTime &start = KDateTime(),
                          const KDateTime &end = KDateTime() );
    void removeIncidence( const Akonadi::Item &incidence );

    void moveItems( const Akonadi::Item &incidence, int delta, int duration );

    void setColor( const QColor &color );

  private:
    Akonadi::ETMCalendar::Ptr mCalendar;
    QMap<Akonadi::Item::Id, QList<QStandardItem*> > mItemMap;
    QStandardItemModel *mModel;
    QColor mColor;
    uint mIndex;
};

class TimelineSubItem : public QStandardItem
{
  public:
    TimelineSubItem( const Akonadi::ETMCalendar::Ptr &calendar,
                     const Akonadi::Item &incidence, TimelineItem *parent );
    ~TimelineSubItem();

    Akonadi::Item  incidence() const
    {
      return mIncidence;
    }

    KDateTime originalStart() const
    {
      return mStart;
    }

    void setOriginalStart( const KDateTime &dt )
    {
      mStart = dt;
    }

    void setStartTime( const QDateTime &dt );
    QDateTime startTime() const;

    void setEndTime( const QDateTime &dt );
    QDateTime endTime() const;

    TimelineItem *parent()
    {
      return mParent;
    }

    void updateToolTip();

  private:
    Akonadi::ETMCalendar::Ptr mCalendar;
    Akonadi::Item mIncidence;
    KDateTime mStart;
    TimelineItem *mParent;
    bool mToolTipNeedsUpdate;
};

}

#endif
