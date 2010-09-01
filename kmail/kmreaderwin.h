// -*- mode: C++; c-file-style: "gnu" -*-
// Header for kmreaderwin the kmail reader
// written by Markus Wuebben <markus.wuebben@kde.org>

#ifndef KMREADERWIN_H
#define KMREADERWIN_H

#include <tqwidget.h>
#include <tqtimer.h>
#include <tqstringlist.h>
#include <kurl.h>
#include <kservice.h>
#include "kmmsgbase.h"
#include "kmmimeparttree.h" // Needed for friend declaration.
#include "interfaces/observer.h"

#include <map>

class TQFrame;
class TQSplitter;
class TQHBox;
class TQListViewItem;
class TQScrollBar;
class TQString;
class TQTabDialog;
class TQTextCodec;

class DwHeaders;
class DwMediaType;

class KActionCollection;
class KAction;
class KActionMenu;
class KSelectAction;
class KRadioAction;
class KToggleAction;
class KConfigBase;
class KHTMLPart;
class KURL;

class KMFolder;
class KMMessage;
class KMMessagePart;
namespace KMail {
  namespace Interface {
    class Observable;
    class BodyPartMemento;
  }
  class PartMetaData;
  class ObjectTreeParser;
  class AttachmentStrategy;
  class HeaderStrategy;
  class HeaderStyle;
  class HtmlWriter;
  class KHtmlPartHtmlWriter;
  class ISubject;
  class HtmlStatusBar;
  class FolderJob;
  class CSSHelper;
}

class partNode; // might be removed when KMime is used instead of mimelib
                //                                      (khz, 29.11.2001)

class NewByteArray; // providing operator+ on a TQByteArray (khz, 21.06.2002)

namespace KParts {
  struct URLArgs;
}

/**
   This class implements a "reader window", that is a window
   used for reading or viewing messages.
*/

class KMReaderWin: public TQWidget, public KMail::Interface::Observer {
  Q_OBJECT

  friend void KMMimePartTree::itemClicked( TQListViewItem* item );
  friend void KMMimePartTree::itemRightClicked( TQListViewItem* item, const TQPoint & );
  friend void KMMimePartTree::slotSaveAs();
  friend void KMMimePartTree::startDrag();

  friend class KMail::ObjectTreeParser;
  friend class KMail::KHtmlPartHtmlWriter;

public:
  KMReaderWin( TQWidget *parent,
	       TQWidget *mainWindow,
	       KActionCollection *actionCollection,
               const char *name=0,
	       int f=0 );
  virtual ~KMReaderWin();

  /**
     \reimp from Interface::Observer
     Updates the current message
   */
  void update( KMail::Interface::Observable * );

  /** Read settings from app's config file. */
  void readConfig();

  /** Write settings to app's config file. Calls sync() if withSync is TRUE. */
  void writeConfig( bool withSync=true ) const;

  const KMail::HeaderStyle * headerStyle() const {
    return mHeaderStyle;
  }
  /** Set the header style and strategy. We only want them to be set
      together. */
  void setHeaderStyleAndStrategy( const KMail::HeaderStyle * style,
				  const KMail::HeaderStrategy * strategy );

  /** Getthe message header strategy. */
  const KMail::HeaderStrategy * headerStrategy() const {
    return mHeaderStrategy;
  }

  /** Get/set the message attachment strategy. */
  const KMail::AttachmentStrategy * attachmentStrategy() const {
    return mAttachmentStrategy;
  }
  void setAttachmentStrategy( const KMail::AttachmentStrategy * strategy );

  /** Get selected override character encoding.
      @return The encoding selected by the user or an empty string if auto-detection
      is selected. */
  TQString overrideEncoding() const { return mOverrideEncoding; }

  /** Set the override character encoding. */
  void setOverrideEncoding( const TQString & encoding );

  void setPrintFont( const TQFont& font );

  /** Get codec corresponding to the currently selected override character encoding.
      @return The override codec or 0 if auto-detection is selected. */
  const TQTextCodec * overrideCodec() const;

  /** Set printing mode */
  virtual void setPrinting(bool enable) { mPrinting = enable; }

  /** Set the message that shall be shown. If msg is 0, an empty page is
      displayed. */
  virtual void setMsg( KMMessage* msg, bool force = false, bool updateOnly = false );

