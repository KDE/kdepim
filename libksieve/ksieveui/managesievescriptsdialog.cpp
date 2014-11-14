/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#include <agentinstance.h>
#include <kmanagesieve/sievejob.h>
#include <ksieveui/util/util.h>

#include <QApplication>
#include <QTreeWidget>
#include <QVBoxLayout>

#include <errno.h>
#include <KSharedConfig>
#include <KGuiItem>
#include <KStandardGuiItem>

using namespace KSieveUi;

CustomManageSieveWidget::CustomManageSieveWidget(QWidget *parent)
    : KSieveUi::ManageSieveWidget(parent)
{

}

CustomManageSieveWidget::~CustomManageSieveWidget()
{

}

bool CustomManageSieveWidget::refreshList()
{
    bool noImapFound = true;
    SieveTreeWidgetItem *last = 0;
    Akonadi::AgentInstance::List lst = KSieveUi::Util::imapAgentInstances();
    foreach (const Akonadi::AgentInstance &type, lst) {
        if (type.status() == Akonadi::AgentInstance::Broken) {
            continue;
        }

        QString serverName = type.name();
        last = new SieveTreeWidgetItem(treeView(), last);
        last->setIcon(0, SmallIcon(QStringLiteral("network-server")));

        const QUrl u = KSieveUi::Util::findSieveUrlForAccount(type.identifier());
        if (u.isEmpty()) {
            QTreeWidgetItem *item = new QTreeWidgetItem(last);
            item->setText(0, i18n("No Sieve URL configured"));
            item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
            treeView()->expandItem(last);
        } else {
            serverName += QString::fromLatin1(" (%1)").arg(u.userName());
            KManageSieve::SieveJob *job = KManageSieve::SieveJob::list(u);
            connect(job, &KManageSieve::SieveJob::gotList, this, &CustomManageSieveWidget::slotGotList);
            mJobs.insert(job, last);
            mUrls.insert(last, u);
            last->startAnimation();
        }
        last->setText(0, serverName);
        noImapFound = false;
    }
    return noImapFound;
}

ManageSieveScriptsDialog::ManageSieveScriptsDialog(QWidget *parent)
    : QDialog(parent),
      mSieveEditor(0),
      mIsNewScript(false),
      mWasActive(false)
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

    mTreeView = new CustomManageSieveWidget(frame);
    connect(mTreeView, &CustomManageSieveWidget::editScript, this, &ManageSieveScriptsDialog::slotEditScript);
    connect(mTreeView, &CustomManageSieveWidget::newScript, this, &ManageSieveScriptsDialog::slotNewScript);
    connect(mTreeView, &CustomManageSieveWidget::updateButtons, this, &ManageSieveScriptsDialog::slotUpdateButtons);
    vlay->addWidget(mTreeView);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    vlay->addLayout(buttonLayout);

    mNewScript = new QPushButton(i18nc("create a new sieve script", "New..."));
    connect(mNewScript, &QPushButton::clicked, mTreeView, &CustomManageSieveWidget::slotNewScript);
    buttonLayout->addWidget(mNewScript);

    mEditScript = new QPushButton(i18n("Edit..."));
    connect(mEditScript, &QPushButton::clicked, mTreeView, &CustomManageSieveWidget::slotEditScript);
    buttonLayout->addWidget(mEditScript);

    mDeleteScript = new QPushButton(i18n("Delete"));
    connect(mDeleteScript, &QPushButton::clicked, mTreeView, &CustomManageSieveWidget::slotDeleteScript);
    buttonLayout->addWidget(mDeleteScript);

    mDeactivateScript = new QPushButton(i18n("Deactivate"));
    connect(mDeactivateScript, &QPushButton::clicked, mTreeView, &CustomManageSieveWidget::slotDeactivateScript);
    buttonLayout->addWidget(mDeactivateScript);

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

    mTreeView->slotRefresh();
}

ManageSieveScriptsDialog::~ManageSieveScriptsDialog()
{
    KConfigGroup group(KSharedConfig::openConfig(), "ManageSieveScriptsDialog");
    group.writeEntry("Size", size());
}

void ManageSieveScriptsDialog::slotUpdateButtons(QTreeWidgetItem *item)
{
    Q_UNUSED(item);
    bool newScriptAction;
    bool editScriptAction;
    bool deleteScriptAction;
    bool desactivateScriptAction;
    mTreeView->enableDisableActions(newScriptAction, editScriptAction, deleteScriptAction, desactivateScriptAction);
    mNewScript->setEnabled(newScriptAction);
    mEditScript->setEnabled(editScriptAction);
    mDeleteScript->setEnabled(deleteScriptAction);
    mDeactivateScript->setEnabled(desactivateScriptAction);
}

