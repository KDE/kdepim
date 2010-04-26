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

class AgendaView;
namespace CalendarViews {

class AgendaViewItem : public DeclarativeAkonadiItem
{
  Q_OBJECT
  Q_PROPERTY( QDate startDate READ startDate WRITE setStartDate )
  Q_PROPERTY( QDate endDate READ endDate WRITE setEndDate )

  public:
    explicit AgendaViewItem( QDeclarativeItem *parent = 0 );
    ~AgendaViewItem();

    virtual qint64 itemId() const { return -1; }
    virtual void setItemId( qint64 /*id*/ ) {}

    QDate startDate() const;
    void setStartDate( const QDate &startDate );
    QDate endDate() const;
    void setEndDate( const QDate &endDate );

  private:
    AgendaView *m_view;
};

}

#endif
