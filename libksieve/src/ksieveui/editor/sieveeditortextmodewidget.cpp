/* Copyright (C) 2013-2015 Laurent Montel <montel@kde.org>
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

#include "sieveeditortextmodewidget.h"
#include "templates/sievetemplatewidget.h"
#include "autocreatescripts/autocreatescriptdialog.h"
#include "editor/sieveinfowidget.h"
#include "editor/sievetextedit.h"
#include "editor/warningwidget/sieveeditorwarning.h"
#include "editor/warningwidget/sieveeditorparsingmissingfeaturewarning.h"
#include "editor/sieveeditortabwidget.h"
#include "sievescriptdebugger/sievescriptdebuggerdialog.h"

#include "scriptsparsing/xmlprintingscriptbuilder.h"
#include "scriptsparsing/parsingresultdialog.h"

#include "kpimtextedit/plaintexteditfindbar.h"
#include "kpimtextedit/plaintexteditorwidget.h"
#include "kpimtextedit/texttospeechwidget.h"
#include "kpimtextedit/textgotolinewidget.h"
#include "KSplitterCollapserButton"
#include "sieveeditorhelphtmlwidget.h"
#include "kpimtextedit/slidecontainer.h"

#include <ksieve/parser.h>
#include <ksieve/error.h>
#include <ksieve/scriptbuilder.h>

#include <KLocalizedString>
#include <KSharedConfig>
#include <KConfigGroup>

#include <QSplitter>
#include <QShortcut>
#include <QPointer>
#include <QPushButton>
#include <QVBoxLayout>
#include <QDebug>

#include <errno.h>

using namespace KSieveUi;

SieveEditorTextModeWidget::SieveEditorTextModeWidget(QWidget *parent)
    : SieveEditorAbstractWidget(parent)
{
    QVBoxLayout *lay = new QVBoxLayout;
    lay->setMargin(0);
    setLayout(lay);

    mMainSplitter = new QSplitter;
    mMainSplitter->setOrientation(Qt::Vertical);
    lay->addWidget(mMainSplitter);

    mTemplateSplitter = new QSplitter;
    mTemplateSplitter->setOrientation(Qt::Horizontal);
    //
    mSieveTemplateWidget = new SieveTemplateWidget(i18n("Sieve Template:"));

    mSieveInfo = new SieveInfoWidget;

    mExtraSplitter = new QSplitter;
    mExtraSplitter->setOrientation(Qt::Vertical);

    mExtraSplitter->addWidget(mSieveTemplateWidget);
    mExtraSplitter->addWidget(mSieveInfo);
    mExtraSplitter->setChildrenCollapsible(false);

    QWidget *textEditWidget = new QWidget;
    QVBoxLayout *textEditLayout = new QVBoxLayout;
    textEditLayout->setMargin(0);

    mTabWidget = new SieveEditorTabWidget;
    mTextToSpeechWidget = new KPIMTextEdit::TextToSpeechWidget(this);
    textEditLayout->addWidget(mTextToSpeechWidget);

    mTextEdit = new SieveTextEdit;
    connect(mTextEdit, &SieveTextEdit::textChanged, this, &SieveEditorTextModeWidget::valueChanged);
    mTabWidget->addTab(mTextEdit, i18n("Editor"));
    mTabWidget->tabBar()->hide();
    textEditLayout->addWidget(mTabWidget);
    connect(mTextEdit, &SieveTextEdit::openHelp, mTabWidget, &SieveEditorTabWidget::slotAddHelpPage);
    connect(mTextEdit, &SieveTextEdit::say, mTextToSpeechWidget, &KPIMTextEdit::TextToSpeechWidget::say);

    mGotoLineSliderContainer = new KPIMTextEdit::SlideContainer(this);
    mGoToLine = new KPIMTextEdit::TextGoToLineWidget;
    mGoToLine->hide();
    mGotoLineSliderContainer->setContent(mGoToLine);
    textEditLayout->addWidget(mGotoLineSliderContainer);
    connect(mGoToLine, &KPIMTextEdit::TextGoToLineWidget::hideGotoLine, mGotoLineSliderContainer, &KPIMTextEdit::SlideContainer::slideOut);

    connect(mGoToLine, &KPIMTextEdit::TextGoToLineWidget::moveToLine, this, &SieveEditorTextModeWidget::slotGoToLine);

    mSliderContainer = new KPIMTextEdit::SlideContainer(this);
    mFindBar = new KPIMTextEdit::PlainTextEditFindBar(mTextEdit, textEditWidget);
    mFindBar->setHideWhenClose(false);
    connect(mFindBar, &KPIMTextEdit::TextEditFindBarBase::hideFindBar, mSliderContainer, &KPIMTextEdit::SlideContainer::slideOut);
    connect(mFindBar, &KPIMTextEdit::TextEditFindBarBase::displayMessageIndicator, mTextEdit, &KPIMTextEdit::PlainTextEditor::slotDisplayMessageIndicator);
    mSliderContainer->setContent(mFindBar);
    textEditLayout->addWidget(mSliderContainer);

    mSieveEditorWarning = new SieveEditorWarning;
    textEditLayout->addWidget(mSieveEditorWarning);

    mSieveParsingWarning = new SieveEditorParsingMissingFeatureWarning(SieveEditorParsingMissingFeatureWarning::TextEditor);
    connect(mSieveParsingWarning, &SieveEditorParsingMissingFeatureWarning::switchToGraphicalMode, this, &SieveEditorTextModeWidget::switchToGraphicalMode);
    textEditLayout->addWidget(mSieveParsingWarning);

    textEditWidget->setLayout(textEditLayout);

    mTemplateSplitter->addWidget(textEditWidget);
    mTemplateSplitter->addWidget(mExtraSplitter);
    mTemplateSplitter->setCollapsible(0, false);
    new KSplitterCollapserButton(mExtraSplitter, mTemplateSplitter);

    connect(mSieveTemplateWidget, &SieveTemplateWidget::insertTemplate, mTextEdit, &SieveTextEdit::insertPlainText);

    //
    QShortcut *shortcut = new QShortcut(this);
    shortcut->setKey(Qt::Key_F + Qt::CTRL);
    connect(shortcut, &QShortcut::activated, this, &SieveEditorTextModeWidget::slotFind);
    connect(mTextEdit, &SieveTextEdit::findText, this, &SieveEditorTextModeWidget::slotFind);

    shortcut = new QShortcut(this);
    shortcut->setKey(Qt::Key_R + Qt::CTRL);
    connect(shortcut, &QShortcut::activated, this, &SieveEditorTextModeWidget::slotReplace);
    connect(mTextEdit, &SieveTextEdit::replaceText, this, &SieveEditorTextModeWidget::slotReplace);

    mDebugTextEdit = new KPIMTextEdit::PlainTextEditorWidget;
    mDebugTextEdit->editor()->setSearchSupport(false);
    mDebugTextEdit->editor()->setReadOnly(true);
    mMainSplitter->addWidget(mTemplateSplitter);
    mMainSplitter->addWidget(mDebugTextEdit);
    mMainSplitter->setChildrenCollapsible(false);

    connect(mTextEdit, &SieveTextEdit::textChanged, this, &SieveEditorTextModeWidget::slotTextChanged);
    connect(mTextEdit, &SieveTextEdit::undoAvailable, this, &SieveEditorTextModeWidget::undoAvailable);
    connect(mTextEdit, &SieveTextEdit::redoAvailable, this, &SieveEditorTextModeWidget::redoAvailable);
    connect(mTextEdit, &SieveTextEdit::copyAvailable, this, &SieveEditorTextModeWidget::copyAvailable);
    readConfig();

    mTextEdit->setFocus();
}

SieveEditorTextModeWidget::~SieveEditorTextModeWidget()
{
    disconnect(mTextEdit, &SieveTextEdit::textChanged, this, &SieveEditorTextModeWidget::slotTextChanged);
    disconnect(mTextEdit, &SieveTextEdit::textChanged, this, &SieveEditorTextModeWidget::valueChanged);
    writeConfig();
}

void SieveEditorTextModeWidget::writeConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "SieveEditor");
    group.writeEntry("mainSplitter", mMainSplitter->sizes());
    group.writeEntry("extraSplitter", mExtraSplitter->sizes());
    group.writeEntry("templateSplitter", mTemplateSplitter->sizes());
}

void SieveEditorTextModeWidget::readConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "SieveEditor");
    QList<int> size;
    size << 400 << 100;

    mMainSplitter->setSizes(group.readEntry("mainSplitter", size));
    mExtraSplitter->setSizes(group.readEntry("extraSplitter", size));
    mTemplateSplitter->setSizes(group.readEntry("templateSplitter", size));
}

void SieveEditorTextModeWidget::slotGoToLine(int line)
{
    if (line > 0) {
        QTextCursor cursor = mTextEdit->textCursor();
        cursor.beginEditBlock();
        cursor.movePosition(QTextCursor::Start);
        cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, (line - 1));
        cursor.endEditBlock();
        mTextEdit->setTextCursor(cursor);
        mTextEdit->setFocus();
    }
}

void SieveEditorTextModeWidget::slotShowGoToLine()
{
    mGoToLine->setMaximumLineCount(mTextEdit->document()->blockCount());
    mGotoLineSliderContainer->slideIn();
    mGoToLine->goToLine();
}

void SieveEditorTextModeWidget::generateXml()
{
#if !defined(NDEBUG)
    const QByteArray script = mTextEdit->toPlainText().toUtf8();
    KSieve::Parser parser(script.begin(),
                          script.begin() + script.length());
    KSieveUi::XMLPrintingScriptBuilder psb;
    parser.setScriptBuilder(&psb);
    const bool result = parser.parse();
    QPointer<ParsingResultDialog> dlg = new ParsingResultDialog(this);
    if (result) {
        dlg->setResultParsing(psb.toDom().toString());
    } else {
        dlg->setResultParsing(i18n("Error during parsing"));
    }
    dlg->exec();
    delete dlg;
#endif
}

void SieveEditorTextModeWidget::autoGenerateScripts()
{
    QPointer<AutoCreateScriptDialog> dlg = new AutoCreateScriptDialog(this);
    dlg->setSieveCapabilities(mSieveCapabilities);
    if (dlg->exec()) {
        QString requires;
        const QString script = dlg->script(requires);
        QString newPlainText = mTextEdit->toPlainText() + script;
        if (!requires.isEmpty()) {
            newPlainText.prepend(requires + QLatin1Char('\n'));
        }
        mTextEdit->setPlainText(newPlainText);
    }
    delete dlg;
}

void SieveEditorTextModeWidget::find()
{
    slotFind();
}

void SieveEditorTextModeWidget::replace()
{
    slotReplace();
}

void SieveEditorTextModeWidget::undo()
{
    mTextEdit->undo();
}

void SieveEditorTextModeWidget::redo()
{
    mTextEdit->redo();
}

void SieveEditorTextModeWidget::paste()
{
    mTextEdit->paste();
}

void SieveEditorTextModeWidget::cut()
{
    mTextEdit->cut();
}

void SieveEditorTextModeWidget::copy()
{
    mTextEdit->copy();
}

void SieveEditorTextModeWidget::selectAll()
{
    mTextEdit->selectAll();
}

bool SieveEditorTextModeWidget::isUndoAvailable() const
{
    return mTextEdit->document()->isUndoAvailable();
}

bool SieveEditorTextModeWidget::isRedoAvailable() const
{
    return mTextEdit->document()->isRedoAvailable();
}

bool SieveEditorTextModeWidget::hasSelection() const
{
    return mTextEdit->textCursor().hasSelection();
}

void SieveEditorTextModeWidget::zoomIn()
{
    QWidget *w = mTabWidget->currentWidget();
    if (w == mTextEdit) {
        mTextEdit->zoomIn();
    } else if (SieveEditorHelpHtmlWidget *page = qobject_cast<SieveEditorHelpHtmlWidget *>(w)) {
        page->zoomIn();
    }
}

void SieveEditorTextModeWidget::zoomOut()
{
    QWidget *w = mTabWidget->currentWidget();
    if (w == mTextEdit) {
        mTextEdit->zoomOut();
    } else if (SieveEditorHelpHtmlWidget *page = qobject_cast<SieveEditorHelpHtmlWidget *>(w)) {
        page->zoomOut();
    }
}

bool SieveEditorTextModeWidget::isWordWrap() const
{
    return mTextEdit->isWordWrap();
}

void SieveEditorTextModeWidget::wordWrap(bool state)
{
    mTextEdit->wordWrap(state);
}

void SieveEditorTextModeWidget::zoomReset()
{
    QWidget *w = mTabWidget->currentWidget();
    if (w == mTextEdit) {
        mTextEdit->slotZoomReset();
    } else if (SieveEditorHelpHtmlWidget *page = qobject_cast<SieveEditorHelpHtmlWidget *>(w)) {
        page->resetZoom();
    }
}

void SieveEditorTextModeWidget::slotFind()
{
    if (mTextEdit->textCursor().hasSelection()) {
        mFindBar->setText(mTextEdit->textCursor().selectedText());
    }
    mTextEdit->moveCursor(QTextCursor::Start);
    mFindBar->showFind();
    mSliderContainer->slideIn();
    mFindBar->focusAndSetCursor();
}

void SieveEditorTextModeWidget::slotReplace()
{
    if (mTextEdit->textCursor().hasSelection()) {
        mFindBar->setText(mTextEdit->textCursor().selectedText());
    }
    mFindBar->showReplace();
    mSliderContainer->slideIn();
    mFindBar->focusAndSetCursor();
}

QString SieveEditorTextModeWidget::currentscript()
{
    return mTextEdit->toPlainText();
}

void SieveEditorTextModeWidget::setImportScript(const QString &script)
{
    mTextEdit->setPlainText(script);
}

void SieveEditorTextModeWidget::slotTextChanged()
{
    const bool enabled = !script().isEmpty();
    Q_EMIT enableButtonOk(enabled);
}

QString SieveEditorTextModeWidget::script() const
{
    return mTextEdit->toPlainText();
}

void SieveEditorTextModeWidget::setScript(const QString &script)
{
    mTextEdit->setPlainText(script);
}

void SieveEditorTextModeWidget::setDebugScript(const QString &debug)
{
    mDebugTextEdit->editor()->clear();
    mDebugTextEdit->editor()->appendHtml(debug);
}

void SieveEditorTextModeWidget::setSieveCapabilities(const QStringList &capabilities)
{
    mSieveCapabilities = capabilities;
    mTextEdit->setSieveCapabilities(mSieveCapabilities);
    mSieveTemplateWidget->setSieveCapabilities(mSieveCapabilities);
    mSieveInfo->setServerInfo(capabilities);
}

void SieveEditorTextModeWidget::showEditorWarning()
{
    mSieveEditorWarning->animatedShow();
}

void SieveEditorTextModeWidget::hideEditorWarning()
{
    mSieveEditorWarning->animatedHide();
    mSieveParsingWarning->animatedHide();
}

void SieveEditorTextModeWidget::showParsingEditorWarning()
{
    mSieveParsingWarning->animatedShow();
}

void SieveEditorTextModeWidget::setParsingEditorWarningError(const QString &script, const QString &error)
{
    mSieveParsingWarning->setErrors(script, error);
}

void SieveEditorTextModeWidget::checkSpelling()
{
    mTextEdit->slotCheckSpelling();
}

void SieveEditorTextModeWidget::comment()
{
    mTextEdit->comment();
}

void SieveEditorTextModeWidget::uncomment()
{
    mTextEdit->uncomment();
}

void SieveEditorTextModeWidget::setReadOnly(bool b)
{
    mTextEdit->setReadOnly(b);
}

void SieveEditorTextModeWidget::upperCase()
{
    mTextEdit->upperCase();
}

void SieveEditorTextModeWidget::lowerCase()
{
    mTextEdit->lowerCase();
}

void SieveEditorTextModeWidget::sentenceCase()
{
    mTextEdit->sentenceCase();
}

void SieveEditorTextModeWidget::reverseCase()
{
    mTextEdit->reverseCase();
}

QString SieveEditorTextModeWidget::currentHelpTitle() const
{
    return mTabWidget->currentHelpTitle();
}

QUrl SieveEditorTextModeWidget::currentHelpUrl() const
{
    return mTabWidget->currentHelpUrl();
}

void SieveEditorTextModeWidget::openBookmarkUrl(const QUrl &url)
{
    mTabWidget->slotAddHelpPage(url);
}

void SieveEditorTextModeWidget::debugSieveScript()
{
    QPointer<KSieveUi::SieveScriptDebuggerDialog> dlg = new KSieveUi::SieveScriptDebuggerDialog(this);
    dlg->setScript(mTextEdit->toPlainText());
    if (dlg->exec()) {
        const QString script = dlg->script();
        mTextEdit->setPlainText(script);
    }
    delete dlg;
}
