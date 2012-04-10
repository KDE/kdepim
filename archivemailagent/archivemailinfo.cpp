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
#include "archivemailinfo.h"

ArchiveMailInfo::ArchiveMailInfo()
  :mSaveCollection(-1)
  ,mSaveSubCollection(false)
{
}

ArchiveMailInfo::ArchiveMailInfo(const KConfigGroup& config)
  :mSaveCollection(-1)
  ,mSaveSubCollection(false)
{
  //TODO
}


ArchiveMailInfo::~ArchiveMailInfo()
{

}

KUrl ArchiveMailInfo::url() const
{
  return mPath;
}

void ArchiveMailInfo::setUrl(const KUrl& url)
{
  mPath = url;
}

bool ArchiveMailInfo::saveSubCollection() const
{
  return mSaveSubCollection;
}

void ArchiveMailInfo::setSaveSubCollection( bool saveSubCol )
{
  mSaveSubCollection = saveSubCol;
}

void ArchiveMailInfo::setSaveCollection(const Akonadi::Collection& collection)
{
  mSaveCollection = collection;
}

Akonadi::Collection ArchiveMailInfo::saveCollection() const
{
  return mSaveCollection;
}
