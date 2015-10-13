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
#include "filterkmailarchive.h"

#include <KLocalizedString>
#include <QFileDialog>
#include <KZip>
#include <KTar>
#include <QUrl>

#include "mailimporter_debug.h"
#include <QApplication>

#include <QSharedPointer>

#include <QMimeDatabase>
#include <QMimeType>

using namespace MailImporter;

class MailImporter::FilterKMailArchivePrivate
{
public:
    FilterKMailArchivePrivate()
        : mTotalFiles(0),
          mFilesDone(0)
    {
    }

    int mTotalFiles;
    int mFilesDone;
};
FilterKMailArchive::FilterKMailArchive()
    : Filter(i18n("Import KMail Archive File"),
             QStringLiteral("Klar\xE4lvdalens Datakonsult AB"),
             i18n("<p><b>KMail Archive File Import Filter</b></p>"
                  "<p>This filter will import archives files previously exported by KMail.</p>"
                  "<p>Archive files contain a complete folder subtree compressed into a single file.</p>")),
      d(new MailImporter::FilterKMailArchivePrivate)
{
}

FilterKMailArchive::~FilterKMailArchive()
{
    delete d;
}

// Input: .inbox.directory
// Output: inbox
// Can also return an empty string if this is no valid dir name
static QString folderNameForDirectoryName(const QString &dirName)
{
    Q_ASSERT(dirName.startsWith(QStringLiteral(".")));
    const QString end = QStringLiteral(".directory");
    const int expectedIndex = dirName.length() - end.length();
    if (dirName.toLower().indexOf(end) != expectedIndex) {
        return QString();
    }
    QString returnName = dirName.left(dirName.length() - end.length());
    returnName = returnName.right(returnName.length() - 1);
    return returnName;
}

bool FilterKMailArchive::importMessage(const KArchiveFile *file, const QString &folderPath)
{
    if (filterInfo()->shouldTerminate()) {
        return false;
    }

    qApp->processEvents();

    KMime::Message::Ptr newMessage(new KMime::Message());
    newMessage->setContent(file->data());
    newMessage->parse();

    Akonadi::Collection collection = parseFolderString(folderPath);
    if (!collection.isValid()) {
        filterInfo()->addErrorLogEntry(i18n("Unable to retrieve folder for folder path %1.", folderPath));
        return false;
    }

    if (filterInfo()->removeDupMessage()) {
        KMime::Headers::MessageID *messageId = newMessage->messageID(false);
        if (messageId && !messageId->asUnicodeString().isEmpty()) {
            if (checkForDuplicates(messageId->asUnicodeString(), collection, folderPath)) {
                d->mTotalFiles--;
                return true;
            }
        }
    }

    const bool result = addAkonadiMessage(collection, newMessage);
    if (result) {
        d->mFilesDone++;
    }
    return result;
}

bool FilterKMailArchive::importFolder(const KArchiveDirectory *folder, const QString &folderPath)
{
    qCDebug(MAILIMPORTER_LOG) << "Importing folder" << folder->name();
    filterInfo()->addInfoLogEntry(i18n("Importing folder '%1'...", folderPath));
    filterInfo()->setTo(filterInfo()->rootCollection().name() + folderPath);
    const KArchiveDirectory *const messageDir =
        dynamic_cast<const KArchiveDirectory *>(folder->entry(QStringLiteral("cur")));
    if (messageDir) {

        int total = messageDir->entries().count();
        int cur = 1;

        foreach (const QString &entryName, messageDir->entries()) {
            filterInfo()->setCurrent(cur * 100 / total);
            filterInfo()->setOverall(d->mFilesDone * 100 / d->mTotalFiles);
            const KArchiveEntry *const entry = messageDir->entry(entryName);

            if (entry->isFile()) {
                const int oldCount = d->mFilesDone;
                if (!importMessage(static_cast<const KArchiveFile *>(entry), folderPath)) {
                    return false;
                }

                // Adjust the counter. Total count can decrease because importMessage() detects a duplicate
                if (oldCount != d->mFilesDone) {
                    cur++;
                } else {
                    total--;
                }
            } else {
                filterInfo()->addErrorLogEntry(i18n("Unexpected subfolder %1 in folder %2.", entryName, folder->name()));
            }
        }
    } else {
        filterInfo()->addErrorLogEntry(i18n("No subfolder named 'cur' in folder %1.", folder->name()));
    }
    return true;
}

