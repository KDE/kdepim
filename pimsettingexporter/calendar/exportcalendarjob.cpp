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

#include "exportcalendarjob.h"

#include "messageviewer/utils/kcursorsaver.h"


#include <KLocale>
#include <KStandardDirs>
#include <KTemporaryFile>

#include <QFile>
#include <QWidget>


ExportCalendarJob::ExportCalendarJob(QWidget *parent, BackupMailUtil::BackupTypes typeSelected, ArchiveStorage *archiveStorage,int numberOfStep)
    :AbstractImportExportJob(parent, archiveStorage, typeSelected, numberOfStep)
{
}

ExportCalendarJob::~ExportCalendarJob()
{

}

void ExportCalendarJob::start()
{
    mArchiveDirectory = archive()->directory();
    createProgressDialog();

    if (mTypeSelected & BackupMailUtil::Resources) {
        backupResources();
        increaseProgressDialog();
        if (wasCanceled()) {
            return;
        }
    }
    if (mTypeSelected & BackupMailUtil::Config) {
        backupConfig();
        increaseProgressDialog();
        if (wasCanceled()) {
            return;
        }
    }
}


void ExportCalendarJob::backupResources()
{
    showInfo(i18n("Backing up resources..."));
    MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );
    //TODO backup calendar
    Q_EMIT info(i18n("Resources backup done."));
}

void ExportCalendarJob::backupConfig()
{
    showInfo(i18n("Backing up config..."));
    MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );

    const QString korganizerStr(QLatin1String("korganizerrc"));
    const QString korganizerrc = KStandardDirs::locateLocal( "config", korganizerStr);
    if (QFile(korganizerrc).exists()) {
        KSharedConfigPtr korganizer = KSharedConfig::openConfig(korganizerrc);

        KTemporaryFile tmp;
        tmp.open();

        KConfig *korganizerConfig = korganizer->copyTo( tmp.fileName() );

        //TODO adapt collection
        korganizerConfig->sync();
        backupFile(tmp.fileName(), BackupMailUtil::configsPath(), korganizerStr);
    }

    const QString korganizerPrintingStr(QLatin1String("korganizer_printing.rc"));
    const QString korganizerPrintingrc = KStandardDirs::locateLocal( "config",  korganizerPrintingStr);
    if (QFile(korganizerPrintingrc).exists()) {
        backupFile(korganizerPrintingrc, BackupMailUtil::configsPath(), korganizerPrintingStr);
    }

    const QString korgacStr(QLatin1String("korgacrc"));
    const QString korgacrc = KStandardDirs::locateLocal( "config", korgacStr );
    if (QFile(korgacrc).exists()) {
        backupFile(korgacrc, BackupMailUtil::configsPath(), korgacStr);
    }

    Q_EMIT info(i18n("Config backup done."));
}

QString ExportCalendarJob::componentName() const
{
    return QLatin1String("KOrganizer");
}

#include "exportcalendarjob.moc"
