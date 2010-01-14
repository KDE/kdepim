/* -*- mode: C++; c-file-style: "gnu" -*-
  Copyright (c) 1997 Markus Wuebben <markus.wuebben@kde.org>
  Copyright (C) 2009 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
  Copyright (c) 2009 Andras Mantia <andras@kdab.net>

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

#ifndef MAILVIEWER_P_H
#define MAILVIEWER_P_H

#include <QObject>

#include <KService>
#include <KSharedConfigPtr>
#include <KUrl>

#include <akonadi/item.h>

#include <kmime/kmime_message.h>

#include <messagecore/messagestatus.h>

#include "viewer.h" //not so nice, it is actually for the enums from MailViewer
#include <kio/job.h>
#include <kresources/groupwise/soap/soapStub.h>

using KPIM::MessageStatus;
namespace GpgME { class Error; }
namespace KIO { class Job; }

namespace Kleo { class SpecialJob; }

class KAction;
class KActionCollection;
class KSelectAction;
class KToggleAction;
class KHBox;
class KWebView;
class QWebElement;

class QPoint;
class QSplitter;
class QStyle;
class QModelIndex;
class QTreeView;

class MimeTreeModel;
class ConfigureWidget;

class WebKitPartHtmlWriter;
class HtmlStatusBar;

namespace KParts {
  struct BrowserArguments;
  class OpenUrlArguments;
}

namespace MessageViewer {

  namespace Interface {
    class BodyPartMemento;
  }
  class EditorWatcher;
  class HtmlWriter;
  class CSSHelper;
  class AttachmentStrategy;
  class ObjectTreeParser;
  class HeaderStrategy;
  class HeaderStyle;
  class NodeHelper;
}

namespace MessageViewer {
/** Private class for MailViewer.
* @author andras@kdab.net
*/
class ViewerPrivate : public QObject {
  Q_OBJECT
public:

  ViewerPrivate(Viewer *aParent,
                    KSharedConfigPtr config,
                    QWidget *mainWindow,
                    KActionCollection* actionCollection);

  virtual ~ViewerPrivate();

    /** Returns message part from given URL or null if invalid. */
  KMime::Content* nodeFromUrl(const KUrl &url);

    /** Returns the message part for a given content index. */
  KMime::Content* nodeForContentIndex( const KMime::ContentIndex& index );

  /** Open the attachment pointed to the node.
   * @param fileName - if not empty, use this file to load the attachment content
  */
  void openAttachment( KMime::Content *node, const QString & fileName );

    /** Delete the attachment the @param node points to. Returns false if the user
  cancelled the deletion, true in all other cases (including failure to delete
  the attachment!) */
  bool deleteAttachment( KMime::Content* node, bool showWarning = true );


  void attachmentProperties( KMime::Content *node );
  void attachmentCopy( const KMime::Content::List & contents );


  /** Edit the attachment the @param node points to. Returns false if the user
  cancelled the editing, true in all other cases! */
  bool editAttachment( KMime::Content* node, bool showWarning = true );

  void emitUrlClicked( const KUrl & url, int button ) {
    emit urlClicked( url, button );
  }

  void emitPopupMenu( const KUrl & url, const QPoint & p ) {
    if ( mMessage )
      emit popupMenu( *mMessage, url, p );
    if ( mMessageItem.isValid() )
      emit popupMenu( mMessageItem, url, p );
  }

  /** Access to the KWebView used for the viewer. Use with
      care! */
  KWebView *htmlPart() const { return mViewer; }

  void showAttachmentPopup( KMime::Content* node, const QString & name, const QPoint & p );

  /** retrieve BodyPartMemento of id \a which for partNode \a node */
   Interface::BodyPartMemento * bodyPartMemento( const KMime::Content * node, const QByteArray & which ) const;

   /** set/replace BodyPartMemento \a memento of id \a which for
      partNode \a node. If there was a BodyPartMemento registered
      already, replaces (deletes) that one. */
   void setBodyPartMemento( const KMime::Content * node, const QByteArray & which, Interface::BodyPartMemento * memento );

   /** deletes all BodyPartMementos. Use this when skipping to another
      message (as opposed to re-loading the same one again). */
   void clearBodyPartMementos();

   /**
   * Sets the current attachment ID and the current attachment temporary filename
   * to the given values.
   * Call this so that slotHandleAttachment() knows which attachment to handle.
   */
  void prepareHandleAttachment(KMime::Content *node, const QString& fileName );


