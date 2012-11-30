/*
    This file is part of Blogilo, A KDE Blogging Client

    It is a modified version of "texteditor.cpp" from
    FlashQard project.

    Copyright (C) 2008-2009 Shahab <shahab@flashqard-project.org>
    Copyright (C) 2010 Mehrdad Momeny <mehrdad.momeny@gmail.com>
    Copyright (C) 2010 Golnaz Nilieh <g382nilieh@gmail.com>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.


    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, see http://www.gnu.org/licenses/
*/

#include "texteditor.h"
#include <qimagewriter.h>
#include <QMessageBox>
// #include "utilities.h"
#include <kstandarddirs.h>
#include <KDebug>
#include <ktoolbar.h>
#include <KSelectAction>
#include <KToggleAction>
#include <klocalizedstring.h>
#include <KColorDialog>
#include "composer/dialogs/addeditimage.h"
#include "composer/dialogs/addeditlink.h"
#include <QContextMenuEvent>
#include <QTimer>
#include <qwebframe.h>
#include <qbuffer.h>
#include <QFile>
#include <QDir>
#include <math.h>
#include "bilbomedia.h"
#include <kmimetype.h>
#include "global.h"
#include <qwebelement.h>
#include <KMenu>
#include <QApplication>

#define FORWARD_ACTION(action1, action2) \
    connect(action1, SIGNAL(triggered()), getAction(action2), SLOT(trigger()));\
    connect(getAction(action2), SIGNAL(changed()), SLOT(adjustActions()));

WebView::WebView ( QWidget *parent )
        : KWebView ( parent )
{
    settings() -> setFontSize ( QWebSettings::DefaultFontSize, 14 );
    page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    setAcceptDrops ( true );
}

void WebView::startEditing()
{
    //NOTE: it needs a mouse click!
    //any better way to make the cursor visible?
    this -> setFocus();
    QMouseEvent mouseEventPress ( QEvent::MouseButtonPress, QPoint ( 10,10 ), Qt::LeftButton, Qt::NoButton, Qt::NoModifier );
    QApplication::sendEvent ( this, &mouseEventPress );
    QTimer::singleShot ( 50, this, SLOT (sendMouseReleaseEvent()) );
}

void WebView::sendMouseReleaseEvent()
{
    QMouseEvent mouseEventRelease ( QEvent::MouseButtonRelease, QPoint ( 10,10 ), Qt::LeftButton, Qt::NoButton, Qt::NoModifier );
    QApplication::sendEvent ( this, &mouseEventRelease );
    pageAction ( QWebPage::MoveToEndOfDocument ) -> trigger();
}

void WebView::dragEnterEvent ( QDragEnterEvent *event )
{
    KWebView::dragEnterEvent(event);
    //uncomment lines below when drag n drop support added
//     if ( event->mimeData()->hasText() )
//         event->acceptProposedAction();
}

void WebView::dropEvent ( QDropEvent *event )
{
    KWebView::dropEvent(event);
    //TODO support drag n drop
//     QString textToInsert = event->mimeData()->text();
//     kDebug()<<textToInsert;
//     QString js = QString("document.execCommand(\"insertHTML\", false, \"%2\")").arg(textToInsert);
//     page()->mainFrame()->evaluateJavaScript(js);
}

