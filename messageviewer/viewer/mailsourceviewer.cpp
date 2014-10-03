/*  -*- mode: C++; c-file-style: "gnu" -*-
 *
 *  This file is part of KMail, the KDE mail client.
 *
 *  Copyright (c) 2002-2003 Carsten Pfeiffer <pfeiffer@kde.org>
 *  Copyright (c) 2003      Zack Rusin <zack@kde.org>
 *
 *  KMail is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License, version 2, as
 *  published by the Free Software Foundation.
 *
 *  KMail is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *  In addition, as a special exception, the copyright holders give
 *  permission to link the code of this program with any edition of
 *  the Qt library by Trolltech AS, Norway (or with modified versions
 *  of Qt that use the same license as Qt), and distribute linked
 *  combinations including the two.  You must obey the GNU General
 *  Public License in all respects for all of the code used other than
 *  Qt.  If you modify this file, you may extend this exception to
 *  your version of the file, but you are not obligated to do so.  If
 *  you do not wish to do so, delete this exception statement from
 *  your version.
 */


#include "mailsourceviewer.h"
#include "utils/util.h"
#include "findbar/findbarsourceview.h"
#include "kpimtextedit/htmlhighlighter.h"
#include "pimcommon/widgets/slidecontainer.h"
#include "pimcommon/util/pimutil.h"
#include <kiconloader.h>
#include <KLocalizedString>
#include <KStandardAction>
#include <kwindowsystem.h>
#include <kglobalsettings.h>
#include <KTabWidget>
#include <KFileDialog>
#include <KMessageBox>
#include <KAction>

#include <QtCore/QRegExp>
#include <QApplication>
#include <QIcon>
#include <QShortcut>
#include <QVBoxLayout>
#include <QContextMenuEvent>
#include <QDebug>
#include <QMenu>

