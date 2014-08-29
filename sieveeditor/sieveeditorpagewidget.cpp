/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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
#include "editor/sieveeditorwidget.h"

#include <kmanagesieve/sievejob.h>

#include <KLocalizedString>
#include <KMessageBox>

#include <QDebug>
#include <QVBoxLayout>

SieveEditorPageWidget::SieveEditorPageWidget(QWidget *parent)
    : QWidget(parent),
      mWasActive(false),
      mIsNewScript(false)
{
    QVBoxLayout *vbox = new QVBoxLayout;
    setLayout(vbox);
    mSieveEditorWidget = new KSieveUi::SieveEditorWidget;
    connect(mSieveEditorWidget, &KSieveUi::SieveEditorWidget::valueChanged, this, &SieveEditorPageWidget::slotValueChanged);
    vbox->addWidget(mSieveEditorWidget);
    connect(mSieveEditorWidget, &KSieveUi::SieveEditorWidget::checkSyntax, this, &SieveEditorPageWidget::slotCheckSyntaxClicked);
    connect(mSieveEditorWidget, SIGNAL(modeEditorChanged(KSieveUi::SieveEditorWidget::EditorMode)), SIGNAL(modeEditorChanged(KSieveUi::SieveEditorWidget::EditorMode)));
    //qDebug()<<"SieveEditorPageWidget::SieveEditorPageWidget "<<this;
}

SieveEditorPageWidget::~SieveEditorPageWidget()
{
    //qDebug()<<" SieveEditorPageWidget::~SieveEditorPageWidget"<<this;
}

void SieveEditorPageWidget::slotCheckSyntaxClicked()
{
    KManageSieve::SieveJob *job = KManageSieve::SieveJob::put(mCurrentURL, mSieveEditorWidget->script(), mWasActive, mWasActive);
    job->setInteractive(false);
    connect(job, SIGNAL(errorMessage(KManageSieve::SieveJob*,bool,QString)),
            this, SLOT(slotPutResultDebug(KManageSieve::SieveJob*,bool,QString)));
}

void SieveEditorPageWidget::slotPutResultDebug(KManageSieve::SieveJob *, bool success , const QString &errorMsg)
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
    KManageSieve::SieveJob *job = KManageSieve::SieveJob::get(url);
    connect(job, SIGNAL(result(KManageSieve::SieveJob*,bool,QString,bool)),
            this, SLOT(slotGetResult(KManageSieve::SieveJob*,bool,QString,bool)));
}

QUrl SieveEditorPageWidget::currentUrl() const
{
    return mCurrentURL;
}

void SieveEditorPageWidget::slotGetResult(KManageSieve::SieveJob *, bool success, const QString &script, bool isActive)
{
    if (!success) {
        return;
    }
    mSieveEditorWidget->setScriptName(mCurrentURL.fileName());
    mSieveEditorWidget->setScript(script);
    mWasActive = isActive;
    mSieveEditorWidget->setModified(false);
}

void SieveEditorPageWidget::saveScript(bool showInformation, bool forceSave)
{
    if (mSieveEditorWidget->isModified() || forceSave) {
        KManageSieve::SieveJob *job = KManageSieve::SieveJob::put(mCurrentURL, mSieveEditorWidget->script(), mWasActive, mWasActive);
        job->setProperty("showuploadinformation", showInformation);
        connect(job, SIGNAL(result(KManageSieve::SieveJob*,bool,QString,bool)),
                this, SLOT(slotPutResult(KManageSieve::SieveJob*,bool)));
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
            saveScript();
            result = true;
        } else if (resultQuestion == KMessageBox::Cancel) {
            result = true;
        }
    } else {
        if (mSieveEditorWidget->isModified()) {
            const int resultQuestion = KMessageBox::warningYesNoCancel(this, i18n("Script '%1' was changed. Do you want to save it ?", mCurrentURL.fileName()));
            if (resultQuestion == KMessageBox::Yes) {
                saveScript();
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

KSieveUi::SieveEditorWidget::EditorMode SieveEditorPageWidget::pageMode() const
{
    return mSieveEditorWidget->mode();
}