void WebView::contextMenuEvent(QContextMenuEvent* event)
{
    QWebHitTestResult hitTest = page()->mainFrame()->hitTestContent(event->pos());
    QWebElement elm = hitTest.element();
    QWebElement linkElm = hitTest.linkElement();

    KMenu *menu = new KMenu;
    menu->addAction(page()->action(QWebPage::Undo));
    menu->addAction(page()->action(QWebPage::Redo));
    menu->addSeparator();
    menu->addAction(page()->action(QWebPage::Cut));
    menu->addAction(page()->action(QWebPage::Copy));
    menu->addAction(page()->action(QWebPage::Paste));
    menu->addSeparator();
    menu->addAction(page()->action(QWebPage::SelectAll));
    menu->addSeparator();

    KAction* actEditImage = new KAction(KIcon(""), i18n("Edit Image"), menu);//TODO we need an icon
    KAction* actRemoveImage = new KAction(KIcon("edit-delete"), i18n("Remove Image"), menu);
    KAction* actEditLink = new KAction(KIcon("insert-link"), i18n("Edit Hyperlink"), menu);
    KAction* actRemoveLink = new KAction(KIcon("remove-link"), i18n("Remove Hyperlink"), menu);
    if(elm.tagName().toLower() == "img"){
        menu->addSeparator();
        menu->addAction(actEditImage);
        menu->addAction(actRemoveImage);
    }
    if( !linkElm.isNull() ) {
        menu->addSeparator();
        menu->addAction(actEditLink);
        menu->addAction(actRemoveLink);
    }
    QAction *res = menu->exec(event->globalPos());
    delete menu;
    if(res == actEditImage){
        QMap<QString, QString> img;
        img.insert("url", elm.attribute("src"));
        img.insert("width", elm.attribute("width"));
        img.insert("height", elm.attribute("height"));
        img.insert("title", elm.attribute("title"));
        img.insert("alt", elm.attribute("alt"));
        img.insert("align", elm.attribute("align"));
        if( !linkElm.isNull() )
            img.insert("link", linkElm.attribute("href"));
        QPointer<AddEditImage> dlg = new AddEditImage(this, img);
        if(dlg->exec()){
            img = dlg->selectedMediaProperties();
            elm.setAttribute("src", img["url"]);
            if(img["align"].isEmpty())
                elm.removeAttribute("align");
            else
                elm.setAttribute("align", img["align"]);
            if(img.value("width").isEmpty() || img.value("width")=="0")
                elm.removeAttribute("width");
            else
                elm.setAttribute("width", img["width"]);
            if(img.value("height").isEmpty() || img.value("height")=="0")
                elm.removeAttribute("height");
            else
                elm.setAttribute("height", img["height"]);
            if(img.value("title").isEmpty())
                elm.removeAttribute("title");
            else
                elm.setAttribute("title", img["title"]);
            if(img.value("alt").isEmpty())
                elm.removeAttribute("alt");
            else
                elm.setAttribute("alt", img["alt"]);
            if( !linkElm.isNull() ){
                if(img.value("link").isEmpty())
                    linkElm.removeFromDocument();
                else {
                    linkElm.setAttribute("href", img.value("link"));
                }
            } /* :(( Does not work:
            else if(!img.value("link").isEmpty()){
                QWebElement tmpLink;
                tmpLink.setOuterXml(QString("<a href='%1'></a>").arg(img["link"]));
                kDebug()<<tmpLink.toOuterXml();
                tmpLink.appendInside(elm);
                kDebug()<<tmpLink.toOuterXml();
                elm.replace(tmpLink);
            }*/
        }
        dlg->deleteLater();
    } else if(res == actRemoveImage){
        elm.removeFromDocument();
    } else if(res == actEditLink){
        QPointer<AddEditLink> dlg = new AddEditLink( linkElm.attribute("href"),
                                                        linkElm.attribute("title"),
                                                        linkElm.attribute("target"),
                                                        this);
        if(dlg->exec()){
            Link lnk = dlg->result();
            linkElm.setAttribute("href", lnk.address);
            if(lnk.target.isEmpty()){
                linkElm.removeAttribute("target");
            } else {
                linkElm.setAttribute("target", lnk.target);
            }
            if(lnk.title.isEmpty()){
                linkElm.removeAttribute("title");
            } else {
                linkElm.setAttribute("title", lnk.title);
            }
        }
        dlg->deleteLater();
    } else if(res == actRemoveLink) {
        page()->mainFrame()->evaluateJavaScript("document.execCommand(\"unLink\", false, null)");
    }
}

//----------------------------------------------------------------------

