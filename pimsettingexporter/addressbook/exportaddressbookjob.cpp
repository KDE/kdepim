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

#include <QWidget>


ExportAddressbookJob::ExportAddressbookJob(QWidget *parent, ArchiveStorage *archiveStorage)
    :AbstractImportExportJob(parent,archiveStorage,/*typeSelected,numberOfStep*/0,0 /*TODO fix it*/)
{
}

ExportAddressbookJob::~ExportAddressbookJob()
{

}

void ExportAddressbookJob::start()
{
    mArchiveDirectory = archive()->directory();
    backupConfig();
    //TODO
}


void ExportAddressbookJob::backupResources()
{
    showInfo(i18n("Backing up resources..."));
    MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );
    //TODO backup calendar
    Q_EMIT info(i18n("Resources backup done."));
}

void ExportAddressbookJob::backupConfig()
{
    //kaddressbookrc
}

QString ExportAddressbookJob::componentName() const
{
    return QLatin1String("KAddressBook");
}

#include "exportaddressbookjob.moc"
