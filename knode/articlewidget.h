/*
    KNode, the KDE newsreader
    Copyright (c) 2005 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#ifndef KNODE_ARTICLEWIDGET_H
#define KNODE_ARTICLEWIDGET_H

#include <QByteArray>
#include <QMap>
#include <QWidget>

#include <kurl.h>

#include <kmime/kmime_content.h>

#include "knjobdata.h"

class QStringList;
class QTimer;

class KAction;
class KActionCollection;
class KActionMenu;
class KHTMLPart;
class KUrl;
class KSelectAction;
class KToggleAction;
class KXMLGUIClient;

namespace Kpgp {
  class Block;
}

class KNArticle;
class KNArticleCollection;

namespace KNode {

class CSSHelper;

/**
  Widget to display a news article.
*/
class ArticleWidget : public QWidget, public KNJobConsumer {

  Q_OBJECT

  public:
    /// Construct a new article widget.
    ArticleWidget( QWidget *parent,
                   KXMLGUIClient *guiClient,
                   KActionCollection *actionCollection );
    /// Destroy the article widget.
    ~ArticleWidget();

    /// read config settings
    void readConfig();
    /// write config settings (call only for the main viewer)
    void writeConfig();

    /** display the given article
     * @param article The article to display.
     */
    void setArticle( KNArticle *article );
    /// returns the currently shown article
    KNArticle *article() const { return mArticle; }

    /// notify all instances about a config change
    static void configChanged();
    /** check whether the given article is displayed in any instance
     * @param article The article to check.
     */
    static bool articleVisible( KNArticle *article );
    /** notify all instances that the given article has been removed
     * @param article The removed article.
     */
    static void articleRemoved( KNArticle *article );
    /** notify all instances that the given article has changed
     * @param article The changed article.
     */
    static void articleChanged( KNArticle *article );
    /** notify all instances about an error during loading the given article
     * @param article The article that couldn't be loaded.
     * @param error The error message.
     */
    static void articleLoadError( KNArticle *article, const QString &error );
    /** notify all instances that the given collection has been removed
     * @param coll The removed article collection (a group or a folder).
     */
    static void collectionRemoved( KNArticleCollection *coll );
    /// cleanup all instances
    static void cleanup();

    /// checks whether the readers is scrolled down to the bottom
    bool atBottom() const;

  public slots:
    /// scroll up by one line
    void scrollUp();
    /// scroll down by one line
    void scrollDown();
    /// scroll up by one page
    void scrollPrior();
    /// scroll down by one page
    void scrollNext();

  protected:
    /// process download jobs for view source action
    void processJob( KNJobData *j );

  private:
    void initActions();

    /// enable article dependent actions
    void enableActions();
    /// disable article dependent actions
    void disableActions();

    /// clears the article viewer
    void clear();
    /// displays the current article or clears the view if no article is set
    void displayArticle();
    /// displays the given error message in the viewer
    void displayErrorMessage( const QString &msg );

    /// display the message header (should be replaced by KMail's HeaderStyle class)
    void displayHeader();
    /** displays the given text block, including quote and signature handling
     *  @param lines A list of lines to display.
     */
    void displayBodyBlock( const QStringList &lines );
    /// displays a signature block header
    QString displaySigHeader( const Kpgp::Block &block );
    /// displays a signature footer
    void displaySigFooter( const QString &signClass );
    /// displays the given attachment
    void displayAttachment( KMime::Content *att, int partNum );

    /// HTML conversion flags for toHtmlString()
    enum ConversionFlags {
      None = 0,
      ParseURL = 1,
      FancyFormatting = 2,
      AllowROT13 = 4
    };
    /// convert the given string into a HTML string
    QString toHtmlString( const QString &line, int flags = ParseURL );
    /// convert the given image into a data:/ URL
    static QString imgToDataUrl( const QImage &image, const char* fmt );

    /** calculates the quoting depth of the given line
     *  @returns -1 if no quoting was found, the quoting level otherwise
     */
    static int quotingDepth( const QString &line, const QString &quoteChars );
    /// checks whether the given attachment can be shown inline
    bool inlinePossible( KMime::Content *c );
    /** Checks if the given charset is supported.
     * @param charset The charset to check.
     */
    bool canDecodeText( const QByteArray &charset ) const;

    /// regenerated viewer content without changing scrollbar position
    void updateContents();

    /** stores the given attachment into a temporary file
     *  @returns the filename the attachment has been stored to
     */
    QString writeAttachmentToTempFile( KMime::Content *att, int partNum );
    /// removes all temporary files
    void removeTempFiles();

  private slots:
    /// called if the user clicked on an URL
    void slotURLClicked( const KUrl &url, bool forceOpen = false );
    /// called if the user RMB clicked on an URL
    void slotURLPopup( const QString &url, const QPoint &point );

    /// mark as read timeout
    void slotTimeout();

    void slotSave();
    void slotPrint();
    void slotCopySelection();
    void slotSelectAll();
    void slotFind();
    void slotViewSource();
    void slotReply();
    void slotRemail();
    void slotForward();
    void slotCancel();
    void slotSupersede();
    void slotToggleFixedFont();
    void slotToggleFancyFormating();
    void slotToggleRot13();

    void slotFancyHeaders();
    void slotStandardHeaders();
    void slotAllHeaders();

    void slotIconAttachments();
    void slotInlineAttachments();
    void slotHideAttachments();

    void slotSetCharset( const QString &charset );
    void slotSetCharsetKeyboard();

    void slotOpenURL();
    void slotCopyURL();
    void slotAddBookmark();
    void slotAddToAddressBook();
    void slotOpenInAddressBook();
    void slotOpenAttachment();
    void slotSaveAttachment();

  private:
    /// the currently shown article
    KNArticle *mArticle;
    /// attachments of the current article
    KMime::Content::List mAttachments;
    /// mapping of temporary file names to part numbers
    QMap<QString, int> mAttachementMap;

    KHTMLPart *mViewer;
    CSSHelper *mCSSHelper;

    QStringList mTempDirs, mTempFiles;

    QString mHeaderStyle;
    QString mAttachmentStyle;
    bool mShowHtml;
    bool mRot13;
    bool mForceCharset;
    QByteArray mOverrideCharset;

    /// mark as read timer
    QTimer *mTimer;

    /// the last RMB clicked URL
    KUrl mCurrentURL;

    /// list of all instances of this class
    static QList<ArticleWidget*> mInstances;

    KXMLGUIClient *mGuiClient;
    KActionCollection *mActionCollection;

    QAction *mSaveAction;
    QAction *mPrintAction;
    QAction *mCopySelectionAction;
    QAction *mSelectAllAction;
    QAction *mFindAction;
    KAction *mViewSourceAction;
    KAction *mCharsetSelectKeyb;
    KAction *mReplyAction;
    KAction *mRemailAction;
    KAction *mForwardAction;
    KAction *mCancelAction;
    KAction *mSupersedeAction;
    KActionMenu *mHeaderStyleMenu;
    KActionMenu *mAttachmentStyleMenu;
    KToggleAction *mFixedFontToggle;
    KToggleAction *mFancyToggle;
    KToggleAction *mRot13Toggle;
    KSelectAction *mCharsetSelect;
};

}

#endif
