/*
 * kmail: KDE mail client
 * Copyright (c) 1996-1998 Stefan Taferner <taferner@kde.org>
 * Copyright (c) 2001 Aaron J. Seigo <aseigo@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef searchwindow_h
#define searchwindow_h

#include <tqvaluelist.h>
#include <tqptrlist.h>
#include <tqstringlist.h>
#include <tqguardedptr.h>
#include <tqtimer.h>

#include <kdialogbase.h>
#include <kxmlguiclient.h>

class TQCheckBox;
class TQComboBox;
class TQGridLayout;
class TQLabel;
class TQLineEdit;
class KListView;
class TQListViewItem;
class TQPushButton;
class TQRadioButton;
class KAction;
class KActionMenu;
class KMFolder;
class KMFolderSearch;
class KMFolderImap;
class KMFolderMgr;
class KMMainWidget;
class KMMessage;
class KMSearchPattern;
class KMSearchPatternEdit;
class KStatusBar;
class DwBoyerMoore;
namespace KMail {
  class FolderRequester;
}

typedef TQPtrList<KMMsgBase> KMMessageList;

namespace KMail {

  /**
   * The SearchWindow class provides a dialog for triggering a search on
   * folders and storing that search as a search folder. It shows the search
   * results in a listview and allows triggering of operations such as printing
   * or moving on them.
   */
class SearchWindow: public KDialogBase, virtual public KXMLGUIClient
{
  Q_OBJECT

public:
  /**
   * Creates a new search window.
   * @param parent The parent widget.
   * @param name The (widget) name of the dialog.
   * @param curFolder The folder which will be pre-selected as the base folder
   * of search operations.
   * @param modal Whether the dialog is to be shown modal.
   */
  SearchWindow( KMMainWidget* parent, const char* name=0,
                       KMFolder *curFolder=0, bool modal=false );
  virtual ~SearchWindow();

  /**
   * Changes the base folder for search operations to a different folder.
   * @param curFolder The folder to use as the new base for searches.
   */
  void activateFolder( KMFolder* curFolder );

  /**
   * Provides access to the list of currently selected message in the listview.
   * @return The list of currenty selected search result messages.
   */
  KMMessageList selectedMessages();

  /**
   * Provides access to the currently selected message.
   * @return the currently selected message.
   */
  KMMessage* message();

  void setSearchPattern( const KMSearchPattern& pattern );

protected slots:
  /** Update status line widget. */
  virtual void updStatus(void);

  virtual void slotClose();
  virtual void slotSearch();
  virtual void slotStop();
  void scheduleRename( const TQString &);
  void renameSearchFolder();
  void openSearchFolder();
  void folderInvalidated(KMFolder *);
  virtual bool slotShowMsg( TQListViewItem * );
  void slotViewSelectedMsg();
  virtual bool slotViewMsg( TQListViewItem * );
  void slotCurrentChanged( TQListViewItem * );
  virtual void updateContextMenuActions();
  virtual void slotContextMenuRequested( TQListViewItem*, const TQPoint &, int );
  virtual void copySelectedToFolder( int menuId );
  virtual void moveSelectedToFolder( int menuId );
  virtual void slotFolderActivated();
  void slotClearSelection();
  void slotReplyToMsg();
  void slotReplyAllToMsg();
  void slotReplyListToMsg();
  void slotForwardInlineMsg();
  void slotForwardAttachedMsg();
  void slotForwardDigestMsg();
  void slotRedirectMsg();
  void slotSaveMsg();
  void slotSaveAttachments();
  void slotPrintMsg();
  void slotCopyMsgs();
  void slotCutMsgs();

  /** GUI cleanup after search */
  virtual void searchDone();
  virtual void slotAddMsg(int idx);
  virtual void slotRemoveMsg(KMFolder *, Q_UINT32 serNum);
  void enableGUI();

  void setEnabledSearchButton(bool);

protected:

  /** Reimplemented to react to Escape. */
  virtual void keyPressEvent(TQKeyEvent*);

  /** Reimplemented to stop searching when the window is closed */
  virtual void closeEvent(TQCloseEvent*);

protected:
  bool mStopped;
  bool mCloseRequested;
  int mFetchingInProgress;
  int mSortColumn;
  SortOrder mSortOrder;
  TQGuardedPtr<KMFolderSearch> mFolder;
  TQTimer *mTimer;

  // GC'd by Qt
  TQRadioButton *mChkbxAllFolders;
  TQRadioButton *mChkbxSpecificFolders;
  KMail::FolderRequester *mCbxFolders;
  TQPushButton *mBtnSearch;
  TQPushButton *mBtnStop;
  TQCheckBox *mChkSubFolders;
  KListView* mLbxMatches;
  TQLabel *mSearchFolderLbl;
  TQLineEdit *mSearchFolderEdt;
  TQPushButton *mSearchFolderOpenBtn;
  TQPushButton *mSearchResultOpenBtn;
  KStatusBar* mStatusBar;
  TQWidget* mLastFocus; // to remember the position of the focus
  TQMap<int,KMFolder*> mMenuToFolder;
  KAction *mReplyAction, *mReplyAllAction, *mReplyListAction, *mSaveAsAction,
    *mForwardInlineAction, *mForwardAttachedAction, *mForwardDigestAction,
    *mRedirectAction, *mPrintAction, *mClearAction, *mSaveAtchAction,
    *mCopyAction, *mCutAction;
  KActionMenu *mForwardActionMenu;
  TQValueList<TQGuardedPtr<KMFolder> > mFolders;
  TQTimer mRenameTimer;

  // not owned by us
  KMMainWidget* mKMMainWidget;
  KMSearchPatternEdit *mPatternEdit;
  KMSearchPattern *mSearchPattern;

  static const int MSGID_COLUMN;

private:
  KMMessage *indexToMessage( TQListViewItem *item );
};

} // namespace KMail
#endif /*searchwindow_h*/
