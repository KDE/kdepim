/*
  Copyright (c) 2000,2001,2004 Cornelius Schumacher <schumacher@kde.org>
  Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
  Copyright (c) 2010 Andras Mantia <andras@kdab.com>
  Copyright (C) 2010 Casey Link <casey@kdab.com>

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
#ifndef INCIDENCEEDITOR_EDITORFREEBUSY_H
#define INCIDENCEEDITOR_EDITORFREEBUSY_H

#include "attendeeeditor.h"

#include <KCalCore/FreeBusy>

#include <QDateTime>
#include <QTimer>
#include <QAbstractItemModel>
#include <QDialog>

class QTreeWidget;
class QLabel;
class FreeBusyItem;
class RowController;

namespace KDGantt {
  class DateTimeGrid;
  class GraphicsView;
}

class KDateTime;

namespace IncidenceEditorNG {
class EditorFreeBusy : public QDialog
{
  Q_OBJECT
  public:
    explicit EditorFreeBusy( int spacing = 8, QWidget *parent = 0 );
    virtual ~EditorFreeBusy();

    void setUpdateEnabled( bool enabled );
    bool updateEnabled() const;

    void insertAttendee( AttendeeData::Ptr , bool readFBList = true );
    void removeAttendee( AttendeeData::Ptr  );
    void clearAttendees();


    void triggerReload();
    void cancelReload();

    int indexOfItem( FreeBusyItem* item );

  signals:
    void dateTimesChanged( const QDateTime &, const QDateTime & );

  public slots:
    void slotInsertFreeBusy( const KCalCore::FreeBusy::Ptr &fb, const QString &email );

    void setDateTimes( const KDateTime &, const KDateTime  & );

    void editFreeBusyUrl( const QModelIndex& index );

    void slotOrganizerChanged( const QString &newOrganizer );

  protected slots:
    void slotUpdateGanttView( const QDateTime &, const QDateTime & );
    void slotScaleChanged( int );
    void slotCenterOnStart() ;
    void slotZoomToTime();
    void slotPickDate();
    void showAttendeeStatusMenu();

    // Force the download of FB information
    void manualReload();
    // Only download FB if the auto-download option is set in config
    void autoReload();

    void slotIntervalColorRectangleMoved( const QDateTime &start, const QDateTime &end );

    void removeAttendee();

  protected:
    void timerEvent( QTimerEvent * );
    AttendeeData::Ptr currentAttendee() const;
    /* reimpl */
//     Q3ListViewItem *hasExampleAttendee() const;
    void updateCurrentItem();
    void clearSelection() const;
    void changeStatusForMe( KCalCore::Attendee::PartStat status );
    virtual bool eventFilter( QObject *watched, QEvent *event );

  private slots:
    void splitterMoved();

  private:
    void updateFreeBusyData( FreeBusyItem * );

    bool findFreeSlot( KDateTime &dtFrom, KDateTime &dtTo );
    bool tryDate( KDateTime &tryFrom, KDateTime &tryTo );
    bool tryDate( FreeBusyItem *attendee,
                  KDateTime &tryFrom, KDateTime &tryTo );
    void updateStatusSummary();
    void reload();
    FreeBusyItem* selectedItem() const;

    KDGantt::GraphicsView *mGanttView;
    QTreeWidget *mLeftView;
    RowController *mRowController;
    KDGantt::DateTimeGrid *mGanttGrid;
//     KDIntervalColorRectangle *mEventRectangle;
    QLabel *mStatusSummaryLabel;
    bool mIsOrganizer;
    KComboBox *mScaleCombo;

    QDateTime mDtStart, mDtEnd;

    QTimer mReloadTimer;

    bool mForceDownload;

    QString mCurrentOrganizer;
    QList<FreeBusyItem*> mFreeBusyItems; //TODO: holds all the items. if a tree like structure is needed, instead of the list, add it as a data of mLeftView items
};

}
#endif