  /**
   * This should be called when setting a message that was constructed from another message, which
   * is the case when viewing encapsulated messages in the seperate reader window.
   * We need to know the serial number of the original message, and at which part index the encapsulated
   * message was at that original message, so that deleting and editing attachments can work on the
   * original message.
   *
   * This is a HACK. There really shouldn't be a copy of the original mail.
   *
   * @see slotDeleteAttachment, slotEditAttachment, fillCommandInfo
   */
  void setOriginalMsg( unsigned long serNumOfOriginalMessage, int nodeIdOffset );

  /** Instead of settings a message to be shown sets a message part
      to be shown */
  void setMsgPart( KMMessagePart* aMsgPart, bool aHTML,
		   const TQString& aFileName, const TQString& pname );

  void setMsgPart( partNode * node );

  /** Show or hide the Mime Tree Viewer if configuration
      is set to smart mode.  */
  void showHideMimeTree( bool isPlainTextTopLevel );

  /** Store message id of last viewed message,
      normally no need to call this function directly,
      since correct value is set automatically in
      parseMsg(KMMessage* aMsg, bool onlyProcessHeaders). */
  void setIdOfLastViewedMessage( const TQString & msgId )
    { mIdOfLastViewedMessage = msgId; }

  /** Clear the reader and discard the current message. */
  void clear(bool force = false) { setMsg(0, force); }

  /** Saves the relative position of the scroll view. Call this before calling update()
      if you want to preserve the current view. */
  void saveRelativePosition();

  /** Re-parse the current message. */
  void update(bool force = false);

  /** Print current message. */
  virtual void printMsg(void);

  /** Return selected text */
  TQString copyText();

  /** Get/set auto-delete msg flag. */
  bool autoDelete(void) const { return mAutoDelete; }
  void setAutoDelete(bool f) { mAutoDelete=f; }

  /** Override default html mail setting */
  bool htmlOverride() const { return mHtmlOverride; }
  void setHtmlOverride( bool override );

  /** Override default load external references setting */
  bool htmlLoadExtOverride() const { return mHtmlLoadExtOverride; }
  void setHtmlLoadExtOverride( bool override );

  /** Is html mail to be supported? Takes into account override */
  bool htmlMail();

  /** Is loading ext. references to be supported? Takes into account override */
  bool htmlLoadExternal();

  /** Returns the MD5 hash for the list of new features */
  static TQString newFeaturesMD5();

  /** Display a generic HTML splash page instead of a message */
  void displaySplashPage( const TQString &info );

  /** Display the about page instead of a message */
  void displayAboutPage();

  /** Display the 'please wait' page instead of a message */
  void displayBusyPage();
  /** Display the 'we are currently in offline mode' page instead of a message */
  void displayOfflinePage();

  /** Enable the displaying of messages again after an URL was displayed */
  void enableMsgDisplay();

  /**
   * View message part of type message/RFC822 in extra viewer window.
   * @param msgPart the part to display
   * @param nodeId the part index of the message part that is displayed
   */
  void atmViewMsg( KMMessagePart* msgPart, int nodeId );

  bool atBottom() const;

  bool isFixedFont() { return mUseFixedFont; }
  void setUseFixedFont( bool useFixedFont ) { mUseFixedFont = useFixedFont; }

  /** Return the HtmlWriter connected to the KHTMLPart we use */
  KMail::HtmlWriter * htmlWriter() { return mHtmlWriter; }

  // Action to reply to a message
  // but action( "some_name" ) some name could be used instead.
  KToggleAction *toggleFixFontAction() { return mToggleFixFontAction; }
  KAction *mailToComposeAction() { return mMailToComposeAction; }
  KAction *mailToReplyAction() { return mMailToReplyAction; }
  KAction *mailToForwardAction() { return mMailToForwardAction; }
  KAction *addAddrBookAction() { return mAddAddrBookAction; }
  KAction *openAddrBookAction() { return mOpenAddrBookAction; }
  KAction *copyAction() { return mCopyAction; }
  KAction *selectAllAction() { return mSelectAllAction; }
  KAction *copyURLAction() { return mCopyURLAction; }
  KAction *urlOpenAction() { return mUrlOpenAction; }
  KAction *urlSaveAsAction() { return mUrlSaveAsAction; }
  KAction *addBookmarksAction() { return mAddBookmarksAction;}
  KAction *startImChatAction() { return mStartIMChatAction; }
  // This function returns the complete data that were in this
  // message parts - *after* all encryption has been removed that
  // could be removed.
  // - This is used to store the message in decrypted form.
  void objectTreeToDecryptedMsg( partNode* node,
                                 NewByteArray& resultingData,
                                 KMMessage& theMessage,
                                 bool weAreReplacingTheRootNode = false,
                                 int recCount = 0 );