bool FilterKMailArchive::importDirectory(const KArchiveDirectory *directory, const QString &folderPath)
{
    qCDebug(MAILIMPORTER_LOG) << "Importing directory" << directory->name();
    foreach (const QString &entryName, directory->entries()) {
        const KArchiveEntry *const entry = directory->entry(entryName);

        if (entry->isDirectory()) {

            const KArchiveDirectory *dir = static_cast<const KArchiveDirectory *>(entry);

            if (!dir->name().startsWith(QStringLiteral("."))) {
                if (!importFolder(dir, folderPath + QLatin1Char('/') + dir->name())) {
                    return false;
                }
            }

            // Entry starts with a dot, so we assume it is a subdirectory
            else {

                const QString folderName = folderNameForDirectoryName(entry->name());
                if (folderName.isEmpty()) {
                    filterInfo()->addErrorLogEntry(i18n("Unexpected subdirectory named '%1'.", entry->name()));
                } else {

                    if (!importDirectory(dir, folderPath + QLatin1Char('/') + folderName)) {
                        return false;
                    }
                }
            }
        }
    }

    return true;
}

int FilterKMailArchive::countFiles(const KArchiveDirectory *directory) const
{
    int count = 0;
    foreach (const QString &entryName, directory->entries()) {
        const KArchiveEntry *const entry = directory->entry(entryName);
        if (entry->isFile()) {
            count++;
        } else {
            count += countFiles(static_cast<const KArchiveDirectory *>(entry));
        }
    }
    return count;
}

void FilterKMailArchive::import()
{
    Q_ASSERT(filterInfo()->rootCollection().isValid());

    const QString archiveFile = QFileDialog::getOpenFileName(filterInfo()->parent(), i18n("Select KMail Archive File to Import"), QString(), i18n("KMail Archive Files (*.tar *.tar.gz *.tar.bz2 *.zip)"));
    if (archiveFile.isEmpty()) {
        filterInfo()->alert(i18n("Please select an archive file that should be imported."));
        return;
    }
    importMails(archiveFile);
}

void FilterKMailArchive::importMails(const QString  &archiveFile)
{
    filterInfo()->setFrom(archiveFile);

    QMimeDatabase db;
    QMimeType mimeType = db.mimeTypeForFile(archiveFile, QMimeDatabase::MatchExtension);
    typedef QSharedPointer<KArchive> KArchivePtr;
    KArchivePtr archive;
    if (!mimeType.globPatterns().filter(QStringLiteral("tar"), Qt::CaseInsensitive).isEmpty()) {
        archive = KArchivePtr(new KTar(archiveFile));
    } else if (!mimeType.globPatterns().filter(QStringLiteral("zip"), Qt::CaseInsensitive).isEmpty()) {
        archive = KArchivePtr(new KZip(archiveFile));
    } else {
        filterInfo()->alert(i18n("The file '%1' does not appear to be a valid archive.", archiveFile));
        return;
    }

    if (!archive->open(QIODevice::ReadOnly)) {
        filterInfo()->alert(i18n("Unable to open archive file '%1'", archiveFile));
        return;
    }

    filterInfo()->setOverall(0);
    filterInfo()->addInfoLogEntry(i18n("Counting files in archive..."));
    d->mTotalFiles = countFiles(archive->directory());

    if (importDirectory(archive->directory(), QString())) {
        filterInfo()->setOverall(100);
        filterInfo()->setCurrent(100);
        filterInfo()->addInfoLogEntry(i18n("Importing the archive file '%1' into the folder '%2' succeeded.",
                                           archiveFile, filterInfo()->rootCollection().name()));
        filterInfo()->addInfoLogEntry(i18np("1 message was imported.", "%1 messages were imported.",
                                            d->mFilesDone));
    } else {
        filterInfo()->addInfoLogEntry(i18n("Importing the archive failed."));
    }
    archive->close();
}
