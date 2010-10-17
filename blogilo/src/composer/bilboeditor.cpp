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

#include "texteditor/texteditor.h"
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

#define FORWARD_ACTION(action1, action2) \
    connect(action1, SIGNAL(triggered()), editor->getAction(action2), SLOT(trigger()));\
    connect(editor->getAction(action2), SIGNAL(changed()), SLOT(adjustActions()));

BilboEditor::BilboEditor( QWidget *parent )
        : KTabWidget( parent )
{
    createUi();
    connect( editor, SIGNAL( textChanged() ), this, SIGNAL( textChanged() ) );
    connect( htmlEditor->document(), SIGNAL( textChanged( KTextEditor::Document * ) ),
             this, SIGNAL( textChanged() ) );
    connect( Settings::self(), SIGNAL(configChanged()),
             this, SLOT(slotSettingsChanged()) );
//     editor->setCheckSpellingEnabled( Settings::enableCheckSpelling() );
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
    editor = new TextEditor( tabVisual );
//     editor->enableFindReplace( true );
//     connect( editor, SIGNAL( sigRemoteImageArrived( const KUrl ) ), this, 
//              SLOT( sltReloadImage( const KUrl ) ) );
//     connect( editor, SIGNAL( sigMediaTypeFound( BilboMedia* ) ), this, 
//              SLOT( sltMediaTypeFound( BilboMedia* ) ) );

    

//     QLabel *label = new QLabel( i18n( "Media list:" ), tabVisual );
//     label->setMaximumHeight( 30 );

//     lstMediaFiles = new MediaListWidget( tabVisual );
//     lstMediaFiles->setViewMode( QListView::IconMode );
//     lstMediaFiles->setIconSize( QSize( 32, 32 ) );
//     lstMediaFiles->setGridSize( QSize( 60, 48 ) );
//     lstMediaFiles->setDragDropMode( QAbstractItemView::NoDragDrop );
//     lstMediaFiles->setResizeMode( QListView::Adjust );
//     lstMediaFiles->setMaximumHeight( 60 );
//     connect( lstMediaFiles, SIGNAL( sigSetProperties( const int, const int,
//                                     const int, const QString, const QString, const QString ) ), 
//             this, SLOT( sltSetImageProperties( const int, const int, const int, 
//                         const QString, const QString, const QString ) ) );
//     connect( lstMediaFiles, SIGNAL( sigRemoveMedia( const int ) ), this, SLOT( sltRemoveMedia( const int ) ) );

    QVBoxLayout *vLayout = new QVBoxLayout( tabVisual );
//     vLayout->addWidget( barVisual );
    vLayout->addWidget( editor );
//     vLayout->addWidget( label );
//     vLayout->addWidget( lstMediaFiles );

    connect( editor, SIGNAL( checkSpellingChanged( bool ) ), this, SLOT( sltSyncSpellCheckingButton( bool ) ) );

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

//     QPalette palette = QApplication::palette();
//     codeBackground = palette.color( QPalette::Active, QPalette::Midlight );

/*
    ///defaultCharFormat
//     defaultCharFormat = editor->currentCharFormat();
    const QFont defaultFont = editor->document()->defaultFont();
    defaultCharFormat.setFont( defaultFont );
    defaultCharFormat.setForeground( editor->currentCharFormat().foreground() );
    defaultCharFormat.setProperty( QTextFormat::FontSizeAdjustment, QVariant( 0 ) );
    defaultCharFormat.setBackground( palette.color( QPalette::Active,
                                                    QPalette::Base ) );
    defaultCharFormat.setProperty( BilboTextFormat::HasCodeStyle, QVariant( false ) );

    ///defaultBlockFormat
    defaultBlockFormat = editor->textCursor().blockFormat();*/

//     createActions();
}


void BilboEditor::sltAddEditLink()
{
    linkDialog = new AddEditLink( this );
    linkDialog->setAttribute( Qt::WA_DeleteOnClose );
    linkDialog->setWindowModality( Qt::WindowModal );
    connect( linkDialog, SIGNAL( addLink( const QString&, const QString&, const QString& ) ),
             this, SLOT( sltSetLink( const QString&, const QString&, const QString& ) ) );

//     QTextCharFormat f = editor->currentCharFormat();
//     if ( !f.isAnchor() ) {
//         linkDialog->show();
//     } else {
//         linkDialog->show( f.anchorHref(), f.stringProperty( BilboTextFormat::AnchorTitle )
//                           , f.stringProperty( BilboTextFormat::AnchorTarget ) );
//     }
}

