/*
  Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
  Author: Kevin Krammer, krake@kdab.com
  Author: Sergio Martins, sergio.martins@kdab.com

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
#ifndef EVENTVIEWS_EVENTVIEW_P_H
#define EVENTVIEWS_EVENTVIEW_P_H

#include "eventview.h"

#include <KDateTime>

class KCheckableProxyModel;

namespace EventViews
{

class EventViewPrivate
{
public: /// Methods
    EventViewPrivate();
    ~EventViewPrivate();

    /**
      This is called when the new event dialog is shown. It sends
      all events in mTypeAheadEvents to the receiver.
     */
    void finishTypeAhead();

public: // virtual functions
    void setUpModels();

public: /// Members
    Akonadi::ETMCalendar::Ptr calendar;
    CalendarSupport::CollectionSelection *customCollectionSelection;
    KCheckableProxyModel *collectionSelectionModel;

    QByteArray identifier;
    KDateTime startDateTime;
    KDateTime endDateTime;
    KDateTime actualStartDateTime;
    KDateTime actualEndDateTime;

    /* When we receive a QEvent with a key_Return release
     * we will only show a new event dialog if we previously received a
     * key_Return press, otherwise a new event dialog appears when
     * you hit return in some yes/no dialog */
    bool mReturnPressed;
    bool mDateRangeSelectionEnabled;
    bool mTypeAhead;
    QObject *mTypeAheadReceiver;
    QList<QEvent *> mTypeAheadEvents;
    static CalendarSupport::CollectionSelection *sGlobalCollectionSelection;

    KHolidays::HolidayRegionPtr mHolidayRegion;
    PrefsPtr mPrefs;
    KCalPrefsPtr mKCalPrefs;

    Akonadi::IncidenceChanger *mChanger;
    EventView::Changes mChanges;
    Akonadi::Collection::Id mCollectionId;
};

} // EventViews

#endif
