/***************************************************************************
            filter_thunderbird.cxx  -  Thunderbird mail import
                             -------------------
    begin                : Januar 26 2005
    copyright            : (C) 2005 by Danny Kukawka
    email                : danny.kukawka@web.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "filter_thunderbird.hxx"

#include <klocale.h>
#include <kfiledialog.h>
#include <ktemporaryfile.h>


/** Default constructor. */
FilterThunderbird::FilterThunderbird(void) :
        Filter(i18n("Import Thunderbird/Mozilla Local Mails and Folder Structure"),
               "Danny Kukawka",
               i18n("<p><b>Thunderbird/Mozilla import filter</b></p>"
                    "<p>Select your base Thunderbird/Mozilla mailfolder"
                    " (usually ~/.thunderbird/*.default/Mail/Local Folders/).</p>"
                    "<p><b>Note:</b> Never choose a Folder which <u>does not</u> contain mbox-files (for example,"
                    " a maildir): if you do, you will get many new folders.</p>"
                    "<p>Since it is possible to recreate the folder structure, the folders "
                    "will be stored under: \"Thunderbird-Import\".</p>"))
{}

/** Destructor. */
FilterThunderbird::~FilterThunderbird(void)
{
    endImport();
}

/** Recursive import of Evolution's mboxes. */
void FilterThunderbird::import(FilterInfo *info)
{
    /**
     * We ask the user to choose Evolution's root directory.
     * This should be usually ~/.thunderbird/xxxx.default/Mail/Local Folders/
     */
    QString thunderDir = QDir::homePath() + "/.thunderbird/";
    QDir d( thunderDir );
    if ( !d.exists() ) {
        thunderDir = QDir::homePath();
    }

    KFileDialog *kfd;
    kfd = new KFileDialog( thunderDir, "", 0 );
    kfd->setMode(KFile::Directory | KFile::LocalOnly);
    kfd->exec();
    mailDir  = kfd->selectedFile();

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

        /** Recursive import of the MailArchives */
        QDir dir(mailDir);
        const QStringList rootSubDirs = dir.entryList(QStringList("[^\\.]*"), QDir::Dirs, QDir::Name); // Removal of . and ..
        int currentDir = 1, numSubDirs = rootSubDirs.size();
        for(QStringList::ConstIterator filename = rootSubDirs.constBegin() ; filename != rootSubDirs.constEnd() ; ++filename, ++currentDir) {
            if(info->shouldTerminate()) break;
            importDirContents(info, dir.filePath(*filename), *filename, *filename);
            info->setOverall((int) ((float) currentDir / numSubDirs * 100));
        }

        /** import last but not least all archives from the root-dir */
        QDir importDir (mailDir);
        const QStringList files = importDir.entryList(QStringList("[^\\.]*"), QDir::Files, QDir::Name);
        for ( QStringList::ConstIterator mailFile = files.constBegin(); mailFile != files.constEnd(); ++mailFile) {
            if(info->shouldTerminate()) break;
            QString temp_mailfile = *mailFile;
            if (temp_mailfile.endsWith(QLatin1String(".msf")) || temp_mailfile.endsWith(QLatin1String("msgFilterRules.dat"))) {}
            else {
                info->addLog( i18n("Start import file %1...", temp_mailfile ) );
                importMBox(info, mailDir + temp_mailfile , temp_mailfile, QString());
            }
        }

        info->addLog( i18n("Finished importing emails from %1", mailDir ));
        if(count_duplicates > 0) {
            info->addLog( i18np("1 duplicate message not imported", "%1 duplicate messages not imported", count_duplicates));
        }
    }
    if (info->shouldTerminate()) info->addLog( i18n("Finished import, canceled by user."));
    info->setCurrent(100);
    info->setOverall(100);
    delete kfd;
}

/**
 * Import of a directory contents.
 * @param info Information storage for the operation.
 * @param dirName The name of the directory to import.
 * @param KMailRootDir The directory's root directory in KMail's folder structure.
 * @param KMailSubDir The directory's direct ancestor in KMail's folder structure.
 */
