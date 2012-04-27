/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>

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

#include "restoredata.h"
#include <KZip>

RestoreData::RestoreData(Util::BackupTypes typeSelected,const QString& filename)
  :mArchive(new KZip(filename))
{
  if(typeSelected & Util::Identity)
    restoreIdentity();
  if(typeSelected & Util::MailTransport)
    restoreTransports();
  if(typeSelected & Util::Mails)
    restoreMails();
  if(typeSelected & Util::Resources)
    restoreResources();
  if(typeSelected & Util::Config)
    restoreConfig();
  if(typeSelected & Util::AkonadiDb)
    restoreAkonadiDb();
}

RestoreData::~RestoreData()
{
  delete mArchive;
}

void RestoreData::restoreTransports()
{

}

void RestoreData::restoreResources()
{

}

void RestoreData::restoreMails()
{

}

void RestoreData::restoreConfig()
{

}

void RestoreData::restoreIdentity()
{

}

void RestoreData::restoreAkonadiDb()
{

}


#include "restoredata.moc"
