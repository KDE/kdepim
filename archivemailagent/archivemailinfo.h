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

#include <KConfigGroup>
#include <Akonadi/Collection>
#include <KUrl>


class ArchiveMailInfo
{
public:
  explicit ArchiveMailInfo();
  explicit ArchiveMailInfo(const KConfigGroup& config);
  ~ArchiveMailInfo();

  enum ArchiveUnits {
    ArchiveDays = 0,
    ArchiveWeeks,
    ArchiveMonths,
    ArchiveMaxUnits
  };

  Akonadi::Collection::Id saveCollectionId() const;
  void setSaveCollectionId(Akonadi::Collection::Id collectionId);

  void setSaveSubCollection(bool b);
  bool saveSubCollection() const;

  void setUrl(const KUrl& url);
  KUrl url() const;

  void load(const KConfigGroup& config);
  void save(KConfigGroup & config );

private:
  Akonadi::Collection::Id mSaveCollectionId;
  KUrl mPath;
  bool mSaveSubCollection;
};

#endif // ARCHIVEMAILINFO_H
