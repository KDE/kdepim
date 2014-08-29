/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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

#include "pimsettingsbackuprestore.h"
#include "archivestorage.h"

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

#include <KLocalizedString>
#include <KMessageBox>

#include <QDebug>
#include <QDateTime>
#include <QLocale>

PimSettingsBackupRestore::PimSettingsBackupRestore(QWidget *parentWidget, QObject *parent)
    : QObject(parent),
      mImportExportData(0),
      mArchiveStorage(0),
      mParentWidget(parentWidget)
{

}

PimSettingsBackupRestore::~PimSettingsBackupRestore()
{
    delete mImportExportData;
}

void PimSettingsBackupRestore::setStoredParameters(const QHash<Utils::AppsType, Utils::importExportParameters> &stored)
{
    mStored = stored;
}

bool PimSettingsBackupRestore::openArchive(const QString &filename, bool readWrite)
{
    mArchiveStorage = new ArchiveStorage(filename,this);
    if (!mArchiveStorage->openArchive(readWrite)) {
        delete mArchiveStorage;
        mArchiveStorage = 0;
        return false;
    }
    return true;
}

void PimSettingsBackupRestore::backupStart(const QString &filename)
{
    if (!openArchive(filename, true)) {
        Q_EMIT jobFailed();
        deleteLater();
        return;
    }
    updateActions(true);
    mAction = Backup;
    mStoreIterator = mStored.constBegin();
    const QDateTime now = QDateTime::currentDateTime();
    Q_EMIT addInfo(QLatin1Char('[') + QLocale().toString(( now ), QLocale::ShortFormat) + QLatin1Char(']'));
    Q_EMIT addInfo(i18n("Start to backup data in \'%1\'", mArchiveStorage->filename()));
    Q_EMIT addEndLine();
    //Add version
    Utils::addVersion(mArchiveStorage->archive());
    backupNextStep();
}

