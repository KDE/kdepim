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
  :AbstractData(filename,typeSelected)
{
}

RestoreData::~RestoreData()
{
}

void RestoreData::startRestore()
{
  bool good = mArchive->open(QIODevice::ReadOnly);
  if(!good) {
    //TODO
  }
  if(mTypeSelected & Util::Identity)
    restoreIdentity();
  if(mTypeSelected & Util::MailTransport)
    restoreTransports();
  if(mTypeSelected & Util::Mails)
    restoreMails();
  if(mTypeSelected & Util::Resources)
    restoreResources();
  if(mTypeSelected & Util::Config)
    restoreConfig();
  if(mTypeSelected & Util::AkonadiDb)
    restoreAkonadiDb();
  closeArchive();
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
