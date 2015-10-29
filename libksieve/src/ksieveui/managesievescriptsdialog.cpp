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

#include "managesievescriptsdialog.h"
#include "widgets/custommanagesievewidget.h"
#include "widgets/managesievetreeview.h"
#include "editor/sievetextedit.h"
#include "editor/sieveeditor.h"
#include "widgets/sievetreewidgetitem.h"

#include <KLocalizedString>
#include <kiconloader.h>
#include <kwindowsystem.h>
#include <QPushButton>
#include <kmessagebox.h>
#include <KConfigGroup>

#include <kmanagesieve/sievejob.h>

#include <QApplication>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QDebug>

#include <errno.h>
#include <KSharedConfig>
#include <KGuiItem>
#include <KStandardGuiItem>

using namespace KSieveUi;

class KSieveUi::ManageSieveScriptsDialogPrivate
{
public:
    ManageSieveScriptsDialogPrivate()
        : mSieveEditor(Q_NULLPTR),
          mIsNewScript(false),
          mWasActive(false)
    {

    }

    CustomManageSieveWidget *mTreeView;
    SieveEditor *mSieveEditor;

    QUrl mCurrentURL;
    QStringList mCurrentCapabilities;

    QPushButton *mNewScript;
    QPushButton *mEditScript;
    QPushButton *mDeleteScript;
    QPushButton *mDeactivateScript;

    bool mIsNewScript : 1;
    bool mWasActive : 1;
};

ManageSieveScriptsDialog::ManageSieveScriptsDialog(QWidget *parent)
    : QDialog(parent),
      d(new KSieveUi::ManageSieveScriptsDialogPrivate)
{
    setWindowTitle(i18n("Manage Sieve Scripts"));
    setModal(false);
    setAttribute(Qt::WA_GroupLeader);
    setAttribute(Qt::WA_DeleteOnClose);
    KWindowSystem::setIcons(winId(), qApp->windowIcon().pixmap(IconSize(KIconLoader::Desktop), IconSize(KIconLoader::Desktop)), qApp->windowIcon().pixmap(IconSize(KIconLoader::Small), IconSize(KIconLoader::Small)));
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QFrame *frame = new QFrame;
    mainLayout->addWidget(frame);
    QVBoxLayout *vlay = new QVBoxLayout(frame);
    vlay->setSpacing(0);
    vlay->setMargin(0);

    d->mTreeView = new CustomManageSieveWidget(frame);
    connect(d->mTreeView, &CustomManageSieveWidget::editScript, this, &ManageSieveScriptsDialog::slotEditScript);
    connect(d->mTreeView, &CustomManageSieveWidget::newScript, this, &ManageSieveScriptsDialog::slotNewScript);
    connect(d->mTreeView, &CustomManageSieveWidget::updateButtons, this, &ManageSieveScriptsDialog::slotUpdateButtons);
    vlay->addWidget(d->mTreeView);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    vlay->addLayout(buttonLayout);

    d->mNewScript = new QPushButton(i18nc("create a new sieve script", "New..."));
    connect(d->mNewScript, &QPushButton::clicked, d->mTreeView, &CustomManageSieveWidget::slotNewScript);
    buttonLayout->addWidget(d->mNewScript);

    d->mEditScript = new QPushButton(i18n("Edit..."));
    connect(d->mEditScript, &QPushButton::clicked, d->mTreeView, &CustomManageSieveWidget::slotEditScript);
    buttonLayout->addWidget(d->mEditScript);

    d->mDeleteScript = new QPushButton(i18n("Delete"));
    connect(d->mDeleteScript, &QPushButton::clicked, d->mTreeView, &CustomManageSieveWidget::slotDeleteScript);
    buttonLayout->addWidget(d->mDeleteScript);

    d->mDeactivateScript = new QPushButton(i18n("Deactivate"));
    connect(d->mDeactivateScript, &QPushButton::clicked, d->mTreeView, &CustomManageSieveWidget::slotDeactivateScript);
    buttonLayout->addWidget(d->mDeactivateScript);

    QPushButton *close = new QPushButton;
    KGuiItem::assign(close, KStandardGuiItem::close());
    connect(close, &QPushButton::clicked, this, &ManageSieveScriptsDialog::accept);
    buttonLayout->addWidget(close);

    KConfigGroup group(KSharedConfig::openConfig(), "ManageSieveScriptsDialog");
    const QSize size = group.readEntry("Size", QSize());
    if (size.isValid()) {
        resize(size);
    } else {
        resize(sizeHint().width(), sizeHint().height());
    }

    d->mTreeView->slotRefresh();
}

ManageSieveScriptsDialog::~ManageSieveScriptsDialog()
{
    KConfigGroup group(KSharedConfig::openConfig(), "ManageSieveScriptsDialog");
    group.writeEntry("Size", size());
    // prevent QTreeWidget signals triggered by it's desctructor from calling our slots
    d->mTreeView->disconnect(this);
    delete d;
}

void ManageSieveScriptsDialog::hideEvent(QHideEvent *event)
{
    if (!event->spontaneous()) {
        Q_EMIT finished();
    }
}