TextEditor::TextEditor ( QWidget *parent )
        : QWidget ( parent ), webView(new WebView(this))
{
    objectsAreModified = false;
    readOnly = false;
    QFile file ( KStandardDirs::locate ( "data", "blogilo/TextEditorInitialHtml" ) );
    kDebug() <<file.fileName();
    if ( !file.open ( QIODevice::ReadOnly ) )
        QMessageBox::warning ( 0, i18n ( "TextEditor" ),
                               i18n ( "TextEditor: Cannot open template file." ) );
    else
        webView -> setContent ( file.readAll());//, "application/xhtml+xml" );

    connect ( webView->page(), SIGNAL (selectionChanged()), this, SLOT (adjustActions()) );
    connect ( webView->page(), SIGNAL (selectionChanged()), this, SIGNAL (selectionChanged()) );
    connect ( webView->page(), SIGNAL (contentsChanged()), this, SLOT (somethingEdittedSlot()) );
    connect ( webView->page(), SIGNAL (contentsChanged()), this, SIGNAL (textChanged()) );

    createLayout();
    createActions();
    adjustActions();
    adjustFontSizes();
}

void TextEditor::setReadOnly ( bool _readOnly )
{
    readOnly = _readOnly;
    evaluateJavaScript ( QString ( "setReadOnly(%1)" ).arg ( readOnly?"true":"false" ), false );
}

void TextEditor::somethingEdittedSlot()
{
    adjustActions();
}

void TextEditor::updateStyle()
{
    evaluateJavaScript ( "updateCSS()", false );
//    setZoomFactor(Settings::settings() -> cardAppearanceZoomFactor);
}

void TextEditor::dropMimeDataSlot ( const QMimeData *mime )
{
    if ( mime -> hasUrls() ) {
        QList<QUrl> urls = mime->urls();
        if ( !urls.size() )
            return;

//       Downloader downloader(this);
//       insertImage(downloader.download(urls[0]));
    }
}

void TextEditor::anObjectIsModifiedSlot()
{
    objectsAreModified = true;
}

void TextEditor::createLayout()
{
    barVisual = new KToolBar ( this );
    barVisual->setIconSize ( QSize ( 22, 22 ) );
    barVisual->setToolButtonStyle ( Qt::ToolButtonIconOnly );
    QVBoxLayout *mainLayout = new QVBoxLayout ( this );
    mainLayout->addWidget ( barVisual );
    mainLayout -> addWidget ( webView );
}

