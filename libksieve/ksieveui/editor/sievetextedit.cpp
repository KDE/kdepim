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

#include "sievetextedit.h"
#include "editor/sievelinenumberarea.h"
#include "editor/sieveeditorutil.h"
#include "sievesyntaxhighlighter.h"


#include <KGlobalSettings>
#include <KLocalizedString>
#include <KAction>

#include <QCompleter>
#include <QStringListModel>
#include <QKeyEvent>
#include <QPainter>
#include <QScrollBar>
#include <QDebug>
#include <QAction>
#include <QMenu>

using namespace KSieveUi;

SieveTextEdit::SieveTextEdit( QWidget *parent )
    : PimCommon::PlainTextEditor( parent ),
      mShowHelpMenu(true)
{
    setWordWrapMode ( QTextOption::NoWrap );
    setFont( KGlobalSettings::fixedFont() );
    m_syntaxHighlighter = new PimCommon::SieveSyntaxHighlighter( document() );
    m_sieveLineNumberArea = new SieveLineNumberArea(this);

    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(slotUpdateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(slotUpdateLineNumberArea(QRect,int)));

    slotUpdateLineNumberAreaWidth(0);

    initCompleter();
}

SieveTextEdit::~SieveTextEdit()
{
}

void SieveTextEdit::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    const QRect cr = contentsRect();
    m_sieveLineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

int SieveTextEdit::lineNumberAreaWidth() const
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    const int space = 3 + fontMetrics().width(QLatin1Char('9')) * digits;
    return space;
}

void SieveTextEdit::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(m_sieveLineNumberArea);
    painter.fillRect(event->rect(), Qt::lightGray);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();
    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            const QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::black);
            painter.drawText(0, top, m_sieveLineNumberArea->width(), fontMetrics().height(),
                             Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }
}

void SieveTextEdit::slotUpdateLineNumberAreaWidth(int)
{ 
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void SieveTextEdit::slotUpdateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        m_sieveLineNumberArea->scroll(0, dy);
    else
        m_sieveLineNumberArea->update(0, rect.y(), m_sieveLineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        slotUpdateLineNumberAreaWidth(0);
}

QStringList SieveTextEdit::completerList() const
{
    QStringList listWord;

    listWord << QLatin1String( "require" ) <<QLatin1String( "stop" );
    listWord << QLatin1String( ":contains" ) <<QLatin1String( ":matches" ) <<QLatin1String( ":is" ) <<QLatin1String( ":over" ) <<QLatin1String( ":under" ) <<QLatin1String( ":all" ) <<QLatin1String( ":domain" ) <<QLatin1String( ":localpart" );
    listWord << QLatin1String( "if" ) <<QLatin1String( "elsif" ) <<QLatin1String( "else" );
    listWord << QLatin1String( "keep" ) <<QLatin1String( "reject" ) <<QLatin1String( "discard" ) <<QLatin1String( "redirect" )  <<QLatin1String( "addflag" ) <<QLatin1String( "setflag" );
    listWord << QLatin1String( "address" ) <<QLatin1String( "allof" ) <<QLatin1String( "anyof" ) <<QLatin1String( "exists" ) <<QLatin1String( "false" ) <<QLatin1String( "header" ) <<QLatin1String("not" ) <<QLatin1String( "size" ) <<QLatin1String( "true" );
    listWord << QLatin1String( ":days" ) <<QLatin1String(":seconds") <<QLatin1String(":subject") <<QLatin1String(":addresses") <<QLatin1String(":text");
    listWord << QLatin1String(":name") << QLatin1String(":headers") <<QLatin1String(":first") <<QLatin1String(":importance");
    listWord << QLatin1String(":message") << QLatin1String(":from");

    return listWord;
}

void SieveTextEdit::setCompleterList(const QStringList &list)
{
    m_completer->setModel( new QStringListModel( list, m_completer ) );
}

void SieveTextEdit::initCompleter()
{
    QStringList listWord = completerList();

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
    const int extra = completion.length() - m_completer->completionPrefix().length();
    tc.movePosition(QTextCursor::Left);
    tc.movePosition(QTextCursor::EndOfWord);
    tc.insertText(completion.right(extra));
    setTextCursor(tc);
}

QString SieveTextEdit::selectedWord(const QPoint &pos) const
{
    QTextCursor wordSelectCursor(pos.isNull() ? textCursor() : cursorForPosition(pos));
    wordSelectCursor.clearSelection();
    wordSelectCursor.select(QTextCursor::WordUnderCursor);
    const QString word = wordSelectCursor.selectedText();
    return word;
}

bool SieveTextEdit::event(QEvent* ev)
{
    if (ev->type() == QEvent::ShortcutOverride) {
        QKeyEvent *e = static_cast<QKeyEvent *>( ev );
        if (overrideShortcut(e)) {
            e->accept();
            return true;
        }
    }
    return PimCommon::PlainTextEditor::event(ev);
}

bool SieveTextEdit::overrideShortcut(QKeyEvent* event)
{
    if (event->key() == Qt::Key_F1) {
        if (openVariableHelp()) {
            return true;
        }
    }
    return PlainTextEditor::overrideShortcut(event);
}

bool SieveTextEdit::openVariableHelp()
{
    if (!textCursor().hasSelection()) {
        const QString word = selectedWord();
        const KSieveUi::SieveEditorUtil::HelpVariableName type =  KSieveUi::SieveEditorUtil::strToVariableName(word);
        if (type != KSieveUi::SieveEditorUtil::UnknownHelp) {
            const QString url = KSieveUi::SieveEditorUtil::helpUrl(type);
            if (!url.isEmpty()) {
                return true;
            }
        }
    }
    return false;
}

void SieveTextEdit::keyPressEvent(QKeyEvent* e)
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
    } else if (handleShortcut(e)) {
        return;
    }
    QPlainTextEdit::keyPressEvent(e);
    if (e->key() == Qt::Key_F1 && !textCursor().hasSelection()) {
        const QString word = selectedWord();
        const KSieveUi::SieveEditorUtil::HelpVariableName type =  KSieveUi::SieveEditorUtil::strToVariableName(word);
        if (type != KSieveUi::SieveEditorUtil::UnknownHelp) {
            const QString url = KSieveUi::SieveEditorUtil::helpUrl(type);
            if (!url.isEmpty())
                Q_EMIT openHelp(word, url);
        }
        return;
    }
    const QString text = wordUnderCursor();

    if ( text.length() < 2 ) // min 2 char for completion
        return;

    m_completer->setCompletionPrefix( text );

    QRect cr = cursorRect();
    cr.setWidth( m_completer->popup()->sizeHintForColumn(0)
                 + m_completer->popup()->verticalScrollBar()->sizeHint().width() );
    m_completer->complete( cr );
}

