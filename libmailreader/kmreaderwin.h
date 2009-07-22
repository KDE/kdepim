/* -*- mode: C++; c-file-style: "gnu" -*-
  This file is part of KMail, the KDE mail client.
  Copyright (c) 1997 Markus Wuebben <markus.wuebben@kde.org>

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
*/

#ifndef KMREADERWIN_H
#define KMREADERWIN_H

#include "mailviewer_export.h"

#include <QWidget>
#include <QTimer>
#include <QStringList>
#include <QCloseEvent>
#include <QEvent>
#include <QList>
#include <QResizeEvent>
#include <kurl.h>
#include <kservice.h>
#include "libkdepim/messagestatus.h"
#include <kvbox.h>
using KPIM::MessageStatus;

//Akonadi includes
#include <akonadi/item.h>

//TODO(Andras) Just a note so I won't forget: use MailViewer as namespace and library name instead of KMail/MailReader before moving back to trunk

class QSplitter;
class KHBox;
class QTreeWidgetItem;
class QString;
class QTextCodec;
class QTreeView;

class KActionCollection;
class KAction;
class KSelectAction;
class KToggleAction;
class KToggleAction;
class KHTMLPart;
class KUrl;
class KConfigSkeleton;

namespace MailViewer {
 class MimeTreeModel;
 class ConfigureWidget;
}


namespace KMime {
    class Message;
    class Content;
}
typedef boost::shared_ptr<KMime::Message> MessagePtr;

namespace KMail {
  class ObjectTreeParser;
  class AttachmentStrategy;
  class HeaderStrategy;
  class HeaderStyle;
  class HtmlWriter;
  class KHtmlPartHtmlWriter;
  class HtmlStatusBar;
  class CSSHelper;
}

namespace KParts {
  struct BrowserArguments;
  class OpenUrlArguments;
}

/**
   This class implements a "reader window", that is a window
   used for reading or viewing messages.
*/

//TODO(Andras) once only those methods are public that really need to be public, probably export the whole class instead of just some methods
class KMReaderWin: public QWidget {
  Q_OBJECT
  friend class KMail::ObjectTreeParser;
  friend class KMail::KHtmlPartHtmlWriter;

public:
  /**
   * Create a mail viewer widget
   * @param config a config object from where the configuration is read
   * @param parent parent widget
   * @param mainWindow the application's main window
   * @param actionCollection the action collection where the widget's actions will belong to
   * @param f window flags
   */
  MAILVIEWER_EXPORT KMReaderWin( QWidget *parent,  KSharedConfigPtr config = KSharedConfigPtr(), QWidget *mainWindow = 0,
               KActionCollection *actionCollection = 0, Qt::WindowFlags f = 0 );
  virtual ~KMReaderWin();

  /**
   * The current message displayed in the viewer.
   * @return
   */
  MAILVIEWER_EXPORT KMime::Message *message() const { return mMessage;}

   /** Get codec corresponding to the currently selected override character encoding.
      @return The override codec or 0 if auto-detection is selected. */
  MAILVIEWER_EXPORT const QTextCodec * overrideCodec() const;

  enum UpdateMode {
    Force = 0,
    Delayed
  };

  enum Ownership {
    Transfer= 0,
    Keep
  };

  /** Set the message that shall be shown. If msg is 0, an empty page is
      displayed. */
  MAILVIEWER_EXPORT void setMessage(KMime::Message* msg, UpdateMode updateMode = Delayed, Ownership = Keep);

  MAILVIEWER_EXPORT void setMessageItem(const Akonadi::Item& item, UpdateMode updateMode = Delayed );

  /** Clear the reader and discard the current message. */
  MAILVIEWER_EXPORT void clear(UpdateMode updateMode = Delayed ) { setMessage(0, updateMode); }

  /** Saves the relative position of the scroll view. Call this before calling update()
      if you want to preserve the current view. */
  MAILVIEWER_EXPORT void saveRelativePosition();