  /** This function returns the complete data that were in this
  * message parts - *after* all encryption has been removed that
  * could be removed.
  * - This is used to store the message in decrypted form.
  */
  void objectTreeToDecryptedMsg( KMime::Content* node,
                                 QByteArray& resultingData,
                                 KMime::Message& theMessage,
                                 bool weAreReplacingTheRootNode = false,
                                 int recCount = 0 );

  QString createAtmFileLink( const QString& atmFileName ) const;
  KService::Ptr getServiceOffer( KMime::Content *content);
  bool saveContent( KMime::Content* content, const KUrl& url, bool encoded );
  void saveAttachments( const KMime::Content::List & contents );
  KMime::Content::List allContents( const KMime::Content * content );
  KMime::Content::List selectedContents();
  void attachmentOpenWith( KMime::Content *node );
  void attachmentOpen( KMime::Content *node );


  /** Return the HtmlWriter connected to the KWebView we use */
  HtmlWriter * htmlWriter() { return mHtmlWriter; }

  CSSHelper* cssHelper() const;

  NodeHelper* nodeHelper() { return mNodeHelper; }

  Akonadi::Item messageItem() { return mMessageItem; }

  /** Returns whether the message should be decryted. */
  bool decryptMessage() const;

  /** Calculate the pixel size */
  int pointsToPixel(int pointSize) const;

    /** Display a generic HTML splash page instead of a message.
  * @param info - the text to be displayed in HTML format
  */
  void displaySplashPage( const QString &info );

  /** Enable the displaying of messages again after an splash (or other) page was displayed */
  void enableMessageDisplay();

  /** Feeds the HTML viewer with the contents of the given message.
    HTML begin/end parts are written around the message. */
  void displayMessage();

  /** Parse the root message and add it's contents to the reader window. */
  void parseMsg();

  /** Creates a nice mail header depending on the current selected
    header style. */
  QString writeMsgHeader( KMime::Message* aMsg, KMime::Content* vCardNode = 0, bool topLevel = false );

  /** show window containing information about a vCard. */
  void showVCard(KMime::Content *msgPart);

  /** HTML initialization. */
  virtual void initHtmlWidget(void);

  /** Event filter */
  bool eventFilter( QObject *obj, QEvent *ev );

  /** Read settings from app's config file. */
  void readConfig();

  /** Write settings to app's config file. Calls sync() if withSync is true. */
  void writeConfig( bool withSync=true );

   /** Get the message header style. */
  const HeaderStyle * headerStyle() const {
    return mHeaderStyle;
  }

  /** Set the header style and strategy. We only want them to be set
      together. */
  void setHeaderStyleAndStrategy( const HeaderStyle * style,
                                  const HeaderStrategy * strategy );

  /** Get the message header strategy. */
  const HeaderStrategy * headerStrategy() const {
    return mHeaderStrategy;
  }

  /** Get/set the message attachment strategy. */
  const AttachmentStrategy * attachmentStrategy() const {
    return mAttachmentStrategy;
  }
  void setAttachmentStrategy( const AttachmentStrategy * strategy );

  /** Get selected override character encoding.
      @return The encoding selected by the user or an empty string if auto-detection
      is selected. */
  QString overrideEncoding() const { return mOverrideEncoding; }

  /** Set the override character encoding. */
  void setOverrideEncoding( const QString & encoding );

  void setPrintFont( const QFont& font );

  /** Set printing mode */
  virtual void setPrinting(bool enable) { mPrinting = enable; }


  /** Print message. */
  void printMessage( KMime::Message* message );
  void printMessage( const Akonadi::Item &msg );

    /** Set the Akonadi item that will be displayed.
  * @param item - the Akonadi item to be displayed. If it doesn't hold a mail (KMime::Message::Ptr as payload data),
  *               an empty page is shown.
  * @param updateMode - update the display immediately or not. See MailViewer::UpdateMode.
  */
  void setMessageItem(const Akonadi::Item& item, Viewer::UpdateMode updateMode = Viewer::Delayed );


  /** Set the message that shall be shown.
  * @param msg - the message to be shown. If 0, an empty page is displayed.
  * @param updateMode - update the display immediately or not. See MailViewer::UpdateMode.
  *  @param MailViewer::Ownership - Transfer means the ownership of the msg pointer is taken by the lib
  */
  void setMessage(KMime::Message* msg, Viewer::UpdateMode updateMode = Viewer::Delayed, Viewer::Ownership = Viewer::Keep);

  /** Instead of settings a message to be shown sets a message part
      to be shown */
  void setMessagePart( KMime::Content* aMsgPart, bool aHTML,
                   const QString& aFileName, const QString& pname );

  void setMessagePart( KMime::Content * node );