namespace MessageViewer {


MailSourceViewTextBrowserWidget::MailSourceViewTextBrowserWidget( QWidget *parent )
    :QWidget( parent )
{
    QVBoxLayout *lay = new QVBoxLayout;
    setLayout( lay );
    lay->setMargin( 0 );
    mTextBrowser = new MailSourceViewTextBrowser();
    mTextBrowser->setLineWrapMode( QPlainTextEdit::NoWrap );
    mTextBrowser->setTextInteractionFlags( Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard );
    connect( mTextBrowser, SIGNAL(findText()), SLOT(slotFind()) );
    lay->addWidget( mTextBrowser );
    mSliderContainer = new PimCommon::SlideContainer(this);

    mFindBar = new FindBarSourceView( mTextBrowser, this );
    connect(mFindBar, SIGNAL(hideFindBar()), mSliderContainer, SLOT(slideOut()));
    mSliderContainer->setContent(mFindBar);

    lay->addWidget( mSliderContainer );
    QShortcut *shortcut = new QShortcut( this );
    shortcut->setKey( Qt::Key_F+Qt::CTRL );
    connect( shortcut, SIGNAL(activated()), SLOT(slotFind()) );
}

void MailSourceViewTextBrowserWidget::slotFind()
{
    if ( mTextBrowser->textCursor().hasSelection() )
        mFindBar->setText( mTextBrowser->textCursor().selectedText() );
    mSliderContainer->slideIn();
    mFindBar->focusAndSetCursor();
}

void MailSourceViewTextBrowserWidget::setText( const QString& text )
{
    mTextBrowser->setPlainText( text );
}

void MailSourceViewTextBrowserWidget::setPlainText( const QString& text )
{
    mTextBrowser->setPlainText( text );
}

void MailSourceViewTextBrowserWidget::setFixedFont()
{
    mTextBrowser->setFont( KGlobalSettings::fixedFont() );
}

MessageViewer::MailSourceViewTextBrowser *MailSourceViewTextBrowserWidget::textBrowser() const
{
    return mTextBrowser;
}

MailSourceViewTextBrowser::MailSourceViewTextBrowser( QWidget *parent )
    :QPlainTextEdit( parent )
{
}

void MailSourceViewTextBrowser::contextMenuEvent( QContextMenuEvent *event )
{
    QMenu *popup = createStandardContextMenu();
    if (popup) {
        popup->addSeparator();
        popup->addAction(KStandardAction::find(this, SIGNAL(findText()), this));
        //Code from KTextBrowser
        KIconTheme::assignIconsToContextMenu( isReadOnly() ? KIconTheme::ReadOnlyText
                                                           : KIconTheme::TextEditor,
                                              popup->actions() );
        popup->addSeparator();
        popup->addAction( KIcon(QLatin1String("preferences-desktop-text-to-speech")),i18n("Speak Text"),this,SLOT(slotSpeakText()));

        popup->addSeparator();
        popup->addAction(KStandardAction::saveAs(this, SLOT(slotSaveAs()), this));

        popup->exec( event->globalPos() );
        delete popup;
    }
}

void MailSourceViewTextBrowser::slotSaveAs()
{
    PimCommon::Util::saveTextAs( toPlainText(), QString(), this );
}

void MailSourceViewTextBrowser::slotSpeakText()
{
    QString text;
    if ( textCursor().hasSelection() ) {
        text = textCursor().selectedText();
    } else {
        text = toPlainText();
    }
    MessageViewer::Util::speakSelectedText( text, this);
}

void MailSourceHighlighter::highlightBlock ( const QString & text ) {
    // all visible ascii except space and :
    const QRegExp regexp( QLatin1String("^([\\x21-9;-\\x7E]+:\\s)") );

    // keep the previous state
    setCurrentBlockState( previousBlockState() );
    // If a header is found
    if( regexp.indexIn( text ) != -1 )
    {
        const int headersState = -1; // Also the initial State
        // Content- header starts a new mime part, and therefore new headers
        // If a Content-* header is found, change State to headers until a blank line is found.
        if ( text.startsWith( QLatin1String( "Content-" ) ) )
        {
            setCurrentBlockState( headersState );
        }
        // highligth it if in headers state
        if( ( currentBlockState() == headersState ) )
        {
            QFont font = document()->defaultFont ();
            font.setBold( true );
            setFormat( 0, regexp.matchedLength(), font );
        }
    }
    // Change to body state
    else if ( text.isEmpty() )
    {
        const int bodyState = 0;
        setCurrentBlockState( bodyState );
    }
}

const QString HTMLPrettyFormatter::reformat( const QString &src )
{
    const QRegExp cleanLeadingWhitespace( QLatin1String("(?:\\n)+\\w*") );
    QStringList tmpSource;
    QString source( src );
    int pos = 0;
    QString indent;

    //First make sure that each tag is surrounded by newlines
    while( (pos = htmlTagRegExp.indexIn( source, pos ) ) != -1 )
    {
        source.insert(pos, QLatin1Char('\n'));
        pos += htmlTagRegExp.matchedLength() + 1;
        source.insert(pos, QLatin1Char('\n'));
        pos++;
    }

    // Then split the source on newlines skiping empty parts.
    // Now a line is either a tag or pure data.
    tmpSource = source.split(QLatin1Char('\n'), QString::SkipEmptyParts );

    // Then clean any leading whitespace
    for( int i = 0; i != tmpSource.length(); ++i )
    {
        tmpSource[i] = tmpSource[i].remove( cleanLeadingWhitespace );
    }

    // Then indent as appropriate
    for( int i = 0; i != tmpSource.length(); ++i )  {
        if( htmlTagRegExp.indexIn( tmpSource.at(i) ) != -1 ) // A tag
        {
            if( htmlTagRegExp.cap( 3 ) == QLatin1String( "/" ) ||
                    htmlTagRegExp.cap( 2 ) == QLatin1String( "img" ) ||
                    htmlTagRegExp.cap( 2 ) == QLatin1String( "br" ) ) {
                //Self closing tag or no closure needed
                continue;
            }
            if( htmlTagRegExp.cap( 1 ) == QLatin1String( "/" ) ) {
                // End tag
                indent.chop( 2 );
                tmpSource[i].prepend( indent );
                continue;
            }
            // start tag
            tmpSource[i].prepend( indent );
            indent.append( QLatin1String("  ") );
            continue;
        }
        // Data
        tmpSource[i].prepend( indent );
    }

    // Finally reassemble and return :)
    return tmpSource.join( QLatin1String("\n") );
}

MailSourceViewer::MailSourceViewer( QWidget *parent )
    : KDialog( parent )
{
    setAttribute( Qt::WA_DeleteOnClose );
    setButtons( Close );

    QVBoxLayout *layout = new QVBoxLayout( mainWidget() );
    layout->setMargin( 0 );
    connect( this, SIGNAL(closeClicked()), SLOT(close()) );

    mRawBrowser = new MailSourceViewTextBrowserWidget();

#ifndef NDEBUG
    mTabWidget = new KTabWidget;
    layout->addWidget( mTabWidget );

    mTabWidget->addTab( mRawBrowser, i18nc( "Unchanged mail message", "Raw Source" ) );
    mTabWidget->setTabToolTip( 0, i18n( "Raw, unmodified mail as it is stored on the filesystem or on the server" ) );

    mHtmlBrowser = new MailSourceViewTextBrowserWidget();
    mTabWidget->addTab( mHtmlBrowser, i18nc( "Mail message as shown, in HTML format", "HTML Source" ) );
    mTabWidget->setTabToolTip( 1, i18n( "HTML code for displaying the message to the user" ) );
    new KPIMTextEdit::HtmlHighlighter( mHtmlBrowser->textBrowser()->document() );

    mTabWidget->setCurrentIndex( 0 );
#else
    layout->addWidget( mRawBrowser );
#endif

    // combining the shortcuts in one qkeysequence() did not work...
    QShortcut* shortcut = new QShortcut( this );
    shortcut->setKey( Qt::Key_Escape );
    connect( shortcut, SIGNAL(activated()), SLOT(close()) );
    shortcut = new QShortcut( this );
    shortcut->setKey( Qt::Key_W+Qt::CTRL );
    connect( shortcut, SIGNAL(activated()), SLOT(close()) );

    KWindowSystem::setIcons( winId(),
                             qApp->windowIcon().pixmap( IconSize( KIconLoader::Desktop ),
                                                        IconSize( KIconLoader::Desktop ) ),
                             qApp->windowIcon().pixmap( IconSize( KIconLoader::Small ),
                                                        IconSize( KIconLoader::Small ) ) );
    new MailSourceHighlighter( mRawBrowser->textBrowser()->document() );
    mRawBrowser->textBrowser()->setFocus();
}

MailSourceViewer::~MailSourceViewer()
{
}

void MailSourceViewer::setRawSource( const QString &source )
{
    mRawBrowser->setText( source );
}

void MailSourceViewer::setDisplayedSource( const QString &source )
{
#ifndef NDEBUG
    mHtmlBrowser->setPlainText( HTMLPrettyFormatter::reformat( source ) );
#else
    Q_UNUSED( source );
#endif
}

void MailSourceViewer::setFixedFont()
{
    mRawBrowser->setFixedFont();
#ifndef NDEBUG
    mHtmlBrowser->setFixedFont();
#endif
}

}
