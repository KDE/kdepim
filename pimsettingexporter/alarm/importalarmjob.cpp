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

#include "importalarmjob.h"
#include "archivestorage.h"

ImportAlarmJob::ImportAlarmJob(QWidget *parent, ArchiveStorage *archiveStorage)
    : AbstractImportExportJob(parent,archiveStorage,/*typeSelected,numberOfStep*/0,0 /*TODO fix it*/)
{
    Q_UNUSED( parent );
    Q_UNUSED( archiveStorage );
}

ImportAlarmJob::~ImportAlarmJob()
{

}


void ImportAlarmJob::start()
{
    mArchiveDirectory = archive()->directory();
    restoreConfig();
    //TODO
}

void ImportAlarmJob::restoreResources()
{
    //TODO
}

void ImportAlarmJob::restoreConfig()
{
    //TODO
}

QString ImportAlarmJob::componentName() const
{
    return QLatin1String("KAlarm");
}

#include "importalarmjob.moc"
