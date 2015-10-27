/*
  Copyright (c) 2012-2015 Montel Laurent <montel@kde.org>

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

#include "pimsettingexporterwindow.h"
#include "dialog/showarchivestructuredialog.h"
#include "importexportprogressindicatorgui.h"
#include "widgets/logwidget.h"
#include "pimsettingexportgui_debug.h"

#include "pimsettingexporterkernel.h"
#include "dialog/selectiontypedialog.h"
#include "utils.h"
#include "pimsettingsbackuprestoreui.h"

#include "dialog/backupfilestructureinfodialog.h"

#include <MailCommon/MailKernel>
#include <MailCommon/FilterManager>

#include "PimCommon/PimUtil"

#include <AkonadiWidgets/ControlGui>

#include <KStandardAction>
#include <KConfigGroup>
#include <KActionCollection>
#include <KMessageBox>
#include <KLocalizedString>
#include <QStatusBar>
#include <KRecentFilesAction>
#include <QPointer>
#include <KSharedConfig>
#include <QStandardPaths>
#include <QLocale>
#include <QFileDialog>
#include <QCommandLineParser>

PimSettingExporterWindow::PimSettingExporterWindow(QWidget *parent)
    : KXmlGuiWindow(parent),
      mBackupAction(Q_NULLPTR),
      mRestoreAction(Q_NULLPTR),
      mSaveLogAction(Q_NULLPTR),
      mArchiveStructureInfo(Q_NULLPTR),
      mShowArchiveInformationsAction(Q_NULLPTR),
      mPimSettingsBackupRestoreUI(Q_NULLPTR)
{
    //Initialize filtermanager
    (void)MailCommon::FilterManager::instance();
    PimSettingExporterKernel *kernel = new PimSettingExporterKernel(this);
    CommonKernel->registerKernelIf(kernel);   //register KernelIf early, it is used by the Filter classes
    CommonKernel->registerSettingsIf(kernel);   //SettingsIf is used in FolderTreeWidget

    setupActions(true);
    setupGUI(Keys | StatusBar | Save | Create, QStringLiteral("pimsettingexporter.rc"));
    mLogWidget = new LogWidget(this);

    setCentralWidget(mLogWidget);
    resize(800, 600);
    Akonadi::ControlGui::widgetNeedsAkonadi(this);
    statusBar()->hide();
}

PimSettingExporterWindow::~PimSettingExporterWindow()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup groupConfig = config->group(QStringLiteral("Recent File"));
    mRecentFilesAction->saveEntries(groupConfig);
}

void PimSettingExporterWindow::initializeBackupRestoreUi()
{
    mPimSettingsBackupRestoreUI = new PimSettingsBackupRestoreUI(this, this);
    connect(mPimSettingsBackupRestoreUI, &PimSettingsBackupRestore::addInfo, this, &PimSettingExporterWindow::slotAddInfo);
    connect(mPimSettingsBackupRestoreUI, &PimSettingsBackupRestore::addEndLine, this, &PimSettingExporterWindow::slotAddEndLine);
    connect(mPimSettingsBackupRestoreUI, &PimSettingsBackupRestore::addError, this, &PimSettingExporterWindow::slotAddError);
    connect(mPimSettingsBackupRestoreUI, &PimSettingsBackupRestore::addTitle, this, &PimSettingExporterWindow::slotAddTitle);
    connect(mPimSettingsBackupRestoreUI, &PimSettingsBackupRestore::updateActions, this, &PimSettingExporterWindow::slotUpdateActions);
    connect(mPimSettingsBackupRestoreUI, &PimSettingsBackupRestore::jobFinished, this, &PimSettingExporterWindow::slotJobFinished);
    connect(mPimSettingsBackupRestoreUI, &PimSettingsBackupRestore::backupDone, this, &PimSettingExporterWindow::slotShowBackupFinishDialogInformation);
    connect(mPimSettingsBackupRestoreUI, &PimSettingsBackupRestore::jobFailed, this, &PimSettingExporterWindow::slotJobFailed);
}

void PimSettingExporterWindow::slotJobFinished()
{
    mPimSettingsBackupRestoreUI->nextStep();
}

void PimSettingExporterWindow::slotJobFailed()
{
    mPimSettingsBackupRestoreUI->closeArchive();
}

void PimSettingExporterWindow::slotShowBackupFinishDialogInformation()
{
    KMessageBox::information(this, i18n("For restoring data, you must use \"pimsettingexporter\". Be careful it can overwrite existing settings, data."), i18n("Backup infos."), QStringLiteral("setProgressDialogLabelBackupInfos"));
}

void PimSettingExporterWindow::handleCommandLine(const QCommandLineParser &parser)
{
    QString templateFile;
    if (parser.isSet(QStringLiteral("template"))) {
        templateFile = parser.value(QStringLiteral("template"));
    }
    if (parser.isSet(QStringLiteral("import"))) {
        if (!parser.positionalArguments().isEmpty()) {
            loadData(parser.positionalArguments().at(0), templateFile);
        }
    } else if (parser.isSet(QStringLiteral("export"))) {
        if (!parser.positionalArguments().isEmpty()) {
            backupData(parser.positionalArguments().at(0), templateFile);
        }
    }
}

void PimSettingExporterWindow::setupActions(bool canZipFile)
{
    KActionCollection *ac = actionCollection();

    mBackupAction = ac->addAction(QStringLiteral("backup"), this, SLOT(slotBackupData()));
    mBackupAction->setText(i18n("Back Up Data..."));
    mBackupAction->setEnabled(canZipFile);

    mRestoreAction = ac->addAction(QStringLiteral("restore"), this, SLOT(slotRestoreData()));
    mRestoreAction->setText(i18n("Restore Data..."));
    mRestoreAction->setEnabled(canZipFile);

    mSaveLogAction = ac->addAction(QStringLiteral("save_log"), this, SLOT(slotSaveLog()));
    mSaveLogAction->setText(i18n("Save log..."));

    mArchiveStructureInfo = ac->addAction(QStringLiteral("show_structure_info"), this, SLOT(slotShowStructureInfos()));
    mArchiveStructureInfo->setText(i18n("Show Archive Structure Information..."));

    mShowArchiveInformationsAction = ac->addAction(QStringLiteral("show_archive_info"), this, SLOT(slotShowArchiveInformations()));
    mShowArchiveInformationsAction->setText(i18n("Show Archive Information..."));

    mShowArchiveInformationsAboutCurrentArchiveAction = ac->addAction(QStringLiteral("show_current_archive_info"), this, SLOT(slotShowCurrentArchiveInformations()));
    mShowArchiveInformationsAboutCurrentArchiveAction->setText(i18n("Show Information on current Archive..."));
    mShowArchiveInformationsAboutCurrentArchiveAction->setEnabled(false);

    KStandardAction::quit(this, SLOT(close()), ac);
    mRecentFilesAction = KStandardAction::openRecent(this, SLOT(slotRestoreFile(QUrl)), ac);

    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup groupConfig = config->group(QStringLiteral("Recent File"));
    mRecentFilesAction->loadEntries(groupConfig);
}

void PimSettingExporterWindow::slotUpdateActions(bool inAction)
{
    mBackupAction->setEnabled(!inAction);
    mRestoreAction->setEnabled(!inAction);
    mSaveLogAction->setEnabled(!inAction);
    mArchiveStructureInfo->setEnabled(!inAction);
    mShowArchiveInformationsAction->setEnabled(!inAction);
    mShowArchiveInformationsAboutCurrentArchiveAction->setEnabled(!inAction && !mLastArchiveFileName.isEmpty());
}

void PimSettingExporterWindow::slotRestoreFile(const QUrl &url)
{
    if (!url.isEmpty()) {
        loadData(url.path());
    }
}

void PimSettingExporterWindow::slotShowArchiveInformations()
{
    const QString filename = QFileDialog::getOpenFileName(this, i18n("Select Archive"), QString(), i18n("Zip file (*.zip)"));
    if (filename.isEmpty()) {
        return;
    }

    QPointer<ShowArchiveStructureDialog> dlg = new ShowArchiveStructureDialog(filename, this);
    dlg->exec();
    delete dlg;
}

void PimSettingExporterWindow::slotSaveLog()
{
    if (mLogWidget->isEmpty()) {
        KMessageBox::information(this, i18n("Log is empty."), i18n("Save log"));
        return;
    }
    const QString log = mLogWidget->toHtml();
    const QString filter(i18n("HTML Files (*.html)"));
    PimCommon::Util::saveTextAs(log, filter, this);
}

void PimSettingExporterWindow::slotBackupData()
{
    if (KMessageBox::warningYesNo(this, i18n("Before to backup data, close all kdepim applications. Do you want to continue?"), i18n("Backup")) == KMessageBox::No) {
        return;
    }

    const QString filename = QFileDialog::getSaveFileName(this, i18n("Create backup"), QString(), i18n("Zip file (*.zip)"));
    if (filename.isEmpty()) {
        return;
    }
    mRecentFilesAction->addUrl(QUrl::fromLocalFile(filename));
    backupData(filename);
}

void PimSettingExporterWindow::backupData(const QString &filename, const QString &templateFile)
{
    QPointer<SelectionTypeDialog> dialog = new SelectionTypeDialog(this);
    dialog->loadTemplate(templateFile);
    if (dialog->exec()) {
        mLogWidget->clear();
        initializeBackupRestoreUi();
        mPimSettingsBackupRestoreUI->setStoredParameters(dialog->storedType());
        delete dialog;
        if (!mPimSettingsBackupRestoreUI->backupStart(filename)) {
            qCDebug(PIMSETTINGEXPORTERGUI_LOG) << " backup Start failed";
        }
        mLastArchiveFileName = filename;
    } else {
        delete dialog;
    }
}

void PimSettingExporterWindow::slotAddInfo(const QString &info)
{
    mLogWidget->addInfoLogEntry(info);
    qApp->processEvents();
}

void PimSettingExporterWindow::slotAddError(const QString &info)
{
    mLogWidget->addErrorLogEntry(info);
    qApp->processEvents();
}

void PimSettingExporterWindow::slotAddTitle(const QString &info)
{
    mLogWidget->addTitleLogEntry(info);
    qApp->processEvents();
}

void PimSettingExporterWindow::slotAddEndLine()
{
    mLogWidget->addEndLineLogEntry();
    qApp->processEvents();
}

void PimSettingExporterWindow::slotRestoreData()
{
    const QString filename = QFileDialog::getOpenFileName(this, i18n("Restore backup"), QString(), i18n("Zip File (*.zip)"));
    if (filename.isEmpty()) {
        return;
    }
    loadData(filename);
}

void PimSettingExporterWindow::loadData(const QString &filename, const QString &templateFile)
{
    if (KMessageBox::warningYesNo(this, i18n("Before to restore data, close all kdepim applications. Do you want to continue?"), i18n("Backup")) == KMessageBox::No) {
        return;
    }
    QPointer<SelectionTypeDialog> dialog = new SelectionTypeDialog(this);
    dialog->loadTemplate(templateFile);
    if (dialog->exec()) {
        mLogWidget->clear();
        mPimSettingsBackupRestoreUI->setStoredParameters(dialog->storedType());
        delete dialog;
        if (!mPimSettingsBackupRestoreUI->restoreStart(filename)) {
            qCDebug(PIMSETTINGEXPORTERGUI_LOG) << " PimSettingExporterWindow restore failed";
        }
    } else {
        delete dialog;
    }
}

void PimSettingExporterWindow::slotShowStructureInfos()
{
    QPointer<BackupFileStructureInfoDialog> dlg = new BackupFileStructureInfoDialog(this);
    dlg->exec();
    delete dlg;
}


void PimSettingExporterWindow::slotShowCurrentArchiveInformations()
{
    if (!mLastArchiveFileName.isEmpty()) {
        QPointer<ShowArchiveStructureDialog> dlg = new ShowArchiveStructureDialog(mLastArchiveFileName, this);
        dlg->exec();
        delete dlg;
    }
}
