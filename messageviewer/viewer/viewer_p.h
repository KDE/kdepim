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

#include "viewer/nodehelper.h"
#include "viewer.h" //not so nice, it is actually for the enums from MailViewer

#include <akonadi/item.h>
#include <akonadi/monitor.h>
#include <kio/job.h>
#include <kmime/kmime_message.h>
#include <kservice.h>
#include <ksharedconfig.h>
#include <kurl.h>
#include <KCalCore/Todo>
#include <KCalCore/Event>

#include <QObject>
#include <QTimer>
#include <QWebElement>

namespace GpgME { class Error; }
namespace KIO { 
class Job;
}

namespace Kleo
{
class SpecialJob;
}

class KAction;
class KActionCollection;
class KSelectAction;
class KToggleAction;
class KHBox;
class KMenu;

class QPoint;
class QSplitter;
class QModelIndex;

namespace GrantleeTheme {
class GrantleeThemeManager;
}

namespace PimCommon {
class TranslatorWidget;
}

namespace MessageViewer {
class TodoEdit;
class EventEdit;
class EditorWatcher;
class HtmlWriter;
class CSSHelper;
class AttachmentStrategy;
class ObjectTreeParser;
class HeaderStrategy;
class HeaderStyle;
class FindBarMailWebView;
class WebKitPartHtmlWriter;
class HtmlStatusBar;
class MailWebView;
class ScamDetectionWarningWidget;
class MimePartTreeView;
class OpenAttachmentFolderWidget;

/**
\brief Private class for the Viewer, the main widget in the messageviewer library.

This class creates all subwidgets, like the MailWebView, the HtmlStatusBar and the FindBarMailWebView.

Also, ViewerPrivate creates and exposes all actions.

\par Displaying a message

Before displaying a message, a message needs to be set. This can be done in two ways, with
setMessageItem() and with setMessage(). setMessageItem() is the preferred way, as the viewer can
then remember the Akonadi::Item belonging to the message. The Akonadi::Item is needed when modifying
the message, for example when editing or deleting an attachment.
Sometimes passing an Akonadi::Item to the viewer is not possible, for example when double-clicking
an attached message, in which case a new KMime::Message is constructed out of the attachment, and a
separate window is opened for it. In this case, the KMime::Message has no associated Akonadi::Item.
If there is an Akonadi::Item available, it will be monitored for changes and the viewer
automatically updated on external changes.

Once a message is set, update() is called. update() can also be called after the message has already
been displayed. As an example, this is the case when the user decides to decrypt the message. The
decryption can happen async, and once the decryption is finished, update() is called to display the
now decrypted content. See the documentation of ObjectTreeParser on how exactly decryption is
handled.
update() is just a thin wrapper that calls updateReaderWin(). The only difference is that update()
has a timer that prevents too many slow calls to updateReaderWin() in a short time frame.
updateReaderWin() again is only a thin wrapper that resets some state and then calls
displayMessage().
displayMessage() itself is again a thin wrapper, which starts the HtmlWriter and then calls
parseMsg().
Finally, parseMsg() does the real work. It uses ObjectTreeParser::parseObjectTree() to let the
ObjectTreeParser parse the message and generate the HTML code for it.
As mentioned before, it can happen that the ObjectTreeParser needs to do some operation that happens
async, for example decrypting. In this case, the ObjectTreeParser will create a BodyPartMemento,
which basically is a wrapper around the job that does the async operation. Once the async operation
is finished. the BodyPartMemento will trigger an update() of ViewerPrivate, so that
ObjectTreeParser::parseObjectTree() gets called again and the ObjectTreeParser then can generate
HTML which has the decrypted content of the message. Again, see the documentation of ObjectTreeParser
for the details.
Additionally, parseMsg() does some evil hack for saving unencrypted messages should the config
option for that be set.

\par Displaying a MIME part of the message

The viewer can show only a part of the message, for example by clicking on a MIME part in the
message structure viewer or by double-clicking an attached message. In this case, setMessagePart()
is called. There are two of these functions. One even has special handling for images, special
handling for binary attachments and special handling of attached messages. In the last case, a new
KMime::Message is constructed and set as the main message with setMessage().

\par Attachment Handling

Some of those actions are actions that operate on a single attachment. For those, there is usually
a slot, like slotAttachmentCopy(). These actions are triggered from the attachment context menu,
which is shown in showAttachmentPopup(). The actions are connected to slotHandleAttachment() when
they are activated.
The action to edit an attachment uses the EditorWatcher to detect when editing with an external
editor is finished. Upon finishing, slotAttachmentEditDone() is called, which then creates an
ItemModifyJob to store the changes of the attachment. A map of currently active EditorWatcher and
their KMime::Content is available in mEditorWatchers.
For most attachment actions, the attachment is first written to a temp file. The action is then
executed on this temp file. Writing the attachment to a temp file is done with
NodeHelper::writeNodeToTempFile(). This method is called before opening or copying an attachment or
when rendering the attachment list. The ObjectTreeParser also calls NodeHelper::writeNodeToTempFile()
in some places. Once the temp file is written, NodeHelper::tempFileUrlFromNode() can be used to get
the file name of the temp file for a specific MIME part. This is for example used by the handler for
'attachment:' URLs, AttachmentURLHandler.

Since URLs for attachments are in the "attachment:" scheme, dragging them as-is to outside applications
wouldn't work, since other applications don't understand this scheme. Therefore, the viewer has
special handling for dragging URLs: In eventFilter(), drags are detected, and the URL handler is
called to deal with the drag. The attachment URL handler then starts a drag with the file:// URL
of the temp file of the attachment, which it gets with NodeHelper::tempFileUrlFromNode().

TODO: How are attachment handled that are loaded on demand? How does prepareHandleAttachment() work?
TODO: This temp file handling is a big mess and could use a rewrite, especially in the face of load
      on demand. There shouldn't be the need to write out tempfiles until really needed.

Some header styles display an attachment list in the header. The HTML code for the attachment list
cannot be generated by the HeaderStyle itself, since that does not know about all attachments.
Therefore, the attachment list needs to be created by ViewerPrivate. For this, the HeaderStyle
writes out a placeholder for the attachment list when it creates the HTML for the header. Once the
ObjectTreeParser is finished with the message, injectAttachments() is called. injectAttachments()
searches for the placeholder and replaces that with the real HTML code for the attachments.

One of the attachment actions is to scoll to the attachment. That action is only available when
right-clicking the header. The action scrolls to the attachment in the body and draws a yellow frame
around the attachment. This is done in scrollToAttachment(). The attachment in the body and the div
which is used for the colored frame are both created by the ObjectTreeParser.

\par Misc

ViewerPrivate holds the NodeHelper, which is passed on to the ObjectTreeParser when it needs it.
It also holds the HeaderStyle, HeaderStrategy, AttachmentStrategy, CSSHelper, HtmlWriter and more,
some of them again passed to the ObjectTreeParser when it needs it.

@author andras@kdab.net
 */
class ViewerPrivate : public QObject {
    Q_OBJECT
public:

