/*
  Copyright (c) 2014-2015 Montel Laurent <montel@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

*/

#include "sieveeditorwidget.h"

#include "sieve-editor.h"
#include "sieveeditortextmodewidget.h"
#include "scriptsparsing/parsingutil.h"
#include "autocreatescripts/sieveeditorgraphicalmodewidget.h"
#include "autocreatescripts/sievescriptparsingerrordialog.h"
#include "sieveeditormenubar.h"

#include <kns3/uploaddialog.h>
#include <KLocalizedString>
#include <KIconLoader>
#include <KStandardGuiItem>
#include <QTemporaryDir>
#include <kzip.h>
#include <KIconEngine>
#include <KStandardAction>
#include <QTemporaryFile>

#include <QPushButton>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QLabel>
#include <QLineEdit>
#include <QToolBar>
#include "libksieve_debug.h"
#include <QAction>
#include <QPointer>
#include <QDir>

using namespace KSieveUi;

SieveEditorWidget::SieveEditorWidget(bool useMenuBar, QWidget *parent)
    : QWidget(parent),
      mMode(TextMode),
      mModified(false)
{
    QVBoxLayout *lay = new QVBoxLayout;
#if !defined(NDEBUG)
    mGenerateXml = 0;
#endif

    QToolBar *toolbar = new QToolBar;
    toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    mCheckSyntax = new QAction(i18n("Check Syntax"), this);
    connect(mCheckSyntax, &QAction::triggered, this, &SieveEditorWidget::slotCheckSyntax);
    toolbar->addAction(mCheckSyntax);
    mSaveAs = KStandardAction::saveAs(this, SLOT(slotSaveAs()), this);
    toolbar->addAction(mSaveAs);
    toolbar->addAction(i18n("Import..."), this, SLOT(slotImport()));

    mAutoGenerateScript = new QAction(i18n("Autogenerate Script..."), this);
    connect(mAutoGenerateScript, &QAction::triggered, this, &SieveEditorWidget::slotAutoGenerateScripts);
    toolbar->addAction(mAutoGenerateScript);
    mSwitchMode = new QAction(this);
    toolbar->addAction(mSwitchMode);
    connect(mSwitchMode, &QAction::triggered, this, &SieveEditorWidget::slotSwitchMode);
#if !defined(NDEBUG)
    if (!qEnvironmentVariableIsEmpty("KDEPIM_SIEVEEDITOR_DEBUG")) {
        //Not necessary to translate it.
        mGenerateXml = new QAction(QStringLiteral("Generate xml"), this);
        connect(mGenerateXml, &QAction::triggered, this, &SieveEditorWidget::slotGenerateXml);
        toolbar->addAction(mGenerateXml);
    }
#endif

    QStringList overlays;
    overlays << QStringLiteral("list-add");
    mUpload = new QAction(QIcon(new KIconEngine(QStringLiteral("get-hot-new-stuff"), KIconLoader::global(), overlays)), i18n("Share..."), this);
    connect(mUpload, &QAction::triggered, this, &SieveEditorWidget::slotUploadScripts);
    connect(mUpload, &QAction::triggered, this, &SieveEditorWidget::slotUploadScripts);
    //Add action to toolBar

    toolbar->addAction(mUpload);

    if (useMenuBar) {
        SieveEditorMenuBar *menuBar = new SieveEditorMenuBar;
        connect(this, &SieveEditorWidget::changeModeEditor, menuBar, &SieveEditorMenuBar::setEditorMode);
        connect(menuBar, &SieveEditorMenuBar::copy, this, &SieveEditorWidget::copy);
        connect(menuBar, &SieveEditorMenuBar::find, this, &SieveEditorWidget::find);
        connect(menuBar, &SieveEditorMenuBar::replace, this, &SieveEditorWidget::replace);
        connect(menuBar, &SieveEditorMenuBar::undo, this, &SieveEditorWidget::undo);
        connect(menuBar, &SieveEditorMenuBar::redo, this, &SieveEditorWidget::redo);
        connect(menuBar, &SieveEditorMenuBar::paste, this, &SieveEditorWidget::paste);
        connect(menuBar, &SieveEditorMenuBar::cut, this, &SieveEditorWidget::cut);
        connect(menuBar, &SieveEditorMenuBar::selectAll, this, &SieveEditorWidget::selectAll);
        connect(menuBar, &SieveEditorMenuBar::gotoLine, this, &SieveEditorWidget::goToLine);
        connect(menuBar, &SieveEditorMenuBar::comment, this, &SieveEditorWidget::comment);
        connect(menuBar, &SieveEditorMenuBar::uncomment, this, &SieveEditorWidget::uncomment);
        connect(menuBar, &SieveEditorMenuBar::zoomIn, this, &SieveEditorWidget::zoomIn);
        connect(menuBar, &SieveEditorMenuBar::zoomOut, this, &SieveEditorWidget::zoomOut);
        connect(menuBar, &SieveEditorMenuBar::zoomReset, this, &SieveEditorWidget::zoomReset);
        connect(menuBar, &SieveEditorMenuBar::debugSieveScript, this, &SieveEditorWidget::debugSieveScript);
        connect(menuBar, &SieveEditorMenuBar::wordWrap, this, &SieveEditorWidget::wordWrap);

        connect(this, &SieveEditorWidget::copyAvailable, menuBar, &SieveEditorMenuBar::slotCopyAvailable);
        connect(this, &SieveEditorWidget::redoAvailable, menuBar, &SieveEditorMenuBar::slotRedoAvailable);
        connect(this, &SieveEditorWidget::undoAvailable, menuBar, &SieveEditorMenuBar::slotUndoAvailable);
        menuBar->fileMenu()->addAction(mSaveAs);
        menuBar->fileMenu()->addSeparator();
        menuBar->fileMenu()->addAction(mUpload);
        menuBar->toolsMenu()->addSeparator();
        menuBar->toolsMenu()->addAction(mAutoGenerateScript);
        menuBar->toolsMenu()->addAction(mCheckSyntax);
        lay->addWidget(menuBar);
    }

    lay->addWidget(toolbar);

    QHBoxLayout *nameLayout = new QHBoxLayout;
    QLabel *label = new QLabel(i18n("Script name:"));
    nameLayout->addWidget(label);
    mScriptName = new QLineEdit;
    mScriptName->setReadOnly(true);
    nameLayout->addWidget(mScriptName);
    lay->addLayout(nameLayout);

    lay->setMargin(0);
    setLayout(lay);
    mStackedWidget = new QStackedWidget;

    mTextModeWidget = new SieveEditorTextModeWidget;
    connect(mTextModeWidget, &SieveEditorTextModeWidget::valueChanged, this, &SieveEditorWidget::slotModified);
    mStackedWidget->addWidget(mTextModeWidget);
    mGraphicalModeWidget = new SieveEditorGraphicalModeWidget;
    connect(mGraphicalModeWidget, &SieveEditorGraphicalModeWidget::valueChanged, this, &SieveEditorWidget::slotModified);
    mStackedWidget->addWidget(mGraphicalModeWidget);

    lay->addWidget(mStackedWidget);
    connect(mTextModeWidget, &SieveEditorTextModeWidget::enableButtonOk, this, &SieveEditorWidget::slotEnableButtonOk);
    connect(mGraphicalModeWidget, &SieveEditorGraphicalModeWidget::enableButtonOk, this, &SieveEditorWidget::slotEnableButtonOk);
    connect(mGraphicalModeWidget, &SieveEditorGraphicalModeWidget::switchTextMode, this, &SieveEditorWidget::slotSwitchTextMode);
    connect(mTextModeWidget, &SieveEditorTextModeWidget::switchToGraphicalMode, this, &SieveEditorWidget::slotSwitchToGraphicalMode);
    connect(mTextModeWidget, &SieveEditorTextModeWidget::undoAvailable, this, &SieveEditorWidget::undoAvailable);
    connect(mTextModeWidget, &SieveEditorTextModeWidget::redoAvailable, this, &SieveEditorWidget::redoAvailable);
    connect(mTextModeWidget, &SieveEditorTextModeWidget::copyAvailable, this, &SieveEditorWidget::copyAvailable);
    if (KSieveUi::EditorSettings::useGraphicEditorByDefault()) {
        changeMode(GraphicMode);
    } else {
        changeSwitchButtonText();
    }
}

