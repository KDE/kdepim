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
#ifndef BACKUPJOB_H
#define BACKUPJOB_H

#include <kurl.h>
#include <tqptrlist.h>

#include <tqobject.h>

class KMFolder;
class KMMessage;
class KArchive;
class KProcess;
class TQWidget;

namespace KPIM {
  class ProgressItem;
}

namespace KMail
{
  class FolderJob;

/**
 * Writes an entire folder structure to an archive file.
 * The archive is structured like a hierarchy of maildir folders. However, every type of folder
 * works as the source, i.e. also online IMAP folders.
 *
 * The job deletes itself after it finished.
 */
class BackupJob : public TQObject
{
  Q_OBJECT

  public:

    // These enum values have to stay in sync with the format combobox of ArchiveFolderDialog!
    enum ArchiveType { Zip = 0, Tar = 1, TarBz2 = 2, TarGz = 3 };

    explicit BackupJob( TQWidget *parent = 0 );
    ~BackupJob();
    void setRootFolder( KMFolder *rootFolder );
    void setSaveLocation( const KURL &savePath );
    void setArchiveType( ArchiveType type );
    void setDeleteFoldersAfterCompletion( bool deleteThem );
    void start();

  private slots:

    void messageRetrieved( KMMessage *message );
    void folderJobFinished( KMail::FolderJob *job );
    void processCurrentMessage();
    void cancelJob();

  private:

    void queueFolders( KMFolder *root );
    void archiveNextFolder();
    void archiveNextMessage();
    TQString stripRootPath( const TQString &path ) const;
    bool hasChildren( KMFolder *folder ) const;
    void finish();
    void abort( const TQString &errorMessage );
    bool writeDirHelper( const TQString &directoryPath, const TQString &permissionPath );

    KURL mMailArchivePath;
    ArchiveType mArchiveType;
    KMFolder *mRootFolder;
    KArchive *mArchive;
    TQWidget *mParentWidget;
    bool mCurrentFolderOpen;
    int mArchivedMessages;
    uint mArchivedSize;
    KPIM::ProgressItem *mProgressItem;
    bool mAborted;
    bool mDeleteFoldersAfterCompletion;

    // True if we obtained ownership of the kMMessage after calling getMsg(), since we need
    // to call ungetMsg() then. For that, we also remember the original index.
    bool mUnget;
    int mMessageIndex;

    TQPtrList<KMFolder> mPendingFolders;
    KMFolder *mCurrentFolder;
    TQValueList<unsigned long> mPendingMessages;
    KMMessage *mCurrentMessage;
    FolderJob *mCurrentJob;
};

}

#endif
