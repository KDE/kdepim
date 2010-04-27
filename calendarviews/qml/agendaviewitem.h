/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#ifndef CALENDARVIEWS_AGENDAVIEWITEM_H
#define CALENDARVIEWS_AGENDAVIEWITEM_H

#include "declarativeakonadiitem.h"
#include <QtCore/QDate>

namespace EventViews {
  
class AgendaView;

class AgendaViewItem : public DeclarativeAkonadiItem
{
  Q_OBJECT
  Q_ENUMS( Range )
  Q_PROPERTY( QDate startDate READ startDate WRITE setStartDate )
  Q_PROPERTY( QDate endDate READ endDate WRITE setEndDate )
  Q_PROPERTY( QObject* calendar READ calendar WRITE setCalendar )
  Q_PROPERTY( qint64 selectedItemId READ selectedItemId NOTIFY itemSelected )

  public:
    enum Range {
      Day = 0,
      Week,
      Month
    };

  public:
    explicit AgendaViewItem( QDeclarativeItem *parent = 0 );
    ~AgendaViewItem();

    virtual qint64 itemId() const { return -1; }
    virtual void setItemId( qint64 /*id*/ ) {}

    QDate startDate() const;
    void setStartDate( const QDate &startDate );
    QDate endDate() const;
    void setEndDate( const QDate &endDate );
    QObject *calendar() const;
    void setCalendar( QObject* calendarObj );

    /** Show the appropriate range for given date. */
    Q_INVOKABLE void showRange( const QDate &date, /* Range */ int range ); // TODO: Figure out how to export enums to QML

    qint64 selectedItemId() const;

  public slots:
    /** Show the following date range of equal length right after the current one. */
    void gotoNext();
    /** Show the preceeding date range. */
    void gotoPrevious();

  signals:
    void itemSelected();

  private:
    AgendaView *m_view;
};

}

#endif
