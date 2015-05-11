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

#include "pimsettingexporterkernel.h"
#include "dialog/selectiontypedialog.h"
#include "utils.h"
#include "pimsettingsbackuprestoreui.h"

#include "dialog/backupfilestructureinfodialog.h"

#include <mailcommon/kernel/mailkernel.h>
#include <mailcommon/filter/filtermanager.h>

#include "pimcommon/util/pimutil.h"


#include <Akonadi/Control>

#include <KStandardAction>
#include <KAction>
#include <KActionCollection>
#include <KFileDialog>
#include <KMessageBox>
#include <KStandardDirs>
#include <KLocalizedString>
#include <KStatusBar>
#include <KRecentFilesAction>
#include <KCmdLineArgs>

#include <QPointer>

PimSettingExporterWindow::PimSettingExporterWindow(QWidget *parent)
    : KXmlGuiWindow(parent),
      mBackupAction(0),
      mRestoreAction(0),
      mSaveLogAction(0),
      mArchiveStructureInfo(0),
      mShowArchiveInformationsAction(0),
      mPimSettingsBackupRestoreUI(0)
{
    KGlobal::locale()->insertCatalog( QLatin1String("libmailcommon") );
    KGlobal::locale()->insertCatalog( QLatin1String("libpimcommon") );
    //Initialize filtermanager
    (void)MailCommon::FilterManager::instance();
    PimSettingExporterKernel *kernel = new PimSettingExporterKernel( this );
    CommonKernel->registerKernelIf( kernel ); //register KernelIf early, it is used by the Filter classes
    CommonKernel->registerSettingsIf( kernel ); //SettingsIf is used in FolderTreeWidget

    bool canZipFile = canZip();
    setupActions(canZipFile);
    setupGUI(Keys | StatusBar | Save | Create, QLatin1String("pimsettingexporter.rc"));
    mLogWidget = new LogWidget(this);

    setCentralWidget(mLogWidget);
    resize( 800, 600 );
    Akonadi::Control::widgetNeedsAkonadi(this);
    if (!canZipFile) {
        KMessageBox::error(this,i18n("Zip program not found. Install it before to launch this application."),i18n("Zip program not found."));
    }
    statusBar()->hide();
    initializeBackupRestoreUi();
}

PimSettingExporterWindow::~PimSettingExporterWindow()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup groupConfig = config->group( QLatin1String("Recent File") );
    mRecentFilesAction->saveEntries(groupConfig);
}

void PimSettingExporterWindow::initializeBackupRestoreUi()
{
    mPimSettingsBackupRestoreUI = new PimSettingsBackupRestoreUI(this, this);
    connect(mPimSettingsBackupRestoreUI, SIGNAL(addInfo(QString)), this, SLOT(slotAddInfo(QString)));
    connect(mPimSettingsBackupRestoreUI, SIGNAL(addEndLine()), this, SLOT(slotAddEndLine()));
    connect(mPimSettingsBackupRestoreUI, SIGNAL(addError(QString)), this, SLOT(slotAddError(QString)));
    connect(mPimSettingsBackupRestoreUI, SIGNAL(addTitle(QString)), this, SLOT(slotAddTitle(QString)));
    connect(mPimSettingsBackupRestoreUI, SIGNAL(updateActions(bool)), this, SLOT(slotUpdateActions(bool)));
    connect(mPimSettingsBackupRestoreUI, SIGNAL(jobFinished()), this, SLOT(slotJobFinished()));
    connect(mPimSettingsBackupRestoreUI, SIGNAL(backupDone()), this, SLOT(slotShowBackupFinishDialogInformation()));
    connect(mPimSettingsBackupRestoreUI, SIGNAL(jobFailed()), this, SLOT(slotJobFailed()));
}

void PimSettingExporterWindow::slotJobFailed()
{
    //Nothing
}

void PimSettingExporterWindow::slotShowBackupFinishDialogInformation()
{
    KMessageBox::information(this, i18n("For restoring data, you must use \"pimsettingexporter\". Be careful it can overwrite existing settings, data."), i18n("Backup infos."), QLatin1String("ShowInfoBackupInfos"));
}

void PimSettingExporterWindow::handleCommandLine()
{
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    QString templateFile;
    if (args->isSet("template")) {
        templateFile = args->getOption("template");
    }
    if ( args->isSet( "import" ) ) {
        if (args->count() > 0) {
            loadData(args->url(0).path(), templateFile);
        }
    } else if (args->isSet( "export" )) {
        if (args->count() > 0) {
            backupData(args->url(0).path(), templateFile);
        }
    }
    args->clear();
}