    ViewerPrivate(Viewer *aParent, QWidget *mainWindow, KActionCollection *actionCollection );

    virtual ~ViewerPrivate();

    /** Returns message part from given URL or null if invalid. The URL's path is a KMime::ContentIndex path, or an index for the extra nodes,
   followed by : and the ContentIndex path. */
    KMime::Content* nodeFromUrl(const KUrl &url);

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

    /** Access to the MailWebView used for the viewer. Use with
      care! */
    MailWebView *htmlPart() const { return mViewer; }

    void showAttachmentPopup( KMime::Content* node, const QString & name, const QPoint & p );

    /**
   * Sets the current attachment ID and the current attachment temporary filename
   * to the given values.
   * Call this so that slotHandleAttachment() knows which attachment to handle.
   */
    void prepareHandleAttachment(KMime::Content *node, const QString& fileName );

    void postProcessMessage( ObjectTreeParser *otp, KMMsgEncryptionState encryptionState );

    QString createAtmFileLink( const QString& atmFileName ) const;
    KService::Ptr getServiceOffer( KMime::Content *content);
    KMime::Content::List selectedContents();
    void attachmentOpenWith( KMime::Content *node, KService::Ptr offer = KService::Ptr() );
    void attachmentOpen( KMime::Content *node );


    /** Return the HtmlWriter connected to the MailWebView we use */
    HtmlWriter * htmlWriter() const { return mHtmlWriter; }