void PimSettingsBackupRestore::backupNextStep()
{
    if (mStoreIterator != mStored.constEnd()) {
        switch(mStoreIterator.key()) {
        case Utils::KMail:
            if (mStoreIterator.value().numberSteps != 0) {
                mImportExportData = new ExportMailJob(mParentWidget, mStoreIterator.value().types, mArchiveStorage, mStoreIterator.value().numberSteps);
                executeJob();
            }
            break;
        case Utils::KAddressBook:
            if (mStoreIterator.value().numberSteps != 0) {
                mImportExportData = new ExportAddressbookJob(mParentWidget, mStoreIterator.value().types, mArchiveStorage, mStoreIterator.value().numberSteps);
                executeJob();
            }
            break;
        case Utils::KAlarm:
            if (mStoreIterator.value().numberSteps != 0) {
                mImportExportData = new ExportAlarmJob(mParentWidget, mStoreIterator.value().types, mArchiveStorage, mStoreIterator.value().numberSteps);
                executeJob();
            }
            break;
        case Utils::KOrganizer:
            if (mStoreIterator.value().numberSteps != 0) {
                mImportExportData = new ExportCalendarJob(mParentWidget, mStoreIterator.value().types, mArchiveStorage, mStoreIterator.value().numberSteps);
                executeJob();
            }
            break;
        case Utils::KJots:
            if (mStoreIterator.value().numberSteps != 0) {
                mImportExportData = new ExportJotJob(mParentWidget, mStoreIterator.value().types, mArchiveStorage, mStoreIterator.value().numberSteps);
                executeJob();
            }
            break;
        case Utils::KNotes:
            if (mStoreIterator.value().numberSteps != 0) {
                mImportExportData = new ExportNotesJob(mParentWidget, mStoreIterator.value().types, mArchiveStorage, mStoreIterator.value().numberSteps);
                executeJob();
            }
            break;
        case Utils::Akregator:
            if (mStoreIterator.value().numberSteps != 0) {
                mImportExportData = new ExportAkregatorJob(mParentWidget, mStoreIterator.value().types, mArchiveStorage, mStoreIterator.value().numberSteps);
                executeJob();
            }
            break;
        case Utils::Blogilo:
            if (mStoreIterator.value().numberSteps != 0) {
                mImportExportData = new ExportBlogiloJob(mParentWidget, mStoreIterator.value().types, mArchiveStorage, mStoreIterator.value().numberSteps);
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

void PimSettingsBackupRestore::backupFinished()
{
    Q_EMIT addInfo(i18n("Backup in \'%1\' done.", mArchiveStorage->filename()));
    //At the end
    mArchiveStorage->closeArchive();
    delete mArchiveStorage;
    mArchiveStorage = 0;
    delete mImportExportData;
    mImportExportData = 0;
    Q_EMIT backupDone();
    //KMessageBox::information(this, i18n("For restoring data, you must use \"pimsettingexporter\". Be careful it can overwrite existing settings, data."), i18n("Backup infos."), QLatin1String("ShowInfoBackupInfos"));
    Q_EMIT updateActions(false);
    deleteLater();
}

void PimSettingsBackupRestore::restoreNextStep()
{
    if (mStoreIterator != mStored.constEnd()) {
        switch(mStoreIterator.key()) {
        case Utils::KMail:
            if (mStoreIterator.value().numberSteps != 0) {
                mImportExportData = new ImportMailJob(mParentWidget, mStoreIterator.value().types, mArchiveStorage, mStoreIterator.value().numberSteps);
                executeJob();
            }
            break;
        case Utils::KAddressBook:
            if (mStoreIterator.value().numberSteps != 0) {
                mImportExportData = new ImportAddressbookJob(mParentWidget, mStoreIterator.value().types, mArchiveStorage, mStoreIterator.value().numberSteps);
                executeJob();
            }
            break;
        case Utils::KAlarm:
            if (mStoreIterator.value().numberSteps != 0) {
                mImportExportData = new ImportAlarmJob(mParentWidget, mStoreIterator.value().types, mArchiveStorage, mStoreIterator.value().numberSteps);
                executeJob();
            }
            break;
        case Utils::KOrganizer:
            if (mStoreIterator.value().numberSteps != 0) {
                mImportExportData = new ImportCalendarJob(mParentWidget, mStoreIterator.value().types, mArchiveStorage, mStoreIterator.value().numberSteps);
                executeJob();
            }
            break;
        case Utils::KJots:
            if (mStoreIterator.value().numberSteps != 0) {
                mImportExportData = new ImportJotJob(mParentWidget, mStoreIterator.value().types, mArchiveStorage, mStoreIterator.value().numberSteps);
                executeJob();
            }
            break;
        case Utils::KNotes:
            if (mStoreIterator.value().numberSteps != 0) {
                mImportExportData = new ImportNotesJob(mParentWidget, mStoreIterator.value().types, mArchiveStorage, mStoreIterator.value().numberSteps);
                executeJob();
            }
            break;
        case Utils::Akregator:
            if (mStoreIterator.value().numberSteps != 0) {
                mImportExportData = new ImportAkregatorJob(mParentWidget, mStoreIterator.value().types, mArchiveStorage, mStoreIterator.value().numberSteps);
                executeJob();
            }
            break;
        case Utils::Blogilo:
            if (mStoreIterator.value().numberSteps != 0) {
                mImportExportData = new ImportBlogiloJob(mParentWidget, mStoreIterator.value().types, mArchiveStorage, mStoreIterator.value().numberSteps);
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

void PimSettingsBackupRestore::restoreStart(const QString &filename)
{
    if (!openArchive(filename, false)) {
        Q_EMIT jobFailed();
        deleteLater();
        return;
    }
    Q_EMIT updateActions(true);
    mAction = Restore;
    mStoreIterator = mStored.constBegin();
    const int version = Utils::archiveVersion(mArchiveStorage->archive());
    if (version > Utils::currentArchiveVersion()) {
        if (KMessageBox::No == KMessageBox::questionYesNo(mParentWidget, i18n("The archive was created by a newer version of this program. It might contain additional data which will be skipped during import. Do you want to import it?"), i18n("Not correct version")))
            return;
    }
    qDebug()<<" version "<<version;
    AbstractImportExportJob::setArchiveVersion(version);

    const QDateTime now = QDateTime::currentDateTime();
    Q_EMIT addInfo(QLatin1Char('[') + QLocale().toString(( now ), QLocale::ShortFormat) + QLatin1Char(']'));

    Q_EMIT addInfo(i18n("Start to restore data from \'%1\'", mArchiveStorage->filename()));
    Q_EMIT addEndLine();
    restoreNextStep();
}

void PimSettingsBackupRestore::restoreFinished()
{
    Q_EMIT addInfo(i18n("Restoring data from \'%1\' done.", mArchiveStorage->filename()));
    //At the end
    mArchiveStorage->closeArchive();
    delete mArchiveStorage;
    mArchiveStorage = 0;
    delete mImportExportData;
    mImportExportData = 0;
    Q_EMIT updateActions(false);
    deleteLater();
}

void PimSettingsBackupRestore::executeJob()
{
    connect(mImportExportData, SIGNAL(info(QString)), SIGNAL(addInfo(QString)));
    connect(mImportExportData, SIGNAL(error(QString)), SIGNAL(addError(QString)));
    connect(mImportExportData, SIGNAL(title(QString)), SIGNAL(addTitle(QString)));
    connect(mImportExportData, SIGNAL(endLine()), SIGNAL(addEndLine()));
    connect(mImportExportData, SIGNAL(jobFinished()), SIGNAL(jobFinished()));
    mImportExportData->start();
}

void PimSettingsBackupRestore::slotJobFinished()
{
    ++mStoreIterator;
    Q_EMIT addEndLine();
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
