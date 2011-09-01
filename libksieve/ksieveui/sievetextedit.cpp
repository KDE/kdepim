/* Copyright (C) 2011 Laurent Montel <montel@kde.org>
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

#include "sievetextedit.h"
#include "sievesyntaxhighlighter.h"
#include <kglobalsettings.h>
#include <QCompleter>
#include <QStringListModel>
#include <QKeyEvent>
#include <QAbstractItemView>
#include <QScrollBar>

using namespace KSieveUi;

SieveTextEdit::SieveTextEdit( QWidget *parent )
  :KTextEdit( parent )
{
  setFocus();
  setAcceptRichText( false );
  setCheckSpellingEnabled( false );
  setWordWrapMode ( QTextOption::NoWrap );
  setFont( KGlobalSettings::fixedFont() );
  (void) new SieveSyntaxHighlighter( document() );
  initCompleter();
}

SieveTextEdit::~SieveTextEdit()
{
}


void SieveTextEdit::initCompleter()
{
  QStringList listWord;

  listWord<< QLatin1String( "require" )<<QLatin1String( "stop" );
  listWord << QLatin1String( ":contains" )<<QLatin1String( ":matches" )<<QLatin1String( ":is" )<<QLatin1String( ":over" )<<QLatin1String( ":under" );
  listWord << QLatin1String( "if" )<<QLatin1String( "elsif" )<<QLatin1String( "else" );
  listWord << QLatin1String( "keep" )<<QLatin1String( "reject" )<<QLatin1String( "discard" )<<QLatin1String( "redirect" )<<QLatin1String( "fileinto" );
  listWord << QLatin1String( "address" )<<QLatin1String( "allof" )<<QLatin1String( "anyof" )<<QLatin1String( "exists" )<<QLatin1String( "false" )<<QLatin1String( "header" )<<QLatin1String("not" )<<QLatin1String( "size" )<<QLatin1String( "true" );
  
  m_completer = new QCompleter( this );
  m_completer->setModel( new QStringListModel( listWord, m_completer ) );
  m_completer->setModelSorting( QCompleter::CaseSensitivelySortedModel );
  m_completer->setCaseSensitivity( Qt::CaseInsensitive );
  
  m_completer->setWidget( this );
  m_completer->setCompletionMode( QCompleter::PopupCompletion );
  
  connect( m_completer, SIGNAL(activated(QString)), this, SLOT(slotInsertCompletion(QString)) );
}

void SieveTextEdit::slotInsertCompletion( const QString& completion )
{
  QTextCursor tc = textCursor();
  int extra = completion.length() - m_completer->completionPrefix().length();
  tc.movePosition(QTextCursor::Left);
  tc.movePosition(QTextCursor::EndOfWord);
  tc.insertText(completion.right(extra));
  setTextCursor(tc);

}

void SieveTextEdit::keyPressEvent(QKeyEvent* e)
{
  if( m_completer->popup()->isVisible() ) {
    switch (e->key()) {
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
  KTextEdit::keyPressEvent(e);
  QString text = wordUnderCursor();
  if( text.length() < 2 ) // min 2 char for completion
    return;

  m_completer->setCompletionPrefix( text );

  QRect cr = cursorRect();
  cr.setWidth( m_completer->popup()->sizeHintForColumn(0)
               + m_completer->popup()->verticalScrollBar()->sizeHint().width() );
  m_completer->complete( cr );
}

QString SieveTextEdit::wordUnderCursor()
{
    static QString eow = QLatin1String( "~!@#$%^&*()+{}|\"<>,./;'[]\\-= " ); // everything without ':', '?' and '_'
    QTextCursor tc = textCursor();

    tc.anchor();
    while( 1 ) {
        // vHanda: I don't understand why the cursor seems to give a pos 1 past the last char instead
        // of just the last char.
        int pos = tc.position() - 1;
        if( pos < 0 || eow.contains( document()->characterAt(pos) ) )
            break;
        tc.movePosition( QTextCursor::Left, QTextCursor::KeepAnchor );
    }
    return tc.selectedText();
}

#include "sievetextedit.moc"