  /** Show or hide the Mime Tree Viewer if configuration
      is set to smart mode.  */
  void showHideMimeTree();

  /** View message part of type message/RFC822 in extra viewer window. */
  void atmViewMsg(KMime::Content* msgPart);

  void adjustLayout();
  void saveSplitterSizes( KConfigGroup & c ) const;
  void createWidgets();
  void createActions();

  void showContextMenu( KMime::Content* content, const QPoint& point);

  KToggleAction * actionForHeaderStyle( const HeaderStyle *,
                                       const HeaderStrategy * );
  KToggleAction * actionForAttachmentStrategy( const AttachmentStrategy * );
  /** Read override codec from configuration */
  void readGlobalOverrideCodec();

     /** Get codec corresponding to the currently selected override character encoding.
      @return The override codec or 0 if auto-detection is selected. */
  const QTextCodec * overrideCodec() const;


  QString renderAttachments( KMime::Content *node, const QColor &bgColor );

  KMime::Content* findContentByType(KMime::Content *content, const QByteArray &type); //TODO(Andras) move to NodeHelper

  /** Return a QTextCodec for the specified charset.
   * This function is a bit more tolerant, than QTextCodec::codecForName */
  static const QTextCodec* codecForName(const QByteArray& _str); //TODO(Andras) move to a utility class?

  /** Saves the relative position of the scroll view. Call this before calling update()
      if you want to preserve the current view. */
  void saveRelativePosition();

  bool htmlMail() const;
  bool htmlLoadExternal() const;

    /** Return selected text */
  QString copyText();

  /** Get the html override setting */
  bool htmlOverride() const;

  /** Override default html mail setting */
  void setHtmlOverride( bool override );

  /** Get the load external references override setting */
  bool htmlLoadExtOverride() const;

/** Override default load external references setting */
  void setHtmlLoadExtOverride( bool override );

  /** Enforce message decryption. */
  void setDecryptMessageOverwrite( bool overwrite = true );

  /** Show signature details. */
  bool showSignatureDetails() const;

  /** Show signature details. */
  void setShowSignatureDetails( bool showDetails = true ) ;

  /* show or hide the list that points to the attachments */
  bool showAttachmentQuicklist() const;

  /* show or hide the list that points to the attachments */
  void setShowAttachmentQuicklist( bool showAttachmentQuicklist = true );

  void emitNoDrag() {emit noDrag(); }

  void scrollToAttachment( const KMime::Content *node );
  void setUseFixedFont( bool useFixedFont );


  bool noMDNsWhenEncrypted() const { return mNoMDNsWhenEncrypted; }


  bool disregardUmask() const;
  void setDisregardUmask( bool b);

  void attachmentView( KMime::Content *atmNode );
  void attachmentEncryptWithChiasmus( KMime::Content * content );

private slots:
  void slotAtmDecryptWithChiasmusResult( const GpgME::Error &, const QVariant & );
  void slotAtmDecryptWithChiasmusUploadResult( KJob * );


public slots:
  /** An URL has been activate with a click. */
  void slotUrlOpen( const QUrl &url);

  /** The mouse has moved on or off an URL. */
  void slotUrlOn(const QString & link, const QString & title, const QString & textContent);

  /** The user presses the right mouse button on an URL. */
  void slotUrlPopup(const QString &, const QPoint& mousePos);

  /** The user selected "Find" from the menu. */
  void slotFind();

  /** The user toggled the "Fixed Font" flag from the view menu. */
  void slotToggleFixedFont();
  void slotToggleMimePartTree();

  /** Show the message source */
  void slotShowMessageSource();

  /** Refresh the reader window */
  void updateReaderWin();

  void slotMimePartSelected( const QModelIndex &index );

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
  void slotDelayedResize();

  /** Print message. Called on as a response of finished() signal of mPartHtmlWriter
      after rendering is finished.
      In the very end it deletes the KMReaderWin window that was created
      for the purpose of rendering. */
  void slotPrintMsg();

  void slotSetEncoding();
  void injectAttachments();
  void slotSettingsChanged();
  void slotMimeTreeContextMenuRequested( const QPoint& pos );
  void slotAttachmentOpenWith();
  void slotAttachmentOpen();
  void slotAttachmentSaveAs();
  void slotAttachmentSaveAll();
  void slotAttachmentView();
  void slotAttachmentProperties();
  void slotAttachmentCopy();
  void slotAttachmentDelete();
  void slotAttachmentEdit();
  void slotAttachmentEditDone(EditorWatcher* editorWatcher);
  void slotLevelQuote( int l );

