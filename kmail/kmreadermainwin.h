// -*- mode: C++; c-file-style: "gnu" -*-

#ifndef KMReaderMainWin_h
#define KMReaderMainWin_h

#include "secondarywindow.h"

#include <kurl.h>

class KMReaderWin;
class KMMessage;
class KMMessagePart;
class KAction;
class KActionMenu;
class KMFolderIndex;
class KMFolder;
class KFontAction;
class KFontSizeAction;
template <typename T, typename S> class TQMap;

namespace KMail {
class MessageActions;
}

class KMReaderMainWin : public KMail::SecondaryWindow
{
  Q_OBJECT

public:
  KMReaderMainWin( bool htmlOverride, bool htmlLoadExtOverride, char *name = 0 );
  KMReaderMainWin( char *name = 0 );
  KMReaderMainWin(KMMessagePart* aMsgPart,
    bool aHTML, const TQString& aFileName, const TQString& pname,
    const TQString & encoding, char *name = 0 );
  virtual ~KMReaderMainWin();

  void setUseFixedFont( bool useFixedFont );

  // take ownership of and show @param msg
  void showMsg( const TQString & encoding, KMMessage *msg );

  /**
   * Sets up action list for forward menu.
  */
  void setupForwardingActionsList();

private slots:
  void slotMsgPopup(KMMessage &aMsg, const KURL &aUrl, const TQPoint& aPoint);

  /** Copy selected messages to folder with corresponding to given menuid */
  void copySelectedToFolder( int menuId );
  void slotTrashMsg();
  void slotPrintMsg();
  void slotForwardInlineMsg();
  void slotForwardAttachedMsg();
  void slotForwardDigestMsg();
  void slotRedirectMsg();
  void slotShowMsgSrc();
  void slotMarkAll();
  void slotCopy();
  void slotFind();
  void slotFindNext();
  void slotFontAction(const TQString &);
  void slotSizeAction(int);
  void slotCreateTodo();
  void slotEditToolbars();

  void slotConfigChanged();
  void slotUpdateToolbars();

  void slotFolderRemoved( TQObject* folderPtr );

private:
  void initKMReaderMainWin();
  void setupAccel();

  KMReaderWin *mReaderWin;
  KMMessage *mMsg;
  KURL mUrl;
  TQMap<int,KMFolder*> mMenuToFolder;
  // a few actions duplicated from kmmainwidget
  KAction *mTrashAction, *mPrintAction, *mSaveAsAction, *mForwardInlineAction,
          *mForwardAttachedAction, *mForwardDigestAction, *mRedirectAction,
          *mViewSourceAction;
  KActionMenu *mForwardActionMenu;
  KFontAction *fontAction;
  KFontSizeAction *fontSizeAction;
  KMail::MessageActions *mMsgActions;

};

#endif /*KMReaderMainWin_h*/
