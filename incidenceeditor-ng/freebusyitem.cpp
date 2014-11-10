/*
  Copyright (c) 2000,2001,2004 Cornelius Schumacher <schumacher@kde.org>
  Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
  Copyright (c) 2010 Andras Mantia <andras@kdab.com>
  Copyright (C) 2010 Casey Link <casey@kdab.com>

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

#include "freebusyitem.h"

#include <Akonadi/Calendar/FreeBusyManager>

using namespace IncidenceEditorNG;

FreeBusyItem::FreeBusyItem(const KCalCore::Attendee::Ptr &attendee, QWidget *parentWidget)
    : mAttendee(attendee), mTimerID(0), mIsDownloading(false), mParentWidget(parentWidget)
{
    Q_ASSERT(attendee);
    setFreeBusy(KCalCore::FreeBusy::Ptr());
}

KCalCore::Attendee::Ptr FreeBusyItem::attendee() const
{
    return mAttendee;
}

void FreeBusyItem::setFreeBusy(const KCalCore::FreeBusy::Ptr &fb)
{
    mFreeBusy = fb;
    mIsDownloading = false;
}

KCalCore::FreeBusy::Ptr FreeBusyItem::freeBusy() const
{
    return mFreeBusy;
}

QString FreeBusyItem::email() const
{
    return mAttendee->email();
}

void FreeBusyItem::setUpdateTimerID(int id)
{
    mTimerID = id;
}

int FreeBusyItem::updateTimerID() const
{
    return mTimerID;
}

void FreeBusyItem::startDownload(bool forceDownload)
{
    mIsDownloading = true;
    Akonadi::FreeBusyManager *m = Akonadi::FreeBusyManager::self();
    if (!m->retrieveFreeBusy(attendee()->email(), forceDownload, mParentWidget)) {
        mIsDownloading = false;
    }
}

void FreeBusyItem::setIsDownloading(bool d)
{
    mIsDownloading = d;
}

bool FreeBusyItem::isDownloading() const
{
    return mIsDownloading;
}