SieveEditorWidget::~SieveEditorWidget()
{
}

void SieveEditorWidget::setReadOnly(bool b)
{
    mTextModeWidget->setReadOnly(b);
    mGraphicalModeWidget->setDisabled(b);
}

void SieveEditorWidget::slotModified()
{
    mModified = true;
    Q_EMIT valueChanged(mModified);
}

bool SieveEditorWidget::isModified() const
{
    return mModified;
}

void SieveEditorWidget::undo()
{
    if (mMode == TextMode) {
        mTextModeWidget->undo();
    }
}

void SieveEditorWidget::redo()
{
    if (mMode == TextMode) {
        mTextModeWidget->redo();
    }
}

void SieveEditorWidget::goToLine()
{
    if (mMode == TextMode) {
        mTextModeWidget->slotShowGoToLine();
    }
}

void SieveEditorWidget::cut()
{
    if (mMode == TextMode) {
        mTextModeWidget->cut();
    }
}

void SieveEditorWidget::paste()
{
    if (mMode == TextMode) {
        mTextModeWidget->paste();
    }
}

void SieveEditorWidget::copy()
{
    if (mMode == TextMode) {
        mTextModeWidget->copy();
    }
}

void SieveEditorWidget::zoomIn()
{
    if (mMode == TextMode) {
        mTextModeWidget->zoomIn();
    }
}

