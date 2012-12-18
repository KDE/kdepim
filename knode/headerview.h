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

#include <QMouseEvent>
#include <QKeyEvent>
#include <QEvent>

#include <k3listview.h>
#include <kmime/kmime_dateformatter.h>

class KMenu;
class KNHdrViewItem;

/** Information shared by all items in a list view */
struct KPaintInfo {

  // Popup ids for toggle-able columns
  enum ColumnIds {
    COL_SIZE,
    COL_ATTACHMENT,
    COL_IMPORTANT,
    COL_TOACT,
    COL_SPAM_HAM,
    COL_WATCHED_IGNORED,
    COL_STATUS,
    COL_SIGNED,
    COL_CRYPTO,
    COL_RECEIVER,
    COL_SCORE
  };

  KPaintInfo() :
    pixmapOn( false ),

    showSize( false ),
    showAttachment( false ),
    showImportant( false ),
    showToAct( false ),
    showSpamHam( false ),
    showWatchedIgnored( false ),
    showStatus( false ),
    showSigned( false ),
    showCrypto( false ),
    showReceiver( false ),
    showScore( false ),

    scoreCol( -1 ),
    flagCol( -1 ),
    senderCol( -1 ),
    receiverCol( -1 ),
    subCol( -1 ),
    dateCol( -1 ),
    sizeCol( -1 ),
    attachmentCol( -1 ),
    importantCol( -1 ),
    toActCol( -1 ),
    spamHamCol( -1 ),
    watchedIgnoredCol( -1 ),
    statusCol( -1 ),
    signedCol( -1 ),
    cryptoCol( -1 ),

    orderOfArrival( false ),
    status( false ),
    showCryptoIcons( false ),
    showAttachmentIcon( false )
    {}

  bool pixmapOn;
  QPixmap pixmap;
  QColor colFore;
  QColor colBack;
  QColor colNew;
  QColor colUnread;
  QColor colFlag;
  QColor colToAct;
  QColor colCloseToQuota;

  bool showSize;
  bool showAttachment;
  bool showImportant;
  bool showToAct;
  bool showSpamHam;
  bool showWatchedIgnored;
  bool showStatus;
  bool showSigned;
  bool showCrypto;
  bool showReceiver;
  bool showScore;

  int scoreCol;
  int flagCol;
  int senderCol;
  int receiverCol;
  int subCol;
  int dateCol;
  int sizeCol;
  int attachmentCol;
  int importantCol;
  int toActCol;
  int spamHamCol;
  int watchedIgnoredCol;
  int statusCol;
  int signedCol;
  int cryptoCol;

  bool orderOfArrival;
  bool status;
  bool showCryptoIcons;
  bool showAttachmentIcon;
};

/** Header view, displays the article listing of the currently selected
 *  news group or folder.
 */
class KNHeaderView : public K3ListView  {

  Q_OBJECT

  friend class KNHdrViewItem;

  public:
    explicit KNHeaderView( QWidget *parent );
    ~KNHeaderView();

    void setActive( Q3ListViewItem *item );
    void clear();

    void ensureItemVisibleWithMargin( const Q3ListViewItem *i );

    virtual void setSorting( int column, bool ascending = true );
    bool sortByThreadChangeDate() const      { return mSortByThreadChangeDate; }
    void setSortByThreadChangeDate( bool b ) { mSortByThreadChangeDate = b; }

    bool nextUnreadArticle();
    bool nextUnreadThread();

    void readConfig();
    void writeConfig();

    const KPaintInfo* paintInfo() const { return &mPaintInfo; }

  signals:
    void itemSelected( Q3ListViewItem* );
    void doubleClick( Q3ListViewItem* );
    void sortingChanged( int );

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
    virtual Q3DragObject* dragObject();

  private:
    int mSortCol;
    bool mSortAsc;
    bool mSortByThreadChangeDate;
    int mDelayedCenter;
    KNHdrViewItem *mActiveItem;
    KPaintInfo mPaintInfo;
    KMime::DateFormatter mDateFormatter;
    KMenu *mPopup;
    bool mShowingFolder;
    bool mInitDone;

  private slots:
    void slotCenterDelayed();
    void slotSizeChanged( int, int, int );

};

#if 0
class KNHeaderViewToolTip : public QToolTip {

  public:
    KNHeaderViewToolTip( KNHeaderView *parent );

  protected:
    void maybeTip( const QPoint &p );

  private:
    KNHeaderView *listView;

};
#endif

#endif
