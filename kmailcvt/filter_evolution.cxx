/*
 *  filter_evolution.cxx
 *  Author : Simon MARTIN <simartin@users.sourceforge.net>
 *  Copyright (c) 2004 Simon MARTIN
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

#include "filter_evolution.hxx"

#include <klocale.h>
#include <kfiledialog.h>
#include <ktemporaryfile.h>


/** Default constructor. */
FilterEvolution::FilterEvolution(void) :
        Filter(i18n("Import Evolution 1.x Local Mails and Folder Structure"),
               "Simon MARTIN<br /><br />( Filter accelerated by Danny Kukawka )",
               i18n("<p><b>Evolution 1.x import filter</b></p>"
                    "<p>Select the base directory of Evolution's mails (usually ~/evolution/local).</p>"
                    "<p>Since it is possible to recreate the folder structure, the folders "
                    "will be stored under: \"Evolution-Import\".</p>"))
{}

/** Destructor. */
FilterEvolution::~FilterEvolution(void)
{
}

/** Recursive import of Evolution's mboxes. */
void FilterEvolution::import(FilterInfo *info)
{
    // We ask the user to choose Evolution's root directory.
    QString evolDir = QDir::homePath() + "/evolution/local";
    QDir d( evolDir );
    if ( !d.exists() ) {
        evolDir = QDir::homePath();
    }

    mailDir = KFileDialog::getExistingDirectory(evolDir, info->parent());

    if (mailDir.isEmpty()) {
        info->alert(i18n("No directory selected."));
    }
    /**
     * If the user only select homedir no import needed because
     * there should be no files and we surely import wrong files.
     */
    else if ( mailDir == QDir::homePath() || mailDir == (QDir::homePath() + '/')) {
        info->addLog(i18n("No files found for import."));
    } else {
        info->setOverall(0);
        // Recursive import of the MBoxes.
        QDir dir(mailDir);
        QStringList rootSubDirs = dir.entryList(QStringList("[^\\.]*"), QDir::Dirs, QDir::Name); // Removal of . and ..
        int currentDir = 1, numSubDirs = rootSubDirs.size();
        for(QStringList::Iterator filename = rootSubDirs.begin() ; filename != rootSubDirs.end() ; ++filename, ++currentDir) {
            importDirContents(info, dir.filePath(*filename), *filename, QString());
            info->setOverall((int) ((float) currentDir / numSubDirs * 100));
        }
    }
    info->addLog( i18n("Finished importing emails from %1", mailDir ));
    info->setCurrent(100);
    info->setOverall(100);
}

/**
 * Import of a directory contents.
 * @param info Information storage for the operation.
 * @param dirName The name of the directory to import.
 * @param KMailRootDir The directory's root directory in KMail's folder structure.
 * @param KMailSubDir The directory's direct ancestor in KMail's folder structure.
 */
void FilterEvolution::importDirContents(FilterInfo *info, const QString& dirName, const QString& KMailRootDir, const QString& KMailSubDir)
{
    // If there is a mbox, we import it
    QDir dir(dirName);
    if(dir.exists("mbox")) {
        importMBox(info, dirName + "/mbox", KMailRootDir, KMailSubDir);
    }
    // If there are subfolders, we import them one by one
    if(dir.exists("subfolders")) {
        QDir subfolders(dirName + "/subfolders");
        QStringList subDirs = subfolders.entryList(QStringList("[^\\.]*"), QDir::Dirs, QDir::Name);
        for(QStringList::Iterator filename = subDirs.begin() ; filename != subDirs.end() ; ++filename) {
            QString kSubDir;
            if(!KMailSubDir.isNull()) {
                kSubDir = KMailSubDir + '/' + *filename;
            } else {
                kSubDir = *filename;
            }
            importDirContents(info, subfolders.filePath(*filename), KMailRootDir, kSubDir);
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
void FilterEvolution::importMBox(FilterInfo *info, const QString& mboxName, const QString& rootDir, const QString& targetDir)
{
    QFile mbox(mboxName);
    bool first_msg = true;
    QString tmp_from = mboxName;
    if (!mbox.open(QIODevice::ReadOnly)) {
        info->alert(i18n("Unable to open %1, skipping", mboxName));
    } else {
        QFileInfo filenameInfo(mboxName);

        info->setCurrent(0);
        if( mboxName.length() > 20 ) {
            QString tmp_info = mboxName;
            tmp_info = tmp_info.replace( mailDir, ".." );
            if (tmp_info.contains("subfolders/"))
                tmp_info.remove("subfolders/");
            info->setFrom( tmp_info );
            tmp_from = tmp_info;
        } else
            info->setFrom(mboxName);
        if(targetDir.contains("subfolders/")) {
            QString tmp_info = targetDir;
            tmp_info.remove("subfolders/");
            info->setTo(tmp_info);
        } else
            info->setTo(targetDir);

        info->addLog(i18n("Importing emails from %1...", tmp_from));

        QByteArray input(MAX_LINE,'\0');
        long l = 0;

        while (!mbox.atEnd()) {
            KTemporaryFile tmp;
            tmp.open();
            /* comment by Danny:
            * Don't use QTextStream to read from mbox, better use QDataStream. QTextStream only
            * support Unicode/Latin1/Locale. So you lost information from emails with
            * charset!=Unicode/Latin1/Locale (e.g. KOI8-R) and Content-Transfer-Encoding != base64
            * (e.g. 8Bit). It also not help to convert the QTextStream to Unicode. By this you
            * get Unicode/UTF-email but KMail can't detect the correct charset.
            */
            QByteArray seperate;

            if(!first_msg)
                tmp.write( input, l );
            l = mbox.readLine( input.data(),MAX_LINE); // read the first line, prevent "From "
            tmp.write( input, l );

            while ( ! mbox.atEnd() &&  (l = mbox.readLine(input.data(),MAX_LINE)) && ((seperate = input.data()).left(5) != "From ")) {
                tmp.write( input, l );
            }
            tmp.flush();
            first_msg = false;

            QString destFolder = rootDir;
            if(!targetDir.isNull()) {
                destFolder = "Evolution-Import/" + destFolder + '/' + targetDir;
            } else {
                destFolder = "Evolution-Import/" + destFolder;
            }

            /* comment by Danny Kukawka:
             * addMessage() == old function, need more time and check for duplicates
             * addMessage_fastImport == new function, faster and no check for duplicates
             */
            if(info->removeDupMsg)
                addMessage( info, destFolder, tmp.fileName() );
            else
                addMessage_fastImport( info, destFolder, tmp.fileName() );

            int currentPercentage = (int) (((float) mbox.pos() / filenameInfo.size()) * 100);
            info->setCurrent(currentPercentage);
            if (info->shouldTerminate()) return;
        }

        if (count_duplicates > 0) {
            info->addLog( i18np("1 duplicate message not imported", "%1 duplicate messages not imported", count_duplicates));
        }
        count_duplicates = 0;
        mbox.close();
    }
}