    CSSHelper* cssHelper() const;

    NodeHelper* nodeHelper() const { return mNodeHelper; }

    Viewer *viewer() const { return q; }

    Akonadi::Item messageItem() const { return mMessageItem; }

    KMime::Message::Ptr message() const { return mMessage; }

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

    /** Parse the given content and generate HTML out of it for display */
    void parseContent( KMime::Content *content );

    /** Creates a nice mail header depending on the current selected
    header style. */
    QString writeMsgHeader( KMime::Message *aMsg, KMime::Content* vCardNode = 0,
                            bool topLevel = false );

    /** show window containing information about a vCard. */
    void showVCard(KMime::Content *msgPart);

    void setZoomTextOnly( bool textOnly );

    void saveMainFrameScreenshotInFile(const QString &filename);

private:
    /** HTML initialization. */
    void initHtmlWidget();
    void createOpenWithMenu( KMenu *topMenu, const QString &contentTypeStr, bool fromCurrentContent );
public:
    /** Event filter */
    bool eventFilter( QObject *obj, QEvent *ev );

    /** Read settings from app's config file. */
    void readConfig();

    /** Write settings to app's config file. Calls sync() if withSync is true. */
    void writeConfig( bool withSync=true );

    /** Get the message header style. */
    HeaderStyle * headerStyle() const {
        return mHeaderStyle;
    }

    /** Set the header style and strategy. We only want them to be set
      together. */
    void setHeaderStyleAndStrategy( HeaderStyle * style,
                                    HeaderStrategy * strategy, bool writeInConfigFile = false );