  /** Re-parse the current message. */
  MAILVIEWER_EXPORT void update(UpdateMode updateMode = Delayed);

  /** Print message. */
  MAILVIEWER_EXPORT void printMessage(  KMime::Message* aMsg );

  /** Return selected text */
  MAILVIEWER_EXPORT QString copyText();

  /** Override default html mail setting */
  MAILVIEWER_EXPORT bool htmlOverride() const { return mHtmlOverride; }
  MAILVIEWER_EXPORT void setHtmlOverride( bool override );

  /** Override default load external references setting */
  MAILVIEWER_EXPORT bool htmlLoadExtOverride() const { return mHtmlLoadExtOverride; }
  MAILVIEWER_EXPORT void setHtmlLoadExtOverride( bool override );

  /** Display a generic HTML splash page instead of a message */
  MAILVIEWER_EXPORT void displaySplashPage( const QString &info );

  /** Enable the displaying of messages again after an URL was displayed */
  MAILVIEWER_EXPORT void enableMessageDisplay();

  MAILVIEWER_EXPORT bool atBottom() const;

  MAILVIEWER_EXPORT bool isFixedFont() { return mUseFixedFont; }
  MAILVIEWER_EXPORT void setUseFixedFont( bool useFixedFont ) { mUseFixedFont = useFixedFont; }

  // Action to reply to a message
  // but action( "some_name" ) some name could be used instead.
  KToggleAction *toggleFixFontAction() { return mToggleFixFontAction; }
  KAction *copyAction() { return mCopyAction; }
  KAction *selectAllAction() { return mSelectAllAction; }
  KAction *copyURLAction() { return mCopyURLAction; }
  KAction *urlOpenAction() { return mUrlOpenAction; }
  // This function returns the complete data that were in this
  // message parts - *after* all encryption has been removed that
  // could be removed.
  // - This is used to store the message in decrypted form.
  void objectTreeToDecryptedMsg( KMime::Content* node,
                                 QByteArray& resultingData,
                                 KMime::Message& theMessage,
                                 bool weAreReplacingTheRootNode = false,
                                 int recCount = 0 );

  /** Returns message part from given URL or null if invalid. */
  KMime::Content* partNodeFromUrl(const KUrl &url);

  KMime::Content * partNodeForId( int id );

  /** Returns id of message part from given URL or -1 if invalid. */
  static int msgPartFromUrl(const KUrl &url);

  /** Access to the KHTMLPart used for the viewer. Use with
      care! */
  MAILVIEWER_EXPORT KHTMLPart * htmlPart() const { return mViewer; }

  MAILVIEWER_EXPORT void openAttachment( int id, const QString & name );

  MAILVIEWER_EXPORT void emitUrlClicked( const KUrl & url, int button ) {
    emit urlClicked( url, button );
  }

  void emitPopupMenu( const KUrl & url, const QPoint & p ) {
    if ( message() )
      emit popupMenu( *message(), url, p );
  }

  /**
   * Sets the current attachment ID and the current attachment temporary filename
   * to the given values.
   * Call this so that slotHandleAttachment() knows which attachment to handle.
   */
  void prepareHandleAttachment( int id, const QString& fileName );

  void showAttachmentPopup( int id, const QString & name, const QPoint & p );

  /** Set the serial number of the message this reader window is currently
   *  waiting for. Used to discard updates for already deselected messages. */
  void setWaitingForSerNum( unsigned long serNum ) { mWaitingForSerNum = serNum; }

  QWidget* mainWindow() { return mMainWindow; }

  /** Enforce message decryption. */
  MAILVIEWER_EXPORT void setDecryptMessageOverwrite( bool overwrite = true ) { mDecrytMessageOverwrite = overwrite; }

  /** Show signature details. */
  MAILVIEWER_EXPORT bool showSignatureDetails() const { return mShowSignatureDetails; }

