/*
  Copyright (c) 2009 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

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

#ifndef MAILCOMMON_BACKUPJOB_H
#define MAILCOMMON_BACKUPJOB_H

#include "mailcommon_export.h"
#include "progresswidget/progressmanager.h"

#include <Collection>
#include <Item>

#include <KUrl>

#include <QList>
#include <QObject>
#include <QPointer>

class KArchive;
class KJob;

class QWidget;

namespace Akonadi {
class ItemFetchJob;
}

namespace MailCommon {

/**
 * Writes an entire folder structure to an archive file. The archive is
 * structured like a hierarchy of maildir folders. However, every type of
 * folder works as the source, i.e. also online IMAP folders.
 *
 * The job deletes itself after it finished.
 */
class  MAILCOMMON_EXPORT BackupJob : public QObject
{
    Q_OBJECT

public:
    // These enum values have to stay in sync with the format combobox of ArchiveFolderDialog!
    enum ArchiveType {
        Zip = 0,
        Tar = 1,
        TarBz2 = 2,
        TarGz = 3
    };

    explicit BackupJob( QWidget *parent = 0 );
    ~BackupJob();

    void setRootFolder( const Akonadi::Collection &rootFolder );
    void setSaveLocation( const KUrl &savePath );
    void setArchiveType( ArchiveType type );
    void setDeleteFoldersAfterCompletion( bool deleteThem );
    void setRecursive( bool recursive );
    void setDisplayMessageBox(bool display);
    void setRealPath(const QString &path);


    void start();
signals:
    void backupDone(const QString&);
    void error(const QString&);

private slots:
    void itemFetchJobResult( KJob *job );
    void cancelJob();
    void archiveNextFolder();
    void onArchiveNextFolderDone( KJob *job );
    void archiveNextMessage();

private:
    bool queueFolders( const Akonadi::Collection &root );
    void processMessage( const Akonadi::Item &item );
    QString pathForCollection( const Akonadi::Collection &collection ) const;
    QString subdirPathForCollection( const Akonadi::Collection &collection ) const;
    bool hasChildren( const Akonadi::Collection &collection ) const;
    void finish();
    void abort( const QString &errorMessage );
    bool writeDirHelper( const QString &directoryPath );

    // Helper function to return the name of the given collection.
    // Some Collection's don't have the name fetched. However, in mAllFolders,
    // we have a list of Collection's that have that information in them, so
    // we can just look it up there.
    QString collectionName( const Akonadi::Collection &collection ) const;

    QString mRealPath;
    KUrl mMailArchivePath;
    time_t mArchiveTime;
    ArchiveType mArchiveType;
    Akonadi::Collection mRootFolder;
    KArchive *mArchive;
    QWidget *mParentWidget;
    int mArchivedMessages;
    uint mArchivedSize;
    QPointer<KPIM::ProgressItem> mProgressItem;
    bool mAborted;
    bool mDeleteFoldersAfterCompletion;
    bool mRecursive;

    Akonadi::Collection::List mPendingFolders;
    Akonadi::Collection::List mAllFolders;
    Akonadi::Collection mCurrentFolder;
    Akonadi::Item::List mPendingMessages;
    Akonadi::ItemFetchJob *mCurrentJob;
    bool mDisplayMessageBox;
};

}

#endif
