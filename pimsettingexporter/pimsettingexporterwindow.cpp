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

#include "pimsettingexporterkernel.h"
#include "selectiontypedialog.h"
#include "utils.h"
#include "archivestorage.h"

#include <mailcommon/kernel/mailkernel.h>

#include <Akonadi/Control>

#include <KStandardAction>
#include <KAction>
#include <KActionCollection>
#include <KFileDialog>
#include <KMessageBox>
#include <KStandardDirs>
#include <KLocale>

#include <QPointer>

PimSettingExporterWindow::PimSettingExporterWindow(QWidget *parent)
    : KXmlGuiWindow(parent),
      mBackupData(0),
      mRestoreData(0)
{
    KGlobal::locale()->insertCatalog( QLatin1String("libmailcommon") );
    KGlobal::locale()->insertCatalog( QLatin1String("libpimcommon") );

    PimSettingExporterKernel *kernel = new PimSettingExporterKernel( this );
    CommonKernel->registerKernelIf( kernel ); //register KernelIf early, it is used by the Filter classes
    CommonKernel->registerSettingsIf( kernel ); //SettingsIf is used in FolderTreeWidget

    bool canZipFile = canZip();
    setupActions(canZipFile);
    setupGUI(Keys | StatusBar | Save | Create, QLatin1String("pimsettingexporter.rc"));
    mLogWidget = new LogWidget(this);

    setCentralWidget(mLogWidget);
    resize( 640, 480 );
    Akonadi::Control::widgetNeedsAkonadi(this);
    if (!canZipFile) {
        KMessageBox::error(this,i18n("Zip program not found. Install it before to launch this application."),i18n("Zip program not found."));
    }
}

PimSettingExporterWindow::~PimSettingExporterWindow()
{
    delete mRestoreData;
    delete mBackupData;
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

    KStandardAction::quit( this, SLOT(close()), ac );
}

