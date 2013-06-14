/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>
  
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

#include "importaddressbookjob.h"
#include "archivestorage.h"

ImportAddressbookJob::ImportAddressbookJob(QWidget *parent, ArchiveStorage *archiveStorage)
    : AbstractImportExportJob(parent,archiveStorage,/*typeSelected,numberOfStep*/0,0 /*TODO fix it*/)
{
    Q_UNUSED( parent );
    Q_UNUSED( archiveStorage );
}

ImportAddressbookJob::~ImportAddressbookJob()
{

}

void ImportAddressbookJob::start()
{
    mArchiveDirectory = archive()->directory();
    restoreConfig();
    //TODO
}

void ImportAddressbookJob::restoreResources()
{
    //TODO
}

void ImportAddressbookJob::restoreConfig()
{
    //TODO
    //kaddressbookrc
}

QString ImportAddressbookJob::componentName() const
{
    return QLatin1String("KAddressBook");
}

#include "importaddressbookjob.moc"