void PimSettingExporterWindow::setupActions(bool canZipFile)
{
    KActionCollection* ac=actionCollection();

    mBackupAction = ac->addAction(QLatin1String("backup"), this, SLOT(slotBackupData()));
    mBackupAction->setText(i18n("Back Up Data..."));
    mBackupAction->setEnabled(canZipFile);

    mRestoreAction = ac->addAction(QLatin1String("restore"), this, SLOT(slotRestoreData()));
    mRestoreAction->setText(i18n("Restore Data..."));
    mRestoreAction->setEnabled(canZipFile);

    mSaveLogAction = ac->addAction(QLatin1String("save_log"), this, SLOT(slotSaveLog()));
    mSaveLogAction->setText(i18n("Save log..."));

    mArchiveStructureInfo = ac->addAction(QLatin1String("show_structure_info"), this, SLOT(slotShowStructureInfos()));
    mArchiveStructureInfo->setText(i18n("Show Archive Structure Information..."));

    mShowArchiveInformationsAction = ac->addAction(QLatin1String("show_archive_info"), this, SLOT(slotShowArchiveInformations()));
    mShowArchiveInformationsAction->setText(i18n("Show Archive Information..."));


    KStandardAction::quit( this, SLOT(close()), ac );
    mRecentFilesAction = KStandardAction::openRecent(this, SLOT(slotRestoreFile(KUrl)), ac);

    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup groupConfig = config->group( QLatin1String("Recent File") );
    mRecentFilesAction->loadEntries(groupConfig);
}

void PimSettingExporterWindow::slotUpdateActions(bool inAction)
{
    mBackupAction->setEnabled(!inAction);
    mRestoreAction->setEnabled(!inAction);
    mSaveLogAction->setEnabled(!inAction);
    mArchiveStructureInfo->setEnabled(!inAction);
    mShowArchiveInformationsAction->setEnabled(!inAction);
}

void PimSettingExporterWindow::slotRestoreFile(const KUrl &url)
{
    if (!url.isEmpty()) {
        loadData(url.path());
    }
}

void PimSettingExporterWindow::slotShowArchiveInformations()
{
    const QString filename = KFileDialog::getOpenFileName(KUrl("kfiledialog:///pimsettingexporter"), QLatin1String("*.zip"), this, i18n("Select Archive"));
    if (filename.isEmpty())
        return;

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
    const QString filter(QLatin1String("*.html"));
    PimCommon::Util::saveTextAs(log, filter, this);
}

void PimSettingExporterWindow::slotBackupData()
{
    if (KMessageBox::warningYesNo(this,i18n("Before to backup data, close all kdepim applications. Do you want to continue?"),i18n("Backup"))== KMessageBox::No)
        return;

    const QString filename = KFileDialog::getSaveFileName(KUrl("kfiledialog:///pimsettingexporter"),QLatin1String("*.zip"),this,i18n("Create backup"),KFileDialog::ConfirmOverwrite);
    if (filename.isEmpty())
        return;
    mRecentFilesAction->addUrl(KUrl(filename));
    backupData(filename);
}

void PimSettingExporterWindow::backupData(const QString &filename, const QString &templateFile)
{
    QPointer<SelectionTypeDialog> dialog = new SelectionTypeDialog(this);
    dialog->loadTemplate(templateFile);
    if (dialog->exec()) {
        mLogWidget->clear();
        mPimSettingsBackupRestoreUI->setStoredParameters(dialog->storedType());
        delete dialog;
        if (!mPimSettingsBackupRestoreUI->backupStart(filename) ) {
            qDebug() << " backup Start failed";
        }
    } else {
        delete dialog;
    }
}


void PimSettingExporterWindow::slotAddInfo(const QString& info)
{
    mLogWidget->addInfoLogEntry(info);
}

void PimSettingExporterWindow::slotAddError(const QString& info)
{
    mLogWidget->addErrorLogEntry(info);
}

void PimSettingExporterWindow::slotAddTitle(const QString &info)
{
    mLogWidget->addTitleLogEntry(info);
}

void PimSettingExporterWindow::slotAddEndLine()
{
    mLogWidget->addEndLineLogEntry();
}

void PimSettingExporterWindow::slotRestoreData()
{
    const QString filename = KFileDialog::getOpenFileName(KUrl("kfiledialog:///pimsettingexporter"), QLatin1String("*.zip"), this, i18n("Restore backup"));
    if (filename.isEmpty())
        return;
    loadData(filename);
}

void PimSettingExporterWindow::loadData(const QString &filename, const QString &templateFile)
{
    if (KMessageBox::warningYesNo(this,i18n("Before to restore data, close all kdepim applications. Do you want to continue?"),i18n("Backup"))== KMessageBox::No)
        return;
    QPointer<SelectionTypeDialog> dialog = new SelectionTypeDialog(this);
    dialog->loadTemplate(templateFile);
    if (dialog->exec()) {
        mLogWidget->clear();
        mPimSettingsBackupRestoreUI->setStoredParameters(dialog->storedType());
        delete dialog;
        if (!mPimSettingsBackupRestoreUI->restoreStart(filename)) {
            qDebug()<<" PimSettingExporterWindow restore failed";
        }
    } else {
        delete dialog;
    }
}

bool PimSettingExporterWindow::canZip() const
{
    const QString zip = KStandardDirs::findExe( QLatin1String("zip") );
    if (zip.isEmpty()) {
        return false;
    }
    return true;
}

void PimSettingExporterWindow::slotShowStructureInfos()
{
    QPointer<BackupFileStructureInfoDialog> dlg = new BackupFileStructureInfoDialog(this);
    dlg->exec();
    delete dlg;
}

