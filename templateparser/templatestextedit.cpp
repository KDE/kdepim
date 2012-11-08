/* Copyright (C) 2011, 2012 Laurent Montel <montel@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "templatestextedit.h"
#include "templatessyntaxhighlighter.h"

#include <KGlobalSettings>

#include <QCompleter>
#include <QKeyEvent>
#include <QScrollBar>
#include <QStringListModel>

using namespace TemplateParser;

TemplatesTextEdit::TemplatesTextEdit( QWidget *parent )
  :KTextEdit( parent )
{
  setFocus();
  const QFont f = KGlobalSettings::fixedFont();
  setFont( f );

  setWordWrapMode ( QTextOption::NoWrap );
  (void) new TemplatesSyntaxHighlighter( document() );
  initCompleter();
}

TemplatesTextEdit::~TemplatesTextEdit()
{
}

void TemplatesTextEdit::initCompleter()
{
  QStringList listWord;
  listWord << QLatin1String( "%QUOTE" )
           << QLatin1String( "%FORCEDPLAIN" )
           << QLatin1String( "%FORCEDHTML" )
           << QLatin1String( "%QHEADERS" )
           << QLatin1String( "%HEADERS" )
           << QLatin1String( "%TEXT" )
           << QLatin1String( "%OTEXTSIZE" )
           << QLatin1String( "%OTEXT" )
           << QLatin1String( "%OADDRESSEESADDR" )
           << QLatin1String( "%CCADDR" )
           << QLatin1String( "%CCNAME" )
           << QLatin1String( "%CCFNAME" )
           << QLatin1String( "%CCLNAME" )
           << QLatin1String( "%TOADDR" )
           << QLatin1String( "%TONAME" )
           << QLatin1String( "%TOFNAME" )
           << QLatin1String( "%TOLNAME" )
           << QLatin1String( "%TOLIST" )
           << QLatin1String( "%FROMADDR" )
           << QLatin1String( "%FROMNAME" )
           << QLatin1String( "%FROMFNAME" )
           << QLatin1String( "%FROMLNAME" )
           << QLatin1String( "%FULLSUBJECT" )
           << QLatin1String( "%FULLSUBJ" )
           << QLatin1String( "%MSGID" )
           << QLatin1String( "%HEADER( " )
           << QLatin1String( "%OCCADDR" )
           << QLatin1String( "%OCCNAME" )
           << QLatin1String( "%OCCFNAME" )
           << QLatin1String( "%OCCLNAME" )
           << QLatin1String( "%OTOADDR" )
           << QLatin1String( "%OTONAME" )
           << QLatin1String( "%OTOFNAME" )
           << QLatin1String( "%OTOLNAME" )
           << QLatin1String( "%OTOLIST" )
           << QLatin1String( "%OTO" )
           << QLatin1String( "%OFROMADDR" )
           << QLatin1String( "%OFROMNAME" )
           << QLatin1String( "%OFROMFNAME" )
           << QLatin1String( "%OFROMLNAME" )
           << QLatin1String( "%OFULLSUBJECT" )
           << QLatin1String( "%OFULLSUBJ" )
           << QLatin1String( "%OMSGID" )
           << QLatin1String( "%DATEEN" )
           << QLatin1String( "%DATESHORT" )
           << QLatin1String( "%DATE" )
           << QLatin1String( "%DOW" )
           << QLatin1String( "%TIMELONGEN" )
           << QLatin1String( "%TIMELONG" )
           << QLatin1String( "%TIME" )
           << QLatin1String( "%ODATEEN" )
           << QLatin1String( "%ODATESHORT" )
           << QLatin1String( "%ODATE" )
           << QLatin1String( "%ODOW" )
           << QLatin1String( "%OTIMELONGEN" )
           << QLatin1String( "%OTIMELONG" )
           << QLatin1String( "%OTIME" )
           << QLatin1String( "%BLANK" )
           << QLatin1String( "%NOP" )
           << QLatin1String( "%CLEAR" )
           << QLatin1String( "%DEBUGOFF" )
           << QLatin1String( "%DEBUG" )
           << QLatin1String( "%CURSOR" )
           << QLatin1String( "%SIGNATURE" );

  listWord << QLatin1String( "%REM=\"\"%-" )
           << QLatin1String( "%INSERT=\"\"" )
           << QLatin1String( "%SYSTEM=\"\"" )
           << QLatin1String( "%PUT=\"\"" )
           << QLatin1String( "%QUOTEPIPE=\"\"" )
           << QLatin1String( "%MSGPIPE=\"\"" )
           << QLatin1String( "%BODYPIPE=\"\"" )
           << QLatin1String( "%CLEARPIPE=\"\"" )
           << QLatin1String( "%TEXTPIPE=\"\"" )
           << QLatin1String( "%OHEADER=\"\"" )
           << QLatin1String( "%HEADER=\".*\"" );

  m_completer = new QCompleter( this );
  m_completer->setModel( new QStringListModel( listWord, m_completer ) );
  m_completer->setModelSorting( QCompleter::CaseSensitivelySortedModel );
  m_completer->setCaseSensitivity( Qt::CaseInsensitive );

  m_completer->setWidget( this );
  m_completer->setCompletionMode( QCompleter::PopupCompletion );

  connect( m_completer, SIGNAL(activated(QString)), this, SLOT(slotInsertCompletion(QString)) );
}

void TemplatesTextEdit::slotInsertCompletion( const QString &completion )
{
  QTextCursor tc = textCursor();
  int extra = completion.length() - m_completer->completionPrefix().length();
  tc.movePosition( QTextCursor::Left );
  tc.movePosition( QTextCursor::EndOfWord );
  tc.insertText( completion.right( extra ) );
  setTextCursor( tc );

}

void TemplatesTextEdit::keyPressEvent( QKeyEvent *e )
{
  if( m_completer->popup()->isVisible() ) {
    switch ( e->key() ) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
    case Qt::Key_Escape:
    case Qt::Key_Tab:
    case Qt::Key_Backtab:
      e->ignore();
      return; // let the completer do default behavior
    default:
      break;
    }
  }
  KTextEdit::keyPressEvent( e );
  const QString text = wordUnderCursor();
  if( text.length() < 2 ) {
    // min 2 char for completion
    return;
  }

  m_completer->setCompletionPrefix( text );

  QRect cr = cursorRect();
  cr.setWidth( m_completer->popup()->sizeHintForColumn( 0 ) +
               m_completer->popup()->verticalScrollBar()->sizeHint().width() );
  m_completer->complete( cr );
}

QString TemplatesTextEdit::wordUnderCursor()
{
    static QString eow =
      QLatin1String( "~!@#$^&*()+{}|\"<>,./;'[]\\-= " ); // everything without ':', '?' and '_'
    QTextCursor tc = textCursor();

    tc.anchor();
    while ( 1 ) {
      // vHanda: I don't understand why the cursor seems to give a pos 1 past
      // the last char instead of just the last char.
      int pos = tc.position() - 1;
      if ( pos < 0 || eow.contains( document()->characterAt(pos) ) 
		   || document()->characterAt(pos) == QChar(QChar::LineSeparator) 
		   || document()->characterAt(pos) == QChar(QChar::ParagraphSeparator)) {
        break;
      }
      tc.movePosition( QTextCursor::Left, QTextCursor::KeepAnchor );
    }
    return tc.selectedText();
}

#include "templatestextedit.moc"

