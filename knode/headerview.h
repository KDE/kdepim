/*
    KNode, the KDE newsreader
    Copyright (c) 1999-2004 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#ifndef KNHEADERVIEW_H
#define KNHEADERVIEW_H

#include <qtooltip.h>

#include <klistview.h>
#include <kfoldertree.h>
#include <kmime_util.h>

class KPopupMenu;

class KNHeaderView : public KListView  {

  Q_OBJECT

  friend class KNHdrViewItem;

  public:
    KNHeaderView( QWidget *parent, const char *name = 0 );
    ~KNHeaderView();

    void setActive( QListViewItem *item );
    void clear();

    void ensureItemVisibleWithMargin( const QListViewItem *i );

    virtual void setSorting( int column, bool ascending = true );
    bool sortByThreadChangeDate() const      { return mSortByThreadChangeDate; }
    void setSortByThreadChangeDate( bool b ) { mSortByThreadChangeDate = b; }

    bool nextUnreadArticle();
    bool nextUnreadThread();

    void readConfig();
    void writeConfig();

    const KPaintInfo* paintInfo() const { return &mPaintInfo; }

  signals:
    void itemSelected( QListViewItem* );
    void doubleClick( QListViewItem* );
    void sortingChanged( int );
    void focusChanged( QFocusEvent* );
    void focusChangeRequest( QWidget* );

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
    virtual bool event( QEvent *e );
    void contentsMousePressEvent( QMouseEvent *e );
    void contentsMouseDoubleClickEvent( QMouseEvent *e );
    void keyPressEvent( QKeyEvent *e );
    bool eventFilter( QObject *, QEvent * );
    void focusInEvent( QFocusEvent *e );
    void focusOutEvent( QFocusEvent *e );
    virtual QDragObject* dragObject();

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

  private slots:
    void slotCenterDelayed();
    void slotSizeChanged( int, int, int );
    void resetCurrentTime();

};


class KNHeaderViewToolTip : public QToolTip {

  public:
    KNHeaderViewToolTip( KNHeaderView *parent );

  protected:
    void maybeTip( const QPoint &p );

  private:
    KNHeaderView *listView;

};

#endif