void TextEditor::createActions()
{
    actCheckSpelling = new KAction( KIcon( "tools-check-spelling" ),
                                    i18n( "Enable Spell Checking"), this );
    actCheckSpelling->setCheckable( true );
    connect( actCheckSpelling, SIGNAL(triggered(bool)), this,
             SLOT(slotToggleSpellChecking(bool)) );
//     barVisual->addAction( actCheckSpelling ); FIXME: Missing functionality

    barVisual->addSeparator();
    actBold = new KAction( KIcon( "format-text-bold" ), i18nc(
                          "Makes text bold, and its shortcut is (Ctrl+b)",
                          "Bold (Ctrl+b)" ), this );
    actBold->setShortcut( Qt::CTRL + Qt::Key_B );
    actBold->setCheckable( true );
    FORWARD_ACTION(actBold, QWebPage::ToggleBold);
    barVisual->addAction( actBold );

    actItalic = new KAction( KIcon( "format-text-italic" ), i18nc(
                            "Makes text italic, and its shortcut is (Ctrl+i)",
                            "Italic (Ctrl+i)" ), this );
    actItalic->setShortcut( Qt::CTRL + Qt::Key_I );
    actItalic->setCheckable( true );
    FORWARD_ACTION(actItalic, QWebPage::ToggleItalic);
    barVisual->addAction( actItalic );

    actUnderline = new KAction( KIcon( "format-text-underline" ), i18nc(
                               "Makes text underlined, and its shortcut is (Ctrl+u)",
                               "Underline (Ctrl+u)" ), this );
    actUnderline->setShortcut( Qt::CTRL + Qt::Key_U );
    actUnderline->setCheckable( true );
    FORWARD_ACTION(actUnderline, QWebPage::ToggleUnderline);
    barVisual->addAction( actUnderline );

    actStrikeout = new KAction( KIcon( "format-text-strikethrough" ), i18nc(
                                "Strikes the text out, and its shortcut is (Ctrl+l)",
                                "Strike out (Ctrl+l)" ), this );
    actStrikeout->setShortcut( Qt::CTRL + Qt::Key_L );
    actStrikeout->setCheckable( true );
    FORWARD_ACTION(actStrikeout, QWebPage::ToggleStrikethrough);
    barVisual->addAction( actStrikeout );

    actCode = new KAction( KIcon( "format-text-code" ), i18nc( "Sets text font to code style",
                           "Code" ), this );
//     actCode->setCheckable( true );
    connect( actCode, SIGNAL(triggered(bool)), this, SLOT(slotToggleCode(bool)) );
    barVisual->addAction( actCode ); 

    barVisual->addSeparator();

    actFormatType = new KSelectAction( this );
    actFormatType->setEditable( false );
    QStringList formatTypes;
    formatTypes << i18n( "Paragraph" );
    formatTypes << i18n( "Heading 1" );
    formatTypes << i18n( "Heading 2" );
    formatTypes << i18n( "Heading 3" );
    formatTypes << i18n( "Heading 4" );
    formatTypes << i18n( "Heading 5" );
    formatTypes << i18n( "Heading 6" );
    formatTypes << i18n( "Pre Formatted" );
    actFormatType->setItems( formatTypes );
//     actFormatType->setMaxComboViewCount( 3 );
    actFormatType->setCurrentAction( i18n( "Paragraph" ) );
    connect( actFormatType, SIGNAL(triggered(QString)),
             this, SLOT(slotChangeFormatType(QString)) );
    barVisual->addAction( actFormatType );

    actFontIncrease = new KAction( KIcon( "format-font-size-more" ), i18n( "Increase font size" ), this );
    connect( actFontIncrease, SIGNAL(triggered(bool)), this, SLOT(slotIncreaseFontSize()) );
    barVisual->addAction( actFontIncrease );

    actFontDecrease = new KAction( KIcon( "format-font-size-less" ), i18n( "Decrease font size" ), this );
    connect( actFontDecrease, SIGNAL(triggered(bool)), this, SLOT(slotDecreaseFontSize()) );
    barVisual->addAction( actFontDecrease );

    actColorSelect = new KAction( KIcon( "format-text-color" ),
                                  i18nc( "verb, to select text color", "Select Color" ), this );
    connect( actColorSelect, SIGNAL(triggered(bool)), this, SLOT(formatTextColor()) );
    barVisual->addAction( actColorSelect );

    actRemoveFormatting = new KAction( KIcon( "draw-eraser" ), i18n( "Remove formatting" ), this );
    FORWARD_ACTION(actRemoveFormatting, QWebPage::RemoveFormat);
    barVisual->addAction( actRemoveFormatting );

    actBlockQuote = new KAction( KIcon( "format-text-blockquote" ), i18n( "Blockquote" ), this );
    actBlockQuote->setCheckable( true );
    connect( actBlockQuote, SIGNAL(triggered(bool)), this, SLOT(slotToggleBlockQuote(bool)) );
    barVisual->addAction( actBlockQuote );

    barVisual->addSeparator();

    actAddLink = new KAction( KIcon( "insert-link" ), i18nc(
                             "verb, to add a new link or edit an existing one",
                             "Add Hyperlink" ), this );
    connect( actAddLink, SIGNAL(triggered(bool)), this, SLOT(slotAddLink()));
    barVisual->addAction( actAddLink );

    actRemoveLink = new KAction( KIcon( "remove-link" ), i18nc(
                                "verb, to remove an existing link",
                                "Remove Hyperlink" ), this );
    connect( actRemoveLink, SIGNAL(triggered(bool)), this, SLOT(slotRemoveLink()));
    barVisual->addAction( actRemoveLink );

    actAddImage = new KAction( KIcon( "insert-image" ), i18nc( "verb, to insert an image",
                               "Add Image" ), this );
    connect( actAddImage, SIGNAL(triggered(bool)), this, SLOT(slotAddImage()));
    barVisual->addAction( actAddImage );

    barVisual->addSeparator();

    actAlignLeft = new KAction( KIcon( "format-justify-left" ),
                                i18nc( "verb, to align text from left", "Align left" ), this );
    actAlignLeft->setCheckable(true);
    FORWARD_ACTION(actAlignLeft, QWebPage::AlignLeft);
    barVisual->addAction( actAlignLeft );

    actAlignCenter = new KAction( KIcon( "format-justify-center" ),
                                  i18nc( "verb, to align text from center", "Align center" ), this );
    actAlignCenter->setCheckable(true);
    FORWARD_ACTION(actAlignCenter, QWebPage::AlignCenter);
    barVisual->addAction( actAlignCenter );

    actAlignRight = new KAction( KIcon( "format-justify-right" ),
                                 i18nc( "verb, to align text from right", "Align right" ), this );
    actAlignRight->setCheckable(true);
    FORWARD_ACTION(actAlignRight, QWebPage::AlignRight);
    barVisual->addAction( actAlignRight );

    actJustify = new KAction( KIcon( "format-justify-fill" ), i18nc(
                             "verb, to justify text", "Justify" ), this );
    actJustify->setCheckable(true);
    FORWARD_ACTION(actJustify, QWebPage::AlignJustified);
    barVisual->addAction( actJustify );

    actRightToLeft = new KAction( KIcon( "format-text-direction-rtl" ), i18nc(
                                 "Sets text direction as right to left",
                                 "Right to Left" ), this );
    actRightToLeft->setCheckable( true );
    connect( actRightToLeft, SIGNAL(toggled(bool)), this, SLOT(slotChangeLayoutDirection(bool)) );
    barVisual->addAction( actRightToLeft );

    barVisual->addSeparator();

    actOrderedList = new KToggleAction( KIcon( "format-list-ordered" ), i18n( "Ordered List" ), this );
    FORWARD_ACTION(actOrderedList, QWebPage::InsertOrderedList);
    barVisual->addAction( actOrderedList );

    actUnorderedList = new KToggleAction( KIcon( "format-list-unordered" ), i18n( "Unordered List" ), this );
    FORWARD_ACTION(actUnorderedList, QWebPage::InsertUnorderedList);
    barVisual->addAction( actUnorderedList );

    actSplitPost = new KAction( KIcon( "insert-more-mark" ), i18n( "Split text" ), this );
    connect( actSplitPost, SIGNAL(triggered(bool)), this, SLOT(slotAddPostSplitter()) );
    barVisual->addAction( actSplitPost );
}

