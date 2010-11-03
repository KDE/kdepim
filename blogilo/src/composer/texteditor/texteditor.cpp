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
#include "utilities.h"
#include <kstandarddirs.h>
#include <KDebug>
#include <ktoolbar.h>
#include <KSelectAction>
#include <klocalizedstring.h>
#include <KColorDialog>
#include <composer/dialogs/addimagedialog.h>
#include <composer/dialogs/addeditlink.h>
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

#define ATTACHMENT_IMAGE "image"
#define FORWARD_ACTION(action1, action2) \
    connect(action1, SIGNAL(triggered()), getAction(action2), SLOT(trigger()));\
    connect(getAction(action2), SIGNAL(changed()), SLOT(adjustActions()));

ObjectsForJavaScript::ObjectsForJavaScript ( TextEditor *_textEditor )
        : QObject ( _textEditor ), textEditor ( _textEditor ) {
}

QString ObjectsForJavaScript::getCss() {
    return QString();//Settings::settings() -> makeCss();
}

QString ObjectsForJavaScript::getARandomName() {
    QString name;
    return Random::random()->getARandomName();
}

QString ObjectsForJavaScript::getObjectPath ( const QString &id ) {
    TextEditorObjectImage *imageObject = dynamic_cast<TextEditorObjectImage*> ( textEditor -> findObject ( id ) );
    if ( imageObject )
        return imageObject -> getFilePath();
    else
        return QString();
}

void ObjectsForJavaScript::clickedOnObject ( const QString &id ) {
    emit clickedOnObjectSignal ( id );
}

void ObjectsForJavaScript::clickedOnNonObject() {
    emit clickedOnNonObjectSignal();
}

//----------------------------------------------------------------------

WebView::WebView ( QWidget *parent )
        : KWebView ( parent ) {
    settings() -> setFontSize ( QWebSettings::DefaultFontSize, 14 );
    page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    setAcceptDrops ( true );
}

void WebView::startEditing() {
    //NOTE: it needs a mouse click!
    //any better way to make the cursor visible?
    this -> setFocus();
    QMouseEvent mouseEventPress ( QEvent::MouseButtonPress, QPoint ( 10,10 ), Qt::LeftButton, Qt::NoButton, Qt::NoModifier );
    QApplication::sendEvent ( this, &mouseEventPress );
    QTimer::singleShot ( 50, this, SLOT ( sendMouseReleaseEvent() ) );
}

void WebView::sendMouseReleaseEvent() {
    QMouseEvent mouseEventRelease ( QEvent::MouseButtonRelease, QPoint ( 10,10 ), Qt::LeftButton, Qt::NoButton, Qt::NoModifier );
    QApplication::sendEvent ( this, &mouseEventRelease );
    pageAction ( QWebPage::MoveToEndOfDocument ) -> trigger();
}

void WebView::focusOutEvent ( QFocusEvent *event ) {
    QWebView::focusOutEvent ( event );
    emit focusOutSignal();
}

void WebView::focusInEvent ( QFocusEvent *event ) {
    QWebView::focusInEvent ( event );
    //NOTE: enabling the codes below will correct the focus behaviours
    //But will make some problems with font size...
//    QMouseEvent mouseEventPress(QEvent::MouseButtonPress, QPoint(1,1), Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
//    QMouseEvent mouseEventRelease(QEvent::MouseButtonRelease, QPoint(1,1), Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
//    QApplication::sendEvent(this, &mouseEventPress);
//    QApplication::sendEvent(this, &mouseEventRelease);
    //  webView -> setFocus();
}

void WebView::keyPressEvent ( QKeyEvent *event ) {
    if ( event->key() == Qt::Key_Return && ( event->modifiers() & Qt::AltModifier ) )
        emit editingFinishKeyPressed();
    else if ( event->key() == Qt::Key_F12 )
        QMessageBox::information ( this, QString(), page()->mainFrame()->toHtml()
                                   .replace ( '<', "&lt;" ).replace ( '>', "&gt;" ).replace ( '\n', "<br>" ) );
    else
        QWebView::keyPressEvent ( event );
}

void WebView::dragEnterEvent ( QDragEnterEvent *event ) {
    if ( event->mimeData()->hasUrls() )
        event->acceptProposedAction();
}

