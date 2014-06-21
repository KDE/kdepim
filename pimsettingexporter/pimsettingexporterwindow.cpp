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
#include "dialog/showarchivestructuredialog.h"
#include "widgets/logwidget.h"

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

#include "pimsettingexporterkernel.h"
#include "dialog/selectiontypedialog.h"
#include "utils.h"
#include "archivestorage.h"

#include "dialog/backupfilestructureinfodialog.h"

#include <mailcommon/kernel/mailkernel.h>
#include <mailcommon/filter/filtermanager.h>

#include "pimcommon/util/pimutil.h"


#include <AkonadiCore/Control>

#include <KStandardAction>
#include <QAction>
#include <KActionCollection>
#include <KFileDialog>
#include <KMessageBox>
#include <KStandardDirs>
#include <KLocalizedString>
#include <KStatusBar>
#include <KRecentFilesAction>
#include <KCmdLineArgs>
#include <KGlobal>
#include <QPointer>
#include <KSharedConfig>
#include <KLocale>

PimSettingExporterWindow::PimSettingExporterWindow(QWidget *parent)
    : KXmlGuiWindow(parent),
      mAction(Backup),
      mImportExportData(0),
      mArchiveStorage(0),
      mBackupAction(0),
      mRestoreAction(0),
      mSaveLogAction(0),
      mArchiveStructureInfo(0),
      mShowArchiveInformationsAction(0)
{
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
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup groupConfig = config->group( QLatin1String("Recent File") );
    mRecentFilesAction->saveEntries(groupConfig);
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
    mRecentFilesAction = KStandardAction::openRecent(this, SLOT(slotRestoreFile(QUrl)), ac);

    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup groupConfig = config->group( QLatin1String("Recent File") );
    mRecentFilesAction->loadEntries(groupConfig);
}

void PimSettingExporterWindow::updateActions(bool inAction)
{
    mBackupAction->setEnabled(!inAction);
    mRestoreAction->setEnabled(!inAction);
    mSaveLogAction->setEnabled(!inAction);
    mArchiveStructureInfo->setEnabled(!inAction);
    mShowArchiveInformationsAction->setEnabled(!inAction);
}

void PimSettingExporterWindow::slotRestoreFile(const QUrl &url)
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
        mStored = dialog->storedType();

        delete dialog;
        mLogWidget->clear();

        if (mStored.isEmpty())
            return;

        mArchiveStorage = new ArchiveStorage(filename,this);
        if (!mArchiveStorage->openArchive(true)) {
            delete mArchiveStorage;
            mArchiveStorage = 0;
            return;
        }

        backupStart();
    } else {
        delete dialog;
    }
}

void PimSettingExporterWindow::backupStart()
{
    updateActions(true);
    mAction = Backup;
    mStoreIterator = mStored.constBegin();
    const QDateTime now = QDateTime::currentDateTime();
    slotAddInfo(QLatin1Char('[') + KLocale::global()->formatDateTime( now ) + QLatin1Char(']'));
    slotAddInfo(i18n("Start to backup data in \'%1\'", mArchiveStorage->filename()));
    slotAddEndLine();
    //Add version
    Utils::addVersion(mArchiveStorage->archive());
    backupNextStep();
}

void PimSettingExporterWindow::backupNextStep()
{
    if (mStoreIterator != mStored.constEnd()) {
        switch(mStoreIterator.key()) {
        case Utils::KMail:
            if (mStoreIterator.value().numberSteps != 0) {
                mImportExportData = new ExportMailJob(this, mStoreIterator.value().types, mArchiveStorage, mStoreIterator.value().numberSteps);
                executeJob();
            }
            break;
        case Utils::KAddressBook:
            if (mStoreIterator.value().numberSteps != 0) {
                mImportExportData = new ExportAddressbookJob(this, mStoreIterator.value().types, mArchiveStorage, mStoreIterator.value().numberSteps);
                executeJob();
            }
            break;
        case Utils::KAlarm:
            if (mStoreIterator.value().numberSteps != 0) {
                mImportExportData = new ExportAlarmJob(this, mStoreIterator.value().types, mArchiveStorage, mStoreIterator.value().numberSteps);
                executeJob();
            }
            break;
        case Utils::KOrganizer:
            if (mStoreIterator.value().numberSteps != 0) {
                mImportExportData = new ExportCalendarJob(this, mStoreIterator.value().types, mArchiveStorage, mStoreIterator.value().numberSteps);
                executeJob();
            }
            break;
        case Utils::KJots:
            if (mStoreIterator.value().numberSteps != 0) {
                mImportExportData = new ExportJotJob(this, mStoreIterator.value().types, mArchiveStorage, mStoreIterator.value().numberSteps);
                executeJob();
            }
            break;
        case Utils::KNotes:
            if (mStoreIterator.value().numberSteps != 0) {
                mImportExportData = new ExportNotesJob(this, mStoreIterator.value().types, mArchiveStorage, mStoreIterator.value().numberSteps);
                executeJob();
            }
            break;
        case Utils::Akregator:
            if (mStoreIterator.value().numberSteps != 0) {
                mImportExportData = new ExportAkregatorJob(this, mStoreIterator.value().types, mArchiveStorage, mStoreIterator.value().numberSteps);
                executeJob();
            }
            break;
        case Utils::Blogilo:
            if (mStoreIterator.value().numberSteps != 0) {
                mImportExportData = new ExportBlogiloJob(this, mStoreIterator.value().types, mArchiveStorage, mStoreIterator.value().numberSteps);
                executeJob();
            }
            break;
        case Utils::Unknown:
            break;
        }
    } else {
        backupFinished();
    }
}