  /** Returns message part from given URL or null if invalid. */
  partNode* partNodeFromUrl(const KURL &url);

  partNode * partNodeForId( int id );

  KURL tempFileUrlFromPartNode( const partNode * node );

  /** Returns id of message part from given URL or -1 if invalid. */
  static int msgPartFromUrl(const KURL &url);

  void setUpdateAttachment( bool update = true ) { mAtmUpdate = update; }

  /** Access to the KHTMLPart used for the viewer. Use with
      care! */
  KHTMLPart * htmlPart() const { return mViewer; }

  /** Returns the current message or 0 if none. */
  KMMessage* message(KMFolder** folder=0) const;

  void openAttachment( int id, const TQString & name );
  void saveAttachment( const KURL &tempFileName );

  void emitUrlClicked( const KURL & url, int button ) {
    emit urlClicked( url, button );
  }

  void emitPopupMenu( const KURL & url, const TQPoint & p ) {
    if ( message() )
      emit popupMenu( *message(), url, p );
  }

  void showAttachmentPopup( int id, const TQString & name, const TQPoint & p );

  /** Set the serial number of the message this reader window is currently
   *  waiting for. Used to discard updates for already deselected messages. */
  void setWaitingForSerNum( unsigned long serNum ) { mWaitingForSerNum = serNum; }

  TQWidget* mainWindow() { return mMainWindow; }

  /** Returns wether the message should be decryted. */
  bool decryptMessage() const;

  /** Enforce message decryption. */
  void setDecryptMessageOverwrite( bool overwrite = true ) { mDecrytMessageOverwrite = overwrite; }

  /** Show signature details. */
  bool showSignatureDetails() const { return mShowSignatureDetails; }

  /** Show signature details. */
  void setShowSignatureDetails( bool showDetails = true ) { mShowSignatureDetails = showDetails; }

  /* show or hide the list that points to the attachments */
  bool showAttachmentQuicklist() const { return mShowAttachmentQuicklist; }

  /* show or hide the list that points to the attachments */
  void setShowAttachmentQuicklist( bool showAttachmentQuicklist = true ) { mShowAttachmentQuicklist = showAttachmentQuicklist; }

  // This controls whether a Toltec invitation is shown in its raw form or as a replacement text.
  // This can be toggled with the "kmail:showRawToltecMail" link.
  bool showRawToltecMail() const { return mShowRawToltecMail; }
  void setShowRawToltecMail( bool showRawToltecMail ) { mShowRawToltecMail = showRawToltecMail; }

  /* retrieve BodyPartMemento of id \a which for partNode \a node */
  KMail::Interface::BodyPartMemento * bodyPartMemento( const partNode * node, const TQCString & which ) const;

  /* set/replace BodyPartMemento \a memento of id \a which for
     partNode \a node. If there was a BodyPartMemento registered
     already, replaces (deletes) that one. */
  void setBodyPartMemento( const partNode * node, const TQCString & which, KMail::Interface::BodyPartMemento * memento );

  /// Scrolls to the given attachment and marks it with a yellow border
  void scrollToAttachment( const partNode *node );

private:
  /* deletes all BodyPartMementos. Use this when skipping to another
     message (as opposed to re-loading the same one again). */
  void clearBodyPartMementos();

signals:
  /** Emitted after parsing of a message to have it stored
      in unencrypted state in it's folder. */
  void replaceMsgByUnencryptedVersion();

  /** The user presses the right mouse button. 'url' may be 0. */
  void popupMenu(KMMessage &msg, const KURL &url, const TQPoint& mousePos);

  /** The user has clicked onto an URL that is no attachment. */
  void urlClicked(const KURL &url, int button);

  /** Pgp displays a password dialog */
  void noDrag(void);

public slots:

  /** Select message body. */
  void selectAll();

  /** Force update even if message is the same */
  void clearCache();