void WebView::dropEvent ( QDropEvent *event ) {
    emit dropMimeDataSignal ( event -> mimeData() );
}

//----------------------------------------------------------------------

TextEditorObject::TextEditorObject ( TextEditor *_textEditor, const QString &_id )
        : textEditor ( _textEditor ) {
    id = _id;
}

void TextEditorObject::setId ( const QString &_id ) {
    id = _id;
}

QString TextEditorObject::getId() const {
    return id;
}

//----------------------------------------------------------------------

TextEditorObjectImage::TextEditorObjectImage ( TextEditor *_textEditor, const QString &_id, const QImage &image )
        : TextEditorObject ( _textEditor, _id ) {
    setImage ( image );
}

TextEditorObjectImage::TextEditorObjectImage ( TextEditor *_textEditor, const QString &_id, const QByteArray &byteArray )
        : TextEditorObject ( _textEditor, _id ) {
    setByteArray ( byteArray );
}

TextEditorObjectImage::~TextEditorObjectImage() {
    removeFile ( filePath );
}

void TextEditorObjectImage::setImage ( const QImage &image, bool replace ) {
    QBuffer buffer;
    QImageWriter writer ( &buffer, "jpg" );
    if ( !writer.write ( image ) )
        qDebug ( "TextEditorObjectImage::setImage(): could not write to buffer!" );

    setByteArray ( buffer.data(), replace );
}

QImage TextEditorObjectImage::getImage() const {
    QImage image;
    if ( !image.load ( filePath ) )
        qDebug ( "TextEditorObjectImage::getImage(): could not load the image" );
    return image;
}

QString TextEditorObjectImage::getType() const {
    return ATTACHMENT_IMAGE;
}

QByteArray TextEditorObjectImage::getByteArray() const {
    QFile file ( filePath );
    if ( !file.open ( QIODevice::ReadOnly ) ) {
        qDebug ( "TextEditorObjectImage::getByteArray(): could not open the file" );
        return QByteArray();
    }

    return file.readAll();
}

void TextEditorObjectImage::setByteArray ( const QByteArray &byteArray, bool replace ) {
    if ( !filePath.isEmpty() )
        removeFile ( filePath );

    QString initialId = getId();
    if ( replace )
        setId ( Random::random() -> getARandomName() );

#ifdef Q_WS_WIN
    filePath = QDir::toNativeSeparators ( QDir::homePath() ) + QDir::separator() + "flashqard";
    QDir().mkpath ( filePath );
    filePath += QDir::separator() + getId();
#else
    filePath = QDir::tempPath() + QDir::separator() + getId();
#endif
    QFile::remove ( filePath ); //just in case it already exists
    QFile file ( filePath );
    if ( !file.open ( QIODevice::WriteOnly ) )
        QMessageBox::warning ( 0, i18n ( "Text Editor" ),
                               i18n ( "Unable to create a temporary file for embedded object: %1" ).arg ( getId() ) );

    file.write ( byteArray );


    textEditor -> evaluateJavaScript ( QString ( "replaceId(\"%1\", \"%2\")" ).arg ( initialId ).arg ( getId() ), false );
    textEditor -> evaluateJavaScript ( "adjustImagesPath()", false );
    emit modifiedSignal();
}

QString TextEditorObjectImage::getFilePath() const {
    return filePath;
}

void TextEditorObjectImage::removeFile ( const QString &file ) {
    if ( !QFile::remove ( file ) )
        qDebug ( "TextEditorObjectImage::~TextEditorObjectImage(): could not remove file %s", file.toAscii().constData() );
}

//----------------------------------------------------------------------