void BilboEditor::sltSetLink( const QString& address, const QString& target,
                              const QString& title )
{
//     editor->setFocus( Qt::OtherFocusReason );
// 
//     QTextCharFormat charFormat = editor->currentCharFormat();
//     QTextCharFormat f;
//     QTextCursor cursor = editor->textCursor();
// 
//     if ( ( charFormat.isAnchor() ) && ( !editor->textCursor().hasSelection() ) ) {
// 
//         QTextBlock block = cursor.block();
//         QTextBlock::iterator i;
//         for ( i = block.begin(); !( i.atEnd() ); ++i ) {
// 
//             if ( i.fragment().contains( cursor.position() ) ) {
//                 cursor.setPosition( i.fragment().position() );
//                 cursor.movePosition( QTextCursor::NextCharacter,
//                                      QTextCursor::KeepAnchor, i.fragment().length() );
//                 break;
//             }
//         }
//     }
//     f.setAnchor( true );
//     f.setAnchorHref( address );
//     f.setProperty( BilboTextFormat::AnchorTitle, QVariant( title ) );
//     f.setProperty( BilboTextFormat::AnchorTarget, QVariant( target ) );
// 
//     f.setFontUnderline( true );
//     f.setForeground( QBrush( Qt::blue ) );
// 
//     cursor.mergeCharFormat( f );
}

void BilboEditor::sltRemoveLink()
{
    QTextCharFormat f;
    f.setAnchor( false );
    f.setUnderlineStyle( this->defaultCharFormat.underlineStyle() );
    f.setForeground( this->defaultCharFormat.foreground() );

//     editor->textCursor().mergeCharFormat( f );
    editor->setFocus( Qt::MouseFocusReason );
}

void BilboEditor::sltAddImage()
{
    QPointer<AddImageDialog> imageDialog = new AddImageDialog( this );
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
//     editor->textCursor().insertImage( imageFormat );

    editor->setFocus( Qt::OtherFocusReason );
}

void BilboEditor::sltAddPostSplitter()
{
//     QTextBlockFormat f = editor->textCursor().blockFormat();
//     QTextBlockFormat f1 = f;
// 
//     f.setProperty( BilboTextFormat::IsHtmlTagSign, true );
//     f.setProperty( QTextFormat::BlockTrailingHorizontalRulerWidth, 
//              QTextLength( QTextLength::PercentageLength, 80 ) );
//     if ( editor->textCursor().block().text().isEmpty() ) {
//         editor->textCursor().mergeBlockFormat( f );
//     } else {
//         editor->textCursor().insertBlock( f );
//     }
//     editor->textCursor().insertBlock( f1 );
}

void BilboEditor::sltSyncEditors( int index )
{
    kDebug();

    if ( index == 0 ) {
        if ( prev_index == 2 ) {
            preview->stop();
            goto SyncEnd;
        }//An else clause can do the job of goto, No? -Mehrdad :D
        editor->setHtmlContent(htmlEditor->document()->text());
        editor->setFocus();
        editor->startEditing();
    } else if ( index == 1 ) {
        if ( prev_index == 2 ) {
            preview->stop();
            goto SyncEnd;
        }
        htmlEditor->document()->setText( editor->htmlContent() );
        htmlEditor->setFocus();
    } else {
        if ( prev_index == 1 ) {
            editor->setHtmlContent(htmlEditor->document()->text());
        } else {
            htmlEditor->document()->setText( editor->htmlContent() );
        }
        preview->setHtml( currentPostTitle, htmlEditor->document()->text() );
    }
SyncEnd:
    prev_index = index;
}

QString BilboEditor::htmlContent()
{
    if ( this->currentIndex() == 0 ) {
        htmlEditor->document()->setText( editor->htmlContent() );
    }
    return editor->htmlContent();
}

void BilboEditor::setHtmlContent( const QString & content )
{
    this->editor->setHtmlContent(content);
    this->htmlEditor->document()->setText( content );
}

void BilboEditor::setLayoutDirection( Qt::LayoutDirection direction )
{
//     QTextOption textOption = editor->document()->defaultTextOption();
//     textOption.setTextDirection( direction );
//     editor->document()->setDefaultTextOption( textOption );
// 
//     if ( direction == Qt::LeftToRight ) {
//         this->actRightToLeft->setChecked( false );
//     } else {
//         this->actRightToLeft->setChecked( true );
//     }
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
        htmlContent = editor->htmlContent();
    } else {
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
//             list = lstMediaFiles->findItems( media->name(), ( Qt::MatchFixedString | 
//                 Qt::MatchCaseSensitive ) );
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
            editor->setHtmlContent(htmlContent);
        } else {
            htmlEditor->document()->setText( htmlContent );
        }
    }
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
//     editor->setCheckSpellingEnabled( Settings::enableCheckSpelling() );
}

#include "composer/bilboeditor.moc"
