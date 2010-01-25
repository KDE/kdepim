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
#include "filter_kmail_archive.hxx"

#include <KMime/Message>

#include <KLocale>
#include <KFileDialog>
#include <KZip>
#include <KTar>

#include <QSharedPointer>

#include <boost/shared_ptr.hpp>

FilterKMailArchive::FilterKMailArchive()
  : Filter( i18n( "Import KMail Archive File" ),
            "Klar\xE4lvdalens Datakonsult AB",
            i18n( "<p><b>KMail Archive File Import Filter</b></p>"
                  "<p>This filter will import archives files previously exported by KMail.</p>"
                  "<p>Archive files contain a complete folder subtree compressed into a single file.</p>" ) ),
    mFilterInfo( 0 ),
    mFilesDone( 0 )
{
}

FilterKMailArchive::~FilterKMailArchive()
{
}

// Input: .inbox.directory
// Output: inbox
// Can also return an empty string if this is no valid dir name
static QString folderNameForDirectoryName( const QString &dirName )
{
  Q_ASSERT( dirName.startsWith( QLatin1String( "." ) ) );
  const QString end = ".directory";
  const int expectedIndex = dirName.length() - end.length();
  if ( dirName.toLower().indexOf( end ) != expectedIndex )
    return QString();
  QString returnName = dirName.left( dirName.length() - end.length() );
  returnName = returnName.right( returnName.length() - 1 );
  return returnName;
}

bool FilterKMailArchive::importMessage( const KArchiveFile *file, const QString &folderPath )
{
  if ( mFilterInfo->shouldTerminate() )
    return false;

  qApp->processEvents();

  KMime::Message::Ptr newMessage( new KMime::Message() );
  newMessage->setContent( file->data() );
  newMessage->parse();

  Akonadi::Collection collection = parseFolderString( mFilterInfo, folderPath );
  if ( !collection.isValid() ) {
    mFilterInfo->addLog( i18n( "Unable to retrieve folder for folder path %1.", folderPath ) );
    return false;
  }

  if ( mFilterInfo->removeDupMsg ) {
    KMime::Headers::MessageID *messageId = newMessage->messageID( false );
    if ( messageId && !messageId->asUnicodeString().isEmpty() ) {
      if ( checkForDuplicates( mFilterInfo, messageId->asUnicodeString(), collection, folderPath ) ) {
        mTotalFiles--;
        return true;
      }
    }
  }

  const bool result = addAkonadiMessage( mFilterInfo, collection, newMessage );
  if ( result ) {
    mFilesDone++;
  }
  return result;
}

bool FilterKMailArchive::importFolder( const KArchiveDirectory *folder, const QString &folderPath )
{
  kDebug() << "Importing folder" << folder->name();
  mFilterInfo->addLog( i18n( "Importing folder '%1'...", folderPath ) );
  mFilterInfo->setTo( mFilterInfo->rootCollection().name() + folderPath );
  const KArchiveDirectory * const messageDir =
      dynamic_cast<const KArchiveDirectory*>( folder->entry( "cur" ) );
  if ( messageDir ) {

    int total = messageDir->entries().count();
    int cur = 1;

    foreach( const QString &entryName, messageDir->entries() ) {
      mFilterInfo->setCurrent( cur * 100 / total );
      mFilterInfo->setOverall( mFilesDone * 100 / mTotalFiles );
      const KArchiveEntry * const entry = messageDir->entry( entryName );

      if ( entry->isFile() ) {
        const int oldCount = mFilesDone;
        if ( !importMessage( static_cast<const KArchiveFile*>( entry ), folderPath ) )
          return false;

        // Adjust the counter. Total count can decrease because importMessage() detects a duplicate
        if ( oldCount != mFilesDone )
          cur++;
        else
          total--;
      }
      else {
        mFilterInfo->addLog( i18n( "Unexpected subfolder %1 in folder %2.", entryName, folder->name() ) );
      }
    }
  }
  else {
    mFilterInfo->addLog( i18n( "No subfolder named 'cur' in folder %1.", folder->name() ) );
  }
  return true;
}