TextEditor::TextEditor ( QWidget *parent )
        : QWidget ( parent ) {
    objectsAreModified = false;
    readOnly = false;
    objForJavaScript = new ObjectsForJavaScript ( this );
    connect ( objForJavaScript, SIGNAL ( clickedOnObjectSignal ( const QString& ) ), this, SLOT ( clickedOnObjectSlot ( const QString& ) ) );
    connect ( objForJavaScript, SIGNAL ( clickedOnNonObjectSignal() ), this, SLOT ( clickedOnNonObjectSlot() ) );
    webView = new WebView ( this );
    connect ( webView->page()->mainFrame(), SIGNAL ( javaScriptWindowObjectCleared() ),
              this, SLOT ( addJavaScriptObjectSlot() ) );
    QFile file ( KStandardDirs::locate ( "data", "blogilo/TextEditorInitialHtml" ) );
    kDebug() <<file.fileName();
    if ( !file.open ( QIODevice::ReadOnly ) )
        QMessageBox::warning ( 0, i18n ( "TextEditor" ),
                               i18n ( "TextEditor: Can not open template file." ) );
    else
        webView -> setHtml ( QString ( file.readAll() ) );

    connect ( webView->page(), SIGNAL ( selectionChanged() ), this, SLOT ( adjustActions() ) );
    connect ( webView->page(), SIGNAL ( selectionChanged() ), this, SIGNAL ( selectionChanged() ) );
    connect ( webView->page(), SIGNAL ( contentsChanged() ), this, SLOT ( somethingEdittedSlot() ) );
    connect ( webView->page(), SIGNAL ( contentsChanged() ), this, SIGNAL ( contentsChanged() ) );
//     connect ( webView, SIGNAL ( focusOutSignal() ), this, SLOT ( finishEditing() ) );
    connect ( webView, SIGNAL ( editingFinishKeyPressed() ), this, SIGNAL ( editingFinishKeyPressed() ) );
    connect ( webView, SIGNAL ( dropMimeDataSignal ( const QMimeData* ) ), this, SLOT ( dropMimeDataSlot ( const QMimeData* ) ) );
//   webView->page()->action(QWebPage::Reload) -> setShortcut(Qt::Key_F5);

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

void TextEditor::addJavaScriptObjectSlot()
{
    webView->page()->mainFrame()->addToJavaScriptWindowObject ( "objFromCpp", objForJavaScript );
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

void TextEditor::clickedOnObjectSlot ( const QString &id )
{
    emit clickedOnObjectSignal ( findObject ( id ) );
}

void TextEditor::clickedOnNonObjectSlot()
{

    emit clickedOnNonObjectSignal();
}

void TextEditor::anObjectIsModifiedSlot() {
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
    connect( actCheckSpelling, SIGNAL( triggered( bool ) ), this,
             SLOT( slotToggleSpellChecking(bool) ) );
    connect( actCheckSpelling, SIGNAL(triggered(bool)), SLOT(getMediaList()));
    barVisual->addAction( actCheckSpelling );

    barVisual->addSeparator();
    actBold = new KAction( KIcon( "format-text-bold" ), i18nc(
                          "Makes text bold, and its shortcut is (Ctrl+b)",
                          "Bold (Ctrl+b)" ), this );
    actBold->setShortcut( Qt::CTRL + Qt::Key_B );
    actBold->setCheckable( true );
    FORWARD_ACTION(actBold, QWebPage::ToggleBold);
//     connect( actBold, SIGNAL( triggered( bool ) ), this, SLOT( sltSetTextBold( bool ) ) );
    barVisual->addAction( actBold );

    actItalic = new KAction( KIcon( "format-text-italic" ), i18nc(
                            "Makes text italic, and its shortcut is (Ctrl+i)",
                            "Italic (Ctrl+i)" ), this );
    actItalic->setShortcut( Qt::CTRL + Qt::Key_I );
    actItalic->setCheckable( true );
    FORWARD_ACTION(actItalic, QWebPage::ToggleItalic);
//     connect( actItalic, SIGNAL( triggered( bool ) ), editor, SLOT( setTextItalic( bool ) ) );
    barVisual->addAction( actItalic );

    actUnderline = new KAction( KIcon( "format-text-underline" ), i18nc(
                               "Makes text underlined, and its shortcut is (Ctrl+u)",
                               "Underline (Ctrl+u)" ), this );
    actUnderline->setShortcut( Qt::CTRL + Qt::Key_U );
    actUnderline->setCheckable( true );
    FORWARD_ACTION(actUnderline, QWebPage::ToggleUnderline);
//     connect( actUnderline, SIGNAL( triggered( bool ) ), editor, SLOT( setTextUnderline( bool ) ) );
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
    actCode->setCheckable( true );
    connect( actCode, SIGNAL( triggered( bool ) ), this, SLOT( slotToggleCode(bool) ) );
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
    actFormatType->setMaxComboViewCount( 3 );
    actFormatType->setCurrentAction( i18n( "Paragraph" ) );
    connect( actFormatType, SIGNAL( triggered( const QString& ) ),
             this, SLOT(slotChangeFormatType(QString)) );
    barVisual->addAction( actFormatType );

    actFontIncrease = new KAction( KIcon( "format-font-size-more" ), i18n( "Increase font size" ), this );
    connect( actFontIncrease, SIGNAL( triggered( bool ) ), this, SLOT( slotIncreaseFontSize()) );
    barVisual->addAction( actFontIncrease );

    actFontDecrease = new KAction( KIcon( "format-font-size-less" ), i18n( "Decrease font size" ), this );
    connect( actFontDecrease, SIGNAL( triggered( bool ) ), this, SLOT( slotDecreaseFontSize()) );
    barVisual->addAction( actFontDecrease );

    actColorSelect = new KAction( KIcon( "format-text-color" ),
                                  i18nc( "verb, to select text color", "Select Color" ), this );
    connect( actColorSelect, SIGNAL( triggered( bool ) ), this, SLOT(formatTextColor()) );
    barVisual->addAction( actColorSelect );

    actRemoveFormatting = new KAction( KIcon( "draw-eraser" ), i18n(
                                       "Remove formatting" ), this );
    FORWARD_ACTION(actRemoveFormatting, QWebPage::RemoveFormat);
    barVisual->addAction( actRemoveFormatting );

    actBlockQuote = new KAction( KIcon( "format-text-blockquote" ), i18n( "Blockquote" ), this );
    actBlockQuote->setCheckable( true );
    connect( actBlockQuote, SIGNAL( triggered( bool ) ), this, SLOT(slotToggleBlockQuote(bool)) );
    barVisual->addAction( actBlockQuote );

    barVisual->addSeparator();

    actAddLink = new KAction( KIcon( "insert-link" ), i18nc(
                             "verb, to add a new link or edit an existing one",
                             "Add/Edit Link" ), this );
    connect( actAddLink, SIGNAL( triggered( bool ) ), this, SLOT(slotAddEditLink()));
    barVisual->addAction( actAddLink );

    actRemoveLink = new KAction( KIcon( "remove-link" ), i18nc(
                                "verb, to remove an existing link",
                                "Remove Link" ), this );
    connect( actRemoveLink, SIGNAL( triggered( bool ) ), this, SLOT(slotRemoveLink()));
    barVisual->addAction( actRemoveLink );

    actAddImage = new KAction( KIcon( "insert-image" ), i18nc( "verb, to insert an image",
                               "Add Image" ), this );
    connect( actAddImage, SIGNAL( triggered( bool ) ), this, SLOT(slotAddImage()));
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
    connect( actRightToLeft, SIGNAL( toggled(bool) ), this, SLOT(slotChangeLayoutDirection(bool)) );
    barVisual->addAction( actRightToLeft );

    barVisual->addSeparator();

    actOrderedList = new KAction( KIcon( "format-list-ordered" ), i18n( "Ordered List" ), this );
    FORWARD_ACTION(actOrderedList, QWebPage::InsertOrderedList);
    barVisual->addAction( actOrderedList );

    actUnorderedList = new KAction( KIcon( "format-list-unordered" ), i18n( "Unordered List" ), this );
    FORWARD_ACTION(actUnorderedList, QWebPage::InsertUnorderedList);
    barVisual->addAction( actUnorderedList );

    actSplitPost = new KAction( KIcon( "insert-more-mark" ), i18n( "Split text" ), this );
    connect( actSplitPost, SIGNAL( triggered( bool ) ), this, SLOT(slotAddPostSplitter()) );
    barVisual->addAction( actSplitPost );
}

/*
void TextEditor::setDocument(const Document &newDocument)
{
   document = newDocument;
   objectsAreModified = false;
   clearObjects();
   webView -> page() -> undoStack() -> clear();
   addDocumentAttachmentsToObjects();
   evaluateJavaScript(QString("replaceHtml(\"%1\")").arg(document.getText().replace("\"", "\\\"").simplified()), true);
   updateStyle();
}

const Document& TextEditor::getDocument()
{
   document.setText(getHtml());
   objectsAreModified = false;

   addUsedAttachmentsToDocument();
   //undo stack must get cleared otherwise deletion of an image can be
   //undone while the image doesn't really exist in the attachments of
   //the document.
   webView -> page() -> undoStack() -> clear();
   return document;
}*/

void TextEditor::reload()
{
    getAction ( QWebPage::Reload ) -> trigger();
}

void TextEditor::setCurrentTitle ( const QString &title )
{

}

QString TextEditor::htmlContent()
{
    return getHtml();
}

void TextEditor::setHtmlContent ( const QString& arg1 )
{
    QString txt = arg1;
    txt = txt.replace('\"', "\\\"").simplified();
    evaluateJavaScript(QString("replaceHtml(\"%1\")").arg(txt), true);
}

bool TextEditor::updateMediaPaths()
{
    return true;
}

void TextEditor::clear() {
//    setDocument(Document());
}

void TextEditor::startEditing()
{
    webView -> startEditing();
}

/*
void TextEditor::finishEditing()
{
   QString currentHtml = getHtml();
   if ((document.getText() != currentHtml || objectsAreModified) && this->isEnabled() && !readOnly)
   {
      document.setText(currentHtml);
      emit editingFinished();
   }
}*/

bool TextEditor::insertImage ( const QString &imageUrl ) {
    QByteArray byteArray = loadImage ( imageUrl );
    return insertImage ( byteArray );
}

bool TextEditor::insertImage ( const QByteArray &byteArray ) {
    //first just test whether it is a valid image or not
    QImage image;
    if ( !image.loadFromData ( byteArray ) ) {
        QMessageBox::warning ( this, tr ( "Insert Image" ),
                               tr ( "Inserting image failed." ) );
        return false;
    }

    //resize if it is bigger than 700x500
//     if ( image.size().width() > 700 || image.size().height() > 500 )
//         image = image.scaled ( QSize ( 700, 500 ), Qt::KeepAspectRatio, Qt::SmoothTransformation );

    QString id = Random::random() -> getARandomName();
    TextEditorObjectImage *imageObject = new TextEditorObjectImage ( this, id, image );
    connect ( imageObject, SIGNAL ( modifiedSignal() ), this, SLOT ( anObjectIsModifiedSlot() ) );
    objects << imageObject;
    QString path = imageObject -> getFilePath();
#ifdef Q_WS_WIN
    path.replace ( '\\', "\\\\" );
#endif
    QString html = QString ( "<img id=\\\"%1\\\" src=\\\"%2\\\" />" ).arg ( id ).arg ( path );
    execCommand ( "insertHTML", html );

    //simulate a click to activate image editor (in advanced text editor)
    emit clickedOnObjectSignal ( imageObject );
    return true;
}

QByteArray TextEditor::loadImage ( const QString &imagePath ) {
    QFile file ( imagePath );
    file.open ( QIODevice::ReadOnly );
    return file.readAll();
}

// void TextEditor::setFontFamily ( const QString &family ) {
//     execCommand ( "fontName", family );
// }

// QString TextEditor::getFontFamily() const {
//     return const_cast<TextEditor*> ( this ) -> evaluateJavaScript ( "getFontFamily()", false ).toString();
// }

void TextEditor::setFontSize ( int fontSize ) {
    //setFontSize() function requires a number in the range 0-6
    //But the function below requires a number in the range 1-7
    execCommand ( "fontSize", QString::number ( fontSize+1 ) );
}

int TextEditor::getFontSize() const {
    QString fontSizeString = const_cast<TextEditor*> ( this ) -> evaluateJavaScript ( "getFontSize()",
                                                                                      false ).toString();

    fontSizeString.chop ( 2 );
    int fontSizeInt = fontSizeString.toInt();

    fontSizeInt = ceil( static_cast<double> ( fontSizeInt ) /getZoomFactor() - 0.49 );

    return fontSizes.indexOf ( fontSizeInt );
}

/*
void TextEditor::setForegroundColor ( const QColor &color ) {
    execCommand ( "foreColor", color.name() );
}

QColor TextEditor::getForegroundColor() const {
    return rgbToColor ( const_cast<TextEditor*> ( this ) -> evaluateJavaScript ( "getForegroundColor()", false ).toString() );
}

void TextEditor::setBackgroundColor ( const QColor &color ) {
    execCommand ( "backColor", color.name() );
}

QColor TextEditor::getBackgroundColor() const {
    return rgbToColor ( const_cast<TextEditor*> ( this ) -> evaluateJavaScript ( "getBackgroundColor()", false ).toString() );
}

void TextEditor::setPageBackgroundColor ( const QColor &color ) {
    evaluateJavaScript ( QString ( "setPageBackgroundColor(\"%1\")" ).arg ( color.name() ), false );
}

QColor TextEditor::getPageBackgroundColor() const {
    return rgbToColor ( const_cast<TextEditor*> ( this ) -> evaluateJavaScript ( "getPageBackgroundColor()", false ).toString() );
}


void TextEditor::setAlignment ( Qt::Alignment alignment ) {
    if ( alignment&Qt::AlignLeft )
        formatAlignLeft();
    else if ( alignment&Qt::AlignRight )
        formatAlignRight();
    else if ( alignment&Qt::AlignCenter )
        formatAlignCenter();
    else if ( alignment&Qt::AlignJustify )
        formatAlignJustify();
}
*/

void TextEditor::setZoomFactor ( double zoom ) {
    webView -> setTextSizeMultiplier ( zoom );
}

double TextEditor::getZoomFactor() const {
    return webView -> textSizeMultiplier();
}

Qt::Alignment TextEditor::getAlignment() const {
    QString alignment = const_cast<TextEditor*> ( this ) -> evaluateJavaScript ( "getAlignment()", false ).toString();
    return alignment == "left" ? Qt::AlignLeft
           : alignment == "right" ? Qt::AlignRight
           : alignment == "center" ? Qt::AlignCenter
           : Qt::AlignJustify;
}

void TextEditor::execCommand ( const QString &cmd ) {
//   qDebug("TextEditor::execCommand(%s)", cmd.toAscii().constData());
    QWebFrame *frame = webView->page()->mainFrame();
    QString js = QString ( "document.execCommand(\"%1\", false, null)" ).arg ( cmd );
    frame->evaluateJavaScript ( js );
}

void TextEditor::execCommand ( const QString &cmd, const QString &arg ) {
//   qDebug("TextEditor::execCommand(%s, %s) began", cmd.toAscii().constData(), arg.toAscii().constData());
    QWebFrame *frame = webView->page()->mainFrame();
    QString js = QString ( "document.execCommand(\"%1\", false, \"%2\")" ).arg ( cmd ).arg ( arg );
    frame->evaluateJavaScript ( js );
}

bool TextEditor::queryCommandState ( const QString &cmd ) {
    QWebFrame *frame = webView->page()->mainFrame();
    QString js = QString ( "document.queryCommandState(\"%1\", false, null)" ).arg ( cmd );
    QVariant result = frame->evaluateJavaScript ( js );
    return result.toString().simplified().toLower() == "true";
}

#define FOLLOW_CHECK(a1, a2) a1->setChecked(getAction(a2)->isChecked())

void TextEditor::adjustActions() {
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

QAction* TextEditor::getAction ( QWebPage::WebAction action ) const {
    if ( action >= 0 && action <= 66 )
        return webView -> page() -> action ( static_cast<QWebPage::WebAction> ( action ) );
    else
        return 0;
}

void TextEditor::addCommand ( QUndoCommand *command ) {
    webView -> page() -> undoStack() -> push ( command );
}

void TextEditor::focusInEvent ( QFocusEvent *event ) {
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

void TextEditor::formatIncreaseIndent() {
    execCommand ( "indent" );
}

void TextEditor::formatDecreaseIndent() {
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
    QVariant json = evaluateJavaScript("getLocalImages()", false);
    kDebug()<<json.toByteArray();
    QVariantList parsedList = parser.parse(json.toByteArray()).toList();
    foreach(const QVariant& var, parsedList){
        BilboMedia* media = new BilboMedia(this);
        KUrl mediaUrl (var.toMap().value("src").toString());
        media->setLocalUrl( mediaUrl );
        media->setMimeType( KMimeType::findByUrl( mediaUrl, 0, true )->name() );
        media->setName(mediaUrl.fileName());
        media->setBlogId(__currentBlogId);
        list.append(media);
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

void TextEditor::slotAddEditLink()
{
    QPointer<AddEditLink> addLinkDlg = new AddEditLink(this);
    if( addLinkDlg->exec() ){
        Link lnk = addLinkDlg->result();
        QUrl url = guessUrlFromString(lnk.address);
        if(url.isValid()){
            execCommand("createLink", url.toString());
            //TODO use other parameters
        }
    }
}

void TextEditor::slotRemoveLink()
{
///TODO
}

void TextEditor::slotToggleBlockQuote(bool )
{
//     QString selection = webView->selectedText();
//     if(selection.isEmpty())
//         return;
//     kDebug()<<"NOT IMPLEMENTED";
//     return;
//     // We have to remove selection before!
//     QString html = QString ( "<blockquote>%1</blockquote>" ).arg ( selection );
//     execCommand("insertHtml", html);
}

void TextEditor::slotToggleCode(bool )
{
//     QString selection = webView->selectedText();
//     if(selection.isEmpty())
//         return;
//     kDebug()<<"NOT IMPLEMENTED";
//     return;
//     // We have to remove selection before!
//     QString html = QString ( "<code>%1</code>" ).arg ( selection );
//     execCommand("insertHtml", html);
}

void TextEditor::slotAddImage()
{
    QPointer<AddImageDialog> imageDialog = new AddImageDialog( this );
    imageDialog->setWindowModality( Qt::WindowModal );
    imageDialog->exec();
    if( imageDialog->result() == KDialog::Accepted){
        QVariantMap res = imageDialog->selectedMediaProperties();
        QString width = res["width"].toInt() > 0 ? "width=" + res["width"].toString() : QString();
        QString height = res["height"].toInt() > 0 ? "height=" + res["height"].toString() : QString();
        QString title = res["title"].toString().isEmpty() ? QString() : QString("title='%1'").arg(res["title"].toString());
        QString src = res["url"].toString().isEmpty() ? QString() : QString("src='%1'").arg(res["url"].toString());
        QString alt = res["alt"].toString().isEmpty() ? QString() : QString("alt='%1'").arg(res["alt"].toString());
        QString html = QString ( "<img %1 %2 %3 %4 %5 />" )
                                .arg ( width ).arg ( height ).arg( title )
                                .arg( src ).arg( alt );
        if( !res["link"].toString().isEmpty() ){
            QString preHtml = QString("<a href='%1'>").arg(res["link"].toString());
            html = preHtml + html + "</a>";
        }
        execCommand ( "insertHTML", html );
    }
}

void TextEditor::slotAddPostSplitter()
{
//TODO
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
    QString html = const_cast<TextEditor*> ( this ) -> evaluateJavaScript ( "getHtml()", false ).toString();
//     HtmlParser *htmlParser = HtmlParser::htmlParser();
//     int iterator = 0;
//     while ( htmlParser->setTagAttribute ( html, iterator, "img", "src", "" ) );
    return html;
}

// QString TextEditor::addObject(const QByteArray &byteArray, const QString &id)
// {
//    QFile file (QDir::tempPath() + QDir::separator() + id);
//    if (!file.open(QIODevice::WriteOnly))
//    {
//       QMessageBox::warning(0, i18n("Text Editor"),
//                            i18n("Unable to create a temporary file for embedded object: %1").arg(id));
//       return false;
//    }
//    file.write(byteArray);
//    QFileInfo info(file);
//    objects[id] = info.filePath();

//    return info.filePath();
// }

// QString TextEditor::getObjectPath(const QString &id) const
// {
//    return objects[id];
// }

void TextEditor::clearObjects() {
    while ( objects.size() )
        delete objects.takeAt ( 0 );

//    QStringList list = objects.values();
//    for (int i=0; i<list.size(); i++)
//       if (!QFile::remove(list[i]))
//          qDebug("TextEditor::clearObjects(): could not remove file: %s", list[i].toAscii().constData());
//    objects.clear();
}
/*
void TextEditor::addDocumentAttachmentsToObjects()
{
   for (int i=0; i<document.attachmentCount(); i++)
   {
      if (document.getAttachmentType(i) == ATTACHMENT_IMAGE)
      {
         TextEditorObjectImage *imageObject = new TextEditorObjectImage(this, document.getAttachmentName(i),
                                                                        document.attachmentToByteArray(i));
         connect(imageObject, SIGNAL(modifiedSignal()), this, SLOT(anObjectIsModifiedSlot()));
         objects << imageObject;
      }
   }
}*/

// void TextEditor::removeUnusedAttachments()
// {
//    QStringList ids = evaluateJavaScript("getAllImagesId()", false).toString().split(",", QString::SkipEmptyParts);

//    for (int i=0; i<document.attachmentCount(); i++)
//       if (!ids.contains(document.getAttachmentName(i)))
//          document.deleteAttachment(i--);
// }

TextEditorObject* TextEditor::findObject ( const QString &id ) {
    for ( int i=0; i<objects.size(); i++ )
        if ( objects[i] -> getId() == id )
            return objects[i];

    qDebug ( "TextEditor::findObject(): returning 0" );
    return 0;
}

/*
void TextEditor::addUsedAttachmentsToDocument()
{
   document.deleteAllAttachments();

   QStringList usedIds = evaluateJavaScript("getAllImagesId()", false).toString().split(",", QString::SkipEmptyParts);

   for (int i=0; i<objects.size(); i++)
   {
      if (usedIds.contains(objects[i]->getId()))
      {
         document.addAttachment(objects[i]->getByteArray(), objects[i] -> getId(), objects[i] -> getType());
         qDebug("TextEditor::addUsedAttachmentsToDocument(): adding object %s", objects[i]->getId().toAscii().constData());
      }
   }
}*/


//--------------------------------------------------------------------------------

CommandResizeImage::CommandResizeImage ( TextEditorObjectImage *_imageObject, const QSize &_size )
        : imageObject ( _imageObject ), newSize ( _size ) {
    setText ( i18n ( "Resize image" ) );
    initialImage = imageObject -> getImage();
}

void CommandResizeImage::undo() {
    imageObject -> setImage ( initialImage, true );
}

void CommandResizeImage::redo() {
    imageObject -> setImage ( initialImage.scaled ( newSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation ), true );
}

//----------------------------------------------------------------------

CommandFlipImage::CommandFlipImage ( TextEditorObjectImage *_imageObject, Qt::Orientation _orientation )
        : imageObject ( _imageObject ), orientation ( _orientation ) {
    if ( orientation == Qt::Vertical )
        setText ( i18n ( "Flip image" ) );
    else
        setText ( i18n ( "Mirror image" ) );
}

void CommandFlipImage::undo() {
    redo();
}

void CommandFlipImage::redo() {
    imageObject -> setImage ( imageObject->getImage().mirrored ( orientation==Qt::Horizontal, orientation==Qt::Vertical ), true );
}

//----------------------------------------------------------------------

CommandRotateImage::CommandRotateImage ( TextEditorObjectImage *_imageObject, Rotation _rotation )
        : imageObject ( _imageObject ), rotation ( _rotation ) {
    if ( rotation == RotateRight )
        setText ( i18n ( "Rotate image right" ) );
    else
        setText ( i18n ( "Rotate image left" ) );
}

void CommandRotateImage::undo() {
    QMatrix matrix;
    matrix.rotate ( rotation==RotateRight? -90 : 90 );
    imageObject -> setImage ( imageObject->getImage().transformed ( matrix, Qt::SmoothTransformation ), true );
}

void CommandRotateImage::redo() {
    QMatrix matrix;
    matrix.rotate ( rotation==RotateRight? 90 : -90 );
    imageObject -> setImage ( imageObject->getImage().transformed ( matrix, Qt::SmoothTransformation ), true );
}

// ChangeTextDirectionCommand::ChangeTextDirectionCommand(TextEditor *_textEditor, Qt::LayoutDirection _direction)
//    : textEditor(_textEditor), direction(_direction)
// {
//    setText(i18n("Change text direction"));
// }

// void ChangeTextDirectionCommand::undo()
// {
//    qDebug("ChangeTextDirectionCommand::undo()");
//    textEditor -> evaluateJavaScript(QString("setTextDirection(\"%1\", \"%2\")")
//                                     .arg(direction==Qt::LeftToRight?"rtl":"ltr").arg(tagId), true);
// }

// void ChangeTextDirectionCommand::redo()
// {
//    qDebug("ChangeTextDirectionCommand::redo()");
//    tagId = textEditor -> evaluateJavaScript(QString("setTextDirection(\"%1\", \"%2\")")
//                                             .arg(direction==Qt::LeftToRight?"ltr":"rtl").arg(tagId), true).toString();
// }


#include "texteditor.moc"
