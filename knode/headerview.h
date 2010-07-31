/*
    KNode, the KDE newsreader
    Copyright (c) 1999-2005 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#ifndef KNHEADERVIEW_H
#define KNHEADERVIEW_H

#include <tqtooltip.h>

#include <klistview.h>
#include <kfoldertree.h>
#include <kmime_util.h>

class KPopupMenu;
class KNHdrViewItem;

class KNHeaderView : public KListView  {

  Q_OBJECT

  friend class KNHdrViewItem;

  public:
    KNHeaderView( TQWidget *parent, const char *name = 0 );
    ~KNHeaderView();

    void setActive( TQListViewItem *item );
    void clear();

    void ensureItemVisibleWithMargin( const TQListViewItem *i );

    virtual void setSorting( int column, bool ascending = true );
    bool sortByThreadChangeDate() const      { return mSortByThreadChangeDate; }
    void setSortByThreadChangeDate( bool b ) { mSortByThreadChangeDate = b; }

    bool nextUnreadArticle();
    bool nextUnreadThread();

    void readConfig();
    void writeConfig();

    const KPaintInfo* paintInfo() const { return &mPaintInfo; }

  signals:
    void itemSelected( TQListViewItem* );
    void doubleClick( TQListViewItem* );
    void sortingChanged( int );
    void focusChanged( TQFocusEvent* );
    void focusChangeRequest( TQWidget* );

  public slots:
    void nextArticle();
    void prevArticle();
    void incCurrentArticle();
    void decCurrentArticle();
    void selectCurrentArticle();

    void toggleColumn( int column, int mode = -1 );
    void prepareForGroup();
    void prepareForFolder();

  protected:
    void activeRemoved()            { mActiveItem = 0; }
    /**
     * Reimplemented to avoid that KListview reloads the alternate
     * background on palette changes.
     */
    virtual bool event( TQEvent *e );
    void contentsMousePressEvent( TQMouseEvent *e );
    void contentsMouseDoubleClickEvent( TQMouseEvent *e );
    void keyPressEvent( TQKeyEvent *e );
    bool eventFilter( TQObject *, TQEvent * );
    void focusInEvent( TQFocusEvent *e );
    void focusOutEvent( TQFocusEvent *e );
    virtual TQDragObject* dragObject();

  private:
    int mSortCol;
    bool mSortAsc;
    bool mSortByThreadChangeDate;
    int mDelayedCenter;
    KNHdrViewItem *mActiveItem;
    KPaintInfo mPaintInfo;
    KMime::DateFormatter mDateFormatter;
    KPopupMenu *mPopup;
    bool mShowingFolder;
    bool mInitDone;

  private slots:
    void slotCenterDelayed();
    void slotSizeChanged( int, int, int );
    void resetCurrentTime();

};


class KNHeaderViewToolTip : public TQToolTip {

  public:
    KNHeaderViewToolTip( KNHeaderView *parent );

  protected:
    void maybeTip( const TQPoint &p );

  private:
    KNHeaderView *listView;

};

#endif