void PimSettingExporterWindow::backupFinished()
{
    slotAddInfo(i18n("Backup in \'%1\' done.", mArchiveStorage->filename()));
    //At the end
    mArchiveStorage->closeArchive();
    delete mArchiveStorage;
    mArchiveStorage = 0;
    delete mImportExportData;
    mImportExportData = 0;
    KMessageBox::information(this, i18n("For restoring data, you must use \"pimsettingexporter\". Be careful it can overwrite existing settings, data."), i18n("Backup infos."), QLatin1String("ShowInfoBackupInfos"));
    updateActions(false);
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
        mStored = dialog->storedType();

        delete dialog;

        if (mStored.isEmpty())
            return;

        mArchiveStorage = new ArchiveStorage(filename,this);
        if (!mArchiveStorage->openArchive(false)) {
            delete mArchiveStorage;
            mArchiveStorage = 0;
            return;
        }

        restoreStart();
    } else {
        delete dialog;
    }
}

void PimSettingExporterWindow::restoreNextStep()
{
    if (mStoreIterator != mStored.constEnd()) {
        switch(mStoreIterator.key()) {
        case Utils::KMail:
            if (mStoreIterator.value().numberSteps != 0) {
                mImportExportData = new ImportMailJob(this, mStoreIterator.value().types, mArchiveStorage, mStoreIterator.value().numberSteps);
                executeJob();
            }
            break;
        case Utils::KAddressBook:
            if (mStoreIterator.value().numberSteps != 0) {
                mImportExportData = new ImportAddressbookJob(this, mStoreIterator.value().types, mArchiveStorage, mStoreIterator.value().numberSteps);
                executeJob();
            }
            break;
        case Utils::KAlarm:
            if (mStoreIterator.value().numberSteps != 0) {
                mImportExportData = new ImportAlarmJob(this, mStoreIterator.value().types, mArchiveStorage, mStoreIterator.value().numberSteps);
                executeJob();
            }
            break;
        case Utils::KOrganizer:
            if (mStoreIterator.value().numberSteps != 0) {
                mImportExportData = new ImportCalendarJob(this, mStoreIterator.value().types, mArchiveStorage, mStoreIterator.value().numberSteps);
                executeJob();
            }
            break;
        case Utils::KJots:
            if (mStoreIterator.value().numberSteps != 0) {
                mImportExportData = new ImportJotJob(this, mStoreIterator.value().types, mArchiveStorage, mStoreIterator.value().numberSteps);
                executeJob();
            }
            break;
        case Utils::KNotes:
            if (mStoreIterator.value().numberSteps != 0) {
                mImportExportData = new ImportNotesJob(this, mStoreIterator.value().types, mArchiveStorage, mStoreIterator.value().numberSteps);
                executeJob();
            }
            break;
        case Utils::Akregator:
            if (mStoreIterator.value().numberSteps != 0) {
                mImportExportData = new ImportAkregatorJob(this, mStoreIterator.value().types, mArchiveStorage, mStoreIterator.value().numberSteps);
                executeJob();
            }
            break;
        case Utils::Blogilo:
            if (mStoreIterator.value().numberSteps != 0) {
                mImportExportData = new ImportBlogiloJob(this, mStoreIterator.value().types, mArchiveStorage, mStoreIterator.value().numberSteps);
                executeJob();
            }
            break;
        case Utils::Unknown:
	    break;
        }
    } else {
        restoreFinished();
    }
}

void PimSettingExporterWindow::restoreStart()
{
    updateActions(true);
    mAction = Restore;
    mStoreIterator = mStored.constBegin();
    const int version = Utils::archiveVersion(mArchiveStorage->archive());
    if (version > Utils::currentArchiveVersion()) {
        if (KMessageBox::No == KMessageBox::questionYesNo(this, i18n("The archive was created by a newer version of this program. It might contain additional data which will be skipped during import. Do you want to import it?"), i18n("Not correct version")))
            return;
    }
    qDebug()<<" version "<<version;
    AbstractImportExportJob::setArchiveVersion(version);

    const QDateTime now = QDateTime::currentDateTime();
    slotAddInfo(QLatin1Char('[') + KLocale::global()->formatDateTime( now ) + QLatin1Char(']'));

    slotAddInfo(i18n("Start to restore data from \'%1\'", mArchiveStorage->filename()));
    slotAddEndLine();
    restoreNextStep();
}

void PimSettingExporterWindow::restoreFinished()
{
    slotAddInfo(i18n("Restoring data from \'%1\' done.", mArchiveStorage->filename()));
    //At the end
    mArchiveStorage->closeArchive();
    delete mArchiveStorage;
    mArchiveStorage = 0;
    delete mImportExportData;
    mImportExportData = 0;
    updateActions(false);
}

void PimSettingExporterWindow::executeJob()
{
    connect(mImportExportData, SIGNAL(info(QString)), SLOT(slotAddInfo(QString)));
    connect(mImportExportData, SIGNAL(error(QString)), SLOT(slotAddError(QString)));
    connect(mImportExportData, SIGNAL(title(QString)), SLOT(slotAddTitle(QString)));
    connect(mImportExportData, SIGNAL(endLine()), SLOT(slotAddEndLine()));
    connect(mImportExportData, SIGNAL(jobFinished()), SLOT(slotJobFinished()));
    mImportExportData->start();
}

void PimSettingExporterWindow::slotJobFinished()
{
    ++mStoreIterator;
    slotAddEndLine();
    delete mImportExportData;
    mImportExportData = 0;
    switch(mAction) {
    case Backup:
        backupNextStep();
        break;
    case Restore:
        restoreNextStep();
        break;
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