bool FilterKMailArchive::importDirectory( const KArchiveDirectory *directory, const QString &folderPath )
{
  kDebug() << "Importing directory" << directory->name();
  foreach( const QString &entryName, directory->entries() ) {
    const KArchiveEntry * const entry = directory->entry( entryName );

    if ( entry->isDirectory() ) {

      const KArchiveDirectory *dir = static_cast<const KArchiveDirectory*>( entry );

      if ( !dir->name().startsWith( QLatin1String( "." ) ) ) {
        if ( !importFolder( dir, folderPath + '/' + dir->name() ) )
          return false;
      }

      // Entry starts with a dot, so we assume it is a subdirectory
      else {

        const QString folderName = folderNameForDirectoryName( entry->name() );
        if ( folderName.isEmpty() ) {
          mFilterInfo->addLog( i18n( "Unexpected subdirectory named '%1'.", entry->name() ) );
        }
        else {

          if ( !importDirectory( dir, folderPath + '/' + folderName ) )
            return false;
        }
      }
    }
  }

  return true;
}

int FilterKMailArchive::countFiles( const KArchiveDirectory *directory ) const
{
  int count = 0;
  foreach( const QString &entryName, directory->entries() ) {
    const KArchiveEntry * const entry = directory->entry( entryName );
    if ( entry->isFile() )
      count++;
    else
      count += countFiles( static_cast<const KArchiveDirectory*>( entry ) );
  }
  return count;
}

void FilterKMailArchive::import( FilterInfo *info )
{
  Q_ASSERT( info->rootCollection().isValid() );

  mFilterInfo = info;
  KFileDialog fileDialog( KUrl(), QString(), mFilterInfo->parent() );
  fileDialog.setMode( KFile::File | KFile::LocalOnly );
  fileDialog.setCaption( i18n( "Select KMail Archive File to Import" ) );
  fileDialog.setFilter( "*.tar.bz2 *.tar.gz *.tar *.zip|" +
                        i18n( "KMail Archive Files (*.tar, *.tar.gz, *.tar.bz2, *.zip)" ) );
  if ( !fileDialog.exec() ) {
    mFilterInfo->alert( i18n( "Please select an archive file that should be imported." ) );
    return;
  }
  const QString archiveFile = fileDialog.selectedFile();
  mFilterInfo->setFrom( archiveFile );

  KMimeType::Ptr mimeType = KMimeType::findByUrl( archiveFile, 0, true /* local file */ );
  typedef QSharedPointer<KArchive> KArchivePtr;
  KArchivePtr archive;
  if ( !mimeType->patterns().filter( "tar", Qt::CaseInsensitive ).isEmpty() )
    archive = KArchivePtr( new KTar( archiveFile ) );
  else if ( !mimeType->patterns().filter( "zip", Qt::CaseInsensitive ).isEmpty() )
    archive = KArchivePtr( new KZip( archiveFile ) );
  else {
    mFilterInfo->alert( i18n( "The file '%1' does not appear to be a valid archive.", archiveFile ) );
    return;
  }

  if ( !archive->open( QIODevice::ReadOnly ) ) {
    mFilterInfo->alert( i18n( "Unable to open archive file '%1'", archiveFile ) );
    return;
  }

  mFilterInfo->setOverall( 0 );
  mFilterInfo->addLog( i18n( "Counting files in archive..." ) );
  mTotalFiles = countFiles( archive->directory() );

  if ( importDirectory( archive->directory(), QString() ) ) {
    mFilterInfo->setOverall( 100 );
    mFilterInfo->setCurrent( 100 );
    mFilterInfo->addLog( i18n( "Importing the archive file '%1' into the folder '%2' succeeded.",
                               archiveFile, mFilterInfo->rootCollection().name() ) );
    mFilterInfo->addLog( i18np( "1 message was imported.", "%1 messages were imported.",
                                mFilesDone ) );
  }
  else {
    mFilterInfo->addLog( i18n( "Importing the archive failed." ) );
  }
  archive->close();
}
