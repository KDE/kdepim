/*
    KNode, the KDE newsreader
    Copyright (c) 2005 Volker Krause <volker.krause@rwth-aachen.de>

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

#include <qmap.h>
#include <qvaluelist.h>
#include <qwidget.h>

#include <kurl.h>

#include <kmime_content.h>

#include "knjobdata.h"

class QStringList;
class QTimer;

class KAction;
class KActionCollection;
class KActionMenu;
class KHTMLPart;
class KURL;
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
  Widget to display a news article
*/
class ArticleWidget : public QWidget, public KNJobConsumer {

  Q_OBJECT

  public:
    /// Construct a new article widget
    ArticleWidget( QWidget *parent,
                   KXMLGUIClient *guiClient,
                   KActionCollection *actionCollection,
                   const char *name = 0 );
    ~ArticleWidget();

    /// read config settings
    void readConfig();
    /// write config settings (call only for the main viewer)
    void writeConfig();

    /// display the given article
    void setArticle( KNArticle *article );
    /// returns the currently shown article
    KNArticle *article() const { return mArticle; }

    KAction* setCharsetKeyboardAction() const { return mCharsetSelectKeyb; }

    /// notify all instances about a config change
    static void configChanged();
    /// check wether the given article is displayed in any instance
    static bool articleVisible( KNArticle *article );
    /// notify all instances that the given article has been removed
    static void articleRemoved( KNArticle *article );
    /// notify all instances that the given article has changed
    static void articleChanged( KNArticle *article );
    /// notify all instances about an error during loading the given article
    static void articleLoadError( KNArticle *article, const QString &error );
    /// notify all instances that the given collection has been removed
    static void collectionRemoved( KNArticleCollection *coll );
    /// cleanup all instances
    static void cleanup();

    /// checks wether the readers is scrolled down to the bottom
    bool atBottom() const;

  public slots:
    void scrollUp();
    void scrollDown();
    void scrollPrior();
    void scrollNext();

  signals:
    void focusChanged( QFocusEvent* );
    void focusChangeRequest( QWidget* );

  protected:
    /// process download jobs for view source action
    void processJob( KNJobData *j );

    virtual void focusInEvent( QFocusEvent *e );
    virtual void focusOutEvent( QFocusEvent *e );
    virtual bool eventFilter( QObject *o, QEvent *e );

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
    QString displaySigHeader( Kpgp::Block* block );
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
    /// convert the given string into an HTML string
    QString toHtmlString( const QString &line, int flags = ParseURL );
    /// convert the given image into a data:/ URL
    static QString imgToDataUrl( const QImage &image, const char* fmt );

    /** calculates the quoting depth of the given line
     *  @returns -1 if no quoting was found, the quoting level otherwise
     */
    static int quotingDepth( const QString &line, const QString &quoteChars );
    /// checks wether the given attachment can be shown inline
    bool inlinePossible( KMime::Content *c );
    /// checks if the given charset is supported
    bool canDecodeText( const QCString &charset ) const;

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
    void slotURLClicked( const KURL &url, bool forceOpen = false );
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
    QCString mOverrideCharset;

    /// mark as read timer
    QTimer *mTimer;

    /// the last RMB clicked URL
    KURL mCurrentURL;

    /// list of all instances of this class
    static QValueList<ArticleWidget*> mInstances;

    KXMLGUIClient *mGuiClient;
    KActionCollection *mActionCollection;

    KAction *mSaveAction;
    KAction *mPrintAction;
    KAction *mCopySelectionAction;
    KAction *mSelectAllAction;
    KAction *mFindAction;
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