  /** Show signature details. */
  MAILVIEWER_EXPORT void setShowSignatureDetails( bool showDetails = true ) { mShowSignatureDetails = showDetails; }

  /* show or hide the list that points to the attachments */
  MAILVIEWER_EXPORT bool showAttachmentQuicklist() const { return mShowAttachmentQuicklist; }

  /* show or hide the list that points to the attachments */
  MAILVIEWER_EXPORT void setShowAttachmentQuicklist( bool showAttachmentQuicklist = true ) { mShowAttachmentQuicklist = showAttachmentQuicklist; }

  /**
   * Get an instance for the configuration widget. The caller has the ownership and must delete the widget. See also configObject();
   * The caller should also call the widget's slotSettingsChanged() if the configuration has changed.
   */
  MAILVIEWER_EXPORT QWidget* configWidget();

  /**
   * Returns the configuration object that can be used in a KConfigDialog together with configWidget();
   */
  MAILVIEWER_EXPORT KConfigSkeleton *configObject();


signals:
  /** Emitted after parsing of a message to have it stored
      in unencrypted state in it's folder. */
  void replaceMsgByUnencryptedVersion();

  /** The user presses the right mouse button. 'url' may be 0. */
  void popupMenu(KMime::Message &msg, const KUrl &url, const QPoint& mousePos);

  /** The user has clicked onto an URL that is no attachment. */
  void urlClicked(const KUrl &url, int button);

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
  void slotUrlOpen(const KUrl &url, const KParts::OpenUrlArguments &, const KParts::BrowserArguments &);

  /** The mouse has moved on or off an URL. */
  void slotUrlOn(const QString &url);

  /** The user presses the right mouse button on an URL. */
  void slotUrlPopup(const QString &, const QPoint& mousePos);

  /** The user selected "Find" from the menu. */
  void slotFind();

  /** The user toggled the "Fixed Font" flag from the view menu. */
  void slotToggleFixedFont();

  /** Show the message source */
  void slotShowMsgSrc();

  /** Copy the selected text to the clipboard */
  void slotCopySelectedText();

   void slotUrlClicked();

  /** Copy URL in mUrlCurrent to clipboard. Removes "mailto:" at
      beginning of URL before copying. */
  void slotUrlCopy();
  void slotUrlOpen( const KUrl &url = KUrl() );
  /** Save the page to a file */
  void slotUrlSave();
  void slotSaveMsg();
  void slotSaveAttachments();

  void slotMessageArrived( KMime::Message *msg );

  void slotLevelQuote( int l );
  void slotTouchMessage();

  void slotDeleteAttachment( KMime::Content* node );
  void slotEditAttachment( KMime::Content* node );

  /**
   * Does an action for the current attachment.
   * The action is defined by the KMHandleAttachmentCommand::AttachmentAction
   * enum.
   * prepareHandleAttachment() needs to be called before calling this to set the
   * correct attachment ID.
   */
  void slotHandleAttachment( int action );

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

  /** Some attachment operations. */
  void slotAtmView( int id, const QString& name );
  void slotDelayedResize();

  /** Print message. Called on as a response of finished() signal of mPartHtmlWriter
      after rendering is finished.
      In the very end it deletes the KMReaderWin window that was created
      for the purpose of rendering. */
  void slotPrintMsg();

protected:
//TODO(Andras) These were public, but made protected as only ObjectTreeParser uses them. Make public if needed
  /** Is html mail to be supported? Takes into account override */
  bool htmlMail();

  /** Is loading ext. references to be supported? Takes into account override */
  bool htmlLoadExternal();

  /** Return the HtmlWriter connected to the KHTMLPart we use */
  KMail::HtmlWriter * htmlWriter() { return mHtmlWriter; }

  /** Returns whether the message should be decryted. */
  bool decryptMessage() const;

  KMail::CSSHelper* cssHelper() const;
//(Andras) end of moved methods