QString TextEditor::htmlContent()
{
    return getHtml();
}

QString TextEditor::plainTextContent()
{
    return webView->page()->mainFrame()->toPlainText();
}

void TextEditor::setHtmlContent ( const QString& arg1 )
{
    QString txt = arg1;
    txt = txt.replace('\"', "\\\"").simplified();
    evaluateJavaScript(QString("replaceHtml(\"%1\")").arg(txt), true);
}

void TextEditor::clear()
{
//    setDocument(Document());
}

void TextEditor::startEditing()
{
    webView -> startEditing();
}

void TextEditor::setFontSize ( int fontSize )
{
    //setFontSize() function requires a number in the range 0-6
    //But the function below requires a number in the range 1-7
    execCommand ( "fontSize", QString::number ( fontSize+1 ) );
}

int TextEditor::getFontSize() const
{
    QString fontSizeString = const_cast<TextEditor*> ( this ) -> evaluateJavaScript ( "getFontSize()",
                                                                                      false ).toString();

    fontSizeString.chop ( 2 );
    int fontSizeInt = fontSizeString.toInt();

    fontSizeInt = ceil( static_cast<double> ( fontSizeInt ) /getZoomFactor() - 0.49 );

    return fontSizes.indexOf ( fontSizeInt );
}

double TextEditor::getZoomFactor() const
{
    return webView -> textSizeMultiplier();
}

