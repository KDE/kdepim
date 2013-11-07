/*
  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>

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
#include "logwidget.h"

#include "mail/exportmailjob.h"
#include "mail/importmailjob.h"

#include "addressbook/exportaddressbookjob.h"
#include "addressbook/importaddressbookjob.h"

#include "calendar/exportcalendarjob.h"
#include "calendar/importcalendarjob.h"

#include "alarm/exportalarmjob.h"
#include "alarm/importalarmjob.h"

#include "jot/exportjotjob.h"
#include "jot/importjotjob.h"

#include "notes/exportnotesjob.h"
#include "notes/importnotesjob.h"

#include "akregator/exportakregatorjob.h"
#include "akregator/importakregatorjob.h"

#include "blogilo/exportblogilojob.h"
#include "blogilo/importblogilojob.h"

#include "knode/exportknodejob.h"
#include "knode/importknodejob.h"

#include "pimsettingexporterkernel.h"
#include "selectiontypedialog.h"
#include "utils.h"
#include "archivestorage.h"

#include "backupfilestructureinfodialog.h"

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
#include <KLocale>
#include <KStatusBar>
#include <KRecentFilesAction>
#include <QPointer>


PimSettingExporterWindow::PimSettingExporterWindow(QWidget *parent)
    : KXmlGuiWindow(parent),
      mImportExportData(0)
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
}

PimSettingExporterWindow::~PimSettingExporterWindow()
{
    delete mImportExportData;
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup groupConfig = config->group( QLatin1String("Recent File") );
    mRecentFilesAction->saveEntries(groupConfig);
}

void PimSettingExporterWindow::setupActions(bool canZipFile)
{
    KActionCollection* ac=actionCollection();

    KAction *backupAction = ac->addAction(QLatin1String("backup"), this, SLOT(slotBackupData()));
    backupAction->setText(i18n("Back Up Data..."));
    backupAction->setEnabled(canZipFile);

    KAction *restoreAction = ac->addAction(QLatin1String("restore"), this, SLOT(slotRestoreData()));
    restoreAction->setText(i18n("Restore Data..."));
    restoreAction->setEnabled(canZipFile);

    KAction *saveLogAction = ac->addAction(QLatin1String("save_log"), this, SLOT(slotSaveLog()));
    saveLogAction->setText(i18n("Save log..."));

    KAction *archiveStructureInfo = ac->addAction(QLatin1String("show_structure_info"), this, SLOT(slotShowStructureInfos()));
    archiveStructureInfo->setText(i18n("Show Archive Structure Information..."));

    KStandardAction::quit( this, SLOT(close()), ac );
    mRecentFilesAction = KStandardAction::openRecent(this, SLOT(slotRestoreFile(KUrl)), ac);

    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup groupConfig = config->group( QLatin1String("Recent File") );
    mRecentFilesAction->loadEntries(groupConfig);
}

void PimSettingExporterWindow::slotRestoreFile(const KUrl &url)
{
    if (!url.isEmpty()) {
        loadData(url.path());
    }
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

    QPointer<SelectionTypeDialog> dialog = new SelectionTypeDialog(this);
    if (dialog->exec()) {
        const QHash<Utils::AppsType, Utils::importExportParameters> stored = dialog->storedType();

        delete dialog;
        mLogWidget->clear();

        if (stored.isEmpty())
            return;

        ArchiveStorage *archiveStorage = new ArchiveStorage(filename,this);
        if (!archiveStorage->openArchive(true)) {
            delete archiveStorage;
            return;
        }

        slotAddInfo(i18n("Start to backup data in \'%1\'", filename));
        slotAddEndLine();
        //Add version
        Utils::addVersion(archiveStorage->archive());
        QHash<Utils::AppsType, Utils::importExportParameters>::const_iterator i = stored.constBegin();
        while (i != stored.constEnd()) {
            switch(i.key()) {
            case Utils::KMail:
                if (i.value().numberSteps != 0) {
                    mImportExportData = new ExportMailJob(this, i.value().types, archiveStorage, i.value().numberSteps);
                    executeJob();
                }
                break;
            case Utils::KAddressBook:
                if (i.value().numberSteps != 0) {
                    mImportExportData = new ExportAddressbookJob(this, i.value().types, archiveStorage, i.value().numberSteps);
                    executeJob();
                }
                break;
            case Utils::KAlarm:
                if (i.value().numberSteps != 0) {
                    mImportExportData = new ExportAlarmJob(this, i.value().types, archiveStorage, i.value().numberSteps);
                    executeJob();
                }
                break;
            case Utils::KOrganizer:
                if (i.value().numberSteps != 0) {
                    mImportExportData = new ExportCalendarJob(this, i.value().types, archiveStorage, i.value().numberSteps);
                    executeJob();
                }
                break;
            case Utils::KJots:
                if (i.value().numberSteps != 0) {
                    mImportExportData = new ExportJotJob(this, i.value().types, archiveStorage, i.value().numberSteps);
                    executeJob();
                }
                break;
            case Utils::KNotes:
                if (i.value().numberSteps != 0) {
                    mImportExportData = new ExportNotesJob(this, i.value().types, archiveStorage, i.value().numberSteps);
                    executeJob();
                }
                break;
            case Utils::Akregator:
                if (i.value().numberSteps != 0) {
                    mImportExportData = new ExportAkregatorJob(this, i.value().types, archiveStorage, i.value().numberSteps);
                    executeJob();
                }
                break;
            case Utils::Blogilo:
                if (i.value().numberSteps != 0) {
                    mImportExportData = new ExportBlogiloJob(this, i.value().types, archiveStorage, i.value().numberSteps);
                    executeJob();
                }
                break;
            case Utils::KNode:
                if (i.value().numberSteps != 0) {
                    mImportExportData = new ExportKnodeJob(this, i.value().types, archiveStorage, i.value().numberSteps);
                    executeJob();
                }
                break;
            }
            ++i;
        }

        slotAddInfo(i18n("Backup in \'%1\' done.", filename));
        //At the end
        archiveStorage->closeArchive();
        delete archiveStorage;
        delete mImportExportData;
        mImportExportData = 0;
        KMessageBox::information(this, i18n("For restoring data, you must use \"pimsettingexporter\". Be careful it can overwrite existing settings, data."), i18n("Backup infos."), QLatin1String("ShowInfoBackupInfos"));
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

void PimSettingExporterWindow::loadData(const QString &filename)
{
    if (KMessageBox::warningYesNo(this,i18n("Before to restore data, close all kdepim applications. Do you want to continue?"),i18n("Backup"))== KMessageBox::No)
        return;
    QPointer<SelectionTypeDialog> dialog = new SelectionTypeDialog(this);
    if (dialog->exec()) {
        const QHash<Utils::AppsType, Utils::importExportParameters> stored = dialog->storedType();

        delete dialog;
        mLogWidget->clear();

        if (stored.isEmpty())
            return;

        ArchiveStorage *archiveStorage = new ArchiveStorage(filename,this);
        if (!archiveStorage->openArchive(false)) {
            delete archiveStorage;
            return;
        }

        const int version = Utils::archiveVersion(archiveStorage->archive());
        qDebug()<<" version "<<version;
        AbstractImportExportJob::setArchiveVersion(version);

        slotAddInfo(i18n("Start to restore data from \'%1\'", filename));
        slotAddEndLine();
        QHash<Utils::AppsType, Utils::importExportParameters>::const_iterator i = stored.constBegin();
        while (i != stored.constEnd()) {
            switch(i.key()) {
            case Utils::KMail:
                if (i.value().numberSteps != 0) {
                    mImportExportData = new ImportMailJob(this, i.value().types, archiveStorage, i.value().numberSteps);
                    executeJob();
                }
                break;
            case Utils::KAddressBook:
                if (i.value().numberSteps != 0) {
                    mImportExportData = new ImportAddressbookJob(this, i.value().types, archiveStorage, i.value().numberSteps);
                    executeJob();
                }
                break;
            case Utils::KAlarm:
                if (i.value().numberSteps != 0) {
                    mImportExportData = new ImportAlarmJob(this, i.value().types, archiveStorage, i.value().numberSteps);
                    executeJob();
                }
                break;
            case Utils::KOrganizer:
                if (i.value().numberSteps != 0) {
                    mImportExportData = new ImportCalendarJob(this, i.value().types, archiveStorage, i.value().numberSteps);
                    executeJob();
                }
                break;
            case Utils::KJots:
                if (i.value().numberSteps != 0) {
                    mImportExportData = new ImportJotJob(this, i.value().types, archiveStorage, i.value().numberSteps);
                    executeJob();
                }
                break;
            case Utils::KNotes:
                if (i.value().numberSteps != 0) {
                    mImportExportData = new ImportNotesJob(this, i.value().types, archiveStorage, i.value().numberSteps);
                    executeJob();
                }
                break;
            case Utils::Akregator:
                if (i.value().numberSteps != 0) {
                    mImportExportData = new ImportAkregatorJob(this, i.value().types, archiveStorage, i.value().numberSteps);
                    executeJob();
                }
                break;
            case Utils::Blogilo:
                if (i.value().numberSteps != 0) {
                    mImportExportData = new ImportBlogiloJob(this, i.value().types, archiveStorage, i.value().numberSteps);
                    executeJob();
                }
                break;
            case Utils::KNode:
                if (i.value().numberSteps != 0) {
                    mImportExportData = new ImportKnodeJob(this, i.value().types, archiveStorage, i.value().numberSteps);
                    executeJob();
                }
                break;
            }
            ++i;
        }

        slotAddInfo(i18n("Restoring data from \'%1\' done.", filename));
        //At the end
        archiveStorage->closeArchive();
        delete archiveStorage;
        delete mImportExportData;
        mImportExportData = 0;
    } else {
        delete dialog;
    }
}

void PimSettingExporterWindow::executeJob()
{
    connect(mImportExportData, SIGNAL(info(QString)), SLOT(slotAddInfo(QString)));
    connect(mImportExportData, SIGNAL(error(QString)), SLOT(slotAddError(QString)));
    connect(mImportExportData, SIGNAL(title(QString)), SLOT(slotAddTitle(QString)));
    connect(mImportExportData, SIGNAL(endLine()), SLOT(slotAddEndLine()));
    mImportExportData->start();
    slotAddEndLine();
    delete mImportExportData;
    mImportExportData = 0;
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

