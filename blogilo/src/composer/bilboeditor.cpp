/*
    This file is part of Blogilo, A KDE Blogging Client

    Copyright (C) 2008-2010 Mehrdad Momeny <mehrdad.momeny@gmail.com>
    Copyright (C) 2008-2010 Golnaz Nilieh <g382nilieh@gmail.com>

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
#include "dialogs/addeditimage.h"

#include "htmlconvertors/bilbotextformat.h"
#include "htmlconvertors/bilbotexthtmlimporter.h"
#include "htmlconvertors/htmlexporter.h"
#include <settings.h>

#define FORWARD_ACTION(action1, action2) \
    connect(action1, SIGNAL(triggered()), d->wysiwygEditor->getAction(action2), SLOT(trigger()));\
    connect(d->wysiwygEditor->getAction(action2), SIGNAL(changed()), SLOT(adjustActions()));

class BilboEditor::Private
{
public:
    QWidget *tabVisual;
    QWidget *tabHtml;
    QWidget *tabPreview;

    TextEditor *wysiwygEditor;
    KTextEditor::View *htmlEditor;

    BilboBrowser *previewer;

    KToolBar *barVisual;

    QString currentPostTitle;
    int prev_index;
//     QColor codeBackground;
};

BilboEditor::BilboEditor( QWidget *parent )
        : KTabWidget( parent ), d(new Private)
{
    createUi();
    connect( d->wysiwygEditor, SIGNAL( textChanged() ), this, SIGNAL( textChanged() ) );
    connect( d->htmlEditor->document(), SIGNAL( textChanged( KTextEditor::Document * ) ),
             this, SIGNAL( textChanged() ) );
    connect( Settings::self(), SIGNAL(configChanged()),
             this, SLOT(slotSettingsChanged()) );
//     d->wysiwygEditor->setCheckSpellingEnabled( Settings::enableCheckSpelling() );
    d->wysiwygEditor->setFocus();
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
    this->addTab( d->tabPreview, i18nc( "preview of the edited post", "Post Preview" ) );
    connect( this, SIGNAL( currentChanged( int ) ), this, SLOT( slotSyncEditors( int ) ) );
    d->prev_index = 0;

    /// WYSIWYG Editor:
    d->wysiwygEditor = new TextEditor( d->tabVisual );
    QVBoxLayout *vLayout = new QVBoxLayout( d->tabVisual );
    vLayout->addWidget( d->wysiwygEditor );

    ///htmlEditor:
    d->htmlEditor = HtmlEditor::self()->createView( d->tabHtml );
    QGridLayout *hLayout = new QGridLayout( d->tabHtml );
    hLayout->addWidget( d->htmlEditor );

    ///previewer:
    d->previewer = new BilboBrowser( d->tabPreview );
    QGridLayout *gLayout = new QGridLayout( d->tabPreview );
    gLayout->addWidget( d->previewer );

    connect( d->previewer, SIGNAL( sigSetBlogStyle() ), this, SLOT(
            slotSetPostPreview() ) );


    this->setCurrentIndex( 0 );

    d->currentPostTitle = i18n( "Post Title" );
}

void BilboEditor::slotSyncEditors( int index )
{
    kDebug();

    if ( index == 0 ) {
        if ( d->prev_index == 2 ) {
            d->previewer->stop();
            goto SyncEnd;
        }//An else clause can do the job of goto, No? -Mehrdad :D
        d->wysiwygEditor->setHtmlContent(d->htmlEditor->document()->text());
        d->wysiwygEditor->setFocus();
        d->wysiwygEditor->startEditing();
    } else if ( index == 1 ) {
        if ( d->prev_index == 2 ) {
            d->previewer->stop();
            goto SyncEnd;
        }
        d->htmlEditor->document()->setText( d->wysiwygEditor->htmlContent() );
        d->htmlEditor->setFocus();
    } else {
        if ( d->prev_index == 1 ) {
            d->wysiwygEditor->setHtmlContent(d->htmlEditor->document()->text());
        } else {
            d->htmlEditor->document()->setText( d->wysiwygEditor->htmlContent() );
        }
        d->previewer->setHtml( d->currentPostTitle, d->htmlEditor->document()->text() );
    }
SyncEnd:
    d->prev_index = index;
}

QString BilboEditor::plainTextContent()
{
    return d->wysiwygEditor->plainTextContent();
}

QString BilboEditor::htmlContent()
{
    if ( this->currentIndex() == 0 ) {
        d->htmlEditor->document()->setText( d->wysiwygEditor->htmlContent() );
    }
    return d->wysiwygEditor->htmlContent();
}

void BilboEditor::setHtmlContent( const QString & content )
{
    this->d->wysiwygEditor->setHtmlContent(content);
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
    return d->wysiwygEditor->getLocalImages();
}

void BilboEditor::replaceImageSrc(const QString& src, const QString& dest)
{
    d->wysiwygEditor->replaceImageSrc(src, dest);
}

void BilboEditor::slotSetPostPreview()
{
    if ( this->currentIndex() == 2 ) {
        d->previewer->setHtml( d->currentPostTitle, d->htmlEditor->document()->text() );
    }
}

void BilboEditor::slotSettingsChanged()
{
//     d->wysiwygEditor->setCheckSpellingEnabled( Settings::enableCheckSpelling() );
}

#include "composer/bilboeditor.moc"