  /** Toggle display mode between HTML and plain text. */
  void slotToggleHtmlMode();

  /**
   * Does an action for the current attachment.
   * The action is defined by the KMHandleAttachmentCommand::AttachmentAction
   * enum.
   * prepareHandleAttachment() needs to be called before calling this to set the
   * correct attachment ID.
   */
  void slotHandleAttachment( int action );
  /** Copy the selected text to the clipboard */
  void slotCopySelectedText();

    /** Select message body. */
  void selectAll();

  void slotUrlClicked();
  /** Copy URL in mUrlCurrent to clipboard. Removes "mailto:" at
      beginning of URL before copying. */
  void slotUrlCopy();
  void slotSaveMessage();
  /** Re-parse the current message. */
  void update(MessageViewer::Viewer::UpdateMode updateMode = Viewer::Delayed);

  bool hasChildOrSibblingDivWithId( const QWebElement &start, const QString &id );

signals:
  void replaceMsgByUnencryptedVersion();
  void popupMenu(KMime::Message &msg, const KUrl &url, const QPoint& mousePos);
  void popupMenu(const Akonadi::Item &msg, const KUrl &url, const QPoint& mousePos);
  void urlClicked(const KUrl &url, int button);
  void urlClicked( const Akonadi::Item &msg, const KUrl &url );
  void noDrag();
  void requestConfigSync();
  void showReader( KMime::Content* aMsgPart, bool aHTML, const QString& aFileName, const QString& pname, const QString & encoding );

public:
  NodeHelper* mNodeHelper;
  bool mHtmlMail, mHtmlLoadExternal, mHtmlOverride, mHtmlLoadExtOverride;
  KMime::Message *mMessage; //the current message, if it was set manually
  Akonadi::Item mMessageItem; //the message item from Akonadi
  bool mDeleteMessage; //the message was created in the lib, eg. by calling setMessageItem()
  // widgets:
  QSplitter * mSplitter;
  KHBox *mBox;
  HtmlStatusBar *mColorBar;
  QTreeView* mMimePartTree; //FIXME(Andras) port the functionality from KMMimePartTree to a new view class or to here with signals/slots
  MimeTreeModel *mMimePartModel;
  KWebView *mViewer;

  const AttachmentStrategy * mAttachmentStrategy;
  const HeaderStrategy * mHeaderStrategy;
  const HeaderStyle * mHeaderStyle;
  /** where did the user save the attachment last time */
  QString mSaveAttachDir;
  static const int delay;
  QTimer mUpdateReaderWinTimer;
  QTimer mResizeTimer;
  QString mOverrideEncoding;
  QString mOldGlobalOverrideEncoding; // used to detect changes of the global override character encoding
  bool mMsgDisplay;
  bool mNoMDNsWhenEncrypted;

  CSSHelper * mCSSHelper;
  bool mUseFixedFont;
  bool mPrinting;
  bool mShowColorbar;
  //bool mShowCompleteMessage;
  int mMimeTreeMode;
  bool mMimeTreeAtBottom;
  QList<int> mSplitterSizes;
  QString mIdOfLastViewedMessage;
  QWidget *mMainWindow;
  KActionCollection *mActionCollection;
  KAction *mCopyAction, *mCopyURLAction,
      *mUrlOpenAction, *mSelectAllAction,
      *mScrollUpAction, *mScrollDownAction, *mScrollUpMoreAction, *mScrollDownMoreAction,
      *mViewSourceAction, *mSaveMessageAction;
  KSelectAction *mSelectEncodingAction;
  KToggleAction *mToggleFixFontAction, *mToggleDisplayModeAction;
  KToggleAction *mToggleMimePartTreeAction;
  KUrl mUrlClicked;
  HtmlWriter * mHtmlWriter;
  /** Used only to be able to connect and disconnect finished() signal
      in printMsg() and slotPrintMsg() since mHtmlWriter points only to abstract non-QObject class. */
  QPointer<WebKitPartHtmlWriter> mPartHtmlWriter;
  QMap<QByteArray, Interface::BodyPartMemento*> mBodyPartMementoMap;

  int mChoice;
  float mSavedRelativePosition;
  int mLevelQuote;
  bool mDecrytMessageOverwrite;
  bool mShowSignatureDetails;
  bool mShowAttachmentQuicklist;
  bool mExternalWindow;
  bool mDisregardUmask;
  KMime::Content *mCurrentContent;
  QString mCurrentFileName;
  QMap<MessageViewer::EditorWatcher*, KMime::Content*> mEditorWatchers;
  Kleo::SpecialJob *mJob;
  Viewer *const q;
};

}

#endif