  /** reimplemented in order to update the frame width in case of a changed
      GUI style */
  void styleChange( QStyle& oldStyle );

  /** Set the width of the frame to a reasonable value for the current GUI
      style */
  void setStyleDependantFrameWidth();

  /** Watch for palette changes */
  virtual bool event(QEvent *e);

  /** Calculate the pixel size */
  int pointsToPixel(int pointSize) const;

  /** Feeds the HTML viewer with the contents of the given message.
    HTML begin/end parts are written around the message. */
  void displayMessage();

  /** Parse the root message and add it's contents to the reader window. */
  void parseMsg();

  /** Creates a nice mail header depending on the current selected
    header style. */
  QString writeMsgHeader( KMime::Message* aMsg, bool hasVCard = false, bool topLevel = false );

  /** Writes the given message part to a temporary file and returns the
      name of this file or QString() if writing failed.
  */
  QString writeMessagePartToTempFile( KMime::Content* msgPart );

  /**
    Creates a temporary dir for saving attachments, etc.
    Will be automatically deleted when another message is viewed.
    @param param Optional part of the directory name.
  */
  QString createTempDir( const QString &param = QString() );

  /** show window containing infos about a vCard. */
  void showVCard(KMime::Content *msgPart);

  /** HTML initialization. */
  virtual void initHtmlWidget(void);

  /** Some necessary event handling. */
  virtual void closeEvent(QCloseEvent *);
  virtual void resizeEvent(QResizeEvent *);

  /** Cleanup the attachment temp files */
  virtual void removeTempFiles();

  /** Event filter */
  bool eventFilter( QObject *obj, QEvent *ev );

private slots:
  void slotSetEncoding();
  void injectAttachments();
  void slotSettingsChanged();

private:

//TODO(Andras) these methods were moved from public to private to keep the public API clean. In case there is a need from them,
//we can reconsider making them public again
  /** Read settings from app's config file. */
  void readConfig();

  /** Write settings to app's config file. Calls sync() if withSync is true. */
  void writeConfig( bool withSync=true ) const;

   /** Get the message header style. */
  const KMail::HeaderStyle * headerStyle() const {
    return mHeaderStyle;
  }

  /** Set the header style and strategy. We only want them to be set
      together. */
  void setHeaderStyleAndStrategy( const KMail::HeaderStyle * style,
                                  const KMail::HeaderStrategy * strategy );

  /** Get the message header strategy. */
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
  QString overrideEncoding() const { return mOverrideEncoding; }

  /** Set the override character encoding. */
  void setOverrideEncoding( const QString & encoding );

  void setPrintFont( const QFont& font );

  /** Set printing mode */
  virtual void setPrinting(bool enable) { mPrinting = enable; }

  /** Instead of settings a message to be shown sets a message part
      to be shown */
  void setMessagePart( KMime::Content* aMsgPart, bool aHTML,
                   const QString& aFileName, const QString& pname );

  void setMessagePart( KMime::Content * node );

  /** Show or hide the Mime Tree Viewer if configuration
      is set to smart mode.  */
  void showHideMimeTree( bool isPlainTextTopLevel );

  /** View message part of type message/RFC822 in extra viewer window. */
  void atmViewMsg(KMime::Content* msgPart);

  KUrl tempFileUrlFromNode( const KMime::Content *node );

//(Andras) end of moved methods

  void adjustLayout();
  void createWidgets();
  void createActions();
  void saveSplitterSizes( KConfigGroup & c ) const;

  KToggleAction * actionForHeaderStyle( const KMail::HeaderStyle *,
                                       const KMail::HeaderStrategy * );
  KToggleAction * actionForAttachmentStrategy( const KMail::AttachmentStrategy * );
  /** Read override codec from configuration */
  void readGlobalOverrideCodec();

  QString renderAttachments( KMime::Content *node, const QColor &bgColor );