  /** Refresh the reader window */
  void updateReaderWin();

  /** HTML Widget scrollbar and layout handling. */
  void slotScrollUp();
  void slotScrollDown();
  void slotScrollPrior();
  void slotScrollNext();
  void slotJumpDown();
  void slotDocumentChanged();
  void slotDocumentDone();
  void slotTextSelected(bool);

  /** An URL has been activate with a click. */
  void slotUrlOpen(const KURL &url, const KParts::URLArgs &args);

  /** The mouse has moved on or off an URL. */
  void slotUrlOn(const TQString &url);

  /** The user presses the right mouse button on an URL. */
  void slotUrlPopup(const TQString &, const TQPoint& mousePos);

  /** The user selected "Find" from the menu. */
  void slotFind();
  /** The user selected "Find Next" from the menu. */
  void slotFindNext();

  /** The user toggled the "Fixed Font" flag from the view menu. */
  void slotToggleFixedFont();

  /** Copy the selected text to the clipboard */
  void slotCopySelectedText();

   void slotUrlClicked();

  /** Operations on mailto: URLs. */
  void slotMailtoReply();
  void slotMailtoCompose();
  void slotMailtoForward();
  void slotMailtoAddAddrBook();
  void slotMailtoOpenAddrBook();
  /** Copy URL in mUrlCurrent to clipboard. Removes "mailto:" at
      beginning of URL before copying. */
  void slotUrlCopy();
  void slotUrlOpen( const KURL &url = KURL() );
  /** Save the page to a file */
  void slotUrlSave();
  void slotAddBookmarks();
  void slotSaveMsg();
  void slotSaveAttachments();

  void slotMessageArrived( KMMessage *msg );
  /** start IM Chat with addressee */
  void slotIMChat();
  void contactStatusChanged( const TQString &uid);

  void slotLevelQuote( int l );
  void slotTouchMessage();

  /**
   * Find the node ID and the message of the attachment that should be edited or deleted.
   * This is used when setOriginalMsg() was called before, in that case we want to operate
   * on the original message instead of our copy.
   *
   * @see setOriginalMsg
   */
  void fillCommandInfo( partNode *node, KMMessage **msg, int *nodeId );

  void slotDeleteAttachment( partNode* node );
  void slotEditAttachment( partNode* node );

  KMail::CSSHelper* cssHelper();

protected slots:
  void slotCycleHeaderStyles();
  void slotBriefHeaders();
  void slotFancyHeaders();
  void slotEnterpriseHeaders();
  void slotStandardHeaders();
  void slotLongHeaders();
  void slotAllHeaders();

  void slotCycleAttachmentStrategy();
  void slotIconicAttachments();
  void slotSmartAttachments();
  void slotInlineAttachments();
  void slotHideAttachments();
  void slotHeaderOnlyAttachments();

  /** Some attachment operations. */
  void slotAtmView( int id, const TQString& name );
  void slotDelayedResize();
  void slotHandleAttachment( int );

  /** Helper functions used to change message selection in the message list after deleting
   *  an attachment, see slotDeleteAttachment()
   */
  void disconnectMsgAdded();
  void msgAdded( TQListViewItem *item );

protected:
  /** reimplemented in order to update the frame width in case of a changed
      GUI style */
  void styleChange( TQStyle& oldStyle );

  /** Set the width of the frame to a reasonable value for the current GUI
      style */
  void setStyleDependantFrameWidth();

  /** Watch for palette changes */
  virtual bool event(TQEvent *e);

  /** Calculate the pixel size */
  int pointsToPixel(int pointSize) const;

  /** Feeds the HTML viewer with the contents of the given message.
    HTML begin/end parts are written around the message. */
  void displayMessage();

  /** Parse given message and add it's contents to the reader window. */
  virtual void parseMsg( KMMessage* msg  );

  /** Creates a nice mail header depending on the current selected
    header style. */
  TQString writeMsgHeader(KMMessage* aMsg, partNode *vCardNode = 0, bool topLevel=false );

  /** Writes the given message part to a temporary file and returns the
      name of this file or TQString::null if writing failed.
  */
  TQString writeMessagePartToTempFile( KMMessagePart* msgPart, int partNumber );

  /**
    Creates a temporary dir for saving attachments, etc.
    Will be automatically deleted when another message is viewed.
    @param param Optional part of the directory name.
  */
  TQString createTempDir( const TQString &param = TQString() );

