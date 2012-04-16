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
  : mArchiveAge( 1 )
  , mArchiveType( MailCommon::BackupJob::Zip )
  , mArchiveUnit( ArchiveMailInfo::ArchiveDays ) 
  , mSaveCollectionId(-1) 
  , mSaveSubCollection(false)
{
}

ArchiveMailInfo::ArchiveMailInfo(const KConfigGroup& config)
  : mArchiveAge( 1 )
  , mArchiveType( MailCommon::BackupJob::Zip ) 
  , mArchiveUnit( ArchiveMailInfo::ArchiveDays ) 
  , mSaveCollectionId(-1)
  , mSaveSubCollection(false)
{
  readConfig(config);
}


ArchiveMailInfo::~ArchiveMailInfo()
{
//FIXME writeConfig ?
}

bool ArchiveMailInfo::isEmpty() const
{
  return (mSaveCollectionId==-1);
}


void ArchiveMailInfo::setArchiveAge( int age )
{
  mArchiveAge = age;
}
    
int ArchiveMailInfo::archiveAge() const
{
  return mArchiveAge;
}


void ArchiveMailInfo::setArchiveUnit( ArchiveMailInfo::ArchiveUnit unit )
{
  mArchiveUnit = unit;
}

ArchiveMailInfo::ArchiveUnit ArchiveMailInfo::archiveUnit() const
{
  return mArchiveUnit;
}


void ArchiveMailInfo::setArchiveType( MailCommon::BackupJob::ArchiveType type )
{
  mArchiveType = type;
}

MailCommon::BackupJob::ArchiveType ArchiveMailInfo::archiveType() const
{
  return mArchiveType;
}

void ArchiveMailInfo::setLastDateSaved( const QDate& date )
{
  mLastDateSaved = date;
}

QDate ArchiveMailInfo::lastDateSaved() const
{
  return mLastDateSaved;
}


void ArchiveMailInfo::readConfig(const KConfigGroup& config)
{
  mPath = config.readEntry("storePath",KUrl());
  mLastDateSaved = QDate::fromString(config.readEntry("lastDateSaved"));
  mSaveSubCollection = config.readEntry("saveSubCollection",false);
  mArchiveType = static_cast<MailCommon::BackupJob::ArchiveType>( config.readEntry( "archiveType", ( int )MailCommon::BackupJob::Zip ) );
  mArchiveUnit = static_cast<ArchiveUnit>( config.readEntry( "archiveUnit", ( int )ArchiveDays ) );
  Akonadi::Collection::Id tId = config.readEntry("saveCollectionId",mSaveCollectionId);
  mArchiveAge = config.readEntry("archiveAge",1);
  if ( tId >= 0 ) {
    mSaveCollectionId = tId;
  }
}

void ArchiveMailInfo::writeConfig(KConfigGroup & config )
{
  config.writeEntry("storePath",mPath);
  config.writeEntry("lastDateSaved", mLastDateSaved.toString() );
  config.writeEntry("saveSubCollection",mSaveSubCollection);
  config.writeEntry( "archiveType", ( int )mArchiveType );
  config.writeEntry( "archiveUnit", ( int )mArchiveUnit );
  config.writeEntry("saveCollectionId",mSaveCollectionId);
  config.writeEntry("archiveAge",mArchiveAge);
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

void ArchiveMailInfo::setSaveCollectionId(Akonadi::Collection::Id collectionId)
{
  mSaveCollectionId = collectionId;
}

Akonadi::Collection::Id ArchiveMailInfo::saveCollectionId() const
{
  return mSaveCollectionId;
}
