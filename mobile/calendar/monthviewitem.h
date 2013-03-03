/*
    Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>

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
#ifndef MONTHVIEWITEM_H
#define MONTHVIEWITEM_H

#include "declarativeakonadiitem.h"

#include "calendarviews/prefs.h"

namespace EventViews {

class MonthView;

class MonthViewItem : public DeclarativeAkonadiItem
{
  Q_OBJECT
  Q_PROPERTY( QObject* calendar READ calendar WRITE setCalendar )

  public:
    explicit MonthViewItem( QDeclarativeItem *parent = 0 );
    virtual ~MonthViewItem();

    virtual qint64 itemId() const { return -1; }
    virtual void setItemId( qint64 /*id*/ ) {}

    QObject *calendar() const;
    void setCalendar( QObject* calendarObj );

    /// Show the month from @param date.
    Q_INVOKABLE void showMonth( const QDate &date );

    void setPreferences( const PrefsPtr &preferences );
    PrefsPtr preferences() const;

  Q_SIGNALS:
    void dateClicked( const QDate &date );
    void itemSelected( qint64 selectedItemId, const QDate &activeDate );

  public Q_SLOTS:
    void updateConfig();

  private Q_SLOTS:
    void emitDateClicked();
    void emitItemSelected( const Akonadi::Item &item, const QDate &activeDate );

  private:
    MonthView *mView;
};

}

#endif // MONTHVIEWITEM_H