void ManageSieveScriptsDialog::slotUpdateButtons(QTreeWidgetItem *item)
{
    Q_UNUSED(item);
    bool newScriptAction;
    bool editScriptAction;
    bool deleteScriptAction;
    bool desactivateScriptAction;
    d->mTreeView->enableDisableActions(newScriptAction, editScriptAction, deleteScriptAction, desactivateScriptAction);
    d->mNewScript->setEnabled(newScriptAction);
    d->mEditScript->setEnabled(editScriptAction);
    d->mDeleteScript->setEnabled(deleteScriptAction);
    d->mDeactivateScript->setEnabled(desactivateScriptAction);
}

void ManageSieveScriptsDialog::slotEditScript(const QUrl &url, const QStringList &capabilities)
{
    d->mCurrentURL = url;
    d->mCurrentCapabilities = capabilities;
    d->mIsNewScript = false;
    KManageSieve::SieveJob *job = KManageSieve::SieveJob::get(url);
    connect(job, &KManageSieve::SieveJob::result, this, &ManageSieveScriptsDialog::slotGetResult);
}

void ManageSieveScriptsDialog::slotNewScript(const QUrl &url, const QStringList &capabilities)
{
    d->mCurrentCapabilities = capabilities;
    d->mCurrentURL = url;
    d->mIsNewScript = true;
    slotGetResult(Q_NULLPTR, true, QString(), false);
}

void ManageSieveScriptsDialog::slotGetResult(KManageSieve::SieveJob *, bool success, const QString &script, bool isActive)
{
    if (!success) {
        return;
    }

    if (d->mSieveEditor) {
        return;
    }

    disableManagerScriptsDialog(true);
    d->mSieveEditor = new SieveEditor;
    d->mSieveEditor->setScriptName(d->mCurrentURL.fileName());
    d->mSieveEditor->setSieveCapabilities(d->mCurrentCapabilities);
    d->mSieveEditor->setScript(script);
    connect(d->mSieveEditor, &SieveEditor::okClicked, this, &ManageSieveScriptsDialog::slotSieveEditorOkClicked);
    connect(d->mSieveEditor, &SieveEditor::cancelClicked, this, &ManageSieveScriptsDialog::slotSieveEditorCancelClicked);
    connect(d->mSieveEditor, &SieveEditor::checkSyntax, this, &ManageSieveScriptsDialog::slotSieveEditorCheckSyntaxClicked);
    d->mSieveEditor->show();
    d->mWasActive = isActive;
}

void ManageSieveScriptsDialog::slotSieveEditorCheckSyntaxClicked()
{
    if (!d->mSieveEditor) {
        return;
    }
    const QString script = d->mSieveEditor->script();
    if (script.isEmpty()) {
        return;
    }
    KManageSieve::SieveJob *job = KManageSieve::SieveJob::put(d->mCurrentURL, script, d->mWasActive, d->mWasActive);
    job->setInteractive(false);
    connect(job, &KManageSieve::SieveJob::errorMessage, this, &ManageSieveScriptsDialog::slotPutResultDebug);
}

void ManageSieveScriptsDialog::slotSieveEditorOkClicked()
{
    disableManagerScriptsDialog(false);
    if (!d->mSieveEditor) {
        return;
    }
    KManageSieve::SieveJob *job = KManageSieve::SieveJob::put(d->mCurrentURL, d->mSieveEditor->script(), d->mWasActive, d->mWasActive);
    connect(job, &KManageSieve::SieveJob::result, this, &ManageSieveScriptsDialog::slotPutResult);
}

void ManageSieveScriptsDialog::slotSieveEditorCancelClicked()
{
    disableManagerScriptsDialog(false);
    d->mSieveEditor->deleteLater();
    d->mSieveEditor = Q_NULLPTR;
    d->mCurrentURL = QUrl();
    if (d->mIsNewScript) {
        d->mTreeView->slotRefresh();
    }
}

void ManageSieveScriptsDialog::slotPutResultDebug(KManageSieve::SieveJob *, bool success, const QString &errorMsg)
{
    if (success) {
        d->mSieveEditor->addOkMessage(i18n("No errors found."));
    } else {
        if (errorMsg.isEmpty()) {
            d->mSieveEditor->addFailedMessage(i18n("An unknown error was encountered."));
        } else {
            d->mSieveEditor->addFailedMessage(errorMsg);
        }
    }
    //Put original script after check otherwise we will put a script even if we don't click on ok
    KManageSieve::SieveJob *job = KManageSieve::SieveJob::put(d->mCurrentURL, d->mSieveEditor->originalScript(), d->mWasActive, d->mWasActive);
    job->setInteractive(false);
    d->mSieveEditor->resultDone();
}

void ManageSieveScriptsDialog::slotPutResult(KManageSieve::SieveJob *, bool success)
{
    if (success) {
        KMessageBox::information(this, i18n("The Sieve script was successfully uploaded."),
                                 i18n("Sieve Script Upload"));
        d->mSieveEditor->deleteLater();
        d->mSieveEditor = Q_NULLPTR;
        d->mCurrentURL = QUrl();
    } else {
        d->mSieveEditor->show();
    }
}

void ManageSieveScriptsDialog::disableManagerScriptsDialog(bool disable)
{
    setDisabled(disable);
}

