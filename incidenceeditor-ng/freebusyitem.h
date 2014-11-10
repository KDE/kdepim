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

#ifndef INCIDENCEEDITOR_FREEBUSYITEM_H
#define INCIDENCEEDITOR_FREEBUSYITEM_H

#include "incidenceeditors_ng_export.h"

#include <KCalCore/FreeBusy>

namespace IncidenceEditorNG
{

/**
 * The FreeBusyItem is the whole line for a given attendee..
 */
class INCIDENCEEDITORS_NG_EXPORT FreeBusyItem
{
public:
    typedef QSharedPointer<FreeBusyItem> Ptr;

    /**
     * @param parentWidget is passed to Akonadi when fetching free/busy data.
     */
    FreeBusyItem(const KCalCore::Attendee::Ptr &attendee, QWidget *parentWidget);
    ~FreeBusyItem() {}

    KCalCore::Attendee::Ptr attendee() const;
    void setFreeBusy(const KCalCore::FreeBusy::Ptr &fb);
    KCalCore::FreeBusy::Ptr freeBusy() const;

    QString email() const;
    void setUpdateTimerID(int id);
    int updateTimerID() const;

    void startDownload(bool forceDownload);
    void setIsDownloading(bool d);
    bool isDownloading() const;

signals:
    void attendeeChanged(const KCalCore::Attendee::Ptr &attendee);
    void freebusyChanged(const KCalCore::FreeBusy::Ptr fb);

private:
    KCalCore::Attendee::Ptr mAttendee;
    KCalCore::FreeBusy::Ptr mFreeBusy;

    // This is used for the update timer
    int mTimerID;

    // Only run one download job at a time
    bool mIsDownloading;

    QWidget *mParentWidget;
};

}
#endif
