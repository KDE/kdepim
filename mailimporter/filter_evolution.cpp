/*
 *  Author : Simon MARTIN <simartin@users.sourceforge.net>
 *  Copyright (c) 2004 Simon MARTIN <simartin@users.sourceforge.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "filter_evolution.h"

#include <KLocalizedString>
#include <kfiledialog.h>
#include <QTemporaryFile>

using namespace MailImporter;

/** Default constructor. */
FilterEvolution::FilterEvolution() :
    Filter(i18n("Import Evolution 1.x Local Mails and Folder Structure"),
           "Simon MARTIN<br /><br />( Filter accelerated by Danny Kukawka )",
           i18n("<p><b>Evolution 1.x import filter</b></p>"
                "<p>Select the base directory of Evolution's mails (usually ~/evolution/local).</p>"
                "<p>Since it is possible to recreate the folder structure, the folders "
                "will be stored under: \"Evolution-Import\".</p>"))
{
}

/** Destructor. */
FilterEvolution::~FilterEvolution()
{
}

QString FilterEvolution::defaultSettingsPath()
{
    return QDir::homePath() + QLatin1String("/evolution/local");
}

/** Recursive import of Evolution's mboxes. */
void FilterEvolution::import()
{
    // We ask the user to choose Evolution's root directory.
    QString evolDir = defaultSettingsPath();
    QDir d(evolDir);
    if (!d.exists()) {
        evolDir = QDir::homePath();
    }
    importMails(KFileDialog::getExistingDirectory(evolDir, filterInfo()->parent()));
}

void FilterEvolution::importMails(const QString &maildir)
{
    setMailDir(maildir);

    if (mailDir().isEmpty()) {
        filterInfo()->alert(i18n("No directory selected."));
        return;
    }
    /**
    * If the user only select homedir no import needed because
    * there should be no files and we surely import wrong files.
    */
    else if (mailDir() == QDir::homePath() || mailDir() == (QDir::homePath() + QLatin1Char('/'))) {
        filterInfo()->addErrorLogEntry(i18n("No files found for import."));
    } else {
        filterInfo()->setOverall(0);
        // Recursive import of the MBoxes.
        QDir dir(mailDir());
        const QStringList rootSubDirs = dir.entryList(QStringList("[^\\.]*"), QDir::Dirs, QDir::Name); // Removal of . and ..
        int currentDir = 1, numSubDirs = rootSubDirs.size();
        QStringList::ConstIterator end(rootSubDirs.constEnd());
        for (QStringList::ConstIterator filename = rootSubDirs.constBegin() ; filename != end ; ++filename, ++currentDir) {
            importDirContents(dir.filePath(*filename), *filename, QString());
            filterInfo()->setOverall((int)((float) currentDir / numSubDirs * 100));
        }
    }
    filterInfo()->addInfoLogEntry(i18n("Finished importing emails from %1", mailDir()));
    filterInfo()->setCurrent(100);
    filterInfo()->setOverall(100);
}

/**
 * Import of a directory contents.
 * @param info Information storage for the operation.
 * @param dirName The name of the directory to import.
 * @param KMailRootDir The directory's root directory in KMail's folder structure.
 * @param KMailSubDir The directory's direct ancestor in KMail's folder structure.
 */
void FilterEvolution::importDirContents(const QString &dirName, const QString &KMailRootDir, const QString &KMailSubDir)
{
    // If there is a mbox, we import it
    QDir dir(dirName);
    if (dir.exists(QLatin1String("mbox"))) {
        importMBox(dirName + QLatin1String("/mbox"), KMailRootDir, KMailSubDir);
    }
    // If there are subfolders, we import them one by one
    if (dir.exists("subfolders")) {
        QDir subfolders(dirName + QLatin1String("/subfolders"));
        const QStringList subDirs = subfolders.entryList(QStringList("[^\\.]*"), QDir::Dirs, QDir::Name);
        QStringList::ConstIterator end(subDirs.constEnd());

        for (QStringList::ConstIterator filename = subDirs.constBegin() ; filename != end; ++filename) {
            QString kSubDir;
            if (!KMailSubDir.isNull()) {
                kSubDir = KMailSubDir + QLatin1Char('/') + *filename;
            } else {
                kSubDir = *filename;
            }
            importDirContents(subfolders.filePath(*filename), KMailRootDir, kSubDir);
        }
    }
}