bool SieveEditorWidget::isWordWrap() const
{
    if (mMode == TextMode) {
        return mTextModeWidget->isWordWrap();
    }
    return false;
}

void SieveEditorWidget::wordWrap(bool state)
{
    if (mMode == TextMode) {
        mTextModeWidget->wordWrap(state);
    }
}

void SieveEditorWidget::zoomReset()
{
    if (mMode == TextMode) {
        mTextModeWidget->zoomReset();
    }
}

void SieveEditorWidget::zoomOut()
{
    if (mMode == TextMode) {
        mTextModeWidget->zoomOut();
    }
}

void SieveEditorWidget::selectAll()
{
    if (mMode == TextMode) {
        mTextModeWidget->selectAll();
    }
}

void SieveEditorWidget::find()
{
    if (mMode == TextMode) {
        mTextModeWidget->find();
    }
}

void SieveEditorWidget::replace()
{
    if (mMode == TextMode) {
        mTextModeWidget->replace();
    }
}

bool SieveEditorWidget::isUndoAvailable() const
{
    if (mMode == TextMode) {
        return mTextModeWidget->isUndoAvailable();
    }
    return false;
}

bool SieveEditorWidget::isRedoAvailable() const
{
    if (mMode == TextMode) {
        return mTextModeWidget->isRedoAvailable();
    }
    return false;
}

bool SieveEditorWidget::hasSelection() const
{
    if (mMode == TextMode) {
        return mTextModeWidget->hasSelection();
    }
    return false;
}

void SieveEditorWidget::comment()
{
    if (mMode == TextMode) {
        mTextModeWidget->comment();
    }
}

void SieveEditorWidget::uncomment()
{
    if (mMode == TextMode) {
        mTextModeWidget->uncomment();
    }
}

SieveEditorWidget::EditorMode SieveEditorWidget::mode() const
{
    return mMode;
}

void SieveEditorWidget::setModified(bool b)
{
    if (mModified != b) {
        mModified = b;
        Q_EMIT valueChanged(mModified);
    }
}

