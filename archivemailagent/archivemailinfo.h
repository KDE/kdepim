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
#ifndef ARCHIVEMAILINFO_H
#define ARCHIVEMAILINFO_H

#include "mailcommon/backupjob.h"
#include <KConfigGroup>
#include <Akonadi/Collection>
#include <KUrl>
#include <QDate>


class ArchiveMailInfo
{
public:
  explicit ArchiveMailInfo();
  explicit ArchiveMailInfo(const KConfigGroup& config);
  ~ArchiveMailInfo();

  enum ArchiveUnit {
    ArchiveDays = 0,
    ArchiveWeeks,
    ArchiveMonths,
    ArchiveMaxUnits
  };

  KUrl realUrl(const QString &filename) const;

  bool isEmpty() const;

  Akonadi::Collection::Id saveCollectionId() const;
  void setSaveCollectionId(Akonadi::Collection::Id collectionId);

  void setSaveSubCollection(bool b);
  bool saveSubCollection() const;

  void setUrl(const KUrl& url);
  KUrl url() const;

  void readConfig(const KConfigGroup& config);
  void writeConfig(KConfigGroup & config );

  void setArchiveType( MailCommon::BackupJob::ArchiveType type );
  MailCommon::BackupJob::ArchiveType archiveType() const;

  void setArchiveUnit( ArchiveMailInfo::ArchiveUnit unit );
  ArchiveMailInfo::ArchiveUnit archiveUnit() const;

  void setArchiveAge( int age );
  int archiveAge() const;

  void setLastDateSaved( const QDate& date );
  QDate lastDateSaved() const;

  int maximumArchiveCount() const;
  void setMaximumArchiveCount( int max );

private:
  QDate mLastDateSaved;
  int mArchiveAge;
  MailCommon::BackupJob::ArchiveType mArchiveType;
  ArchiveUnit mArchiveUnit;
  Akonadi::Collection::Id mSaveCollectionId;
  KUrl mPath;
  int mMaximumArchiveCount;
  bool mSaveSubCollection;
};

#endif // ARCHIVEMAILINFO_H
