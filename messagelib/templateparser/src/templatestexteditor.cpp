/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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
#include "templatesutil.h"
#include "templatessyntaxhighlighterrules.h"
#include <KPIMTextEdit/TextEditorCompleter>
#include <KPIMTextEdit/SyntaxHighlighterBase>

#include <QCompleter>
#include <QKeyEvent>
#include <QScrollBar>
#include <QStringListModel>
#include <QAbstractItemView>
#include <QFontDatabase>

#include <kpimtextedit/plaintextsyntaxspellcheckinghighlighter.h>

using namespace TemplateParser;

TemplatesTextEditor::TemplatesTextEditor(QWidget *parent)
    : KPIMTextEdit::PlainTextEditor(parent)
{
    setFocus();
    const QFont f = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    setFont(f);
    QStringList excludeKeyWord;
    Q_FOREACH (QString str, TemplateParser::Util::keywords()) {
        excludeKeyWord << str.remove(QLatin1Char('%'));
        excludeKeyWord << str.replace(QStringLiteral("\\("), QStringLiteral("("));
    }
    addIgnoreWords(excludeKeyWord);
    setWordWrapMode(QTextOption::NoWrap);
    initCompleter();
    createHighlighter();
}

TemplatesTextEditor::~TemplatesTextEditor()
{

}

void TemplatesTextEditor::updateHighLighter()
{
    KPIMTextEdit::PlainTextSyntaxSpellCheckingHighlighter *hlighter = dynamic_cast<KPIMTextEdit::PlainTextSyntaxSpellCheckingHighlighter *>(highlighter());
    if (hlighter) {
        hlighter->toggleSpellHighlighting(checkSpellingEnabled());
    }
}

void TemplatesTextEditor::clearDecorator()
{
    //Nothing
}

void TemplatesTextEditor::createHighlighter()
{
    KPIMTextEdit::PlainTextSyntaxSpellCheckingHighlighter *highlighter = new KPIMTextEdit::PlainTextSyntaxSpellCheckingHighlighter(this);
    highlighter->toggleSpellHighlighting(checkSpellingEnabled());
    highlighter->setCurrentLanguage(spellCheckingLanguage());
    TemplatesSyntaxHighlighterRules rules;
    highlighter->setSyntaxHighlighterRules(rules.rules());
    setHighlighter(highlighter);
}

void TemplatesTextEditor::initCompleter()
{
    QStringList listWord;
    QStringList excludeKeyWord;
    Q_FOREACH (QString str, TemplateParser::Util::keywords()) {
        excludeKeyWord << str.replace(QStringLiteral("\\("), QStringLiteral("("));
    }
    listWord << excludeKeyWord;
    listWord << Util::keywordsWithArgs();

    mTextEditorCompleter = new KPIMTextEdit::TextEditorCompleter(this, this);
    mTextEditorCompleter->setCompleterStringList(listWord);
    mTextEditorCompleter->setExcludeOfCharacters(QStringLiteral("~!@#$^&*()+{}|\"<>,./;'[]\\-= "));
}

void TemplatesTextEditor::keyPressEvent(QKeyEvent *e)
{
    if (mTextEditorCompleter->completer()->popup()->isVisible()) {
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
    KPIMTextEdit::PlainTextEditor::keyPressEvent(e);
    mTextEditorCompleter->completeText();
}
