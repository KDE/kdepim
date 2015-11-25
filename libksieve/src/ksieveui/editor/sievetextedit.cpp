/* Copyright (C) 2011-2015 Laurent Montel <montel@kde.org>
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

#include "sievesyntaxspellcheckinghighlighter.h"
#include "sievetextedit.h"
#include "editor/sievelinenumberarea.h"
#include "editor/sieveeditorutil.h"
#include "KPIMTextEdit/EditorUtil"
#include "editor/sievetexteditorspellcheckdecorator.h"
#include "kpimtextedit/plaintextsyntaxspellcheckinghighlighter.h"
#include <KLocalizedString>
#include <QAction>
#include <QIcon>

#include <QAbstractItemView>
#include <QCompleter>
#include <QStringListModel>
#include <QKeyEvent>
#include <QPainter>
#include <QScrollBar>
#include <QMenu>
#include <QFontDatabase>
#include <QDebug>

#include <KPIMTextEdit/SyntaxHighlighterBase>
#include <KPIMTextEdit/TextEditorCompleter>

using namespace KSieveUi;

class KSieveUi::SieveTextEditPrivate
{
public:
    SieveTextEditPrivate()
        : m_sieveLineNumberArea(Q_NULLPTR),
          mTextEditorCompleter(Q_NULLPTR),
          mShowHelpMenu(true)
    {

    }
    PimCommon::SieveSyntaxHighlighterRules mSieveHighliterRules;
    SieveLineNumberArea *m_sieveLineNumberArea;
    KPIMTextEdit::TextEditorCompleter *mTextEditorCompleter;
    bool mShowHelpMenu;
};

SieveTextEdit::SieveTextEdit(QWidget *parent)
    : KPIMTextEdit::PlainTextEditor(parent),
      d(new KSieveUi::SieveTextEditPrivate)
{
    setSpellCheckingConfigFileName(QStringLiteral("sieveeditorrc"));
    setWordWrapMode(QTextOption::NoWrap);
    setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    d->m_sieveLineNumberArea = new SieveLineNumberArea(this);

    connect(this, &SieveTextEdit::blockCountChanged, this, &SieveTextEdit::slotUpdateLineNumberAreaWidth);
    connect(this, &SieveTextEdit::updateRequest, this, &SieveTextEdit::slotUpdateLineNumberArea);

    slotUpdateLineNumberAreaWidth(0);

    initCompleter();
    createHighlighter();
}

SieveTextEdit::~SieveTextEdit()
{
    // disconnect these manually as the destruction of KPIMTextEdit::PlainTextEditorPrivate will trigger them
    disconnect(this, &SieveTextEdit::blockCountChanged, this, &SieveTextEdit::slotUpdateLineNumberAreaWidth);
    disconnect(this, &SieveTextEdit::updateRequest, this, &SieveTextEdit::slotUpdateLineNumberArea);

    delete d;
}

void SieveTextEdit::updateHighLighter()
{
    KSieveUi::SieveSyntaxSpellCheckingHighlighter *hlighter = dynamic_cast<KSieveUi::SieveSyntaxSpellCheckingHighlighter *>(highlighter());
    if (hlighter) {
        hlighter->toggleSpellHighlighting(checkSpellingEnabled());
    }
}

void SieveTextEdit::clearDecorator()
{
    //Nothing
}

void SieveTextEdit::createHighlighter()
{
    KSieveUi::SieveSyntaxSpellCheckingHighlighter *highlighter = new KSieveUi::SieveSyntaxSpellCheckingHighlighter(this);
    highlighter->toggleSpellHighlighting(checkSpellingEnabled());
    highlighter->setCurrentLanguage(spellCheckingLanguage());
    highlighter->setSyntaxHighlighterRules(d->mSieveHighliterRules.rules());
    setHighlighter(highlighter);
}

void SieveTextEdit::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    const QRect cr = contentsRect();
    d->m_sieveLineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
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
    QPainter painter(d->m_sieveLineNumberArea);
    painter.fillRect(event->rect(), Qt::lightGray);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();
    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            const QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::black);
            painter.drawText(0, top, d->m_sieveLineNumberArea->width(), fontMetrics().height(),
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
    if (dy) {
        d->m_sieveLineNumberArea->scroll(0, dy);
    } else {
        d->m_sieveLineNumberArea->update(0, rect.y(), d->m_sieveLineNumberArea->width(), rect.height());
    }

    if (rect.contains(viewport()->rect())) {
        slotUpdateLineNumberAreaWidth(0);
    }
}

QStringList SieveTextEdit::completerList() const
{
    QStringList listWord;

    listWord << QStringLiteral("require") << QStringLiteral("stop");
    listWord << QStringLiteral(":contains") << QStringLiteral(":matches") << QStringLiteral(":is") << QStringLiteral(":over") << QStringLiteral(":under") << QStringLiteral(":all") << QStringLiteral(":domain") << QStringLiteral(":localpart");
    listWord << QStringLiteral("if") << QStringLiteral("elsif") << QStringLiteral("else");
    listWord << QStringLiteral("keep") << QStringLiteral("reject") << QStringLiteral("discard") << QStringLiteral("redirect")  << QStringLiteral("addflag") << QStringLiteral("setflag");
    listWord << QStringLiteral("address") << QStringLiteral("allof") << QStringLiteral("anyof") << QStringLiteral("exists") << QStringLiteral("false") << QStringLiteral("header") << QStringLiteral("not") << QStringLiteral("size") << QStringLiteral("true");
    listWord << QStringLiteral(":days") << QStringLiteral(":seconds") << QStringLiteral(":subject") << QStringLiteral(":addresses") << QStringLiteral(":text");
    listWord << QStringLiteral(":name") << QStringLiteral(":headers") << QStringLiteral(":first") << QStringLiteral(":importance");
    listWord << QStringLiteral(":message") << QStringLiteral(":from");

    return listWord;
}

void SieveTextEdit::setCompleterList(const QStringList &list)
{
    d->mTextEditorCompleter->setCompleterStringList(list);
}

void SieveTextEdit::initCompleter()
{
    const QStringList listWord = completerList();

    d->mTextEditorCompleter = new KPIMTextEdit::TextEditorCompleter(this, this);
    d->mTextEditorCompleter->setCompleterStringList(listWord);
}

bool SieveTextEdit::event(QEvent *ev)
{
    if (ev->type() == QEvent::ShortcutOverride) {
        QKeyEvent *e = static_cast<QKeyEvent *>(ev);
        if (overrideShortcut(e)) {
            e->accept();
            return true;
        }
    }
    return KPIMTextEdit::PlainTextEditor::event(ev);
}

Sonnet::SpellCheckDecorator *SieveTextEdit::createSpellCheckDecorator()
{
    return new SieveTextEditorSpellCheckDecorator(this);
}

bool SieveTextEdit::overrideShortcut(QKeyEvent *event)
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
            const QUrl url = KSieveUi::SieveEditorUtil::helpUrl(type);
            if (!url.isEmpty()) {
                return true;
            }
        }
    }
    return false;
}

void SieveTextEdit::keyPressEvent(QKeyEvent *e)
{
    if (d->mTextEditorCompleter->completer()->popup()->isVisible()) {
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
            const QUrl url = KSieveUi::SieveEditorUtil::helpUrl(type);
            if (!url.isEmpty()) {
                Q_EMIT openHelp(url);
            }
        }
        return;
    }
    d->mTextEditorCompleter->completeText();
}

void SieveTextEdit::setSieveCapabilities(const QStringList &capabilities)
{
    d->mSieveHighliterRules.addCapabilities(capabilities);
    KPIMTextEdit::PlainTextSyntaxSpellCheckingHighlighter *hlighter = dynamic_cast<KPIMTextEdit::PlainTextSyntaxSpellCheckingHighlighter *>(highlighter());
    if (hlighter) {
        hlighter->setSyntaxHighlighterRules(d->mSieveHighliterRules.rules());
    }

    setCompleterList(completerList() + capabilities);
}

void SieveTextEdit::setShowHelpMenu(bool b)
{
    d->mShowHelpMenu = b;
}

void SieveTextEdit::addExtraMenuEntry(QMenu *menu, QPoint pos)
{
    if (!d->mShowHelpMenu) {
        return;
    }

    if (!textCursor().hasSelection()) {
        const QString word = selectedWord(pos);
        const KSieveUi::SieveEditorUtil::HelpVariableName type =  KSieveUi::SieveEditorUtil::strToVariableName(word);
        if (type != KSieveUi::SieveEditorUtil::UnknownHelp) {
            QAction *separator = new QAction(menu);
            separator->setSeparator(true);
            menu->insertAction(menu->actions().at(0), separator);

            QAction *searchAction = new QAction(i18n("Help about: \'%1\'", word), menu);
            searchAction->setShortcut(Qt::Key_F1);
            searchAction->setIcon(QIcon::fromTheme(QStringLiteral("help-hint")));
            searchAction->setData(word);
            connect(searchAction, &QAction::triggered, this, &SieveTextEdit::slotHelp);
            menu->insertAction(menu->actions().at(0), searchAction);
        }
    }
}

QString SieveTextEdit::selectedWord(const QPoint &pos) const
{
    QTextCursor wordSelectCursor(pos.isNull() ? textCursor() : cursorForPosition(pos));
    wordSelectCursor.clearSelection();
    wordSelectCursor.select(QTextCursor::WordUnderCursor);
    const QString word = wordSelectCursor.selectedText();
    return word;
}

void SieveTextEdit::slotHelp()
{
    QAction *act = qobject_cast<QAction *>(sender());
    if (act) {
        const QString word = act->data().toString();
        const KSieveUi::SieveEditorUtil::HelpVariableName type =  KSieveUi::SieveEditorUtil::strToVariableName(word);
        const QUrl url = KSieveUi::SieveEditorUtil::helpUrl(type);
        if (!url.isEmpty()) {
            Q_EMIT openHelp(url);
        }
    }
}

void SieveTextEdit::comment()
{
    QTextCursor textcursor = textCursor();
    if (textcursor.hasSelection()) {
        //Move start block
        textcursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
        QString text = textcursor.selectedText();
        text = QLatin1Char('#') + text;
        text.replace(QChar::ParagraphSeparator, QStringLiteral("\n#"));
        textcursor.insertText(text);
        setTextCursor(textcursor);
    } else {
        textcursor.movePosition(QTextCursor::StartOfBlock);
        textcursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        const QString s = textcursor.selectedText();
        const QString str = QLatin1Char('#') + s;
        textcursor.insertText(str);
        setTextCursor(textcursor);
    }
}

void SieveTextEdit::upperCase()
{
    KPIMTextEdit::EditorUtil editorUtil;
    QTextCursor cursorText = textCursor();
    editorUtil.upperCase(cursorText);
}

void SieveTextEdit::lowerCase()
{
    KPIMTextEdit::EditorUtil editorUtil;
    QTextCursor cursorText = textCursor();
    editorUtil.lowerCase(cursorText);
}

void SieveTextEdit::sentenceCase()
{
    KPIMTextEdit::EditorUtil editorUtil;
    QTextCursor cursorText = textCursor();
    editorUtil.sentenceCase(cursorText);
}

void SieveTextEdit::reverseCase()
{
    KPIMTextEdit::EditorUtil editorUtil;
    QTextCursor cursorText = textCursor();
    editorUtil.reverseCase(cursorText);
}

void SieveTextEdit::uncomment()
{
    QTextCursor textcursor = textCursor();
    if (textcursor.hasSelection()) {
        textcursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
        QString text = textcursor.selectedText();
        if (text.startsWith(QLatin1Char('#'))) {
            text.remove(0, 1);
        }
        QString newText = text;
        for (int i = 0; i < newText.length(); ++i) {
            if (newText.at(i) == QChar::ParagraphSeparator || newText.at(i) == QChar::LineSeparator) {
                ++i;
                if (i < text.length()) {
                    if (newText.at(i) == QLatin1Char('#')) {
                        newText.remove(i, 1);
                    }
                }
            }
        }

        textcursor.insertText(newText);
        setTextCursor(textcursor);
    } else {
        textcursor.movePosition(QTextCursor::StartOfBlock);
        textcursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        QString text = textcursor.selectedText();
        if (text.startsWith(QLatin1Char('#'))) {
            text.remove(0, 1);
        }
        textcursor.insertText(text);
        setTextCursor(textcursor);
    }
}

bool SieveTextEdit::isWordWrap() const
{
    return (wordWrapMode() == QTextOption::WordWrap);
}

void SieveTextEdit::wordWrap(bool state)
{
    setWordWrapMode(state ? QTextOption::WordWrap : QTextOption::NoWrap);
}

