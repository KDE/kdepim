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
    connect(action1, SIGNAL(triggered()), d->editor->getAction(action2), SLOT(trigger()));\
    connect(d->editor->getAction(action2), SIGNAL(changed()), SLOT(adjustActions()));

class BilboEditor::Private
{
public:
    QWidget *tabVisual;
    QWidget *tabHtml;
    QWidget *tabPreview;

    TextEditor *editor;
    KTextEditor::View *htmlEditor;

    BilboBrowser *preview;

    KToolBar *barVisual;

    QString currentPostTitle;
    int prev_index;
//     QColor codeBackground;
};

BilboEditor::BilboEditor( QWidget *parent )
        : KTabWidget( parent ), d(new Private)
{
    createUi();
    connect( d->editor, SIGNAL( textChanged() ), this, SIGNAL( textChanged() ) );
    connect( d->htmlEditor->document(), SIGNAL( textChanged( KTextEditor::Document * ) ),
             this, SIGNAL( textChanged() ) );
    connect( Settings::self(), SIGNAL(configChanged()),
             this, SLOT(slotSettingsChanged()) );
//     d->editor->setCheckSpellingEnabled( Settings::enableCheckSpelling() );
    d->editor->setFocus();
}

BilboEditor::~BilboEditor()
{
    kDebug();
}

void BilboEditor::createUi()
{
    ///this:
    this->resize( 600, 400 );
    d->tabVisual = new QWidget( this );
    d->tabHtml = new QWidget( this );
    d->tabPreview = new QWidget( this );
    this->addTab( d->tabVisual, i18nc( "Software", "Visual Editor" ) );
    this->addTab( d->tabHtml, i18nc( "Software", "Html Editor" ) );
    this->addTab( d->tabPreview, i18nc( "d->preview of the edited post", "Post Preview" ) );
    connect( this, SIGNAL( currentChanged( int ) ), this, SLOT( sltSyncEditors( int ) ) );
    d->prev_index = 0;

    /// Visual d->editor:
    d->editor = new TextEditor( d->tabVisual );
    QVBoxLayout *vLayout = new QVBoxLayout( d->tabVisual );
    vLayout->addWidget( d->editor );

    connect( d->editor, SIGNAL( checkSpellingChanged( bool ) ),
             this, SLOT( sltSyncSpellCheckingButton( bool ) ) );

    ///d->htmlEditor:
    d->htmlEditor = HtmlEditor::self()->createView( d->tabHtml );
    QGridLayout *hLayout = new QGridLayout( d->tabHtml );
    hLayout->addWidget( d->htmlEditor );

    ///d->preview:
    d->preview = new BilboBrowser( d->tabPreview );
    QGridLayout *gLayout = new QGridLayout( d->tabPreview );
    gLayout->addWidget( d->preview );

    connect( d->preview, SIGNAL( sigSetBlogStyle() ), this, SLOT(
            sltSetPostPreview() ) );


    this->setCurrentIndex( 0 );

    d->currentPostTitle = i18n( "Post Title" );
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
//     d->editor->textCursor().insertImage( imageFormat );

    d->editor->setFocus( Qt::OtherFocusReason );
}

void BilboEditor::sltSyncEditors( int index )
{
    kDebug();

    if ( index == 0 ) {
        if ( d->prev_index == 2 ) {
            d->preview->stop();
            goto SyncEnd;
        }//An else clause can do the job of goto, No? -Mehrdad :D
        d->editor->setHtmlContent(d->htmlEditor->document()->text());
        d->editor->setFocus();
        d->editor->startEditing();
    } else if ( index == 1 ) {
        if ( d->prev_index == 2 ) {
            d->preview->stop();
            goto SyncEnd;
        }
        d->htmlEditor->document()->setText( d->editor->htmlContent() );
        d->htmlEditor->setFocus();
    } else {
        if ( d->prev_index == 1 ) {
            d->editor->setHtmlContent(d->htmlEditor->document()->text());
        } else {
            d->htmlEditor->document()->setText( d->editor->htmlContent() );
        }
        d->preview->setHtml( d->currentPostTitle, d->htmlEditor->document()->text() );
    }
SyncEnd:
    d->prev_index = index;
}

QString BilboEditor::htmlContent()
{
    if ( this->currentIndex() == 0 ) {
        d->htmlEditor->document()->setText( d->editor->htmlContent() );
    }
    return d->editor->htmlContent();
}

void BilboEditor::setHtmlContent( const QString & content )
{
    this->d->editor->setHtmlContent(content);
    this->d->htmlEditor->document()->setText( content );
}

void BilboEditor::setCurrentTitle( const QString& title)
{
    if ( title.isEmpty() ) {
        d->currentPostTitle = i18n( "Post Title" );
    } else {
        d->currentPostTitle = title;
    }
}

QList< BilboMedia* > BilboEditor::localImages()
{
    return d->editor->getLocalImages();
}

TextEditor* BilboEditor::editor()
{
    return d->editor;
}

void BilboEditor::sltSetPostPreview()
{
    if ( this->currentIndex() == 2 ) {
        d->preview->setHtml( d->currentPostTitle, d->htmlEditor->document()->text() );
    }
}

void BilboEditor::slotSettingsChanged()
{
//     d->editor->setCheckSpellingEnabled( Settings::enableCheckSpelling() );
}

#include "composer/bilboeditor.moc"
