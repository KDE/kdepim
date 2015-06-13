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

#include "grantleeplaintexteditor.h"
#include <kpimtextedit/htmlhighlighter.h>

#include <QStringListModel>
#include <QCompleter>
#include <QKeyEvent>
#include <QScrollBar>
#include <QAbstractItemView>

#include <KPIMTextEdit/TextEditorCompleter>

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
    mTextEditorCompleter = new KPIMTextEdit::TextEditorCompleter(this, this);
}

void GrantleePlainTextEditor::createCompleterList(const QStringList &extraCompletion)
{
    QStringList listWord;
    listWord << extraCompletion;
    mTextEditorCompleter->setCompleterStringList(extraCompletion);
}


void GrantleePlainTextEditor::keyPressEvent(QKeyEvent *e)
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
    PimCommon::PlainTextEditor::keyPressEvent(e);
    mTextEditorCompleter->completeText();
}


