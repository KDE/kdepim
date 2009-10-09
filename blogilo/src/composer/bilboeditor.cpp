/*
    This file is part of Blogilo, A KDE Blogging Client

    Copyright (C) 2008-2009 Mehrdad Momeny <mehrdad.momeny@gmail.com>
    Copyright (C) 2008-2009 Golnaz Nilieh <g382nilieh@gmail.com>

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

#include "bilboeditor.h"

#include <QTextCharFormat>
#include <klocalizedstring.h>
#include <ktoolbar.h>
#include <kselectaction.h>

#include <kicon.h>
#include <kcolordialog.h>
#include <kdebug.h>

#include <kseparator.h>
#include <ktexteditor/view.h>
#include <ktexteditor/document.h>


#include "bilbomedia.h"
#include "bilboblog.h"
#include "bilbopost.h"

#include "multilinetextedit.h"
#include "medialistwidget.h"
#include "stylegetter.h"
#include "htmleditor.h"
#ifdef WIN32
#include "bilbobrowser_win.h"
#else
#include "bilbobrowser.h"
#endif

#include "dialogs/addeditlink.h"
#include "dialogs/addimagedialog.h"

#include "htmlconvertors/bilbotextformat.h"
#include "htmlconvertors/bilbotexthtmlimporter.h"
#include "htmlconvertors/htmlexporter.h"
#include <settings.h>

BilboEditor::BilboEditor( QWidget *parent )
        : KTabWidget( parent )
{
    createUi();
    connect( editor, SIGNAL( textChanged() ), this, SIGNAL( textChanged() ) );
    connect( htmlEditor->document(), SIGNAL( textChanged( KTextEditor::Document * ) ),
             this, SIGNAL( textChanged() ) );
    connect( Settings::self(), SIGNAL(configChanged()),
             this, SLOT(slotSettingsChanged()) );
    editor->setCheckSpellingEnabled( Settings::enableCheckSpelling() );
    editor->setFocus();
}

BilboEditor::~BilboEditor()
{
    kDebug();
}

void BilboEditor::createUi()
{
    ///this:
    this->resize( 600, 400 );
    tabVisual = new QWidget( this );
    tabHtml = new QWidget( this );
    tabPreview = new QWidget( this );
    this->addTab( tabVisual, i18nc( "Software", "Visual Editor" ) );
    this->addTab( tabHtml, i18nc( "Software", "Html Editor" ) );
    this->addTab( tabPreview, i18nc( "preview of the edited post", "Post Preview" ) );
    connect( this, SIGNAL( currentChanged( int ) ), this, SLOT( sltSyncEditors( int ) ) );
    prev_index = 0;

    /// Visual editor:
    editor = new MultiLineTextEdit( tabVisual );
    editor->enableFindReplace( true );
    connect( editor, SIGNAL( sigRemoteImageArrived( const KUrl ) ), this, 
             SLOT( sltReloadImage( const KUrl ) ) );
    connect( editor, SIGNAL( sigMediaTypeFound( BilboMedia* ) ), this, 
             SLOT( sltMediaTypeFound( BilboMedia* ) ) );

    barVisual = new KToolBar( tabVisual );
    barVisual->setIconSize( QSize( 22, 22 ) );
    barVisual->setToolButtonStyle( Qt::ToolButtonIconOnly );

    QLabel *label = new QLabel( i18n( "Media list:" ), tabVisual );
    label->setMaximumHeight( 30 );

    lstMediaFiles = new MediaListWidget( tabVisual );
    lstMediaFiles->setViewMode( QListView::IconMode );
    lstMediaFiles->setIconSize( QSize( 32, 32 ) );
    lstMediaFiles->setGridSize( QSize( 60, 48 ) );
    lstMediaFiles->setDragDropMode( QAbstractItemView::NoDragDrop );
    lstMediaFiles->setResizeMode( QListView::Adjust );
    lstMediaFiles->setMaximumHeight( 60 );
    connect( lstMediaFiles, SIGNAL( sigSetProperties( const int, const int,
                                    const int, const QString, const QString, const QString ) ), 
            this, SLOT( sltSetImageProperties( const int, const int, const int, 
                        const QString, const QString, const QString ) ) );
    connect( lstMediaFiles, SIGNAL( sigRemoveMedia( const int ) ), this, SLOT( sltRemoveMedia( const int ) ) );

    QVBoxLayout *vLayout = new QVBoxLayout( tabVisual );
    vLayout->addWidget( barVisual );
    vLayout->addWidget( editor );
    vLayout->addWidget( label );
    vLayout->addWidget( lstMediaFiles );

    connect( editor, SIGNAL( checkSpellingChanged( bool ) ), this, SLOT( sltSyncSpellCheckingButton( bool ) ) );

    connect( editor, SIGNAL(currentBlockFormatChanged(QTextBlockFormat)),
             this, SLOT(slotCurrentBlockFormatChanged(QTextBlockFormat)) );
    connect( editor, SIGNAL(currentCharFormatChanged(QTextCharFormat)),
             this, SLOT(slotCurrentCharFormantChanged(QTextCharFormat)) );
//     connect( editor, SIGNAL( cursorPositionChanged() ), this, SLOT( sltSyncToolbar() ) );

    ///htmlEditor:
    htmlEditor = HtmlEditor::self()->createView( tabHtml );
    QGridLayout *hLayout = new QGridLayout( tabHtml );
    hLayout->addWidget( htmlEditor );

    ///preview:
    preview = new BilboBrowser( tabPreview );
    QGridLayout *gLayout = new QGridLayout( tabPreview );
    gLayout->addWidget( preview );

    connect( preview, SIGNAL( sigSetBlogStyle() ), this, SLOT( 
            sltSetPostPreview() ) );


    this->setCurrentIndex( 0 );

    currentPostTitle = i18n( "Post Title" );

    QPalette palette = QApplication::palette();
    codeBackground = palette.color( QPalette::Active, QPalette::Midlight );

    ///defaultCharFormat
    defaultCharFormat = editor->currentCharFormat();

    const QFont defaultFont = editor->document()->defaultFont();
    defaultCharFormat.setFont( defaultFont );
    defaultCharFormat.setForeground( editor->currentCharFormat().foreground() );
    defaultCharFormat.setProperty( QTextFormat::FontSizeAdjustment, QVariant( 0 ) );
    defaultCharFormat.setBackground( palette.color( QPalette::Active,
                                                    QPalette::Base ) );
    defaultCharFormat.setProperty( BilboTextFormat::HasCodeStyle, QVariant( false ) );

    ///defaultBlockFormat
    defaultBlockFormat = editor->textCursor().blockFormat();

    createActions();
}

void BilboEditor::createActions()
{
    actCheckSpelling = new KAction( KIcon( "tools-check-spelling" ),
                                    i18n( "Enable Spell Checking"), this );
    actCheckSpelling->setCheckable( true );
    connect( actCheckSpelling, SIGNAL( triggered( bool ) ), this, 
             SLOT( sltToggleSpellChecking() ) );
    barVisual->addAction( actCheckSpelling );

    barVisual->addSeparator();

    actBold = new KAction( KIcon( "format-text-bold" ), i18nc( 
                          "Makes text bold, and its shortcut is (Ctrl+b)", 
                          "Bold (Ctrl+b)" ), this );
    actBold->setShortcut( Qt::CTRL + Qt::Key_B );
    actBold->setCheckable( true );
    connect( actBold, SIGNAL( triggered( bool ) ), this, SLOT( sltSetTextBold( bool ) ) );
    barVisual->addAction( actBold );

    actItalic = new KAction( KIcon( "format-text-italic" ), i18nc( 
                            "Makes text italic, and its shortcut is (Ctrl+i)",
                            "Italic (Ctrl+i)" ), this );
    actItalic->setShortcut( Qt::CTRL + Qt::Key_I );
    actItalic->setCheckable( true );
    connect( actItalic, SIGNAL( triggered( bool ) ), editor, SLOT( setTextItalic( bool ) ) );
    barVisual->addAction( actItalic );

    actUnderline = new KAction( KIcon( "format-text-underline" ), i18nc( 
                               "Makes text underlined, and its shortcut is (Ctrl+u)",
                               "Underline (Ctrl+u)" ), this );
    actUnderline->setShortcut( Qt::CTRL + Qt::Key_U );
    actUnderline->setCheckable( true );
    connect( actUnderline, SIGNAL( triggered( bool ) ), editor, SLOT( setTextUnderline( bool ) ) );
    barVisual->addAction( actUnderline );

    actStrikeout = new KAction( KIcon( "format-text-strikethrough" ), i18nc(
                                "Strikes the text out, and its shortcut is (Ctrl+l)",
                                "Strike out (Ctrl+l)" ), this );
    actStrikeout->setShortcut( Qt::CTRL + Qt::Key_L );
    actStrikeout->setCheckable( true );
    connect( actStrikeout, SIGNAL( triggered( bool ) ), editor, SLOT( setTextStrikeOut( bool ) ) );
    barVisual->addAction( actStrikeout );

    actCode = new KAction( KIcon( "format-text-code" ), i18nc( "Sets text font to code style",
                           "Code" ), this );
    actCode->setCheckable( true );
    connect( actCode, SIGNAL( triggered( bool ) ), this, SLOT( sltToggleCode() ) );
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
    actFormatType->setItems( formatTypes );
    actFormatType->setMaxComboViewCount( 3 );
    actFormatType->setCurrentAction( i18n( "Paragraph" ) );
    connect( actFormatType, SIGNAL( triggered( const QString& ) ), this, SLOT( 
             sltChangeFormatType( const QString& ) ) );
    barVisual->addAction( actFormatType );

    actFontIncrease = new KAction( KIcon( "format-font-size-more" ), i18n( "Increase font size" ), this );
    connect( actFontIncrease, SIGNAL( triggered( bool ) ), this, SLOT( sltFontSizeIncrease() ) );
    barVisual->addAction( actFontIncrease );

    actFontDecrease = new KAction( KIcon( "format-font-size-less" ), i18n( "Decrease font size" ), this );
    connect( actFontDecrease, SIGNAL( triggered( bool ) ), this, SLOT( sltFontSizeDecrease() ) );
    barVisual->addAction( actFontDecrease );

    actColorSelect = new KAction( KIcon( "format-text-color" ), i18nc( "verb, to select text color", "Select Color" ), this );
    connect( actColorSelect, SIGNAL( triggered( bool ) ), this, SLOT( sltSelectColor() ) );
    barVisual->addAction( actColorSelect );

    actRemoveFormatting = new KAction( KIcon( "draw-eraser" ), i18n( 
                                       "Remove formatting" ), this );
//     actRemoveFormatting->setShortcut( Qt::CTRL + Qt::Key_R );
    connect( actRemoveFormatting, SIGNAL( triggered( bool ) ), this, SLOT( sltRemoveFormatting() ) );
    barVisual->addAction( actRemoveFormatting );

    actBlockQuote = new KAction( KIcon( "format-text-blockquote" ), i18n( "Blockquote" ), this );
    actBlockQuote->setCheckable( true );
    connect( actBlockQuote, SIGNAL( triggered( bool ) ), this, SLOT( sltToggleBlockQuote() ) );
    barVisual->addAction( actBlockQuote );

    barVisual->addSeparator();

    actAddLink = new KAction( KIcon( "insert-link" ), i18nc( 
                             "verb, to add a new link or edit an existing one",
                             "Add/Edit Link" ), this );
    connect( actAddLink, SIGNAL( triggered( bool ) ), this, SLOT( sltAddEditLink() ) );
    barVisual->addAction( actAddLink );

    actRemoveLink = new KAction( KIcon( "remove-link" ), i18nc( 
                                "verb, to remove an existing link", 
                                "Remove Link" ), this );
    connect( actRemoveLink, SIGNAL( triggered( bool ) ), this, SLOT( sltRemoveLink() ) );
    barVisual->addAction( actRemoveLink );

    actAddImage = new KAction( KIcon( "insert-image" ), i18nc( "verb, to insert an image",
                               "Add Image" ), this );
    connect( actAddImage, SIGNAL( triggered( bool ) ), this, SLOT( sltAddImage() ) );
    barVisual->addAction( actAddImage );

    barVisual->addSeparator();

    actAlignLeft = new KAction( KIcon( "format-justify-left" ), i18nc( "verb, to align text from left", "Align left" ), this );
    connect( actAlignLeft, SIGNAL( triggered( bool ) ), this, SLOT( sltAlignLeft() ) );
    barVisual->addAction( actAlignLeft );

    actAlignCenter = new KAction( KIcon( "format-justify-center" ), i18nc( "verb, to align text from center", "Align center" ), this );
    connect( actAlignCenter, SIGNAL( triggered( bool ) ), editor, SLOT( alignCenter() ) );
    barVisual->addAction( actAlignCenter );

    actAlignRight = new KAction( KIcon( "format-justify-right" ), i18nc( "verb, to align text from right", "Align right" ), this );
    connect( actAlignRight, SIGNAL( triggered( bool ) ), this, SLOT( sltAlignRight() ) );
    barVisual->addAction( actAlignRight );

    actJustify = new KAction( KIcon( "format-justify-fill" ), i18nc( 
                             "verb, to justify text", "Justify" ), this );
    connect( actJustify, SIGNAL( triggered( bool ) ), editor, SLOT( alignJustify() ) );
    barVisual->addAction( actJustify );

    actRightToLeft = new KAction( KIcon( "format-text-direction-rtl" ), i18nc(
                                 "Sets text direction as right to left", 
                                 "Right to Left" ), this );
    actRightToLeft->setCheckable( true );
    connect( actRightToLeft, SIGNAL( triggered( bool ) ), this, SLOT( sltChangeLayoutDirection() ) );
    barVisual->addAction( actRightToLeft );


//     actAddMedia = new KAction( KIcon( "mail-attachment" ), i18nc( 
//                               "verb, to add a media file to the post as an attachment", 
//                               "Attach Media" ), this );
//     connect( actAddMedia, SIGNAL( triggered( bool ) ), this, SLOT( sltAddMedia() ) );
//     barVisual->addAction( actAddMedia );

    barVisual->addSeparator();

    actOrderedList = new KAction( KIcon( "format-list-ordered" ), i18n( "Ordered List" ), this );
    connect( actOrderedList, SIGNAL( triggered( bool ) ), this, SLOT( sltAddOrderedList() ) );
    barVisual->addAction( actOrderedList );

    actUnorderedList = new KAction( KIcon( "format-list-unordered" ), i18n( "Unordered List" ), this );
    connect( actUnorderedList, SIGNAL( triggered( bool ) ), this, SLOT( sltAddUnorderedList() ) );
    barVisual->addAction( actUnorderedList );

    actSplitPost = new KAction( KIcon( "insert-more-mark" ), i18n( "Split text" ), this );
    connect( actSplitPost, SIGNAL( triggered( bool ) ), this, SLOT( sltAddPostSplitter() ) );
    barVisual->addAction( actSplitPost );
}

void BilboEditor::sltToggleSpellChecking()
{
    editor->setCheckSpellingEnabled( actCheckSpelling->isChecked() );
}

void BilboEditor::sltSyncSpellCheckingButton( bool check )
{
    actCheckSpelling->setChecked( check );
}

void BilboEditor::sltSetTextBold( bool bold )
{
    if ( !editor->textCursor().blockFormat().hasProperty( BilboTextFormat::HtmlHeading ) ) {
        editor->setTextBold( bold );
    }
}

void BilboEditor::sltToggleCode()
{
    static QString preFontFamily;

    QTextCharFormat charFormat = editor->currentCharFormat();
    QTextCharFormat f;
//     if ( f->fontFamily() != "Courier New,courier" ) {
    if ( charFormat.hasProperty( BilboTextFormat::HasCodeStyle ) &&
         charFormat.boolProperty( BilboTextFormat::HasCodeStyle ) ) {
        f.setProperty( BilboTextFormat::HasCodeStyle, QVariant( false ) );
        f.setBackground( defaultCharFormat.background() );
        f.setFontFamily( preFontFamily );
        editor->textCursor().mergeCharFormat( f );

    } else {
        preFontFamily = editor->fontFamily();
        f.setProperty( BilboTextFormat::HasCodeStyle, QVariant( true ) );
        f.setBackground( codeBackground );
        f.setFontFamily( "Dejavu Sans Mono" );
        editor->textCursor().mergeCharFormat( f );
    }
    editor->setFocus( Qt::OtherFocusReason );
}

void BilboEditor::sltChangeFormatType( const QString& text )
{
    editor->setFocus( Qt::OtherFocusReason );

    QTextCursor cursor = editor->textCursor();
//     QTextBlockFormat bformat = cursor.blockFormat();
    QTextBlockFormat bformat;
    QTextCharFormat cformat;

    if ( text == i18n( "Paragraph" ) ) {
            bformat.setProperty( BilboTextFormat::HtmlHeading, QVariant( 0 ) );
            cformat.setFontWeight( QFont::Normal );
            cformat.setProperty( QTextFormat::FontSizeAdjustment, QVariant( 0 ) );

    } else if ( text == i18n( "Heading 1" ) ) {
            bformat.setProperty( BilboTextFormat::HtmlHeading, QVariant( 1 ) );
            cformat.setFontWeight( QFont::Bold );
            cformat.setProperty( QTextFormat::FontSizeAdjustment, QVariant( 3 ) );

    } else if ( text == i18n( "Heading 2" ) ) {
            bformat.setProperty( BilboTextFormat::HtmlHeading, QVariant( 2 ) );
            cformat.setFontWeight( QFont::Bold );
            cformat.setProperty( QTextFormat::FontSizeAdjustment, QVariant( 2 ) );

    } else if ( text == i18n( "Heading 3" ) ) {
            bformat.setProperty( BilboTextFormat::HtmlHeading, QVariant( 3 ) );
            cformat.setFontWeight( QFont::Bold );
            cformat.setProperty( QTextFormat::FontSizeAdjustment, QVariant( 1 ) );

    } else if ( text == i18n( "Heading 4" ) ) {
            bformat.setProperty( BilboTextFormat::HtmlHeading, QVariant( 4 ) );
            cformat.setFontWeight( QFont::Bold );
            cformat.setProperty( QTextFormat::FontSizeAdjustment, QVariant( 0 ) );

    } else if ( text == i18n( "Heading 5" ) ) {
            bformat.setProperty( BilboTextFormat::HtmlHeading, QVariant( 5 ) );
            cformat.setFontWeight( QFont::Bold );
            cformat.setProperty( QTextFormat::FontSizeAdjustment, QVariant( -1 ) );

    } else {
        bformat.setProperty( BilboTextFormat::HtmlHeading, QVariant( 6 ) );
        cformat.setFontWeight( QFont::Bold );
        cformat.setProperty( QTextFormat::FontSizeAdjustment, QVariant( -2 ) );
    }
//     cformat.clearProperty( BilboTextFormat::HasCodeStyle );

    cursor.beginEditBlock();
    cursor.mergeBlockFormat( bformat );
    cursor.select( QTextCursor::BlockUnderCursor );
    cursor.mergeCharFormat( cformat );
    cursor.endEditBlock();
}

void BilboEditor::sltFontSizeIncrease()
{
    if ( !( editor->textCursor().blockFormat().hasProperty( BilboTextFormat::HtmlHeading ) &&
        editor->textCursor().blockFormat().intProperty( BilboTextFormat::HtmlHeading ) ) ) {
        QTextCharFormat format;
        int idx = editor->currentCharFormat().intProperty( QTextFormat::FontSizeAdjustment );
        if ( idx < 3 ) {
            format.setProperty( QTextFormat::FontSizeAdjustment, QVariant( ++idx ) );
            editor->textCursor().mergeCharFormat( format );
        }
    }
    editor->setFocus( Qt::OtherFocusReason );
}

void BilboEditor::sltFontSizeDecrease()
{
    if ( !( editor->textCursor().blockFormat().hasProperty( BilboTextFormat::HtmlHeading ) &&
        editor->textCursor().blockFormat().intProperty( BilboTextFormat::HtmlHeading ) ) ) {
        QTextCharFormat format;
        int idx = editor->currentCharFormat().intProperty( QTextFormat::FontSizeAdjustment );
        if ( idx > -1 ) {
            format.setProperty( QTextFormat::FontSizeAdjustment, QVariant( --idx ) );
            editor->textCursor().mergeCharFormat( format );
        }
    }
    editor->setFocus( Qt::OtherFocusReason );
}

void BilboEditor::sltAddEditLink()
{
    linkDialog = new AddEditLink( this );
    linkDialog->setAttribute( Qt::WA_DeleteOnClose );
    linkDialog->setWindowModality( Qt::WindowModal );
    connect( linkDialog, SIGNAL( addLink( const QString&, const QString&, const QString& ) ),
             this, SLOT( sltSetLink( const QString&, const QString&, const QString& ) ) );

    QTextCharFormat f = editor->currentCharFormat();
    if ( !f.isAnchor() ) {
        linkDialog->show();
    } else {
        linkDialog->show( f.anchorHref(), f.stringProperty( BilboTextFormat::AnchorTitle )
                          , f.stringProperty( BilboTextFormat::AnchorTarget ) );
    }
}

void BilboEditor::sltSetLink( const QString& address, const QString& target,
                              const QString& title )
{
    editor->setFocus( Qt::OtherFocusReason );

//     QTextCharFormat f = editor->currentCharFormat();
    QTextCharFormat charFormat = editor->currentCharFormat();
    QTextCharFormat f;
    QTextCursor cursor = editor->textCursor();

    if ( ( charFormat.isAnchor() ) && ( !editor->textCursor().hasSelection() ) ) {

        QTextBlock block = cursor.block();
        QTextBlock::iterator i;
        for ( i = block.begin(); !( i.atEnd() ); ++i ) {

            if ( i.fragment().contains( cursor.position() ) ) {
                cursor.setPosition( i.fragment().position() );
                cursor.movePosition( QTextCursor::NextCharacter,
                                     QTextCursor::KeepAnchor, i.fragment().length() );
                break;
            }
        }
    }
    f.setAnchor( true );
    f.setAnchorHref( address );
    f.setProperty( BilboTextFormat::AnchorTitle, QVariant( title ) );
    f.setProperty( BilboTextFormat::AnchorTarget, QVariant( target ) );

    f.setFontUnderline( true );
    f.setForeground( QBrush( Qt::blue ) );

    cursor.mergeCharFormat( f );
}

void BilboEditor::sltRemoveLink()
{
//     QTextCharFormat f = editor->textCursor().charFormat();
    QTextCharFormat f;
    f.setAnchor( false );
    f.setUnderlineStyle( this->defaultCharFormat.underlineStyle() );
    f.setForeground( this->defaultCharFormat.foreground() );

    editor->textCursor().mergeCharFormat( f );
    editor->setFocus( Qt::MouseFocusReason );
}

void BilboEditor::sltSelectColor()
{
    QColor c;

    int result = KColorDialog::getColor( c, editor->textCursor().charFormat().foreground().color(), this );
    if ( result == KColorDialog::Accepted ) {
        editor->setTextForegroundColor( c );
    }
}

void BilboEditor::sltRemoveFormatting()
{
    QTextCharFormat format = defaultCharFormat;
    if ( editor->textCursor().blockFormat().hasProperty( BilboTextFormat::HtmlHeading ) ) {
        format.setProperty( QTextFormat::FontSizeAdjustment, QVariant( 
               editor->textCursor().charFormat().intProperty(
               QTextFormat::FontSizeAdjustment ) ) );
        format.setFontWeight( editor->textCursor().charFormat().fontWeight() );
    }
    editor->textCursor().mergeCharFormat( format );
    editor->setFocus( Qt::OtherFocusReason );
}

// void BilboEditor::sltNewParagraph()
// {
//     editor->textCursor().insertBlock( editor->textCursor().blockFormat(), editor->textCursor().charFormat() );
//     editor->setFocus( Qt::OtherFocusReason );
// }
void BilboEditor::sltAlignRight()
{
    editor->setAlignment( Qt::AlignRight | Qt::AlignAbsolute );
    editor->setFocus( Qt::OtherFocusReason );
}

void BilboEditor::sltAlignLeft()
{
    editor->setAlignment( Qt::AlignLeft | Qt::AlignAbsolute );
    editor->setFocus( Qt::OtherFocusReason );
}

void BilboEditor::sltChangeLayoutDirection()
{
    kDebug();
//     QTextBlockFormat f = editor->textCursor().blockFormat();
    QTextBlockFormat f;
    if ( actRightToLeft->isChecked() ) {
        f.setLayoutDirection( Qt::RightToLeft );
    } else {
        f.setLayoutDirection( Qt::LeftToRight );
    }
    
//     if ( f.layoutDirection() != Qt::RightToLeft ) {
//         f.setLayoutDirection( Qt::RightToLeft );
//     } else {
//         f.setLayoutDirection( Qt::LeftToRight );
//     }
    editor->textCursor().mergeBlockFormat( f );

    editor->setFocus( Qt::OtherFocusReason );
}

void BilboEditor::sltAddImage()
{
    QPointer<AddImageDialog> imageDialog = new AddImageDialog( this );
//     imageDialog->setAttribute( Qt::WA_DeleteOnClose );
    imageDialog->setWindowModality( Qt::WindowModal );

    connect( imageDialog, SIGNAL( sigAddImage( BilboMedia *, const int, const int, 
             const QString, const QString, const QString ) ), this, SLOT( sltSetImage( BilboMedia *, 
             const int, const int, const QString, const QString, const QString ) ) );
    connect( imageDialog, SIGNAL( sigMediaTypeFound( BilboMedia * ) ), this, 
             SLOT( sltMediaTypeFound( BilboMedia * ) ) );
    imageDialog->exec();
    imageDialog->deleteLater();
}

void BilboEditor::sltSetImage( BilboMedia *media, const int width, const int height, 
                        const QString title, const QString link, const QString Alt_text )
{
    QTextImageFormat imageFormat;

    imageFormat.setName( media->remoteUrl().url() );
    if ( width != 0 ) {
        imageFormat.setWidth( width );
    }
    if ( height != 0 ) {;
        imageFormat.setHeight( height );
    }
    if ( !title.isEmpty() ) {
        imageFormat.setProperty( BilboTextFormat::ImageTitle, QVariant( title ) );
    }
    if ( !Alt_text.isEmpty() ) {
        imageFormat.setProperty( BilboTextFormat::ImageAlternateText, QVariant( Alt_text ) );
    }
    if ( !link.isEmpty() ) {
        imageFormat.setAnchor( true );
        imageFormat.setAnchorHref( link );
    }
    editor->textCursor().insertImage( imageFormat );

    editor->document()->setUndoRedoEnabled( false );
    editor->document()->setUndoRedoEnabled( true );

    editor->setFocus( Qt::OtherFocusReason );
}

void BilboEditor::sltReloadImage( const KUrl imagePath )
{
    QString path = imagePath.url();

    if ( this->currentIndex() == 0 ) {

        this->editor->setFocus( Qt::OtherFocusReason );
        QTextCharFormat f;
        QTextCursor cursor = this->editor->textCursor();
        QTextBlock block = this->editor->document()->firstBlock();
        QTextBlock::iterator i;
        do {
            for ( i = block.begin(); !( i.atEnd() ); ++i ) {
                f = i.fragment().charFormat();
                if ( f.isImageFormat() ) {
                    QTextImageFormat imgFormat = f.toImageFormat();
                    if ( imgFormat.name() == path ) {
                        imgFormat.setName( path );

                        cursor.setPosition( i.fragment().position() );
                        cursor.movePosition( QTextCursor::NextCharacter,
                                            QTextCursor::KeepAnchor, i.fragment().length() );
                        if ( cursor.hasSelection() ) {
                            cursor.mergeCharFormat( imgFormat );
                        }
                    }
                }
            }
            block = block.next();
        } while ( block.isValid() );
    }
    if ( mMediaList->contains( path ) ) {
        QList < QListWidgetItem* > list;
        list = lstMediaFiles->findItems( imagePath.fileName(), ( Qt::MatchFixedString | 
                Qt::MatchCaseSensitive ) );
        if ( list.isEmpty() ) {
            kDebug() << "image isn't inserted";
        } else {
            for ( int i = 0; i < list.size(); i++ ) {
                if ( list.at( i )->toolTip() == path ) {
                    list.at( i )->setIcon( mMediaList->value( path )->icon() );
                    break;
                }
            }
        }
    }
    editor->document()->setUndoRedoEnabled( false );
    editor->document()->setUndoRedoEnabled( true );
}

void BilboEditor::sltSetImageProperties( const int index, const int width,
                    const int height, const QString title, const QString link,
                    const QString Alt_text )
{
    this->editor->setFocus( Qt::OtherFocusReason );
    QString path = lstMediaFiles->item( index )->toolTip();

    QTextCharFormat f;
    QTextCursor cursor;
    QTextBlock block = this->editor->document()->firstBlock();
    QTextBlock::iterator i;
    do {
        for ( i = block.begin(); !( i.atEnd() ); ++i ) {
            f = i.fragment().charFormat();
            if ( f.isImageFormat() ) {
                QTextImageFormat imgFormat = f.toImageFormat();
                if ( imgFormat.name() == path ) {
                    imgFormat.setName(path);
                    if ( width != 0 ) {
                        imgFormat.setWidth( width );
                    }
                    if ( height != 0 ) {
                        imgFormat.setHeight( height );
                    }
                    if ( !title.isEmpty() ) {
                        imgFormat.setProperty( BilboTextFormat::ImageTitle, QVariant( title ) );
                    }
                    if ( !Alt_text.isEmpty() ) {
                        imgFormat.setProperty( BilboTextFormat::ImageAlternateText, QVariant( Alt_text ) );
                    }
                    if ( !link.isEmpty() ) {
                        imgFormat.setAnchor( true );
                        imgFormat.setAnchorHref( link );
                    }
                    cursor = this->editor->textCursor();
                    cursor.setPosition( i.fragment().position() );
                    cursor.movePosition( QTextCursor::NextCharacter,
                                         QTextCursor::KeepAnchor, i.fragment().length() );
                    if ( cursor.hasSelection() ) {
                        cursor.mergeCharFormat( imgFormat );
                    }
                }
            }
        }
        block = block.next();
    } while ( block.isValid() );
    this->editor->setFocus( Qt::OtherFocusReason );
}

void BilboEditor::sltRemoveMedia( const int index )
{
    this->editor->setFocus( Qt::OtherFocusReason );
    QString path = lstMediaFiles->item( index )->toolTip();
    delete lstMediaFiles->item( index );

    int count = mMediaList->remove( path );
    kDebug() << count;

    QTextCharFormat f;
    QTextCursor cursor;
    QTextBlock block = this->editor->document()->firstBlock();
    QTextBlock::iterator i;
    do {
        i = block.begin();
        while ( !i.atEnd() ) {
            f = i.fragment().charFormat();
//             if ( ( f.isImageFormat() && f.toImageFormat().name() == path ) ||
//                   ( f.isAnchor() && f.anchorHref() == path ) )
            if ( ( f.isImageFormat() ) && ( f.toImageFormat().name() == path ) ) {

                cursor = this->editor->textCursor();
                cursor.setPosition( i.fragment().position() );
                cursor.movePosition( QTextCursor::NextCharacter,
                                     QTextCursor::KeepAnchor, i.fragment().length() );
                ++i;
                if (i.atEnd()) {
                    cursor.removeSelectedText();
                    break;
                } else {
                    cursor.removeSelectedText();
                    i = block.begin();
                }
            }
            else {
                ++i;
            }
        }
        block = block.next();
    } while ( block.isValid() );

    editor->document()->setUndoRedoEnabled( false );
    editor->document()->setUndoRedoEnabled( true );
}

void BilboEditor::sltMediaTypeFound( BilboMedia * media )
{
    QListWidgetItem *item;
    QString url = media->remoteUrl().url();
    
//     AddMediaDialog *dialog;
//     dialog = qobject_cast<AddMediaDialog*>( sender() );
//     if ( dialog ) {
//         delete dialog;
//     }
    if ( mMediaList->contains( url ) ) {
        //media is already added.
        delete media;
    } else {
        mMediaList->insert( url, media );

        if ( media->mimeType().contains( "image" ) ) {
            item = new QListWidgetItem( media->icon(), media->name(), lstMediaFiles, MediaListWidget::ImageType );
        } else {
            item = new QListWidgetItem( media->icon(), media->name(), lstMediaFiles, MediaListWidget::OtherType );
        }
//         item->setData( Qt::UserRole, QVariant( url ) );
        item->setToolTip( url );
        item->setSizeHint( lstMediaFiles->gridSize() );
    }
}

void BilboEditor::sltAddOrderedList()
{
//  if (editor->textCursor().currentList() == 0) {
    editor->textCursor().createList( QTextListFormat::ListDecimal );
//  } else {
//   QTextListFormat lf = editor->textCursor().currentList()->format();
// //   QTextBlockFormat bf = editor->textCursor().block().blockFormat();
// //   bf.setIndent(lf.indent() - 1);
// //   editor->textCursor().mergeBlockFormat(bf);
//   editor->textCursor().currentList()->remove(editor->textCursor().block());
//  }
}

void BilboEditor::sltAddUnorderedList()
{
//  if (editor->textCursor().currentList() == 0) {
    editor->textCursor().createList( QTextListFormat::ListDisc );
//  } else {
//   QTextListFormat lf = editor->textCursor().currentList()->format();
// //   QTextBlockFormat bf = editor->textCursor().block().blockFormat();
// //   bf.setIndent(lf.indent() - 1);
// //   editor->textCursor().mergeBlockFormat(bf);
//   editor->textCursor().currentList()->remove(editor->textCursor().block());
//  }
}

void BilboEditor::sltToggleBlockQuote()
{
    QTextBlockFormat blockFormat = editor->textCursor().blockFormat();
    QTextBlockFormat f;

    if ( blockFormat.hasProperty( BilboTextFormat::IsBlockQuote ) && 
         blockFormat.boolProperty( BilboTextFormat::IsBlockQuote ) ) {
        f.setProperty( BilboTextFormat::IsBlockQuote, QVariant( false ) );
        f.setLeftMargin( 0 );
        f.setRightMargin( 0 );
    } else {
        f.setProperty( BilboTextFormat::IsBlockQuote, QVariant( true ) );
        f.setLeftMargin( 40 );
        f.setRightMargin( 40 );
    }
    editor->textCursor().mergeBlockFormat( f );
}

void BilboEditor::sltAddPostSplitter()
{
    QTextBlockFormat f = editor->textCursor().blockFormat();
    QTextBlockFormat f1 = f;

    f.setProperty( BilboTextFormat::IsHtmlTagSign, true );
    f.setProperty( QTextFormat::BlockTrailingHorizontalRulerWidth, 
             QTextLength( QTextLength::PercentageLength, 80 ) );
    if ( editor->textCursor().block().text().isEmpty() ) {
        editor->textCursor().mergeBlockFormat( f );
    } else {
        editor->textCursor().insertBlock( f );
    }
    editor->textCursor().insertBlock( f1 );
}

void BilboEditor::slotCurrentCharFormantChanged(const QTextCharFormat &charFormat)
{
    if ( charFormat.fontWeight() == QFont::Bold ) {
        this->actBold->setChecked( true );
    } else {
        this->actBold->setChecked( false );
    }
    this->actItalic->setChecked( charFormat.fontItalic() );
    this->actUnderline->setChecked( charFormat.fontUnderline() );
    this->actStrikeout->setChecked( charFormat.fontStrikeOut() );
    if ( charFormat.hasProperty( BilboTextFormat::HasCodeStyle ) &&
            charFormat.boolProperty( BilboTextFormat::HasCodeStyle ) ) {
        this->actCode->setChecked( true );
    } else {
        this->actCode->setChecked( false );
    }
}

void BilboEditor::slotCurrentBlockFormatChanged(const QTextBlockFormat& blockFormat)
{
        if ( blockFormat.layoutDirection() == Qt::RightToLeft ) {
            this->actRightToLeft->setChecked( true );
        } else {
            this->actRightToLeft->setChecked( false );
        }
        if ( !blockFormat.hasProperty( BilboTextFormat::HtmlHeading ) ) {
            this->actFormatType->setCurrentItem( 0 );
        } else {
            this->actFormatType->setCurrentItem( blockFormat.intProperty(
                                                 BilboTextFormat::HtmlHeading ) );
        }
        if ( blockFormat.hasProperty( BilboTextFormat::IsBlockQuote ) ){
            this->actBlockQuote->setChecked( blockFormat.boolProperty( BilboTextFormat::IsBlockQuote ) );
        }
}

void BilboEditor::sltSyncEditors( int index )
{
    kDebug();
    QTextDocument *doc = editor->document();

    HtmlExporter* htmlExp = new HtmlExporter();
    htmlExp->setDefaultCharFormat( this->defaultCharFormat );
    htmlExp->setDefaultBlockFormat( this->defaultBlockFormat );

    if ( index == 0 ) {
        if ( prev_index == 2 ) {
            preview->stop();
            goto SyncEnd;
        }
        doc->clear();
        BilboTextHtmlImporter( doc, htmlEditor->document()->text() ).import();
        editor->document()->setUndoRedoEnabled(false);//To clear undo/redo history!
        editor->document()->setUndoRedoEnabled(true);
        editor->setTextCursor( QTextCursor( doc ) );
    } else if ( index == 1 ) {
        if ( prev_index == 2 ) {
            preview->stop();
            goto SyncEnd;
        }
//         htmlEditor->setPlainText( htmlExp->toHtml( doc ) );
        htmlEditor->document()->setText( htmlExp->toHtml( doc ) );
    } else {
        if ( prev_index == 1 ) {
            doc->clear();
            BilboTextHtmlImporter( doc, htmlEditor->document()->text() ).import();
            editor->document()->setUndoRedoEnabled(false);//To clear undo/redo history!
            editor->document()->setUndoRedoEnabled(true);
        } else {
//             htmlEditor->setPlainText( htmlExp->toHtml( doc ) );
            htmlEditor->document()->setText( htmlExp->toHtml( doc ) );
        }

        preview->setHtml( currentPostTitle, htmlEditor->document()->text() );
    }
SyncEnd:
    prev_index = index;
    delete htmlExp;
//     doc->deleteLater();
}

QString BilboEditor::htmlContent()
{
    if ( this->currentIndex() == 0 ) {

        HtmlExporter* htmlExp = new HtmlExporter();
        htmlExp->setDefaultCharFormat( this->defaultCharFormat );
        htmlExp->setDefaultBlockFormat( this->defaultBlockFormat );

//         htmlEditor->setPlainText( htmlExp->toHtml( editor->document() ) );
        htmlEditor->document()->setText( htmlExp->toHtml( editor->document() ) );
        delete htmlExp;
    }

//     QString htmlContent = htmlEditor->toPlainText();
    QString htmlContent = htmlEditor->document()->text();
    return htmlContent;
}

void BilboEditor::setHtmlContent( const QString & content )
{
    QTextDocument *doc = editor->document();
    doc->clear();

    if ( !content.isEmpty() ) {
        BilboTextHtmlImporter( doc, content ).import();
    }
    this->editor->setTextCursor( QTextCursor( doc ) );
//     this->htmlEditor->setPlainText( content );
    this->htmlEditor->document()->setText( content );

    editor->document()->setUndoRedoEnabled(false);//To clear undo/redo history!
    editor->document()->setUndoRedoEnabled(true);
}

void BilboEditor::setMediaList( QMap <QString, BilboMedia*> * list )
{
    mMediaList = list;
    editor->setMediaList( list );
}

void BilboEditor::setLayoutDirection( Qt::LayoutDirection direction )
{
//     QTextBlockFormat f = editor->textCursor().blockFormat();
//     f.setLayoutDirection( direction );
//     editor->textCursor().mergeBlockFormat( f );
    QTextOption textOption = editor->document()->defaultTextOption();
    textOption.setTextDirection( direction );
    editor->document()->setDefaultTextOption( textOption );

//     this->defaultBlockFormat.setLayoutDirection( direction );

    if ( direction == Qt::LeftToRight ) {
        this->actRightToLeft->setChecked( false );
    } else {
        this->actRightToLeft->setChecked( true );
    }
}

void BilboEditor::setCurrentTitle( const QString& title)
{
    if ( title.isEmpty() ) {
        currentPostTitle = i18n( "Post Title" );
    } else {
        currentPostTitle = title;
    }
}

bool BilboEditor::updateMediaPaths()
{
    int startIndex = 0;
    int endIndex;
    QString path;
    QString htmlContent;
    bool changed = false;

    if ( this->currentIndex() == 0 ) {
        HtmlExporter* htmlExp = new HtmlExporter();
        htmlContent = htmlExp->toHtml( this->editor->document() );
    } else {
//         htmlContent = this->htmlEditor->toPlainText();
        htmlContent = htmlEditor->document()->text();
    }

    startIndex = htmlContent.indexOf( QRegExp( "<([^<>]*)\"file://" ), startIndex );
    while ( startIndex != -1 ) {
        startIndex = htmlContent.indexOf( "file://", startIndex );
        endIndex = htmlContent.indexOf( '\"', startIndex );
        path = htmlContent.mid( startIndex, ( endIndex - startIndex ) );

        if ( mMediaList->contains( path ) ) {
            BilboMedia *media = mMediaList->value( path );

            htmlContent.replace( startIndex, ( endIndex - startIndex ),
                                 media->remoteUrl().url() );
            changed = true;

            mMediaList->remove( path );
            mMediaList->insert( media->remoteUrl().url(), media );

            QList < QListWidgetItem* > list;
            list = lstMediaFiles->findItems( media->name(), ( Qt::MatchFixedString | 
                Qt::MatchCaseSensitive ) );
            if ( list.isEmpty() ) {
                kDebug() << "media isn't inserted in list widget";
            } else {
                for ( int i = 0; i < list.size(); i++ ) {
                    if ( list.at( i )->toolTip() == path ) {
                        list.at( i )->setToolTip( media->remoteUrl().url() );
                        break;
                    }
                }
            }
        }
        startIndex = htmlContent.indexOf( QRegExp( "<([^<>]*)\"file://" ), endIndex );
    }
    if ( changed ) {
        if ( this->currentIndex() == 0 ) {
            QTextDocument *doc = editor->document();
            doc->clear();
            BilboTextHtmlImporter( doc, htmlContent ).import();
        } else {
//             this->htmlEditor->setPlainText( htmlContent );
            htmlEditor->document()->setText( htmlContent );
        }
    }
    editor->document()->setUndoRedoEnabled(false);//To clear undo/redo history!
    editor->document()->setUndoRedoEnabled( true );
    return true;
}

void BilboEditor::sltSetPostPreview()
{
    if ( this->currentIndex() == 2 ) {
        preview->setHtml( currentPostTitle, htmlEditor->document()->text() );
    }
}

void BilboEditor::slotSettingsChanged()
{
    editor->setCheckSpellingEnabled( Settings::enableCheckSpelling() );
}

#include "composer/bilboeditor.moc"
