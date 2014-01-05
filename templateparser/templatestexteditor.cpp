/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "templatestexteditor.h"
#include "templatessyntaxhighlighter.h"
#include "templatesutil.h"

#include <KGlobalSettings>

#include <QCompleter>
#include <QKeyEvent>
#include <QScrollBar>
#include <QStringListModel>

using namespace TemplateParser;

TemplatesTextEditor::TemplatesTextEditor(QWidget *parent)
    : PimCommon::RichTextEditor(parent)
{
    setFocus();
    const QFont f = KGlobalSettings::fixedFont();
    setFont( f );

    setWordWrapMode ( QTextOption::NoWrap );
    (void) new TemplatesSyntaxHighlighter( document() );
    initCompleter();
}

TemplatesTextEditor::~TemplatesTextEditor()
{

}

void TemplatesTextEditor::initCompleter()
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

void TemplatesTextEditor::slotInsertCompletion( const QString &completion )
{
    QTextCursor tc = textCursor();
    tc.movePosition( QTextCursor::StartOfWord );
    tc.movePosition( QTextCursor::EndOfWord, QTextCursor::KeepAnchor );
    tc.removeSelectedText();
    tc.insertText( completion.right( completion.length()-1 ) );
    setTextCursor( tc );
}

void TemplatesTextEditor::keyPressEvent( QKeyEvent *e )
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
    PimCommon::RichTextEditor::keyPressEvent( e );
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

QString TemplatesTextEditor::wordUnderCursor()
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

