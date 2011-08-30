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
#include <config-messageviewer.h>

#include "mailsourceviewer.h"
#include "findbar/findbarsourceview.h"
#include <kiconloader.h>
#include <KLocalizedString>
#include <kstandardguiitem.h>
#include <kwindowsystem.h>

#include <QtCore/QRegExp>
#include <QtGui/QApplication>
#include <QtGui/QIcon>
#include <QtGui/QPushButton>
#include <QtGui/QShortcut>
#include <QtGui/QTabBar>
#include <QtGui/QVBoxLayout>

namespace MessageViewer {

void MailSourceHighlighter::highlightBlock ( const QString & text ) {
  // all visible ascii except space and :
  const QRegExp regexp( "^([\\x21-9;-\\x7E]+:\\s)" );
  const int headersState = -1; // Also the initial State
  const int bodyState = 0;

  // keep the previous state
  setCurrentBlockState( previousBlockState() );
  // If a header is found
  if( regexp.indexIn( text ) != -1 )
  {
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
    setCurrentBlockState( bodyState );
  }
}

void HTMLSourceHighlighter::highlightBlock ( const QString & text ) {
  int pos = 0;
  if( ( pos = HTMLPrettyFormatter::htmlTagRegExp.indexIn( text ) ) != -1 )
  {
    QFont font = document()->defaultFont();
    font.setBold( true );
    setFormat( pos, HTMLPrettyFormatter::htmlTagRegExp.matchedLength(), font );
  }
}

const QString HTMLPrettyFormatter::reformat( const QString &src )
{
  const QRegExp cleanLeadingWhitespace( "(?:\\n)+\\w*" );
  QStringList tmpSource;
  QString source( src );
  int pos = 0;
  QString indent = "";

  //First make sure that each tag is surrounded by newlines
  while( (pos = htmlTagRegExp.indexIn( source, pos ) ) != -1 )
  {
    source.insert(pos, '\n');
    pos += htmlTagRegExp.matchedLength() + 1;
    source.insert(pos, '\n');
    pos++;
  }

  // Then split the source on newlines skiping empty parts.
  // Now a line is either a tag or pure data.
  tmpSource = source.split('\n', QString::SkipEmptyParts );

  // Then clean any leading whitespace
  for( int i = 0; i != tmpSource.length(); i++ )
  {
    tmpSource[i] = tmpSource[i].remove( cleanLeadingWhitespace );
  }

  // Then indent as appropriate
  for( int i = 0; i != tmpSource.length(); i++ )  {
    if( htmlTagRegExp.indexIn( tmpSource[i] ) != -1 ) // A tag
    {
      if( htmlTagRegExp.cap( 3 ) == "/" || htmlTagRegExp.cap( 2 ) == "img" || htmlTagRegExp.cap( 2 ) == "br" ) {
        //Self closing tag or no closure needed
        continue;
      }
      if( htmlTagRegExp.cap( 1 ) == "/" ) {
        // End tag
        indent.chop( 2 );
        tmpSource[i].prepend( indent );
        continue;
      }
      // start tag
      tmpSource[i].prepend( indent );
      indent.append( "  " );
      continue;
    }
    // Data
    tmpSource[i].prepend( indent );
  }

  // Finally reassemble and return :)
  return tmpSource.join( "\n" );
}

MailSourceViewer::MailSourceViewer( QWidget *parent )
  : KDialog( parent ), mRawSourceHighLighter( 0 )
{
  setAttribute( Qt::WA_DeleteOnClose );
  setButtons( Close );

  QVBoxLayout *layout = new QVBoxLayout( mainWidget() );
  layout->setMargin( 0 );
  mTabWidget = new KTabWidget;
  layout->addWidget( mTabWidget );

  connect( this, SIGNAL(closeClicked()), SLOT(close()) );


  QWidget *widget = new QWidget;
  QVBoxLayout *lay = new QVBoxLayout;
  widget->setLayout( lay );  
  mRawBrowser = new KTextBrowser();
  lay->addWidget( mRawBrowser );
  mFindBar = new FindBarSourceView( mRawBrowser, widget );
  lay->addWidget( mFindBar );
  mTabWidget->addTab( widget, i18nc( "Unchanged mail message", "Raw Source" ) );
  mTabWidget->setTabToolTip( 0, i18n( "Raw, unmodified mail as it is stored on the filesystem or on the server" ) );
  mRawBrowser->setLineWrapMode( QTextEdit::NoWrap );
  mRawBrowser->setTextInteractionFlags( Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard );

#ifndef NDEBUG
  mHtmlBrowser = new KTextBrowser();
  mTabWidget->addTab( mHtmlBrowser, i18nc( "Mail message as shown, in HTML format", "HTML Source" ) );
  mTabWidget->setTabToolTip( 1, i18n( "HTML code for displaying the message to the user" ) );
  mHtmlBrowser->setLineWrapMode( QTextEdit::NoWrap );
  mHtmlBrowser->setTextInteractionFlags( Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard );
  mHtmlSourceHighLighter = new HTMLSourceHighlighter( mHtmlBrowser );
#endif

  mTabWidget->setCurrentIndex( 0 );

  // combining the shortcuts in one qkeysequence() did not work...
  QShortcut* shortcut = new QShortcut( this );
  shortcut->setKey( Qt::Key_Escape );
  connect( shortcut, SIGNAL(activated()), SLOT(close()) );

  shortcut = new QShortcut( this );
  shortcut->setKey( Qt::Key_W+Qt::CTRL );
  connect( shortcut, SIGNAL(activated()), SLOT(close()) );

  shortcut = new QShortcut( this );
  shortcut->setKey( Qt::Key_F+Qt::CTRL );
  connect( shortcut, SIGNAL(activated()), SLOT(slotFind()) );
  
  KWindowSystem::setIcons( winId(),
                  qApp->windowIcon().pixmap( IconSize( KIconLoader::Desktop ),
                  IconSize( KIconLoader::Desktop ) ),
                  qApp->windowIcon().pixmap( IconSize( KIconLoader::Small ),
                  IconSize( KIconLoader::Small ) ) );
  mRawSourceHighLighter = new MailSourceHighlighter( mRawBrowser );
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

void MailSourceViewer::slotFind()
{
  mFindBar->show();
  mFindBar->focusAndSetCursor();  
}
  
#include "mailsourceviewer.moc"
}
