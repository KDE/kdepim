/* Copyright 2009,2010 Klarälvdalens Datakonsult AB

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef FILTER_KMAIL_ARCHIVE_HXX
#define FILTER_KMAIL_ARCHIVE_HXX

#include "filters.hxx"

class KArchiveFile;
class KArchiveDirectory;

class FilterKMailArchive : public Filter
{
public:
  FilterKMailArchive();
  ~FilterKMailArchive();
  void import();
private:

  bool importDirectory( const KArchiveDirectory *directory, const QString &folderPath );
  bool importFolder( const KArchiveDirectory *folder, const QString &folderPath );
  bool importMessage( const KArchiveFile *file, const QString &folderPath );

  int countFiles( const KArchiveDirectory *directory ) const;

  int mTotalFiles;
  int mFilesDone;
};

#endif
