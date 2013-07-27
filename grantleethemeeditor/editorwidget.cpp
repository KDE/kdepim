/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#include "editorwidget.h"

#include <kpimtextedit/htmlhighlighter.h>

#include <QStringListModel>
#include <QCompleter>
#include <QKeyEvent>
#include <QScrollBar>
#include <QDebug>

using namespace GrantleeThemeEditor;

EditorWidget::EditorWidget(QWidget *parent)
    : KTextEdit(parent)
{
    new KPIMTextEdit::HtmlHighlighter(document());
    setAcceptRichText(false);
    initCompleter();
}

EditorWidget::~EditorWidget()
{
}

void EditorWidget::insertFile(const QString &filename)
{
    if (!filename.isEmpty()) {
        QFile file( filename );

        if ( file.open( QIODevice::ReadOnly ) ) {
            const QByteArray data = file.readAll();
            const QString str = QString::fromUtf8(data);
            insertPlainText(str);
        }
    }
}

void EditorWidget::initCompleter()
{
    m_completer = new QCompleter( this );
    m_completer->setModelSorting( QCompleter::CaseSensitivelySortedModel );
    m_completer->setCaseSensitivity( Qt::CaseInsensitive );

    m_completer->setWidget( this );
    m_completer->setCompletionMode( QCompleter::PopupCompletion );

    connect( m_completer, SIGNAL(activated(QString)), this, SLOT(slotInsertCompletion(QString)) );
}

void EditorWidget::createCompleterList(const QStringList &extraCompletion)
{
    QStringList listWord;
    listWord << extraCompletion;
    m_completer->setModel( new QStringListModel( listWord, m_completer ) );
}

void EditorWidget::slotInsertCompletion( const QString &completion )
{
    QTextCursor tc = textCursor();
    const int extra = completion.length() - m_completer->completionPrefix().length();
    tc.movePosition(QTextCursor::Left);
    tc.movePosition(QTextCursor::EndOfWord);
    tc.insertText(completion.right(extra));
    setTextCursor(tc);
}

void EditorWidget::keyPressEvent(QKeyEvent* e)
{
    if ( m_completer->popup()->isVisible() ) {
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
    const QString text = wordUnderCursor();
    if ( text.length() < 2 ) // min 2 char for completion
        return;

    m_completer->setCompletionPrefix( text );

    QRect cr = cursorRect();
    cr.setWidth( m_completer->popup()->sizeHintForColumn(0)
                 + m_completer->popup()->verticalScrollBar()->sizeHint().width() );
    m_completer->complete( cr );
}

QString EditorWidget::wordUnderCursor() const
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

#include "editorwidget.moc"