Qt::Alignment TextEditor::getAlignment() const
{
    QString alignment = const_cast<TextEditor*> ( this ) -> evaluateJavaScript ( "getAlignment()", false ).toString();
    return alignment == "left" ? Qt::AlignLeft
           : alignment == "right" ? Qt::AlignRight
           : alignment == "center" ? Qt::AlignCenter
           : Qt::AlignJustify;
}

void TextEditor::execCommand ( const QString &cmd )
{
    QWebFrame *frame = webView->page()->mainFrame();
    QString js = QString ( "document.execCommand(\"%1\", false, null)" ).arg ( cmd );
    frame->evaluateJavaScript ( js );
}

void TextEditor::execCommand ( const QString &cmd, const QString &arg )
{
    QWebFrame *frame = webView->page()->mainFrame();
    QString js = QString ( "document.execCommand(\"%1\", false, \"%2\")" ).arg ( cmd ).arg ( arg );
    frame->evaluateJavaScript ( js );
}

bool TextEditor::queryCommandState ( const QString &cmd )
{
    QWebFrame *frame = webView->page()->mainFrame();
    QString js = QString ( "document.queryCommandState(\"%1\", false, null)" ).arg ( cmd );
    QVariant result = frame->evaluateJavaScript ( js );
    return result.toString().simplified().toLower() == "true";
}

#define FOLLOW_CHECK(a1, a2) a1->setChecked(getAction(a2)->isChecked())

void TextEditor::adjustActions()
{
    Qt::Alignment alignment = getAlignment();
    actAlignLeft -> setChecked ( alignment&Qt::AlignLeft );
    actAlignRight -> setChecked ( alignment&Qt::AlignRight );
    actAlignCenter -> setChecked ( alignment&Qt::AlignCenter );
    actJustify -> setChecked ( alignment&Qt::AlignJustify );

    QString direction = evaluateJavaScript ( "getTextDirection()", false ).toString();
    actRightToLeft->setChecked(direction=="rtl");

    FOLLOW_CHECK(actBold, QWebPage::ToggleBold);
    FOLLOW_CHECK(actItalic, QWebPage::ToggleItalic);
    FOLLOW_CHECK(actStrikeout, QWebPage::ToggleStrikethrough);
    FOLLOW_CHECK(actUnderline, QWebPage::ToggleUnderline);

//     actStrikeout->setChecked ( queryCommandState ( "strikeThrough" ) );
    actOrderedList->setChecked ( queryCommandState ( "insertOrderedList" ) );
    actUnorderedList->setChecked ( queryCommandState ( "insertUnorderedList" ) );
}

QAction* TextEditor::getAction ( QWebPage::WebAction action ) const
{
    if ( action >= 0 && action <= 66 )
        return webView -> page() -> action ( static_cast<QWebPage::WebAction> ( action ) );
    else
        return 0;
}

void TextEditor::focusInEvent ( QFocusEvent *event )
{
    QWidget::focusInEvent ( event );
    webView -> setFocus();
}

void TextEditor::slotChangeFormatType(const QString &formatText)
{
    if(formatText == i18n("Paragraph")){
        execCommand ( "formatBlock", "p" );
    } else if (formatText == i18n("Heading 1")){
        execCommand ( "formatBlock", "h1" );
    } else if (formatText == i18n("Heading 2")){
        execCommand ( "formatBlock", "h2" );
    } else if (formatText == i18n("Heading 3")){
        execCommand ( "formatBlock", "h3" );
    } else if (formatText == i18n("Heading 4")){
        execCommand ( "formatBlock", "h4" );
    } else if (formatText == i18n("Heading 5")){
        execCommand ( "formatBlock", "h5" );
    } else if (formatText == i18n("Heading 6")){
        execCommand ( "formatBlock", "h6" );
    } else if (formatText == i18n("Pre Formatted")){
        execCommand ( "formatBlock", "pre" );
    }
}

