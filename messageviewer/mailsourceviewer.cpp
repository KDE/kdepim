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
#include <QApplication>
#include <QIcon>
#include <QTabBar>
#include <kwindowsystem.h>
#include <KLocalizedString>

#include <QRegExp>
#include <QShortcut>
#include <kiconloader.h>

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

const QString HTMLPrettyFormatter::reformat( const QString &src )
{
  // Best to be really verbose about this one...
  const QRegExp tag( "<"
                          "(/)?"    //Captures the / if this is an end tag.
                          "(\\w+)"    //Captures TagName
                          "(?:"                //Groups tag contents
                          "(?:\\s+"            //Groups attributes
                          "(?:\\w+)"  //Attribute name
                                  "(?:"                //groups =value portion.
                                      "\\s*=\\s*"            // =
                                      "(?:"        //Groups attribute "value" portion.
                                      "\\\"(?:[^\\\"]*)\\\""    // attVal='double quoted'
                                          "|'(?:[^']*)'"        // attVal='single quoted'
                                          "|(?:[^'"">\\s]+)"    // attVal=urlnospaces
                                      ")"
                                  ")?"        //end optional att value portion.
                             ")+\\s*"        //One or more attribute pairs
                              "|\\s*"            //Some white space.
                          ")"
                       "(/)?>" //Captures the "/" if this is a complete tag.
                      );
  const QRegExp cleanLeadingWhitespace( "(?:\\n)+\\w*" );
  QStringList tmpSource;
  QString source( src );
  int pos = 0;
  QString indent = "";

  //First make sure that each tag is surrounded by newlines
  while( (pos = tag.indexIn( source, pos ) ) != -1 )
  {
    source.insert(pos, '\n');
    pos += tag.matchedLength() + 1;
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

  // Then indent as apropriate
  for( int i = 0; i != tmpSource.length(); i++ )  {
    if( tag.indexIn( tmpSource[i] ) != -1 ) // A tag
    {
      if( tag.cap( 3 ) == "/" ) {
        //Self closing tag "<br/>"
        continue;
      }
      if( tag.cap( 1 ) == "/" ) {
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
  : KTabWidget( parent ), mRawSourceHighLighter( 0 )
{
  setAttribute( Qt::WA_DeleteOnClose );
  mRawBrowser = new KTextBrowser();
  addTab( mRawBrowser, i18nc( "Unchanged mail message", "Raw Source" ) );
  setTabToolTip( 0, i18n( "Raw, unmodified mail as it is stored on the filesystem or on the server" ) );
  mRawBrowser->setLineWrapMode( QTextEdit::NoWrap );
  mRawBrowser->setTextInteractionFlags( Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard );

  mProcessedBrowser = new KTextBrowser();
  addTab( mProcessedBrowser, i18nc( "Mail message after being processed, might be alterred from original", "Processed Source") );
  setTabToolTip( 1, i18n( "Processed mail, for example after decrypting an encrypted part of the mail" ) );
  mProcessedBrowser->setLineWrapMode( QTextEdit::NoWrap );
  mProcessedBrowser->setTextInteractionFlags( Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard );

#ifndef NDEBUG
    mHtmlBrowser = new KTextBrowser();
    addTab( mHtmlBrowser, i18nc( "Mail message as shown, in HTML format", "HTML Source" ) );
    setTabToolTip( 2, i18n( "HTML code for displaying the message to the user" ) );
    mHtmlBrowser->setLineWrapMode( QTextEdit::NoWrap );
    mHtmlBrowser->setTextInteractionFlags( Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard );
#endif

  setCurrentIndex( 0 );

  // combining the shortcuts in one qkeysequence() did not work...
  QShortcut* shortcut = new QShortcut( this );
  shortcut->setKey( Qt::Key_Escape );
  connect( shortcut, SIGNAL( activated() ), SLOT( close() ) );

  shortcut = new QShortcut( this );
  shortcut->setKey( Qt::Key_W+Qt::CTRL );
  connect( shortcut, SIGNAL( activated() ), SLOT( close() ) );
  KWindowSystem::setIcons( winId(),
                  qApp->windowIcon().pixmap( IconSize( KIconLoader::Desktop ),
                  IconSize( KIconLoader::Desktop ) ),
                  qApp->windowIcon().pixmap( IconSize( KIconLoader::Small ),
                  IconSize( KIconLoader::Small ) ) );
  mRawSourceHighLighter = new MailSourceHighlighter( mRawBrowser );
  mProcessedSourceHighLighter = new MailSourceHighlighter( mProcessedBrowser );
}

MailSourceViewer::~MailSourceViewer()
{
}

void MailSourceViewer::setRawSource( const QString &source )
{
  mRawBrowser->setText( source );
}

void MailSourceViewer::setProcessedSource( const QString &source )
{
  mProcessedBrowser->setText( source );
}

void MailSourceViewer::setDisplayedSource( const QString &source )
{
#ifndef NDEBUG
  mHtmlBrowser->setPlainText( HTMLPrettyFormatter::reformat( source ) );
#else
  Q_UNUSED( source );
#endif
}

void MailSourceViewer::showEvent( QShowEvent *event )
{
  Q_UNUSED( event );
  if ( mRawBrowser->document()->toPlainText() == mProcessedBrowser->document()->toPlainText() ){
    if ( count() == 2 ) {
      tabBar()->hide();
    }
    setCurrentIndex( 0 );
    widget( 1 )->hide();
  }
}

#include "mailsourceviewer.moc"
}