/**
 * Import of a MBox file.
 * @param info Information storage for the operation.
 * @param dirName The MBox's name.
 * @param KMailRootDir The directory's root directory in KMail's folder structure.
 * @param KMailSubDir The directory's equivalent in KMail's folder structure. *
 */
void FilterEvolution::importMBox(const QString &mboxName, const QString &rootDir, const QString &targetDir)
{
    QFile mbox(mboxName);
    bool first_msg = true;
    QString tmp_from = mboxName;
    if (!mbox.open(QIODevice::ReadOnly)) {
        filterInfo()->alert(i18n("Unable to open %1, skipping", mboxName));
    } else {
        QFileInfo filenameInfo(mboxName);

        filterInfo()->setCurrent(0);
        if (mboxName.length() > 20) {
            QString tmp_info = mboxName;
            tmp_info = tmp_info.replace(mailDir(), "..");
            if (tmp_info.contains("subfolders/")) {
                tmp_info.remove("subfolders/");
            }
            filterInfo()->setFrom(tmp_info);
            tmp_from = tmp_info;
        } else {
            filterInfo()->setFrom(mboxName);
        }
        if (targetDir.contains("subfolders/")) {
            QString tmp_info = targetDir;
            tmp_info.remove("subfolders/");
            filterInfo()->setTo(tmp_info);
        } else {
            filterInfo()->setTo(targetDir);
        }

        filterInfo()->addInfoLogEntry(i18n("Importing emails from %1...", tmp_from));

        QByteArray input(MAX_LINE, '\0');
        long l = 0;

        while (!mbox.atEnd()) {
            QTemporaryFile tmp;
            tmp.open();
            /* comment by Danny:
            * Don't use QTextStream to read from mbox, better use QDataStream. QTextStream only
            * support Unicode/Latin1/Locale. So you lost information from emails with
            * charset!=Unicode/Latin1/Locale (e.g. KOI8-R) and Content-Transfer-Encoding != base64
            * (e.g. 8Bit). It also not help to convert the QTextStream to Unicode. By this you
            * get Unicode/UTF-email but KMail can't detect the correct charset.
            */
            QByteArray separate;

            if (!first_msg) {
                tmp.write(input, l);
            }
            l = mbox.readLine(input.data(), MAX_LINE); // read the first line, prevent "From "
            tmp.write(input, l);

            while (! mbox.atEnd() && (l = mbox.readLine(input.data(), MAX_LINE)) && ((separate = input.data()).left(5) != "From ")) {
                tmp.write(input, l);
            }
            tmp.flush();
            first_msg = false;

            QString destFolder = rootDir;
            if (!targetDir.isNull()) {
                destFolder = QLatin1String("Evolution-Import/") + destFolder + QLatin1Char('/') + targetDir;
            } else {
                destFolder = QLatin1String("Evolution-Import/") + destFolder;
            }

            /* comment by Danny Kukawka:
            * addMessage() == old function, need more time and check for duplicates
            * addMessage_fastImport == new function, faster and no check for duplicates
            */
            if (filterInfo()->removeDupMessage()) {
                addMessage(destFolder, tmp.fileName());
            } else {
                addMessage_fastImport(destFolder, tmp.fileName());
            }

            const int currentPercentage = (int)(((float) mbox.pos() / filenameInfo.size()) * 100);
            filterInfo()->setCurrent(currentPercentage);
            if (filterInfo()->shouldTerminate()) {
                return;
            }
        }

        if (countDuplicates() > 0) {
            filterInfo()->addInfoLogEntry(i18np("1 duplicate message not imported", "%1 duplicate messages not imported", countDuplicates()));
        }
        setCountDuplicates(0);
        mbox.close();
    }
}