void TextEditor::formatIncreaseIndent()
{
    execCommand ( "indent" );
}

void TextEditor::formatDecreaseIndent()
{
    execCommand ( "outdent" );
}

void TextEditor::slotToggleSpellChecking(bool )
{
    //TODO Add spell checking support!
}

void TextEditor::formatTextColor()
{
    QColor color;
    int res = KColorDialog::getColor(color, this);
    if (res == KColorDialog::Accepted)
        execCommand("foreColor", color.name());
}

QList< BilboMedia* > TextEditor::getLocalImages()
{
    kDebug();
    QList< BilboMedia* > list;
    QWebElementCollection images = webView->page()->mainFrame()->findAllElements("img");
    foreach(const QWebElement& elm, images){
        if(elm.attribute("src").startsWith("file://")){
//             kDebug()<<elm.toOuterXml();
            BilboMedia* media = new BilboMedia(this);
            KUrl mediaUrl (elm.attribute("src"));
            media->setLocalUrl( mediaUrl );
            media->setMimeType( KMimeType::findByUrl( mediaUrl, 0, true )->name() );
            media->setName(mediaUrl.fileName());
            media->setBlogId(__currentBlogId);
            list.append(media);
        }
    }
    return list;
}

void TextEditor::replaceImageSrc(const QString& src, const QString& dest)
{
    QString cmd = QString("replaceImageSrc('%1','%2')").arg(src).arg(dest);
//     kDebug()<<cmd;
    evaluateJavaScript(cmd, true);
//     kDebug()<< var;
    kDebug()<<"Replaced "<<src<<" with "<<dest;
}

static QUrl guessUrlFromString(const QString &string)
{
    QString urlStr = string.trimmed();
    QRegExp test(QLatin1String("^[a-zA-Z]+\\:.*"));

    // Check if it looks like a qualified URL. Try parsing it and see.
    bool hasSchema = test.exactMatch(urlStr);
    if (hasSchema) {
        QUrl url(urlStr, QUrl::TolerantMode);
        if (url.isValid())
            return url;
    }

    // Might be a file.
    if (QFile::exists(urlStr))
        return QUrl::fromLocalFile(urlStr);

    // Might be a shorturl - try to detect the schema.
    if (!hasSchema) {
        int dotIndex = urlStr.indexOf(QLatin1Char('.'));
        if (dotIndex != -1) {
            QString prefix = urlStr.left(dotIndex).toLower();
            QString schema = (prefix == QLatin1String("ftp")) ? prefix : QLatin1String("http");
            QUrl url(schema + QLatin1String("://") + urlStr, QUrl::TolerantMode);
            if (url.isValid())
                return url;
        }
    }

    // Fall back to QUrl's own tolerant parser.
    return QUrl(string, QUrl::TolerantMode);
}

void TextEditor::slotAddLink()
{
    const QString selection = webView->selectedText();
    QPointer<AddEditLink> addLinkDlg = new AddEditLink(this);
    if( addLinkDlg->exec() ){
        Link lnk = addLinkDlg->result();
        QUrl url = guessUrlFromString(lnk.address);
        if(url.isValid()){
            //execCommand( "createLink", url.toString() );
            ///=====
             QString target = lnk.target.isEmpty() ? QString() : QString("target=\'%1\'").arg(lnk.target);
             QString title = lnk.title.isEmpty() ? QString() : QString( "title=\'%1\'").arg(lnk.title);
             QString html = QString ( "<a href=\'%1\' %2 %3>%4</a>" )
                                 .arg ( url.toString() ).arg ( target ).arg( title ).arg ( selection.isEmpty() ? url.toString() : selection );
             //kDebug()<<html;
             execCommand ( "insertHTML", html );
            ///=====
//             kDebug();
//             execCommand( QString("insertLink(%1, %2, %3)").arg(url.toString()).arg(lnk.target).arg(lnk.title) );
        }
    }
    addLinkDlg->deleteLater();
}

