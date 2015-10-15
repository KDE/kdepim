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

#include "plaintexteditorwidget.h"
#include "plaintexteditor.h"
#include "plaintexteditfindbar.h"
#include "texttospeech/texttospeechwidget.h"
#include "kpimtextedit/slidecontainer.h"

#include <QVBoxLayout>
#include <QShortcut>
#include <QTextCursor>

using namespace PimCommon;
class PimCommon::PlainTextEditorWidgetPrivate
{
public:
    PlainTextEditorWidgetPrivate()
        : mFindBar(Q_NULLPTR),
          mEditor(Q_NULLPTR),
          mTextToSpeechWidget(Q_NULLPTR),
          mSliderContainer(Q_NULLPTR)
    {

    }

    PimCommon::PlainTextEditFindBar *mFindBar;
    PlainTextEditor *mEditor;
    PimCommon::TextToSpeechWidget *mTextToSpeechWidget;
    KPIMTextEdit::SlideContainer *mSliderContainer;
};

PlainTextEditorWidget::PlainTextEditorWidget(PlainTextEditor *customEditor, QWidget *parent)
    : QWidget(parent),
      d(new PimCommon::PlainTextEditorWidgetPrivate)
{
    init(customEditor);
}

PlainTextEditorWidget::PlainTextEditorWidget(QWidget *parent)
    : QWidget(parent),
      d(new PimCommon::PlainTextEditorWidgetPrivate)
{
    init();
}

PlainTextEditorWidget::~PlainTextEditorWidget()
{
    delete d;
}

PlainTextEditor *PlainTextEditorWidget::editor() const
{
    return d->mEditor;
}

void PlainTextEditorWidget::clear()
{
    d->mEditor->clear();
}

void PlainTextEditorWidget::setSpellCheckingConfigFileName(const QString &_fileName)
{
    d->mEditor->setSpellCheckingConfigFileName(_fileName);
}

void PlainTextEditorWidget::setPlainText(const QString &text)
{
    d->mEditor->setPlainText(text);
}

QString PlainTextEditorWidget::toPlainText() const
{
    return d->mEditor->toPlainText();
}

void PlainTextEditorWidget::init(PlainTextEditor *customEditor)
{
    QVBoxLayout *lay = new QVBoxLayout;
    lay->setMargin(0);
    d->mTextToSpeechWidget = new PimCommon::TextToSpeechWidget(this);
    lay->addWidget(d->mTextToSpeechWidget);
    if (customEditor) {
        d->mEditor = customEditor;
    } else {
        d->mEditor = new PlainTextEditor;
    }
    lay->addWidget(d->mEditor);
    connect(d->mEditor, &PlainTextEditor::say, d->mTextToSpeechWidget, &PimCommon::TextToSpeechWidget::say);

    d->mSliderContainer = new KPIMTextEdit::SlideContainer(this);

    d->mFindBar = new PimCommon::PlainTextEditFindBar(d->mEditor, this);
    d->mFindBar->setHideWhenClose(false);
    connect(d->mFindBar, &PimCommon::PlainTextEditFindBar::displayMessageIndicator, d->mEditor, &PlainTextEditor::slotDisplayMessageIndicator);
    connect(d->mFindBar, &PimCommon::PlainTextEditFindBar::hideFindBar, d->mSliderContainer, &KPIMTextEdit::SlideContainer::slideOut);
    d->mSliderContainer->setContent(d->mFindBar);

    lay->addWidget(d->mSliderContainer);

    QShortcut *shortcut = new QShortcut(this);
    shortcut->setKey(Qt::Key_F + Qt::CTRL);
    connect(shortcut, &QShortcut::activated, this, &PlainTextEditorWidget::slotFind);
    connect(d->mEditor, &PlainTextEditor::findText, this, &PlainTextEditorWidget::slotFind);

    shortcut = new QShortcut(this);
    shortcut->setKey(Qt::Key_R + Qt::CTRL);
    connect(shortcut, &QShortcut::activated, this, &PlainTextEditorWidget::slotReplace);
    connect(d->mEditor, &PlainTextEditor::replaceText, this, &PlainTextEditorWidget::slotReplace);

    setLayout(lay);
}

bool PlainTextEditorWidget::isReadOnly() const
{
    return d->mEditor->isReadOnly();
}

void PlainTextEditorWidget::setReadOnly(bool readOnly)
{
    d->mEditor->setReadOnly(readOnly);
}

void PlainTextEditorWidget::slotReplace()
{
    if (d->mEditor->searchSupport()) {
        if (d->mEditor->textCursor().hasSelection()) {
            d->mFindBar->setText(d->mEditor->textCursor().selectedText());
        }
        d->mFindBar->showReplace();
        d->mSliderContainer->slideIn();
        d->mFindBar->focusAndSetCursor();
    }
}

void PlainTextEditorWidget::slotFind()
{
    if (d->mEditor->searchSupport()) {
        if (d->mEditor->textCursor().hasSelection()) {
            d->mFindBar->setText(d->mEditor->textCursor().selectedText());
        }
        d->mEditor->moveCursor(QTextCursor::Start);
        d->mFindBar->showFind();
        d->mSliderContainer->slideIn();
        d->mFindBar->focusAndSetCursor();
    }
}

