/*
    This file is part of KOrganizer.

    Copyright (c) 2000,2001,2003 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
#ifndef KOAGENDAITEM_H
#define KOAGENDAITEM_H

#include "cellitem.h"

#include <tqdatetime.h>

class TQToolTipGroup;
class TQDragEnterEvent;
class TQDropEvent;

namespace KCal {
class Calendar;
class Incidence;
}
using namespace KCal;
class KOAgendaItem;

struct MultiItemInfo
{
  int mStartCellXLeft, mStartCellXRight;
  int mStartCellYTop, mStartCellYBottom;
  KOAgendaItem *mFirstMultiItem;
  KOAgendaItem *mPrevMultiItem;
  KOAgendaItem *mNextMultiItem;
  KOAgendaItem *mLastMultiItem;
};

/*
  The KOAgendaItem has to make sure that it receives all mouse events, which are
  to be used for dragging and resizing. That means it has to be installed as
  eventfiler for its children, if it has children, and it has to pass mouse
  events from the cildren to itself. See eventFilter().


  Some comments on the movement of multi-day items:
  Basically, the agenda items are arranged in two implicit double-linked lists.
  The mMultiItemInfo works like before to describe the currently viewed
  multi-item.
  When moving, new events might need to be added to the beginning or the end of
  the multi-item sequence, or events might need to be hidden. I cannot just
  delete this items, since I have to restore/show them if the move is reset
  (i.e. if a drag started). So internally, I keep another doubly-linked list
  which is longer than the one defined by mMultiItemInfo, but includes the
  multi-item sequence, too.

  The mStartMoveInfo stores the first and last item of the multi-item sequence
  when the move started. The prev and next members of mStartMoveInfo are used
  for that longer sequence including all (shown and hidden) items.
*/
class KOAgendaItem : public TQWidget, public KOrg::CellItem
{
    Q_OBJECT
  public:
    KOAgendaItem( Calendar *calendar, Incidence *incidence, const TQDate &qd,
                  TQWidget *parent,
                  int itemPos, int itemCount,
                  const char *name = 0, WFlags f = 0 );

    int cellXLeft() const { return mCellXLeft; }
    int cellXRight() const { return mCellXRight; }
    int cellYTop() const { return mCellYTop; }
    int cellYBottom() const { return mCellYBottom; }
    int cellHeight() const;
    int cellWidth() const;

    int itemPos() const { return mItemPos; }
    int itemCount() const { return mItemCount; }

    void setCellXY(int X, int YTop, int YBottom);
    void setCellY(int YTop, int YBottom);
    void setCellX(int XLeft, int XRight);
    void setCellXRight(int xright);

    /** Start movement */
    void startMove();
    /** Reset to original values */
    void resetMove();
    /** End the movement (i.e. clean up) */
    void endMove();

    void moveRelative(int dx,int dy);
    void expandTop(int dy);
    void expandBottom(int dy);
    void expandLeft(int dx);
    void expandRight(int dx);

    bool isMultiItem();
    KOAgendaItem *prevMoveItem() const { return (mStartMoveInfo)?(mStartMoveInfo->mPrevMultiItem):0; }
    KOAgendaItem *nextMoveItem() const { return (mStartMoveInfo)?(mStartMoveInfo->mNextMultiItem):0; }
    MultiItemInfo *moveInfo() const { return mStartMoveInfo; }
    void setMultiItem(KOAgendaItem *first,KOAgendaItem *prev,
                      KOAgendaItem *next, KOAgendaItem *last);
    KOAgendaItem *prependMoveItem(KOAgendaItem*);
    KOAgendaItem *appendMoveItem(KOAgendaItem*);
    KOAgendaItem *removeMoveItem(KOAgendaItem*);
    KOAgendaItem *firstMultiItem() const { return (mMultiItemInfo)?(mMultiItemInfo->mFirstMultiItem):0; }
    KOAgendaItem *prevMultiItem() const { return (mMultiItemInfo)?(mMultiItemInfo->mPrevMultiItem):0; }
    KOAgendaItem *nextMultiItem() const { return (mMultiItemInfo)?(mMultiItemInfo->mNextMultiItem):0; }
    KOAgendaItem *lastMultiItem() const { return (mMultiItemInfo)?(mMultiItemInfo->mLastMultiItem):0; }

    bool dissociateFromMultiItem();

    bool setIncidence( Incidence * );
    Incidence *incidence() const { return mIncidence; }
    TQDate itemDate() { return mDate; }

    /** Update the date of this item's occurrence (not in the event) */
    void setItemDate( const TQDate &qd );

    void setText ( const TQString & text ) { mLabelText = text; }
    TQString text () { return mLabelText; }

    static TQToolTipGroup *toolTipGroup();

    TQPtrList<KOAgendaItem> conflictItems();
    void setConflictItems(TQPtrList<KOAgendaItem>);
    void addConflictItem(KOAgendaItem *ci);

    TQString label() const;

    bool overlaps( KOrg::CellItem * ) const;

    void setResourceColor( const TQColor& color ) { mResourceColor = color; }
    TQColor resourceColor() {return mResourceColor;}
  signals:
    void removeAgendaItem( KOAgendaItem* );
    void showAgendaItem( KOAgendaItem* );

  public slots:
    void updateIcons();
    void select(bool=true);
    void addAttendee( const TQString & );

  protected:
    void dragEnterEvent(TQDragEnterEvent *e);
    void dropEvent(TQDropEvent *e);
    void paintEvent(TQPaintEvent *e);
    void paintFrame(TQPainter *p, const TQColor &color);
    void paintEventIcon(TQPainter *p, int &x, int ft);
    void paintTodoIcon(TQPainter *p, int &x, int ft);
    void paintAlarmIcon(TQPainter *p, int &x, int ft);

    // paint all visible icons
    void paintIcons(TQPainter *p, int &x, int ft);

    /** private movement functions. startMove needs to be called of only one of
     *  the multitems. it will then loop through the whole series using
     *  startMovePrivate. Same for resetMove and endMove */
    void startMovePrivate();
    void resetMovePrivate();
    void endMovePrivate();


  private:
    int mCellXLeft, mCellXRight;
    int mCellYTop, mCellYBottom;

    Calendar *mCalendar;
    Incidence *mIncidence; // corresponding event or todo
    TQDate mDate; //date this events occurs (for recurrence)
    TQString mLabelText;
    bool mIconAlarm, mIconRecur, mIconReadonly;
    bool mIconReply, mIconGroup, mIconGroupTentative;
    bool mIconOrganizer, mSpecialEvent;

    // For incidences that expand through more than 1 day
    // Will be 1 for single day incidences
    int mItemPos;
    int mItemCount;

    // Multi item pointers
    MultiItemInfo* mMultiItemInfo;
  protected:
    // Variables to remember start position
    MultiItemInfo* mStartMoveInfo;
    //Color of the resource
    TQColor mResourceColor;
  private:
    static TQToolTipGroup *mToolTipGroup;

    bool mSelected;
    TQPtrList<KOAgendaItem> mConflictItems;

    static TQPixmap *alarmPxmp;
    static TQPixmap *recurPxmp;
    static TQPixmap *readonlyPxmp;
    static TQPixmap *replyPxmp;
    static TQPixmap *groupPxmp;
    static TQPixmap *groupPxmpTentative;
    static TQPixmap *organizerPxmp;
};

#endif
