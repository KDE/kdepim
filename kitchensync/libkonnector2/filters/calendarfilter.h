/*
    This file is part of KitchenSync.

    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef CALENDARFILTER_H
#define CALENDARFILTER_H

#include "filter.h"

namespace KSync {

class CalendarSyncee;

class CalendarConfigWidget : public QWidget
{
  Q_OBJECT

  public:
    CalendarConfigWidget( QWidget *parent, const char *name );

    void setCategories( const QStringList &categories );

    void setSelectedCategories( const QStringList &categories );
    QStringList selectedCategories() const;

    void setStartDate( const QDate& );
    QDate startDate() const;

    void setEndDate( const QDate& );
    QDate endDate() const;

    void setUseDate( bool );
    bool useDate() const;

  private slots:
    void useDateChanged( bool );

  private:
    QListView *mView;
    QLabel *mStartLabel;
    KDateEdit *mStartDate;
    QLabel *mEndLabel;
    KDateEdit *mEndDate;
    QCheckBox *mUseDate;
};


class CalendarFilter : public Filter
{
  public:
    CalendarFilter( QObject *parent );
    virtual ~CalendarFilter();
  
    virtual bool supports( Syncee *syncee );
    virtual QWidget *configWidget( QWidget *parent );
    virtual void configWidgetClosed( QWidget *widget );
  
    virtual void convert( Syncee* );
    virtual void reconvert( Syncee* );
  
    QString type() const { return "calendar"; }

  private:
    void doLoad();
    void doSave();

    void filterSyncee( CalendarSyncee*, const QStringList&,
                       const QDate&, const QDate& );
    void unfilterSyncee( CalendarSyncee* );

    CalendarSyncEntry::PtrList mFilteredEntries;
    QStringList mSelectedCategories;
    bool mFilterByDate;
    QDate mStartDate;
    QDate mEndDate;
};

}

#endif