void SieveEditorWidget::slotUploadScripts()
{
    QTemporaryDir tmp;
    QTemporaryFile tmpFile;
    if (tmpFile.open()) {
        QTextStream out(&tmpFile);
        out.setCodec("UTF-8");
        out << script();
        tmpFile.close();
        const QString sourceName = mScriptName->text();
        const QString zipFileName = tmp.path() + QDir::separator() + sourceName + QLatin1String(".zip");
        KZip *zip = new KZip(zipFileName);
        if (zip->open(QIODevice::WriteOnly)) {
            if (zip->addLocalFile(tmpFile.fileName(), sourceName + QLatin1String(".siv"))) {
                zip->close();
                QPointer<KNS3::UploadDialog> dialog = new KNS3::UploadDialog(QStringLiteral("ksieve_script.knsrc"), this);
                dialog->setUploadFile(QUrl::fromLocalFile(zipFileName));
                dialog->setUploadName(sourceName);
                dialog->setDescription(i18nc("Default description for the source", "My Sieve Script"));
                dialog->exec();
                delete dialog;
            } else {
                zip->close();
            }
        }
        delete zip;
    }
}

void SieveEditorWidget::changeMode(EditorMode mode)
{
    if (mode != mMode) {
        mMode = mode;
        mStackedWidget->setCurrentIndex(static_cast<int>(mode));
        const bool isTextMode = (mMode == TextMode);
        mAutoGenerateScript->setEnabled(isTextMode);
#if !defined(NDEBUG)
        if (mGenerateXml) {
            mGenerateXml->setEnabled(isTextMode);
        }
#endif
        if (isTextMode) {
            mCheckSyntax->setEnabled(!mTextModeWidget->currentscript().isEmpty());
        } else {
            mCheckSyntax->setEnabled(false);
        }
        Q_EMIT modeEditorChanged(mode);
        changeModeEditor(isTextMode);
        changeSwitchButtonText();
    }
}

void SieveEditorWidget::changeSwitchButtonText()
{
    mSwitchMode->setText((mMode == TextMode) ?  i18n("Simple Mode") : i18n("Advanced Mode"));
}

void SieveEditorWidget::slotEnableButtonOk(bool b)
{
    Q_EMIT enableButtonOk(b);
    mSaveAs->setEnabled(b);
    if (mMode == TextMode) {
        mCheckSyntax->setEnabled(b);
    } else {
        mCheckSyntax->setEnabled(false);
    }
}

QString SieveEditorWidget::script() const
{
    QString currentScript;
    switch (mMode) {
    case TextMode:
        currentScript = mTextModeWidget->script();
        break;
    case GraphicMode:
        currentScript = mGraphicalModeWidget->currentscript();
        break;
    case Unknown:
        qCDebug(LIBKSIEVE_LOG) << " Unknown Mode!";
        break;
    }
    return currentScript;
}

QString SieveEditorWidget::originalScript() const
{
    return mOriginalScript;
}

void SieveEditorWidget::setScript(const QString &script)
{
    mTextModeWidget->setScript(script);
    mOriginalScript = script;
}

void SieveEditorWidget::addFailedMessage(const QString &err)
{
    addMessageEntry(err, QColor(Qt::darkRed));
}

void SieveEditorWidget::addOkMessage(const QString &msg)
{
    addMessageEntry(msg, QColor(Qt::darkGreen));
}

void SieveEditorWidget::addMessageEntry(const QString &errorMsg, const QColor &color)
{
    QString msg = errorMsg;
    msg.replace(QLatin1Char('\n'), QStringLiteral("<br>"));
    const QString logText = QStringLiteral("<font color=%1>%2</font>")
                            .arg(color.name()).arg(msg);

    setDebugScript(logText);
}

void SieveEditorWidget::setDebugScript(const QString &debug)
{
    mTextModeWidget->setDebugScript(debug);
}

void SieveEditorWidget::setScriptName(const QString &name)
{
    mScriptName->setText(name);
}

void SieveEditorWidget::resultDone()
{
    mCheckSyntax->setEnabled(true);
}

void SieveEditorWidget::setSieveCapabilities(const QStringList &capabilities)
{
    mTextModeWidget->setSieveCapabilities(capabilities);
    mGraphicalModeWidget->setSieveCapabilities(capabilities);
}

void SieveEditorWidget::slotAutoGenerateScripts()
{
    switch (mMode) {
    case TextMode:
        mTextModeWidget->autoGenerateScripts();
        break;
    case GraphicMode:
    case Unknown:
        break;
    }
}

