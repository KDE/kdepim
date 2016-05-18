/*
  Copyright (c) 2014-2016 Montel Laurent <montel@kde.org>

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

#include "sieveeditorpagewidget.h"
#include "ksieveui/sieveeditorwidget.h"
#include "sieveeditorglobalconfig.h"

#include <kmanagesieve/sievejob.h>

#include <KLocalizedString>
#include <KMessageBox>

#include "sieveeditor_debug.h"
#include <QVBoxLayout>

SieveEditorPageWidget::SieveEditorPageWidget(QWidget *parent)
    : QWidget(parent),
      mWasActive(false),
      mIsNewScript(false)
{
    QVBoxLayout *vbox = new QVBoxLayout;
    setLayout(vbox);
    mSieveEditorWidget = new KSieveUi::SieveEditorWidget(false);
    connect(mSieveEditorWidget, &KSieveUi::SieveEditorWidget::valueChanged, this, &SieveEditorPageWidget::slotValueChanged);
    vbox->addWidget(mSieveEditorWidget);
    connect(mSieveEditorWidget, &KSieveUi::SieveEditorWidget::checkSyntax, this, &SieveEditorPageWidget::slotCheckSyntaxClicked);
    connect(mSieveEditorWidget, &KSieveUi::SieveEditorWidget::modeEditorChanged, this, &SieveEditorPageWidget::modeEditorChanged);
    connect(mSieveEditorWidget, &KSieveUi::SieveEditorWidget::undoAvailable, this, &SieveEditorPageWidget::undoAvailable);
    connect(mSieveEditorWidget, &KSieveUi::SieveEditorWidget::redoAvailable, this, &SieveEditorPageWidget::redoAvailable);
    connect(mSieveEditorWidget, &KSieveUi::SieveEditorWidget::copyAvailable, this, &SieveEditorPageWidget::copyAvailable);
    connect(mSieveEditorWidget, &KSieveUi::SieveEditorWidget::sieveEditorTabCurrentChanged, this, &SieveEditorPageWidget::sieveEditorTabCurrentChanged);
}

SieveEditorPageWidget::~SieveEditorPageWidget()
{
}

void SieveEditorPageWidget::slotCheckSyntaxClicked()
{
    const QString script = mSieveEditorWidget->script();
    if (script.isEmpty()) {
        return;
    }
    KManageSieve::SieveJob *job = KManageSieve::SieveJob::put(mCurrentURL, script, mWasActive, mWasActive);
    job->setInteractive(false);
    connect(job, &KManageSieve::SieveJob::errorMessage, this, &SieveEditorPageWidget::slotPutResultDebug);
}

void SieveEditorPageWidget::slotPutResultDebug(KManageSieve::SieveJob *, bool success, const QString &errorMsg)
{
    if (success) {
        mSieveEditorWidget->addOkMessage(i18n("No errors found."));
    } else {
        if (errorMsg.isEmpty()) {
            mSieveEditorWidget->addFailedMessage(i18n("An unknown error was encountered."));
        } else {
            mSieveEditorWidget->addFailedMessage(errorMsg);
        }
    }
    //Put original script after check otherwise we will put a script even if we don't click on ok
    KManageSieve::SieveJob *job = KManageSieve::SieveJob::put(mCurrentURL, mSieveEditorWidget->originalScript(), mWasActive, mWasActive);
    job->setInteractive(false);
    mSieveEditorWidget->resultDone();
}

void SieveEditorPageWidget::setIsNewScript(bool isNewScript)
{
    mIsNewScript = isNewScript;
}

void SieveEditorPageWidget::loadScript(const QUrl &url, const QStringList &capabilities)
{
    mCurrentURL = url;
    mSieveEditorWidget->setSieveCapabilities(capabilities);
    mSieveEditorWidget->setReadOnly(true);
    mSieveEditorWidget->wordWrap(SieveEditorGlobalConfig::self()->wrapText());
    KManageSieve::SieveJob *job = KManageSieve::SieveJob::get(url);
    connect(job, &KManageSieve::SieveJob::result, this, &SieveEditorPageWidget::slotGetResult);
}

QUrl SieveEditorPageWidget::currentUrl() const
{
    return mCurrentURL;
}

void SieveEditorPageWidget::slotGetResult(KManageSieve::SieveJob *, bool success, const QString &script, bool isActive)
{
    mSieveEditorWidget->setReadOnly(false);
    if (!success) {
        return;
    }
    mSieveEditorWidget->setScriptName(mCurrentURL.fileName());
    mSieveEditorWidget->setScript(script);
    mWasActive = isActive;
    mSieveEditorWidget->setModified(false);
}

void SieveEditorPageWidget::uploadScript(bool showInformation, bool forceSave)
{
    if (mSieveEditorWidget->isModified() || forceSave) {
        KManageSieve::SieveJob *job = KManageSieve::SieveJob::put(mCurrentURL, mSieveEditorWidget->script(), mWasActive, mWasActive);
        job->setProperty("showuploadinformation", showInformation);
        connect(job, &KManageSieve::SieveJob::result, this, &SieveEditorPageWidget::slotPutResult);
    }
}

void SieveEditorPageWidget::slotPutResult(KManageSieve::SieveJob *job, bool success)
{
    if (mIsNewScript) {
        Q_EMIT refreshList();
    }
    if (success) {
        if (job->property("showuploadinformation").toBool()) {
            KMessageBox::information(this, i18n("The Sieve script was successfully uploaded."),
                                     i18n("Sieve Script Upload"));
        }
        mIsNewScript = false;
        mSieveEditorWidget->updateOriginalScript();
        mSieveEditorWidget->setModified(false);
    } else {
        //TODO error
    }
}

bool SieveEditorPageWidget::needToSaveScript()
{
    bool result = false;
    if (mIsNewScript) {
        const int resultQuestion = KMessageBox::warningYesNoCancel(this, i18n("Script '%1' is new. Do you want to save it?", mCurrentURL.fileName()));
        if (resultQuestion == KMessageBox::Yes) {
            uploadScript();
            result = true;
        } else if (resultQuestion == KMessageBox::Cancel) {
            result = true;
        }
    } else {
        if (mSieveEditorWidget->isModified()) {
            const int resultQuestion = KMessageBox::warningYesNoCancel(this, i18n("Script '%1' was changed. Do you want to save it ?", mCurrentURL.fileName()));
            if (resultQuestion == KMessageBox::Yes) {
                uploadScript();
                result = true;
            } else if (resultQuestion == KMessageBox::Cancel) {
                result = true;
            }
        }
    }
    return result;
}

void SieveEditorPageWidget::slotValueChanged(bool b)
{
    Q_EMIT scriptModified(b, this);
}

bool SieveEditorPageWidget::isModified() const
{
    return mSieveEditorWidget->isModified();
}

void SieveEditorPageWidget::goToLine()
{
    mSieveEditorWidget->goToLine();
}

void SieveEditorPageWidget::undo()
{
    mSieveEditorWidget->undo();
}

bool SieveEditorPageWidget::isUndoAvailable() const
{
    return mSieveEditorWidget->isUndoAvailable();
}

bool SieveEditorPageWidget::isRedoAvailable() const
{
    return mSieveEditorWidget->isRedoAvailable();
}

bool SieveEditorPageWidget::hasSelection() const
{
    return mSieveEditorWidget->hasSelection();
}

void SieveEditorPageWidget::redo()
{
    mSieveEditorWidget->redo();
}

void SieveEditorPageWidget::find()
{
    mSieveEditorWidget->find();
}

void SieveEditorPageWidget::replace()
{
    mSieveEditorWidget->replace();
}

void SieveEditorPageWidget::shareScript()
{
    mSieveEditorWidget->slotShareScript();
}

void SieveEditorPageWidget::import()
{
    mSieveEditorWidget->slotImport();
}

void SieveEditorPageWidget::comment()
{
    mSieveEditorWidget->comment();
}

void SieveEditorPageWidget::uncomment()
{
    mSieveEditorWidget->uncomment();
}

void SieveEditorPageWidget::checkSpelling()
{
    mSieveEditorWidget->checkSpelling();
}

void SieveEditorPageWidget::autoGenerateScript()
{
    mSieveEditorWidget->slotAutoGenerateScripts();
}

void SieveEditorPageWidget::checkSyntax()
{
    mSieveEditorWidget->slotCheckSyntax();
}

void SieveEditorPageWidget::saveAs()
{
    mSieveEditorWidget->slotSaveAs();
}

void SieveEditorPageWidget::reverseCase()
{
    mSieveEditorWidget->reverseCase();
}

void SieveEditorPageWidget::lowerCase()
{
    mSieveEditorWidget->lowerCase();
}

void SieveEditorPageWidget::debugSieveScript()
{
    mSieveEditorWidget->debugSieveScript();
}

void SieveEditorPageWidget::upperCase()
{
    mSieveEditorWidget->upperCase();
}

void SieveEditorPageWidget::sentenceCase()
{
    mSieveEditorWidget->sentenceCase();
}

KSieveUi::SieveEditorWidget::EditorMode SieveEditorPageWidget::pageMode() const
{
    return mSieveEditorWidget->mode();
}

void SieveEditorPageWidget::paste()
{
    mSieveEditorWidget->paste();
}

void SieveEditorPageWidget::cut()
{
    mSieveEditorWidget->cut();
}

void SieveEditorPageWidget::copy()
{
    mSieveEditorWidget->copy();
}

void SieveEditorPageWidget::selectAll()
{
    mSieveEditorWidget->selectAll();
}

void SieveEditorPageWidget::zoomIn()
{
    mSieveEditorWidget->zoomIn();
}

void SieveEditorPageWidget::zoomOut()
{
    mSieveEditorWidget->zoomOut();
}

void SieveEditorPageWidget::wordWrap(bool state)
{
    mSieveEditorWidget->wordWrap(state);
}

bool SieveEditorPageWidget::isWordWrap() const
{
    return mSieveEditorWidget->isWordWrap();
}

void SieveEditorPageWidget::print()
{
    mSieveEditorWidget->print();
}

void SieveEditorPageWidget::printPreview()
{
    mSieveEditorWidget->printPreview();
}

void SieveEditorPageWidget::zoomReset()
{
    mSieveEditorWidget->zoomReset();
}

void SieveEditorPageWidget::openBookmarkUrl(const QUrl &url)
{
    mSieveEditorWidget->openBookmarkUrl(url);
}

QString SieveEditorPageWidget::currentHelpTitle() const
{
    return mSieveEditorWidget->currentHelpTitle();
}

QUrl SieveEditorPageWidget::currentHelpUrl() const
{
    return mSieveEditorWidget->currentHelpUrl();
}