  KMime::Content* findContentByType(KMime::Content *content, const QByteArray &type);
    /**
   * Fixes an encoding received by a KDE function and returns the proper,
   * MIME-compilant encoding name instead.
   * @see encodingForName
   */
  static QString fixEncoding( const QString &encoding );

  /**
   * Drop-in replacement for KCharsets::encodingForName(). The problem with
   * the KCharsets function is that it returns "human-readable" encoding names
   * like "ISO 8859-15" instead of valid encoding names like "ISO-8859-15".
   * This function fixes this by replacing whitespace with a hyphen.
   */
  static QString encodingForName( const QString &descriptiveName );

  /** Return a QTextCodec for the specified charset.
   * This function is a bit more tolerant, than QTextCodec::codecForName */
  static const QTextCodec* codecForName(const QByteArray& _str);
  /**
   * Return a list of the supported encodings
   * @param usAscii if true, US-Ascii encoding will be prepended to the list.
   */
  static QStringList supportedEncodings( bool usAscii );

private:
  bool mHtmlMail, mHtmlLoadExternal, mHtmlOverride, mHtmlLoadExtOverride;
  int mAtmCurrent;
  QString mAtmCurrentName;
  KMime::Message *mMessage; //the current message, if it was set manually
  Akonadi::Item mMessageItem; //the message item from Akonadi
  bool mDeleteMessage; //the message was created in the lib, eg. by calling setMessageItem()
  // widgets:
  QSplitter * mSplitter;
  KHBox *mBox;
  KMail::HtmlStatusBar *mColorBar;
  QTreeView* mMimePartTree; //FIXME(Andras) port the functionality from KMMimePartTree to a new view class or to here with signals/slots
  MailViewer::MimeTreeModel *mMimePartModel;
  KHTMLPart *mViewer;

  const KMail::AttachmentStrategy * mAttachmentStrategy;
  const KMail::HeaderStrategy * mHeaderStrategy;
  const KMail::HeaderStyle * mHeaderStyle;
  bool mAutoDelete;
  /** where did the user save the attachment last time */
  QString mSaveAttachDir;
  static const int delay;
  QTimer mUpdateReaderWinTimer;
  QTimer mResizeTimer;
  QTimer mDelayedMarkTimer;
  QString mOverrideEncoding;
  QString mOldGlobalOverrideEncoding; // used to detect changes of the global override character encoding
  bool mMsgDisplay;
  bool mNoMDNsWhenEncrypted;
  unsigned long mLastSerNum;
  MessageStatus mLastStatus;

  KMail::CSSHelper * mCSSHelper;
  bool mUseFixedFont;
  bool mPrinting;
  bool mShowColorbar;
  //bool mShowCompleteMessage;
  QStringList mTempFiles;
  QStringList mTempDirs;
  int mMimeTreeMode;
  bool mMimeTreeAtBottom;
  QList<int> mSplitterSizes;
  QString mIdOfLastViewedMessage;
  QWidget *mMainWindow;
  KActionCollection *mActionCollection;
  KAction *mCopyAction, *mCopyURLAction,
      *mUrlOpenAction, *mSelectAllAction,
      *mScrollUpAction, *mScrollDownAction, *mScrollUpMoreAction, *mScrollDownMoreAction, *mViewSourceAction;
  KSelectAction *mSelectEncodingAction;
  KToggleAction *mToggleFixFontAction;
  KUrl mUrlClicked;
  KMail::HtmlWriter * mHtmlWriter;
  /** Used only to be able to connect and disconnect finished() signal
      in printMsg() and slotPrintMsg() since mHtmlWriter points only to abstract non-QObject class. */
  QPointer<KMail::KHtmlPartHtmlWriter> mPartHtmlWriter;

  int mChoice;
  unsigned long mWaitingForSerNum;
  float mSavedRelativePosition;
	int mLevelQuote;
  bool mDecrytMessageOverwrite;
  bool mShowSignatureDetails;
  bool mShowAttachmentQuicklist;
  bool mExternalWindow;
};


#endif

