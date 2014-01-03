/*
  Copyright (c) 2013, 2014 Montel Laurent <montel.org>

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

#include "grantleeplaintexteditor.h"
#include <kpimtextedit/htmlhighlighter.h>

#include <QStringListModel>
#include <QCompleter>
#include <QKeyEvent>
#include <QScrollBar>

using namespace GrantleeThemeEditor;

GrantleePlainTextEditor::GrantleePlainTextEditor(QWidget *parent)
    : PimCommon::PlainTextEditor(parent)
{
    mHtmlHighlighter = new KPIMTextEdit::HtmlHighlighter(document());
    setSpellCheckingSupport(false);
    initCompleter();
}

GrantleePlainTextEditor::~GrantleePlainTextEditor()
{

}

void GrantleePlainTextEditor::initCompleter()
{
    mCompleter = new QCompleter( this );
    mCompleter->setModelSorting( QCompleter::CaseSensitivelySortedModel );
    mCompleter->setCaseSensitivity( Qt::CaseInsensitive );

    mCompleter->setWidget( this );
    mCompleter->setCompletionMode( QCompleter::PopupCompletion );

    connect( mCompleter, SIGNAL(activated(QString)), this, SLOT(slotInsertCompletion(QString)) );
}

void GrantleePlainTextEditor::createCompleterList(const QStringList &extraCompletion)
{
    QStringList listWord;
    listWord << extraCompletion;
    mCompleter->setModel( new QStringListModel( listWord, mCompleter ) );
}

void GrantleePlainTextEditor::slotInsertCompletion( const QString &completion )
{
    QTextCursor tc = textCursor();
    const int extra = completion.length() - mCompleter->completionPrefix().length();
    tc.movePosition(QTextCursor::Left);
    tc.movePosition(QTextCursor::EndOfWord);
    tc.insertText(completion.right(extra));
    setTextCursor(tc);
}

void GrantleePlainTextEditor::keyPressEvent(QKeyEvent* e)
{
    if ( mCompleter->popup()->isVisible() ) {
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
    PimCommon::PlainTextEditor::keyPressEvent(e);
    const QString text = wordUnderCursor();
    if ( text.length() < 2 ) // min 2 char for completion
        return;

    mCompleter->setCompletionPrefix( text );

    QRect cr = cursorRect();
    cr.setWidth( mCompleter->popup()->sizeHintForColumn(0)
                 + mCompleter->popup()->verticalScrollBar()->sizeHint().width() );
    mCompleter->complete( cr );
}

QString GrantleePlainTextEditor::wordUnderCursor() const
{
    static QString eow = QLatin1String( "~!@#$%^&*()+{}|\"<>,./;'[]\\-= " ); // everything without ':', '?' and '_'
    QTextCursor tc = textCursor();

    tc.anchor();
    while( 1 ) {
        // vHanda: I don't understand why the cursor seems to give a pos 1 past the last char instead
        // of just the last char.
        int pos = tc.position() - 1;
        if ( pos < 0 || eow.contains( document()->characterAt(pos) )
             || document()->characterAt(pos) == QChar(QChar::LineSeparator)
             || document()->characterAt(pos) == QChar(QChar::ParagraphSeparator))
            break;
        tc.movePosition( QTextCursor::Left, QTextCursor::KeepAnchor );
    }
    return tc.selectedText();
}