  /** show window containing infos about a vCard. */
  void showVCard(KMMessagePart *msgPart);

  /** HTML initialization. */
  virtual void initHtmlWidget(void);

  /** Some necessary event handling. */
  virtual void closeEvent(TQCloseEvent *);
  virtual void resizeEvent(TQResizeEvent *);

  /** Cleanup the attachment temp files */
  virtual void removeTempFiles();

  /** Event filter */
  bool eventFilter( TQObject *obj, TQEvent *ev );

private slots:
  void slotSetEncoding();
  void injectAttachments();

private:
  void adjustLayout();
  void createWidgets();
  void createActions( KActionCollection * ac );
  void saveSplitterSizes( KConfigBase & c ) const;

  KRadioAction * actionForHeaderStyle( const KMail::HeaderStyle *,
                                       const KMail::HeaderStrategy * );
  KRadioAction * actionForAttachmentStrategy( const KMail::AttachmentStrategy * );
  /** Read override codec from configuration */
  void readGlobalOverrideCodec();

  TQString renderAttachments( partNode *node, const TQColor &bgColor );

private:
  bool mHtmlMail, mHtmlLoadExternal, mHtmlOverride, mHtmlLoadExtOverride;
  int mAtmCurrent;
  TQString mAtmCurrentName;
  KMMessage *mMessage;

  // See setOriginalMsg() for an explaination for those two.
  unsigned long mSerNumOfOriginalMessage;
  int mNodeIdOffset;

  // widgets:
  TQSplitter * mSplitter;
  TQHBox *mBox;
  KMail::HtmlStatusBar *mColorBar;
  KMMimePartTree* mMimePartTree;
  KHTMLPart *mViewer;

  const KMail::AttachmentStrategy * mAttachmentStrategy;
  const KMail::HeaderStrategy * mHeaderStrategy;
  const KMail::HeaderStyle * mHeaderStyle;
  bool mAutoDelete;
  /** where did the user save the attachment last time */
  TQString mSaveAttachDir;
  static const int delay;
  TQTimer mUpdateReaderWinTimer;
  TQTimer mResizeTimer;
  TQTimer mDelayedMarkTimer;
  TQString mOverrideEncoding;
  TQString mOldGlobalOverrideEncoding; // used to detect changes of the global override character encoding
  bool mMsgDisplay;
  bool mNoMDNsWhenEncrypted;
  unsigned long mLastSerNum;

  KMail::CSSHelper * mCSSHelper;
  bool mUseFixedFont;
  bool mPrinting;
  bool mShowColorbar;
  //bool mShowCompleteMessage;
  TQStringList mTempFiles;
  TQStringList mTempDirs;
  int mMimeTreeMode;
  bool mMimeTreeAtBottom;
  TQValueList<int> mSplitterSizes;
  partNode* mRootNode;
  TQString mIdOfLastViewedMessage;
  TQWidget *mMainWindow;
  KActionCollection *mActionCollection;
  KAction *mMailToComposeAction, *mMailToReplyAction, *mMailToForwardAction,
      *mAddAddrBookAction, *mOpenAddrBookAction, *mCopyAction, *mCopyURLAction,
      *mUrlOpenAction, *mUrlSaveAsAction, *mAddBookmarksAction, *mStartIMChatAction, *mSelectAllAction;
  KToggleAction *mHeaderOnlyAttachmentsAction;
  KSelectAction *mSelectEncodingAction;
  KToggleAction *mToggleFixFontAction;

  KURL mHoveredUrl;
  KURL mClickedUrl;
  TQPoint mLastClickPosition;
  TQString mLastClickImagePath;
  bool mCanStartDrag;

  KMail::HtmlWriter * mHtmlWriter;
  std::map<TQCString,KMail::Interface::BodyPartMemento*> mBodyPartMementoMap;
  // an attachment should be updated
  bool mAtmUpdate;
  int mChoice;
  unsigned long mWaitingForSerNum;
  float mSavedRelativePosition;
  int mLevelQuote;
  bool mDecrytMessageOverwrite;
  bool mShowSignatureDetails;
  bool mShowAttachmentQuicklist;
  bool mShowRawToltecMail;
  bool mExternalWindow;
};


#endif