void PimSettingExporterWindow::slotBackupData()
{
    if (KMessageBox::warningYesNo(this,i18n("Before to backup data, close all kdepim applications. Do you want to continue?"),i18n("Backup"))== KMessageBox::No)
        return;

    const QString filename = KFileDialog::getSaveFileName(KUrl("kfiledialog:///pimsettingexporter"),QLatin1String("*.zip"),this,i18n("Create backup"),KFileDialog::ConfirmOverwrite);
    if (filename.isEmpty())
        return;
    QPointer<SelectionTypeDialog> dialog = new SelectionTypeDialog(this);
    if (dialog->exec()) {
        int kmailNumberOfStep = 0;
        int korganizerNumberOfStep = 0;
        int kalarmNumberOfStep = 0;
        int kaddressbookNumberOfStep = 0;
        const Utils::StoredTypes kmailTypeSelected = dialog->kmailTypesSelected(kmailNumberOfStep);
        const Utils::StoredTypes kaddressbookTypeSelected = dialog->kaddressbookTypesSelected(kaddressbookNumberOfStep);
        const Utils::StoredTypes korganizerTypeSelected = dialog->korganizerTypesSelected(korganizerNumberOfStep);
        const Utils::StoredTypes kalarmTypeSelected = dialog->kalarmTypesSelected(kalarmNumberOfStep);
        delete dialog;
        mLogWidget->clear();
        delete mBackupData;

        if ((kmailNumberOfStep==0) && (korganizerNumberOfStep==0) && (kalarmNumberOfStep==0) && (kaddressbookNumberOfStep==0))
            return;

        ArchiveStorage *archiveStorage = new ArchiveStorage(filename,this);
        if (!archiveStorage->openArchive(true)) {
            delete archiveStorage;
            return;
        }

        if (kmailNumberOfStep != 0) {
            mBackupData = new ExportMailJob(this, kmailTypeSelected, archiveStorage, kmailNumberOfStep);
            connect(mBackupData,SIGNAL(info(QString)),SLOT(slotAddInfo(QString)));
            connect(mBackupData,SIGNAL(error(QString)),SLOT(slotAddError(QString)));
            mBackupData->start();
            delete mBackupData;
            mBackupData = 0;
        }

        if (kaddressbookNumberOfStep != 0) {
            mBackupData = new ExportAddressbookJob(this, kaddressbookTypeSelected, archiveStorage, kaddressbookNumberOfStep);
            connect(mBackupData,SIGNAL(info(QString)),SLOT(slotAddInfo(QString)));
            connect(mBackupData,SIGNAL(error(QString)),SLOT(slotAddError(QString)));
            mBackupData->start();
            delete mBackupData;
            mBackupData = 0;
        }

        if (korganizerNumberOfStep != 0) {
            mBackupData = new ExportCalendarJob(this, korganizerTypeSelected, archiveStorage, korganizerNumberOfStep);
            connect(mBackupData,SIGNAL(info(QString)),SLOT(slotAddInfo(QString)));
            connect(mBackupData,SIGNAL(error(QString)),SLOT(slotAddError(QString)));
            mBackupData->start();
            delete mBackupData;
            mBackupData = 0;
        }

        if (kalarmNumberOfStep != 0) {
            mBackupData = new ExportAlarmJob(this, kalarmTypeSelected, archiveStorage, kalarmNumberOfStep);
            connect(mBackupData,SIGNAL(info(QString)),SLOT(slotAddInfo(QString)));
            connect(mBackupData,SIGNAL(error(QString)),SLOT(slotAddError(QString)));
            mBackupData->start();
            delete mBackupData;
            mBackupData = 0;
        }

        //At the end
        archiveStorage->closeArchive();
        delete archiveStorage;
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


void PimSettingExporterWindow::slotRestoreData()
{
    if (KMessageBox::warningYesNo(this,i18n("Before to restore data, close all kdepim applications. Do you want to continue?"),i18n("Backup"))== KMessageBox::No)
        return;
    const QString filename = KFileDialog::getOpenFileName(KUrl("kfiledialog:///pimsettingexporter"), QLatin1String("*.zip"), this, i18n("Restore backup"));
    if (filename.isEmpty())
        return;

    QPointer<SelectionTypeDialog> dialog = new SelectionTypeDialog(this);
    if (dialog->exec()) {
        int kmailNumberOfStep = 0;
        int korganizerNumberOfStep = 0;
        int kalarmNumberOfStep = 0;
        int kaddressbookNumberOfStep = 0;
        const Utils::StoredTypes kmailTypeSelected = dialog->kmailTypesSelected(kmailNumberOfStep);
        const Utils::StoredTypes kaddressbookTypeSelected = dialog->kaddressbookTypesSelected(kaddressbookNumberOfStep);
        const Utils::StoredTypes korganizerTypeSelected = dialog->korganizerTypesSelected(korganizerNumberOfStep);
        const Utils::StoredTypes kalarmTypeSelected = dialog->kalarmTypesSelected(kalarmNumberOfStep);
        delete dialog;
        mLogWidget->clear();
        delete mRestoreData;

        if ((kmailNumberOfStep==0) && (korganizerNumberOfStep==0) && (kalarmNumberOfStep==0) && (kaddressbookNumberOfStep==0))
            return;

        ArchiveStorage *archiveStorage = new ArchiveStorage(filename,this);
        if (!archiveStorage->openArchive(false)) {
            delete archiveStorage;
            return;
        }

        if (kmailNumberOfStep != 0) {
            mRestoreData = new ImportMailJob(this, kmailTypeSelected, archiveStorage, kmailNumberOfStep);
            connect(mRestoreData,SIGNAL(info(QString)),SLOT(slotAddInfo(QString)));
            connect(mRestoreData,SIGNAL(error(QString)),SLOT(slotAddError(QString)));
            mRestoreData->start();
            delete mRestoreData;
            mRestoreData = 0;
        }

        if (kaddressbookNumberOfStep != 0) {
            mRestoreData = new ImportAddressbookJob(this, kaddressbookTypeSelected, archiveStorage, kaddressbookNumberOfStep);
            connect(mRestoreData,SIGNAL(info(QString)),SLOT(slotAddInfo(QString)));
            connect(mRestoreData,SIGNAL(error(QString)),SLOT(slotAddError(QString)));
            mRestoreData->start();
            delete mRestoreData;
            mRestoreData = 0;
        }

        if (korganizerNumberOfStep != 0) {
            mRestoreData = new ImportCalendarJob(this, korganizerTypeSelected, archiveStorage, korganizerNumberOfStep);
            connect(mRestoreData,SIGNAL(info(QString)),SLOT(slotAddInfo(QString)));
            connect(mRestoreData,SIGNAL(error(QString)),SLOT(slotAddError(QString)));
            mRestoreData->start();
            delete mRestoreData;
            mRestoreData = 0;
        }

        if (kalarmNumberOfStep != 0) {
            mRestoreData = new ImportAlarmJob(this, kalarmTypeSelected, archiveStorage, kalarmNumberOfStep);
            connect(mRestoreData,SIGNAL(info(QString)),SLOT(slotAddInfo(QString)));
            connect(mRestoreData,SIGNAL(error(QString)),SLOT(slotAddError(QString)));
            mRestoreData->start();
            delete mRestoreData;
            mRestoreData = 0;
        }

        archiveStorage->closeArchive();
        delete archiveStorage;
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

#include "pimsettingexporterwindow.moc"