void FilterThunderbird::importDirContents(FilterInfo *info, const QString& dirName, const QString& KMailRootDir, const QString& KMailSubDir)
{
    if(info->shouldTerminate()) return;
    /** Here Import all archives in the current dir */
    QDir dir(dirName);

    QDir importDir (dirName);
    const QStringList files = importDir.entryList(QStringList("[^\\.]*"), QDir::Files, QDir::Name);
    for ( QStringList::ConstIterator mailFile = files.constBegin(); mailFile != files.constEnd(); ++mailFile) {
        if(info->shouldTerminate()) break;
        QString temp_mailfile = *mailFile;
        if (temp_mailfile.endsWith(QLatin1String(".msf")) || temp_mailfile.endsWith(QLatin1String("msgFilterRules.dat"))) {}
        else {
            info->addLog( i18n("Start import file %1...", temp_mailfile ) );
            importMBox(info, (dirName + '/' + temp_mailfile) , KMailRootDir, KMailSubDir);
        }
    }

    /** If there are subfolders, we import them one by one */
    QDir subfolders(dirName);
    const QStringList subDirs = subfolders.entryList(QStringList("[^\\.]*"), QDir::Dirs, QDir::Name);
    for(QStringList::ConstIterator filename = subDirs.constBegin() ; filename != subDirs.constEnd() ; ++filename) {
        if(info->shouldTerminate()) break;
        QString kSubDir;
        if(!KMailSubDir.isNull()) {
            kSubDir = KMailSubDir + '/' + *filename;
        } else {
            kSubDir = *filename;
        }
        importDirContents(info, subfolders.filePath(*filename), KMailRootDir, kSubDir);
    }
}

/**
 * Import of a MBox file.
 * @param info Information storage for the operation.
 * @param dirName The MBox's name.
 * @param KMailRootDir The directory's root directory in KMail's folder structure.
 * @param KMailSubDir The directory's equivalent in KMail's folder structure. *
 */
void FilterThunderbird::importMBox(FilterInfo *info, const QString& mboxName, const QString& rootDir, const QString& targetDir)
{
    QFile mbox(mboxName);
    bool first_msg = true;
    if (!mbox.open(QIODevice::ReadOnly)) {
        info->alert(i18n("Unable to open %1, skipping", mboxName));
    } else {
        QFileInfo filenameInfo(mboxName);

        info->setCurrent(0);
        if( mboxName.length() > 20 ) {
            QString tmp_info = mboxName;
            tmp_info = tmp_info.replace( mailDir, "../" );
            if (tmp_info.contains(".sbd"))
                tmp_info.remove(".sbd");
            info->setFrom( tmp_info );
        } else
            info->setFrom(mboxName);
        if(targetDir.contains(".sbd")) {
            QString tmp_info = targetDir;
            tmp_info.remove(".sbd");
            info->setTo(tmp_info);
        } else
            info->setTo(targetDir);

        QByteArray input(MAX_LINE,'\0');
        long l = 0;

        while (!mbox.atEnd()) {
            KTemporaryFile tmp;
            tmp.open();
            /** @todo check if the file is really a mbox, maybe search for 'from' string at start */
            /* comment by Danny:
            * Don't use QTextStream to read from mbox, better use QDataStream. QTextStream only
            * support Unicode/Latin1/Locale. So you lost information from emails with
            * charset!=Unicode/Latin1/Locale (e.g. KOI8-R) and Content-Transfer-Encoding != base64
            * (e.g. 8Bit). It also not help to convert the QTextStream to Unicode. By this you
            * get Unicode/UTF-email but KMail can't detect the correct charset.
            */
            QByteArray separate;

            if(!first_msg)
                tmp.write( input, l );
            l = mbox.readLine( input.data(),MAX_LINE); // read the first line, prevent "From "
            tmp.write( input, l );

            while ( ! mbox.atEnd() &&  (l = mbox.readLine(input.data(),MAX_LINE)) && ((separate = input.data()).left(5) != "From ")) {
                tmp.write( input, l );
            }
            tmp.flush();
            first_msg = false;

            QString destFolder;
            QString _targetDir = targetDir;
            if(!targetDir.isNull()) {
                if(_targetDir.contains(".sbd"))
                    _targetDir.remove(".sbd");
                destFolder += "Thunderbird-Import/" + _targetDir + '/' + filenameInfo.completeBaseName();// mboxName;
            } else {
                destFolder = "Thunderbird-Import/" + rootDir;
                if(destFolder.contains(".sbd"))
                    destFolder.remove(".sbd");
            }

            if(info->removeDupMsg)
                addMessage( info, destFolder, tmp.fileName() );
            else
                addMessage_fastImport( info, destFolder, tmp.fileName() );

            int currentPercentage = (int) (((float) mbox.pos() / filenameInfo.size()) * 100);
            info->setCurrent(currentPercentage);
            if (info->shouldTerminate()) {
                mbox.close();
                return;
            }
        }
        mbox.close();
    }
}