    /** Get the message header strategy. */
    HeaderStrategy * headerStrategy() const {
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

    /** Set printing mode */
    virtual void setPrinting(bool enable) { mPrinting = enable; }


    /** Print message. */
    void printMessage( const Akonadi::Item &msg );
    void printPreviewMessage( const Akonadi::Item &message );

    void resetStateForNewMessage();

    void setMessageInternal( const KMime::Message::Ptr message, Viewer::UpdateMode updateMode );

    /** Set the Akonadi item that will be displayed.
   *  @param item - the Akonadi item to be displayed. If it doesn't hold a mail (KMime::Message::Ptr as payload data),
   *                an empty page is shown.
   *  @param updateMode - update the display immediately or not. See MailViewer::UpdateMode.
   */
    void setMessageItem(const Akonadi::Item& item, Viewer::UpdateMode updateMode = Viewer::Delayed );


    /** Set the message that shall be shown.
  * @param msg - the message to be shown. If 0, an empty page is displayed.
  * @param updateMode - update the display immediately or not. See MailViewer::UpdateMode.
  */
    void setMessage( const KMime::Message::Ptr& msg, Viewer::UpdateMode updateMode = Viewer::Delayed );

    /** Instead of settings a message to be shown sets a message part
      to be shown */
    void setMessagePart( KMime::Content * node );

    /** Show or hide the Mime Tree Viewer if configuration
      is set to smart mode.  */
    void showHideMimeTree();

    /** View message part of type message/RFC822 in extra viewer window. */
    void atmViewMsg( KMime::Message::Ptr message );

    void setExternalWindow( bool b );

    void adjustLayout();
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


    QString renderAttachments( KMime::Content *node, const QColor &bgColor ) const;

    KMime::Content* findContentByType(KMime::Content *content, const QByteArray &type); //TODO(Andras) move to NodeHelper

    /** Return a QTextCodec for the specified charset.
   * This function is a bit more tolerant, than QTextCodec::codecForName */
    static const QTextCodec* codecForName(const QByteArray& _str); //TODO(Andras) move to a utility class?

    /** Saves the relative position of the scroll view. Call this before calling update()
      if you want to preserve the current view. */
    void saveRelativePosition();

    bool htmlMail() const;
    bool htmlLoadExternal() const;

    /** Get the html override setting */
    Viewer::DisplayFormatMessage displayFormatMessageOverwrite() const;

    /** Override default html mail setting */
    void setDisplayFormatMessageOverwrite(Viewer::DisplayFormatMessage format);

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

    // This controls whether a Toltec invitation is shown in its raw form or as a replacement text.
    // This can be toggled with the "kmail:showRawToltecMail" link.
    bool showRawToltecMail() const { return mShowRawToltecMail; }
    void setShowRawToltecMail( bool showRawToltecMail ) { mShowRawToltecMail = showRawToltecMail; }

    void scrollToAttachment( KMime::Content *node );
    void setUseFixedFont( bool useFixedFont );

    void attachmentView( KMime::Content *atmNode );
    void attachmentEncryptWithChiasmus( KMime::Content * content );

    /** Return weather to show or hide the full list of "To" addresses */
    bool showFullToAddressList() const;

    /** Show or hide the full list of "To" addresses */
    void setShowFullToAddressList( bool showFullToAddressList = true );

    /** Return weather to show or hide the full list of "To" addresses */
    bool showFullCcAddressList() const;

    /** Show or hide the full list of "To" addresses */
    void setShowFullCcAddressList( bool showFullCcAddressList = true );

    /** Show/Hide the field with id "field" */
    void toggleFullAddressList(const QString& field);

    void setZoomFactor( qreal zoomFactor );


    void goOnline();
    void goResourceOnline();

    bool isAShortUrl(const KUrl &url) const;

    void showOpenAttachmentFolderWidget(const KUrl &url);

private slots:
    void slotModifyItemDone(KJob* job);
    void slotMessageMayBeAScam();
    void slotMessageIsNotAScam();
    void slotAddToWhiteList();
    void slotToggleCaretBrowsing(bool);
    void slotAtmDecryptWithChiasmusResult( const GpgME::Error &, const QVariant & );
    void slotAtmDecryptWithChiasmusUploadResult( KJob * );

    /** Show hide all fields specified inside this function */
    void toggleFullAddressList();

    void itemFetchResult( KJob *job );

    void slotItemChanged( const Akonadi::Item& item, const QSet<QByteArray>& partIdentifiers );
    void slotItemMoved( const Akonadi::Item&, const Akonadi::Collection&, const Akonadi::Collection& );

    void itemModifiedResult( KJob* job );

    void collectionFetchedForStoringDecryptedMessage( KJob* job );

    void slotClear();

    void slotMessageRendered();

    void slotOpenWithAction(QAction *act);

    void slotOpenWithActionCurrentContent(QAction* act);

    void slotOpenWithDialog();

    void slotOpenWithDialogCurrentContent();


    void saveSplitterSizes() const;
    void slotGrantleeThemesUpdated();

    void slotCreateTodo(const KCalCore::Todo::Ptr &, const Akonadi::Collection &collection);

    void slotCreateEvent(const KCalCore::Event::Ptr &eventPtr, const Akonadi::Collection &collection);

public slots:
    /** An URL has been activate with a click. */
    void slotUrlOpen( const QUrl &url = QUrl());

    /** The mouse has moved on or off an URL. */
    void slotUrlOn(const QString & link, const QString & title, const QString & textContent);

    /** The user presses the right mouse button on an URL. */
    void slotUrlPopup(const QUrl &, const QUrl &imageUrl, const QPoint& mousePos);

    /** The user selected "Find" from the menu. */
    void slotFind();

    void slotTranslate();


    /** The user toggled the "Fixed Font" flag from the view menu. */
    void slotToggleFixedFont();
    void slotToggleMimePartTree();

    /** Show the message source */
    void slotShowMessageSource();

    /** Refresh the reader window */
    void updateReaderWin();

    void slotMimePartSelected( const QModelIndex &index );

    void slotBriefHeaders();
    void slotFancyHeaders();
    void slotEnterpriseHeaders();
    void slotStandardHeaders();
    void slotLongHeaders();
    void slotAllHeaders();
    void slotCustomHeaders();
    void slotGrantleeHeaders();

    void slotIconicAttachments();
    void slotSmartAttachments();
    void slotInlineAttachments();
    void slotHideAttachments();
    void slotHeaderOnlyAttachments();

    /** Some attachment operations. */
    void slotDelayedResize();

    /** Print message. Called on as a response of finished() signal of mPartHtmlWriter
      after rendering is finished.
      In the very end it deletes the KMReaderWin window that was created
      for the purpose of rendering. */
    void slotPrintMsg();
    void slotPrintPreview();


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
    void slotZoomTextOnly();
    void slotLoadExternalReference();


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

    void viewerSelectionChanged();

    /** Select message body. */
    void selectAll();

    void clearSelection();

    /** Copy URL in mUrlCurrent to clipboard. Removes "mailto:" at
      beginning of URL before copying. */
    void slotUrlCopy();
    void slotSaveMessage();
    /** Re-parse the current message. */
    void update(MessageViewer::Viewer::UpdateMode updateMode = Viewer::Delayed);

    void slotZoomIn();
    void slotZoomOut();
    void slotZoomReset();
    void slotSpeakText();
    void slotCopyImageLocation();
    void slotSaveMessageDisplayFormat();
    void slotResetMessageDisplayFormat();
    void slotBlockImage();
    void slotOpenBlockableItems();
    void slotExpandShortUrl();
    void slotShowCreateTodoWidget();
    void slotShowCreateEventWidget();

signals:
    void showStatusBarMessage( const QString &message );
    void replaceMsgByUnencryptedVersion();
    void popupMenu(const Akonadi::Item &msg, const KUrl &url, const KUrl &imageUrl, const QPoint& mousePos);
    void urlClicked( const Akonadi::Item &msg, const KUrl &url );
    void requestConfigSync();
    void showReader( KMime::Content* aMsgPart, bool aHTML, const QString & encoding );
    void showMessage( KMime::Message::Ptr message, const QString& encoding );
    void itemRemoved();
    void makeResourceOnline(MessageViewer::Viewer::ResourceOnlineMode mode);

    void changeDisplayMail(Viewer::DisplayFormatMessage,bool);
    void moveMessageToTrash();
private:
    QString attachmentInjectionHtml() const;
    QString recipientsQuickListLinkHtml( bool, const QString & ) const;
    void initGrantleeThemeName();

public:
    NodeHelper* mNodeHelper;
    bool mHtmlMailGlobalSetting, mHtmlLoadExternalGlobalSetting, mHtmlLoadExtOverride;
    KMime::Message::Ptr mMessage; //the current message, if it was set manually
    Akonadi::Item mMessageItem; //the message item from Akonadi
    // widgets:
    QSplitter * mSplitter;
    KHBox *mBox;
    HtmlStatusBar *mColorBar;
#ifndef QT_NO_TREEVIEW
    MimePartTreeView* mMimePartTree;
#endif
    MailWebView *mViewer;
    FindBarMailWebView *mFindBar;
    PimCommon::TranslatorWidget *mTranslatorWidget;

    const AttachmentStrategy * mAttachmentStrategy;
    HeaderStrategy * mHeaderStrategy;
    HeaderStyle * mHeaderStyle;
    static const int delay;
    QTimer mUpdateReaderWinTimer;
    QTimer mResizeTimer;
    QString mOverrideEncoding;
    QString mOldGlobalOverrideEncoding; // used to detect changes of the global override character encoding

    /// This is true if the viewer currently is displaying a message. Can be false, for example when
    /// the splash/busy page is displayed.
    bool mMsgDisplay;

    CSSHelper * mCSSHelper;
    bool mUseFixedFont;
    bool mPrinting;
    QString mIdOfLastViewedMessage;
    QWidget *mMainWindow;
    KActionCollection *mActionCollection;
    KAction *mCopyAction, *mCopyURLAction,
    *mUrlOpenAction, *mSelectAllAction,
    *mScrollUpAction, *mScrollDownAction, *mScrollUpMoreAction, *mScrollDownMoreAction,
    *mViewSourceAction, *mSaveMessageAction, *mFindInMessageAction, *mSaveMessageDisplayFormat, *mResetMessageDisplayFormat;
    KToggleAction *mHeaderOnlyAttachmentsAction;
    KSelectAction *mSelectEncodingAction;
    KToggleAction *mToggleFixFontAction, *mToggleDisplayModeAction;
#ifndef KDEPIM_NO_WEBKIT
#if QTWEBKIT_VERSION >= QTWEBKIT_VERSION_CHECK(2, 3, 0)
    KToggleAction *mCaretBrowsing;
#endif
#endif
    KAction *mZoomTextOnlyAction, *mZoomInAction, *mZoomOutAction, *mZoomResetAction;
    KToggleAction *mToggleMimePartTreeAction;
    KAction *mSpeakTextAction;
    KAction *mCopyImageLocation;
    KAction *mTranslateAction;
    KAction *mBlockImage;
    KAction *mBlockableItems;
    KAction *mExpandUrlAction;
    KAction *mCreateTodoAction;
    KAction *mCreateEventAction;
    KUrl mHoveredUrl;
    KUrl mClickedUrl;
    KUrl mImageUrl;
    QPoint mLastClickPosition;
    bool mCanStartDrag;
    HtmlWriter * mHtmlWriter;
    /** Used only to be able to connect and disconnect finished() signal
      in printMsg() and slotPrintMsg() since mHtmlWriter points only to abstract non-QObject class. */
    QPointer<WebKitPartHtmlWriter> mPartHtmlWriter;

    float mSavedRelativePosition;
    int mLevelQuote;
    bool mDecrytMessageOverwrite;
    bool mShowSignatureDetails;
    bool mShowAttachmentQuicklist;
    bool mShowRawToltecMail;
    bool mExternalWindow;
    bool mZoomTextOnly;
    int mRecursionCountForDisplayMessage;
    KMime::Content *mCurrentContent;
    KMime::Content *mMessagePartNode;
    QString mCurrentFileName;
    QString mMessagePath;
    QMap<EditorWatcher*, KMime::Content*> mEditorWatchers;
    Kleo::SpecialJob *mJob;
    Viewer *const q;
    bool mShowFullToAddressList;
    bool mShowFullCcAddressList;
    Akonadi::Monitor mMonitor;
    QString mAppName;
    QSet<AbstractMessageLoadedHandler*> mMessageLoadedHandlers;
    Akonadi::Item::Id mPreviouslyViewedItem;

    GrantleeTheme::GrantleeThemeManager *mThemeManager;
    ScamDetectionWarningWidget *mScamDetectionWarning;
    MessageViewer::TodoEdit *mCreateTodo;
    MessageViewer::EventEdit *mCreateEvent;
    MessageViewer::OpenAttachmentFolderWidget *mOpenAttachmentFolderWidget;
    // zoom Factor
    static const qreal zoomBy;
    qreal mZoomFactor;
    Viewer::DisplayFormatMessage mDisplayFormatMessageOverwrite;
};

}

#endif
