/* Copyright 2009 Klarälvdalens Datakonsult AB

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
#include "backupjob.h"

#include "progressmanager.h"

#include <Akonadi/CollectionFetchJob>
#include <Akonadi/CollectionFetchScope>
#include <Akonadi/ItemFetchJob>

#include <klocale.h>
#include <kzip.h>
#include <ktar.h>
#include <kmessagebox.h>
#include <kio/global.h>

#include <QStringList>
#include <QFileInfo>

using namespace KMail;

BackupJob::BackupJob( QWidget *parent )
  : QObject( parent ),
    mArchiveType( Zip ),
    mRootFolder( 0 ),
    mArchive( 0 ),
    mParentWidget( parent ),
    mArchivedMessages( 0 ),
    mArchivedSize( 0 ),
    mProgressItem( 0 ),
    mAborted( false ),
    mDeleteFoldersAfterCompletion( false ),
    mCurrentFolder( Akonadi::Collection() )
{
}

BackupJob::~BackupJob()
{
  mPendingFolders.clear();
  if ( mArchive ) {
    delete mArchive;
    mArchive = 0;
  }
}

void BackupJob::setRootFolder( const Akonadi::Collection &rootFolder )
{
  mRootFolder = rootFolder;
}

void BackupJob::setSaveLocation( const KUrl savePath )
{
  mMailArchivePath = savePath;
}

void BackupJob::setArchiveType( ArchiveType type )
{
  mArchiveType = type;
}

void BackupJob::setDeleteFoldersAfterCompletion( bool deleteThem )
{
  mDeleteFoldersAfterCompletion = deleteThem;
}

bool BackupJob::queueFolders( const Akonadi::Collection &root )
{
  mPendingFolders.append( root );
  // TODO: This should be done async!
  Akonadi::CollectionFetchJob *job = new Akonadi::CollectionFetchJob( root,
      Akonadi::CollectionFetchJob::Recursive );
  job->fetchScope().setAncestorRetrieval( Akonadi::CollectionFetchScope::All );
  job->exec();
  if ( job->error() ) {
    abort( i18n( "Unable to retrieve folder list." ) );
    return false;
  }

  mPendingFolders += job->collections();
  mAllFolders = mPendingFolders;
  return true;
}

bool BackupJob::hasChildren( const Akonadi::Collection &collection ) const
{
  kDebug() << "PORTDEBUG col=" << collection.id();
  foreach( const Akonadi::Collection &curCol, mAllFolders ) {
    kDebug() << "PORTDEBUG curCol.parent()" << curCol.parentCollection();
    if ( collection == curCol.parentCollection() )
      return true;
  }
  return false;
}

void BackupJob::cancelJob()
{
  abort( i18n( "The operation was canceled by the user." ) );
}

void BackupJob::abort( const QString &errorMessage )
{
  // We could be called this twice, since killing the current job below will cause the job to fail,
  // and that will call abort()
  if ( mAborted )
    return;

  mAborted = true;
  if ( mCurrentFolder.isValid() ) {
    mCurrentFolder = Akonadi::Collection();
  }
  if ( mArchive && mArchive->isOpen() ) {
    mArchive->close();
  }
#if 0
  if ( mCurrentJob ) {
    mCurrentJob->kill();
    mCurrentJob = 0;
  }
#endif
  if ( mProgressItem ) {
    mProgressItem->setComplete();
    mProgressItem = 0;
    // The progressmanager will delete it
  }

#if 0
  QString text = i18n( "Failed to archive the folder '%1'.", mRootFolder->name() );
#else
  QString text;
#endif
  text += '\n' + errorMessage;
  KMessageBox::sorry( mParentWidget, text, i18n( "Archiving failed." ) );
  deleteLater();
  // Clean up archive file here?
}

void BackupJob::finish()
{
  if ( mArchive->isOpen() ) {
    if ( !mArchive->close() ) {
      abort( i18n( "Unable to finalize the archive file." ) );
      return;
    }
  }

  mProgressItem->setStatus( i18n( "Archiving finished" ) );
  mProgressItem->setComplete();
  mProgressItem = 0;

  QFileInfo archiveFileInfo( mMailArchivePath.path() );
#if 0
  QString text = i18n( "Archiving folder '%1' successfully completed. "
                       "The archive was written to the file '%2'.",
                       mRootFolder->name(), mMailArchivePath.path() );
#else
  QString text;
#endif
  text += '\n' + i18np( "1 message of size %2 was archived.",
                        "%1 messages with the total size of %2 were archived.",
                        mArchivedMessages, KIO::convertSize( mArchivedSize ) );
  text += '\n' + i18n( "The archive file has a size of %1.",
                       KIO::convertSize( archiveFileInfo.size() ) );
  KMessageBox::information( mParentWidget, text, i18n( "Archiving finished." ) );

  if ( mDeleteFoldersAfterCompletion ) {
    // Some safety checks first...
    if ( archiveFileInfo.size() > 0 && ( mArchivedSize > 0 || mArchivedMessages == 0 ) ) {
      // Sorry for any data loss!
#if 0
      FolderUtil::deleteFolder( mRootFolder, mParentWidget );
#endif
    }
  }

  deleteLater();
}

void BackupJob::archiveNextMessage()
{
  if ( mAborted )
    return;

#if 0
  mCurrentMessage = 0;
  if ( mPendingMessages.isEmpty() ) {
    kDebug() << "===> All messages done in folder " << mCurrentFolder->name();
    archiveNextFolder();
    return;
  }

  unsigned long serNum = mPendingMessages.front();
  mPendingMessages.pop_front();

  KMFolder *folder;
  int index = -1;
  KMMsgDict::instance()->getLocation( serNum, &folder, &index );
  if ( index == -1 ) {
    kWarning() << "Failed to get message location for sernum " << serNum;
    abort( i18n( "Unable to retrieve a message for folder '%1'.", mCurrentFolder->name() ) );
    return;
  }

  Q_ASSERT( folder == mCurrentFolder );
  KMMessage *message = mCurrentFolder->getMsg( index );
  if ( !message ) {
    kWarning() << "Failed to retrieve message with index " << index;
    abort( i18n( "Unable to retrieve a message for folder '%1'.", mCurrentFolder->name() ) );
    return;
  }

  kDebug() << "Going to get next message with subject " << message->subject() << ", "
           << mPendingMessages.size() << " messages left in the folder.";

  if ( message->isComplete() ) {
    // Use a singleshot timer, or otherwise we risk ending up in a very big recursion
    // for folders that have many messages
    mCurrentMessage = message;
    QTimer::singleShot( 0, this, SLOT( processCurrentMessage() ) );
  }
  else if ( message->parent() ) {
    mCurrentJob = message->parent()->createJob( message );
    mCurrentJob->setCancellable( false );
    connect( mCurrentJob, SIGNAL( messageRetrieved( KMMessage* ) ),
             this, SLOT( messageRetrieved( KMMessage* ) ) );
    connect( mCurrentJob, SIGNAL( result( KMail::FolderJob* ) ),
             this, SLOT( folderJobFinished( KMail::FolderJob* ) ) );
    mCurrentJob->start();
  }
  else {
    kWarning() << "Message with subject " << mCurrentMessage->subject()
               << " is neither complete nor has a parent!";
    abort( i18n( "Internal error while trying to retrieve a message from folder '%1'.",
                 mCurrentFolder->name() ) );
  }
#endif
}

void BackupJob::processCurrentMessage()
{
  if ( mAborted )
    return;

#if 0
  if ( mCurrentMessage ) {
    kDebug() << "Processing message with subject " << mCurrentMessage->subject();
    const DwString &messageDWString = mCurrentMessage->asDwString();
    const qint64 messageSize = messageDWString.size();
    const char *messageString = mCurrentMessage->asDwString().c_str();
    QString messageName;
    QFileInfo fileInfo;
    if ( messageName.isEmpty() ) {
      messageName = QString::number( mCurrentMessage->getMsgSerNum() ); // IMAP doesn't have filenames
      if ( mCurrentMessage->storage() ) {
        fileInfo.setFile( mCurrentMessage->storage()->location() );
        // TODO: what permissions etc to take when there is no storage file?
      }
    }
    else {
      // TODO: What if the message is not in the "cur" directory?
      fileInfo.setFile( mCurrentFolder->location() + "/cur/" + mCurrentMessage->fileName() );
      messageName = mCurrentMessage->fileName();
    }

    const QString fileName = stripRootPath( mCurrentFolder->location() ) +
                             "/cur/" + messageName;

    QString user;
    QString group;
    mode_t permissions = 0700;
    time_t creationTime = time( 0 );
    time_t modificationTime = time( 0 );
    time_t accessTime = time( 0 );
    if ( !fileInfo.fileName().isEmpty() ) {
      user = fileInfo.owner();
      group = fileInfo.group();
      permissions = fileInfoToUnixPermissions( fileInfo );
      creationTime = fileInfo.created().toTime_t();
      modificationTime = fileInfo.lastModified().toTime_t();
      accessTime = fileInfo.lastRead().toTime_t();
    }
    else {
      kWarning() << "Unable to find file for message " << fileName;
    }

    if ( !mArchive->writeFile( fileName, user, group,
                               messageString, messageSize, permissions,
                               accessTime, modificationTime, creationTime ) ) {
      abort( i18n( "Failed to write a message into the archive folder '%1'.", mCurrentFolder->name() ) );
      return;
    }

    mArchivedMessages++;
    mArchivedSize += messageSize;
  }
  else {
    // No message? According to ImapJob::slotGetMessageResult(), that means the message is no
    // longer on the server. So ignore this one.
    kWarning() << "Unable to download a message for folder " << mCurrentFolder->name();
  }
  archiveNextMessage();
#endif
}

#if 0
void BackupJob::messageRetrieved( KMMessage *message )
{
  mCurrentMessage = message;
  processCurrentMessage();
}

void BackupJob::folderJobFinished( KMail::FolderJob *job )
{
  if ( mAborted )
    return;

  // The job might finish after it has emitted messageRetrieved(), in which case we have already
  // started a new job. Don't set the current job to 0 in that case.
  if ( job == mCurrentJob ) {
    mCurrentJob = 0;
  }

  if ( job->error() ) {
    if ( mCurrentFolder )
      abort( i18n( "Downloading a message in folder '%1' failed.", mCurrentFolder->name() ) );
    else
      abort( i18n( "Downloading a message in the current folder failed." ) );
  }
}
#endif

bool BackupJob::writeDirHelper( const QString &directoryPath )
{
  kDebug() << "PORTDEBUG Writing dir" << directoryPath;
  // PORT ME: Correct user/group
  kDebug() << "AKONDI PORT: Disabled code here!";
  return mArchive->writeDir( directoryPath, "user", "group" );
}

QString BackupJob::pathForCollection( const Akonadi::Collection &collection ) const
{
  kDebug() << "PORTDEBUG Getting path for collection" << collection.name();
  QString fullPath = collection.name();
  Akonadi::Collection curCol = collection.parentCollection();
  if ( collection != mRootFolder ) {
    while( curCol != mRootFolder ) {
      kDebug() << "PORTDEBUG Looking at" << curCol.name();
      fullPath.prepend( curCol.name() + '/' );
      curCol = curCol.parentCollection();
    }
    Q_ASSERT( curCol == mRootFolder );
    kDebug() << "PORTDEBUG Done, adding" << curCol.name();
    fullPath.prepend( curCol.name() + '/' );
  }
  kDebug() << "PORTDEBUG full path is:" << fullPath;
  return fullPath;
}

QString BackupJob::subdirPathForCollection( const Akonadi::Collection &collection ) const
{
  QString path = pathForCollection( collection );
  kDebug() << "PORTDEBUG Full path is:" << path;
  const int parentDirEndIndex = path.lastIndexOf( collection.name() );
  Q_ASSERT( parentDirEndIndex != -1 );
  path = path.left( parentDirEndIndex );
  kDebug() << "PORTDEBUG now" << path;
  path.append( '.' + collection.name() + ".directory" );
  kDebug() << "PORTDEBUG now" << path;
  return path;
}

void BackupJob::archiveNextFolder()
{
  if ( mAborted )
    return;

  if ( mPendingFolders.isEmpty() ) {
    finish();
    return;
  }

  mCurrentFolder = mPendingFolders.takeAt( 0 );
  kDebug() << "===> Archiving next folder: " << mCurrentFolder.name();
  mProgressItem->setStatus( i18n( "Archiving folder %1", mCurrentFolder.name() ) );

  const QString folderName = mCurrentFolder.name();
  bool success = true;
  if ( hasChildren( mCurrentFolder ) ) {
    if ( !writeDirHelper( subdirPathForCollection( mCurrentFolder ) ) )
      success = false;
  }
  if ( !writeDirHelper( pathForCollection( mCurrentFolder ) ) )
    success = false;
  if ( !writeDirHelper( pathForCollection( mCurrentFolder ) + "/cur" ) )
    success = false;
  if ( !writeDirHelper( pathForCollection( mCurrentFolder ) + "/new" ) )
    success = false;
  if ( !writeDirHelper( pathForCollection( mCurrentFolder ) + "/tmp" ) )
    success = false;
  if ( !success ) {
    abort( i18n( "Unable to create folder structure for folder '%1' within archive file.",
                 mCurrentFolder.name() ) );
    return;
  }

  // TODO: This should be done async!
  Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob( mCurrentFolder );
  job->exec();
  if ( job->error() ) {
    abort( i18n( "Unable to get message list for folder %1.", folderName ) );
    return;
  }
  mPendingMessages += job->items();
  foreach( const Akonadi::Item &item, mPendingMessages )
    kDebug() << "PORTDEBUG Pending items:" << item.id();

  archiveNextMessage();
}

void BackupJob::start()
{
  Q_ASSERT( !mMailArchivePath.isEmpty() );
  Q_ASSERT( mRootFolder.isValid() );

  if ( !queueFolders( mRootFolder ) )
    return;

  switch ( mArchiveType ) {
    case Zip: {
      KZip *zip = new KZip( mMailArchivePath.path() );
      zip->setCompression( KZip::DeflateCompression );
      mArchive = zip;
      break;
    }
    case Tar: {
      mArchive = new KTar( mMailArchivePath.path(), "application/x-tar" );
      break;
    }
    case TarGz: {
      mArchive = new KTar( mMailArchivePath.path(), "application/x-gzip" );
      break;
    }
    case TarBz2: {
      mArchive = new KTar( mMailArchivePath.path(), "application/x-bzip2" );
      break;
    }
  }

  kDebug() << "Starting backup.";
  if ( !mArchive->open( QIODevice::WriteOnly ) ) {
    abort( i18n( "Unable to open archive for writing." ) );
    return;
  }

  mProgressItem = KPIM::ProgressManager::createProgressItem(
      "BackupJob",
      i18n( "Archiving" ),
      QString(),
      true );
  mProgressItem->setUsesBusyIndicator( true );
  connect( mProgressItem, SIGNAL(progressItemCanceled(KPIM::ProgressItem*)),
           this, SLOT(cancelJob()) );

  archiveNextFolder();
}

#include "backupjob.moc"