QString SieveTextEdit::wordUnderCursor() const
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

void SieveTextEdit::setSieveCapabilities( const QStringList &capabilities )
{
    m_syntaxHighlighter->addCapabilities(capabilities);
    setCompleterList(completerList() + capabilities);
}

void SieveTextEdit::setShowHelpMenu(bool b)
{
    mShowHelpMenu = b;
}

void SieveTextEdit::addExtraMenuEntry(QMenu *menu, const QPoint &pos)
{
    if (!mShowHelpMenu)
        return;

    if (!textCursor().hasSelection()) {
        const QString word = selectedWord(pos);
        const KSieveUi::SieveEditorUtil::HelpVariableName type =  KSieveUi::SieveEditorUtil::strToVariableName(word);
        if (type != KSieveUi::SieveEditorUtil::UnknownHelp) {
            QAction *separator = new QAction(menu);
            separator->setSeparator(true);
            menu->insertAction(menu->actions().at(0), separator);

            KAction *searchAction = new KAction(i18n("Help about: \'%1\'",word), menu);
            searchAction->setShortcut(Qt::Key_F1);
            searchAction->setIcon(KIcon(QLatin1String("help-hint")));
            searchAction->setData(word);
            connect(searchAction, SIGNAL(triggered()), SLOT(slotHelp()));
            menu->insertAction(menu->actions().at(0), searchAction);
        }
    }
}

void SieveTextEdit::slotHelp()
{
    QAction *act = qobject_cast<QAction*>(sender());
    if (act) {
        const QString word = act->data().toString();
        const KSieveUi::SieveEditorUtil::HelpVariableName type =  KSieveUi::SieveEditorUtil::strToVariableName(act->data().toString());
        const QString url = KSieveUi::SieveEditorUtil::helpUrl(type);
        if (!url.isEmpty())
            Q_EMIT openHelp(word, url);
    }
}
