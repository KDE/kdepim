/* kmfldsearch
 * (c) 1999 Stefan Taferner, (c) 2001 Aaron J. Seigo
 * This code is under GPL
 */
#ifndef kmfldsearch_h
#define kmfldsearch_h

#include <qvaluelist.h>
#include <qptrlist.h>
#include <qstringlist.h>
#include <qguardedptr.h>

#include <kdialogbase.h>
#include <kxmlguiclient.h>
#include <mimelib/string.h>

class QCheckBox;
class QComboBox;
class QGridLayout;
class QLabel;
class QLineEdit;
class KListView;
class QListViewItem;
class QPushButton;
class QRadioButton;
class KAction;
class KActionMenu;
class KMFolder;
class KMFolderSearch;
class KMFolderComboBox;
class KMFolderImap;
class KMFolderMgr;
class KMMainWidget;
class KMMessage;
class KMSearchPattern;
class KMSearchPatternEdit;
class KStatusBar;
class DwBoyerMoore;

typedef QPtrList<KMMsgBase> KMMessageList;

class KMFldSearch: public KDialogBase, virtual public KXMLGUIClient
{
  Q_OBJECT

public:
  KMFldSearch(KMMainWidget* parent, const char* name=0,
              KMFolder *curFolder=0, bool modal=FALSE);
  virtual ~KMFldSearch();

  void activateFolder(KMFolder* curFolder);
  KMMessageList selectedMessages();
  KMMessage* message();

protected slots:
  /** Update status line widget. */
  virtual void updStatus(void);

  virtual void slotClose();
  virtual void slotSearch();
  virtual void slotStop();
  void updateCreateButton( const QString &);
  void renameSearchFolder();
  virtual bool slotShowMsg(QListViewItem *);
  virtual void updateContextMenuActions();
  virtual void slotContextMenuRequested( QListViewItem*, const QPoint &, int );
  virtual void copySelectedToFolder( int menuId );
  virtual void moveSelectedToFolder( int menuId );
  virtual void slotFolderActivated(int nr);
  void slotClearSelection();
  void slotReplyToMsg();
  void slotReplyAllToMsg();
  void slotReplyListToMsg();
  void slotForwardMsg();
  void slotForwardAttachedMsg();
  void slotSaveMsg();
  void slotPrintMsg();

  /** GUI cleanup after search */
  virtual void searchDone();
  virtual void slotAddMsg(int idx);
  virtual void slotRemoveMsg(int idx);

protected:
  void enableGUI();

  /** Return the KMMessage corresponding to the selected listviewitem */
  KMMessage* getSelectedMsg();

  /** Reimplemented to react to Escape. */
  virtual void keyPressEvent(QKeyEvent*);

  /** Reimplemented to stop searching when the window is closed */
  virtual void closeEvent(QCloseEvent*);

protected:
  bool mStopped;
  bool mCloseRequested;
  int mFetchingInProgress;
  QGuardedPtr<KMFolderSearch> mFolder;
  QTimer *mTimer;

  // GC'd by Qt
  QRadioButton *mChkbxAllFolders;
  QRadioButton *mChkbxSpecificFolders;
  KMFolderComboBox *mCbxFolders;
  QPushButton *mBtnSearch;
  QPushButton *mBtnStop;
  QCheckBox *mChkSubFolders;
  KListView* mLbxMatches;
  QLabel *mSearchFolderLbl;
  QLineEdit *mSearchFolderEdt;
  QPushButton *mSearchFolderBtn;
  KStatusBar* mStatusBar;
  QWidget* mLastFocus; // to remember the position of the focus
  QMap<int,KMFolder*> mMenuToFolder;
  KAction *mReplyAction, *mReplyAllAction, *mReplyListAction, *mSaveAsAction,
      *mForwardAction, *mForwardAttachedAction, *mPrintAction, *mClearAction;
  KActionMenu *mForwardActionMenu;
  QValueList<QGuardedPtr<KMFolder> > mFolders;

  // not owned by us
  KMMainWidget* mKMMainWidget;
  KMSearchPatternEdit *mPatternEdit;
  KMSearchPattern *mSearchPattern;

  static const int MSGID_COLUMN;
};
#endif /*kmfldsearch_h*/
