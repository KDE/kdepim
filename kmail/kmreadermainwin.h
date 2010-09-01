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

  /**
   * take ownership of and show @param msg
   *
   * The last two paramters, serNumOfOriginalMessage and nodeIdOffset, are needed when @p msg
   * is derived from another message, e.g. the user views an encapsulated message in this window.
   * Then, the reader needs to know about that original message, so those to paramters are passed
   * onto setOriginalMsg() of KMReaderWin.
   */
  void showMsg( const TQString & encoding, KMMessage *msg,
                unsigned long serNumOfOriginalMessage = 0, int nodeIdOffset = -1 );

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

  /// This closes the window if the setting to close the window after replying or
  /// forwarding is set.
  void slotReplyOrForwardFinished();

private:
  void initKMReaderMainWin();
  void setupAccel();

  /**
   * @see the KMMainWidget function with the same name.
   */
  void setupForwardActions();

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
