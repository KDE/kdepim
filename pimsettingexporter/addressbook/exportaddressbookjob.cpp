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

#include "exportaddressbookjob.h"
#include "messageviewer/utils/kcursorsaver.h"

#include <KLocale>
#include <KStandardDirs>
#include <KTemporaryFile>

#include <QWidget>


ExportAddressbookJob::ExportAddressbookJob(QWidget *parent, BackupMailUtil::BackupTypes typeSelected, ArchiveStorage *archiveStorage,int numberOfStep)
    : AbstractImportExportJob(parent, archiveStorage, typeSelected, numberOfStep)
{
}

ExportAddressbookJob::~ExportAddressbookJob()
{

}

void ExportAddressbookJob::start()
{
    mArchiveDirectory = archive()->directory();
    //FIXME
    backupConfig();
    backupResources();
}


void ExportAddressbookJob::backupResources()
{
    showInfo(i18n("Backing up resources..."));
    MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );
    //TODO backup addressbook resources
    Q_EMIT info(i18n("Resources backup done."));
}

void ExportAddressbookJob::backupConfig()
{
    //kaddressbookrc
    showInfo(i18n("Backing up config..."));
    MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );

    const QString kaddressbookStr(QLatin1String("kaddressbookrc"));
    const QString kaddressbookrc = KStandardDirs::locateLocal( "config", kaddressbookStr);
    if (QFile(kaddressbookrc).exists()) {
        KSharedConfigPtr kaddressbook = KSharedConfig::openConfig(kaddressbookrc);

        KTemporaryFile tmp;
        tmp.open();

        //TODO adapt collection path
        KConfig *kaddressBookConfig = kaddressbook->copyTo( tmp.fileName() );
        kaddressBookConfig->sync();
        backupFile(tmp.fileName(), BackupMailUtil::configsPath(), kaddressbookStr);
    }
    Q_EMIT info(i18n("Config backup done."));
}

QString ExportAddressbookJob::componentName() const
{
    return QLatin1String("KAddressBook");
}

#include "exportaddressbookjob.moc"
