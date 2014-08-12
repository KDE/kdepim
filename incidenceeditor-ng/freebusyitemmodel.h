/*
  Copyright (C) 2010 Casey Link <unnamedrambler@gmail.com>
  Copyright (C) 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

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

#ifndef INCIDENCEEDITOR_FREEBUSYITEMMODEL_H
#define INCIDENCEEDITOR_FREEBUSYITEMMODEL_H

#include "incidenceeditors_ng_export.h"
#include "freebusyitem.h"

#include <QAbstractItemModel>
#include <QTimer>

namespace IncidenceEditorNG {

class ItemPrivateData;

/**
 * The FreeBusyItemModel is a 2-level tree structure.
 *
 * The top level parent nodes represent the freebusy items, and
 * the 2nd-level child nodes represent the FreeBusyPeriods of the parent
 * freebusy item.
 */
class INCIDENCEEDITORS_NG_EXPORT FreeBusyItemModel : public QAbstractItemModel
{
  Q_OBJECT
  public:
    enum Roles {
        AttendeeRole = Qt::UserRole,
        FreeBusyRole,
        FreeBusyPeriodRole
    };

    explicit FreeBusyItemModel( QObject *parent = 0 );
    virtual ~FreeBusyItemModel();

    virtual QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;
    virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const;
    virtual int columnCount( const QModelIndex &parent = QModelIndex() ) const;
    virtual QModelIndex index( int row, int column = 0,
                               const QModelIndex &parent = QModelIndex() ) const;
    virtual QModelIndex parent( const QModelIndex &child ) const;
    virtual QVariant headerData( int section, Qt::Orientation orientation,
                                 int role = Qt::DisplayRole ) const;

    void addItem( const FreeBusyItem::Ptr &freebusy );

    void clear();
    void removeAttendee( const KCalCore::Attendee::Ptr &attendee );
    void removeItem( const FreeBusyItem::Ptr &freebusy );
    void removeRow( int row );

    bool containsAttendee( const KCalCore::Attendee::Ptr &attendee );

    /**
     * Queues a reload of free/busy data.
     * All current attendees will have their free/busy data
     * redownloaded from Akonadi.
     */
    void triggerReload();

    /**
     * cancel reloading
     */
    void cancelReload();

    /**
     * Reload FB items
     */
    void reload();

  public slots:
    void slotInsertFreeBusy( const KCalCore::FreeBusy::Ptr &fb, const QString &email );

  protected:
    void timerEvent( QTimerEvent * );

  private slots:
    // Force the download of FB information
    void manualReload();
    // Only download FB if the auto-download option is set in config
    void autoReload();

  private:
    void setFreeBusyPeriods( const QModelIndex &parent,
                             const KCalCore::FreeBusyPeriod::List &list );
    void updateFreeBusyData( const FreeBusyItem::Ptr & );

    QTimer mReloadTimer;
    bool mForceDownload;
    QList<FreeBusyItem::Ptr> mFreeBusyItems;
    ItemPrivateData *mRootData;
};

}

#endif
