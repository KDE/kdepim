/*
    This file is part of KOrganizer.

    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef TimeSpanWidget_H
#define TimeSpanWidget_H

#include <tqwidget.h>

namespace KCal {
class Event;
}

class QSplitter;
class QListView;
class LineView;
class TimeLine;

class TimeSpanWidget : public QWidget
{
    Q_OBJECT
  public:
    TimeSpanWidget( TQWidget *parent=0, const char *name=0 );
    virtual ~TimeSpanWidget();
    
    void addItem( KCal::Event * );

    TQValueList<int> splitterSizes();
    void setSplitterSizes( TQValueList<int> );
    
    void clear();
    
    void setDateRange( const TQDateTime &start, const TQDateTime &end );
    
    TQDateTime startDateTime();
    TQDateTime endDateTime();

  public slots:
    void updateView();

    void zoomIn();
    void zoomOut();
    void centerView();
    
  signals:
    void dateRangeChanged();
    
  private:
    TQSplitter *mSplitter;
    TQListView *mList;
    TimeLine *mTimeLine;
    LineView *mLineView;
    
    TQDateTime mStartDate;
    TQDateTime mEndDate;
    int mSecsPerPixel;
};

#endif