void ManageSieveScriptsDialog::slotEditScript(const QUrl &url, const QStringList &capabilities)
{
    mCurrentURL = url;
    mCurrentCapabilities = capabilities;
    mIsNewScript = false;
    KManageSieve::SieveJob *job = KManageSieve::SieveJob::get(url);
    connect(job, &KManageSieve::SieveJob::result, this, &ManageSieveScriptsDialog::slotGetResult);
}

void ManageSieveScriptsDialog::slotNewScript(const QUrl &url, const QStringList &capabilities)
{
    mCurrentCapabilities = capabilities;
    mCurrentURL = url;
    mIsNewScript = true;
    slotGetResult(0, true, QString(), false);
}

void ManageSieveScriptsDialog::slotGetResult(KManageSieve::SieveJob *, bool success, const QString &script, bool isActive)
{
    if (!success) {
        return;
    }

    if (mSieveEditor) {
        return;
    }

    disableManagerScriptsDialog(true);
    mSieveEditor = new SieveEditor;
    mSieveEditor->setScriptName(mCurrentURL.fileName());
    mSieveEditor->setSieveCapabilities(mCurrentCapabilities);
    mSieveEditor->setScript(script);
    connect(mSieveEditor, &SieveEditor::okClicked, this, &ManageSieveScriptsDialog::slotSieveEditorOkClicked);
    connect(mSieveEditor, &SieveEditor::cancelClicked, this, &ManageSieveScriptsDialog::slotSieveEditorCancelClicked);
    connect(mSieveEditor, &SieveEditor::checkSyntax, this, &ManageSieveScriptsDialog::slotSieveEditorCheckSyntaxClicked);
    mSieveEditor->show();
    mWasActive = isActive;
}

void ManageSieveScriptsDialog::slotSieveEditorCheckSyntaxClicked()
{
    if (!mSieveEditor) {
        return;
    }
    KManageSieve::SieveJob *job = KManageSieve::SieveJob::put(mCurrentURL, mSieveEditor->script(), mWasActive, mWasActive);
    job->setInteractive(false);
    connect(job, &KManageSieve::SieveJob::errorMessage, this, &ManageSieveScriptsDialog::slotPutResultDebug);
}

void ManageSieveScriptsDialog::slotSieveEditorOkClicked()
{
    if (!mSieveEditor) {
        return;
    }
    disableManagerScriptsDialog(false);
    KManageSieve::SieveJob *job = KManageSieve::SieveJob::put(mCurrentURL, mSieveEditor->script(), mWasActive, mWasActive);
    connect(job, &KManageSieve::SieveJob::result, this, &ManageSieveScriptsDialog::slotPutResult);
}

void ManageSieveScriptsDialog::slotSieveEditorCancelClicked()
{
    disableManagerScriptsDialog(false);
    mSieveEditor->deleteLater();
    mSieveEditor = 0;
    mCurrentURL = QUrl();
    if (mIsNewScript) {
        mTreeView->slotRefresh();
    }
}

void ManageSieveScriptsDialog::slotPutResultDebug(KManageSieve::SieveJob *, bool success , const QString &errorMsg)
{
    if (success) {
        mSieveEditor->addOkMessage(i18n("No errors found."));
    } else {
        if (errorMsg.isEmpty()) {
            mSieveEditor->addFailedMessage(i18n("An unknown error was encountered."));
        } else {
            mSieveEditor->addFailedMessage(errorMsg);
        }
    }
    //Put original script after check otherwise we will put a script even if we don't click on ok
    KManageSieve::SieveJob *job = KManageSieve::SieveJob::put(mCurrentURL, mSieveEditor->originalScript(), mWasActive, mWasActive);
    job->setInteractive(false);
    mSieveEditor->resultDone();
}

void ManageSieveScriptsDialog::slotPutResult(KManageSieve::SieveJob *, bool success)
{
    if (success) {
        KMessageBox::information(this, i18n("The Sieve script was successfully uploaded."),
                                 i18n("Sieve Script Upload"));
        mSieveEditor->deleteLater();
        mSieveEditor = 0;
        mCurrentURL = QUrl();
    } else {
        mSieveEditor->show();
    }
}

void ManageSieveScriptsDialog::disableManagerScriptsDialog(bool disable)
{
    setDisabled(disable);
}

