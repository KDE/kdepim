/* Copyright (C) 2011, 2012, 2013 Laurent Montel <montel@kde.org>
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
#include "templatesutil.h"

#include <KGlobalSettings>

#include <QCompleter>
#include <QKeyEvent>
#include <QScrollBar>
#include <QStringListModel>

using namespace TemplateParser;

TemplatesTextEdit::TemplatesTextEdit( QWidget *parent )
  : PimCommon::CustomTextEdit( QLatin1String("templateparserrc"), parent )
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
  listWord << Util::keywords();
  listWord << Util::keywordsWithArgs();

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
  tc.movePosition( QTextCursor::StartOfWord );
  tc.movePosition( QTextCursor::EndOfWord, QTextCursor::KeepAnchor );
  tc.removeSelectedText();
  tc.insertText( completion.right( completion.length()-1 ) );
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
      if ( pos < 0 ||
           eow.contains( document()->characterAt( pos ) ) ||
           document()->characterAt( pos ) == QChar( QChar::LineSeparator ) ||
           document()->characterAt( pos ) == QChar( QChar::ParagraphSeparator ) ) {
        break;
      }
      tc.movePosition( QTextCursor::Left, QTextCursor::KeepAnchor );
    }
    return tc.selectedText();
}

#include "templatestextedit.moc"