void SieveEditorWidget::slotCheckSyntax()
{
    switch (mMode) {
    case TextMode:
        mCheckSyntax->setEnabled(false);
        Q_EMIT checkSyntax();
        break;
    case GraphicMode:
    case Unknown:
        break;
    }
}

void SieveEditorWidget::slotGenerateXml()
{
#if !defined(NDEBUG)
    switch (mMode) {
    case TextMode:
        mTextModeWidget->generateXml();
        break;
    case GraphicMode:
    case Unknown:
        break;
    }
#endif
}

void SieveEditorWidget::checkSpelling()
{
    switch (mMode) {
    case TextMode:
        mTextModeWidget->checkSpelling();
        break;
    case GraphicMode:
    case Unknown:
        break;
    }
}

void SieveEditorWidget::slotSaveAs()
{
    switch (mMode) {
    case TextMode:
        mTextModeWidget->saveAs(mScriptName->text());
        break;
    case GraphicMode:
        mGraphicalModeWidget->saveAs(mScriptName->text());
        break;
    case Unknown:
        qCDebug(LIBKSIEVE_LOG) << " Unknown mode";
        break;
    }
}

void SieveEditorWidget::slotImport()
{
    switch (mMode) {
    case TextMode:
        mTextModeWidget->slotImport();
        break;
    case GraphicMode:
        mGraphicalModeWidget->slotImport();
        break;
    case Unknown:
        qCDebug(LIBKSIEVE_LOG) << " Unknown mode";
        break;
    }
}

void SieveEditorWidget::slotSwitchToGraphicalMode()
{
    mTextModeWidget->hideEditorWarning();
    changeMode(GraphicMode);
}

void SieveEditorWidget::slotSwitchMode()
{
    switch (mMode) {
    case TextMode: {
        bool result = false;
        const QDomDocument doc = ParsingUtil::parseScript(mTextModeWidget->currentscript(), result);
        if (result) {
            QString error;
            mGraphicalModeWidget->loadScript(doc, error);
            mTextModeWidget->hideEditorWarning();
            if (!error.isEmpty()) {
                mTextModeWidget->setParsingEditorWarningError(mTextModeWidget->currentscript(), error);
                mTextModeWidget->showParsingEditorWarning();
            } else {
                changeMode(GraphicMode);
            }
        } else {
            mTextModeWidget->showEditorWarning();
            qCDebug(LIBKSIEVE_LOG) << "cannot parse file";
        }
        break;
    }
    case GraphicMode: {
        const QString script = mGraphicalModeWidget->currentscript();
        changeMode(TextMode);
        mTextModeWidget->setScript(script);
        break;
    }
    case Unknown:
        qCDebug(LIBKSIEVE_LOG) << " Unknown mode";
        break;
    }
}

void SieveEditorWidget::slotSwitchTextMode(const QString &script)
{
    changeMode(TextMode);
    mTextModeWidget->setScript(script);
}

void SieveEditorWidget::reverseCase()
{
    if (mMode == TextMode) {
        mTextModeWidget->reverseCase();
    }
}

void SieveEditorWidget::lowerCase()
{
    if (mMode == TextMode) {
        mTextModeWidget->lowerCase();
    }
}

void SieveEditorWidget::debugSieveScript()
{
    if (mMode == TextMode) {
        mTextModeWidget->debugSieveScript();
    }
}

void SieveEditorWidget::upperCase()
{
    if (mMode == TextMode) {
        mTextModeWidget->upperCase();
    }
}

void SieveEditorWidget::sentenceCase()
{
    if (mMode == TextMode) {
        mTextModeWidget->sentenceCase();
    }
}

void SieveEditorWidget::openBookmarkUrl(const QUrl &url)
{
    if (mMode == TextMode) {
        mTextModeWidget->openBookmarkUrl(url);
    }
}

QString SieveEditorWidget::currentHelpTitle() const
{
    if (mMode == TextMode) {
        return mTextModeWidget->currentHelpTitle();
    }
    return QString();
}

QUrl SieveEditorWidget::currentHelpUrl() const
{
    if (mMode == TextMode) {
        return mTextModeWidget->currentHelpUrl();
    }
    return QUrl();
}