void TextEditor::slotRemoveLink()
{
    execCommand("unLink");
}

void TextEditor::slotToggleBlockQuote(bool )
{
    execCommand("formatBlock", "BLOCKQUOTE");
}

void TextEditor::slotToggleCode(bool )
{
    QString selection = webView->selectedText();
    if(selection.isEmpty())
        return;
//     // We have to remove selection before!
    QString html = QString ( "<code>%1</code>" ).arg ( selection );
    execCommand("insertHtml", html);
}

void TextEditor::slotAddImage()
{
    QPointer<AddEditImage> imageDialog = new AddEditImage( this );
    imageDialog->setWindowModality( Qt::WindowModal );
    imageDialog->exec();
    if( imageDialog->result() == KDialog::Accepted){
        QMap<QString, QString> res = imageDialog->selectedMediaProperties();
        QString width = res["width"].toInt() > 0 ? "width=" + res["width"] : QString();
        QString height = res["height"].toInt() > 0 ? "height=" + res["height"] : QString();
        QString title = res["title"].isEmpty() ? QString() : QString("title='%1'").arg(res["title"]);
        QString src = res["url"].isEmpty() ? QString() : QString("src='%1'").arg(res["url"]);
        QString alt = res["alt"].isEmpty() ? QString() : QString("alt='%1'").arg(res["alt"]);
        QString align = res["align"].isEmpty() ? QString() : QString( "align='%1'").arg(res["align"]);
        QString html = QString ( "<img %1 %2 %3 %4 %5 %6 />" )
                                .arg ( width ).arg ( height ).arg( title )
                                .arg( src ).arg( alt ).arg(align);
        if( !res["link"].isEmpty() ){
            QString preHtml = QString("<a href='%1'>").arg(res["link"]);
            html = preHtml + html + "</a>";
        }
        execCommand ( "insertHTML", html );
    }
    imageDialog->deleteLater();
}

void TextEditor::slotAddPostSplitter()
{
    execCommand("insertHTML", "<hr/><!--split-->");
}

void TextEditor::slotChangeLayoutDirection(bool rightToLeft)
{
    if(rightToLeft)
        getAction(QWebPage::SetTextDirectionRightToLeft)->trigger();
    else
        getAction(QWebPage::SetTextDirectionLeftToRight)->trigger();
}

void TextEditor::slotDecreaseFontSize()
{
    setFontSize( getFontSize() - 1 );
}

void TextEditor::slotIncreaseFontSize()
{
    setFontSize( getFontSize() + 1 );
}


QVariant TextEditor::evaluateJavaScript ( const QString &cmd, bool itIsEditting )
{
//   qDebug("TextEditor::evaluateJavaScript: %s", cmd.toAscii().constData());
    QVariant returnValue = webView -> page() -> mainFrame() -> evaluateJavaScript ( cmd );
    if ( itIsEditting )
        somethingEdittedSlot();
    return returnValue;
}

void TextEditor::adjustFontSizes()
{
    //we want to set font sizes like x-large xx-large,... but
    //the function getFontSize() which is called by javaScript always
    //returns in px like 42px.
    //so we create a table of values.
    //There are 7 values: xx-small, x-small, small, medium,
    //large, x-large, xx-large

    //NOTE: test it in other platforms...

    fontSizes << 9 << 11 << 14 << 17 << 21 << 28 << 42;
}

QColor TextEditor::rgbToColor ( QString rgb ) const
{
    rgb.chop ( 1 );
    rgb.remove ( 0, 4 );
    QStringList list = rgb.split ( ',' );
    if ( list.size() == 3 )
        return QColor ( list[0].toInt(), list[1].toInt(), list[2].toInt() );
    else
        return QColor();
}

QString TextEditor::getHtml() const
{
    QString html = const_cast<TextEditor*>( this )->evaluateJavaScript( "getHtml()", false ).toString();
    html.remove(" xmlns=\"http://www.w3.org/1999/xhtml\"", Qt::CaseInsensitive);
    return html;
}


#include "composer/texteditor.moc"
