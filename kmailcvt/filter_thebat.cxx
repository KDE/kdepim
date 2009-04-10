/***************************************************************************
            filter_thebat.hxx  -  TheBat! mail import
                             -------------------
    begin                : April 07 2005
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

#include "filter_thebat.hxx"


#include <QRegExp>

#include <klocale.h>
#include <kfiledialog.h>
#include <ktemporaryfile.h>

#include <kmessagebox.h>

/** Default constructor. */
FilterTheBat::FilterTheBat( void ) :
        Filter( i18n( "Import The Bat! Mails and Folder Structure" ),
                "Danny Kukawka",
                i18n( "<p><b>The Bat! import filter</b></p>"
                      "<p>Select the base directory of the \'The Bat!\' local mailfolder you "
                      "want to import.</p>"
                      "<p><b>Note:</b> This filter imports the *.tbb-files from \'The Bat!\' "
                      "local folder, e.g. from POP accounts, and not from IMAP/DIMAP accounts.</p>"
                      "<p>Since it is possible to recreate the folder structure, the folders "
                      "will be stored under: \"TheBat-Import\" in your local account.</p>" ) )
{}

/** Destructor. */
FilterTheBat::~FilterTheBat( void )
{
}

/** Recursive import of The Bat! maildir. */
void FilterTheBat::import( FilterInfo *info )
{
    QString _homeDir = QDir::homePath();

    KFileDialog *kfd;
    kfd = new KFileDialog( _homeDir, "", 0 );
    kfd->setMode( KFile::Directory | KFile::LocalOnly );
    kfd->exec();
    mailDir = kfd->selectedFile();

    if ( mailDir.isEmpty() ) {
        info->alert( i18n( "No directory selected." ) );
    }
    /**
     * If the user only select homedir no import needed because
     * there should be no files and we surely import wrong files.
     */
    else if ( mailDir == QDir::homePath() || mailDir == ( QDir::homePath() + '/' ) ) {
        info->addLog( i18n( "No files found for import." ) );
    } else {
        info->setOverall(0);

        /** Recursive import of the MailFolders */
        QDir dir(mailDir);
        QStringList rootSubDirs = dir.entryList(QStringList("[^\\.]*"), QDir::Dirs , QDir::Name);
        int currentDir = 1, numSubDirs = rootSubDirs.size();
        for(QStringList::Iterator filename = rootSubDirs.begin() ; filename != rootSubDirs.end() ; ++filename, ++currentDir) {
            importDirContents(info, dir.filePath(*filename));
            info->setOverall((int) ((float) currentDir / numSubDirs * 100));
            if(info->shouldTerminate()) break;
        }
    }

    info->addLog( i18n("Finished importing emails from %1", mailDir ));
    if (count_duplicates > 0) {
        info->addLog( i18np("1 duplicate message not imported", "%1 duplicate messages not imported", count_duplicates));
    }
    if (info->shouldTerminate()) info->addLog( i18n("Finished import, canceled by user."));

    count_duplicates = 0;
    info->setCurrent(100);
    info->setOverall(100);
    delete kfd;
}

/**
 * Import of a directory contents.
 * @param info Information storage for the operation.
 * @param dirName The name of the directory to import.
 */
void FilterTheBat::importDirContents( FilterInfo *info, const QString& dirName)
{
    if(info->shouldTerminate()) return;

    /** Here Import all archives in the current dir */
    QDir dir(dirName);
    QDir importDir (dirName);
    QStringList files = importDir.entryList(QStringList("*.[tT][bB][bB]"), QDir::Files, QDir::Name);
    for ( QStringList::Iterator mailFile = files.begin(); mailFile != files.end(); ++mailFile) {
        QString temp_mailfile = *mailFile;
        importFiles(info, (dirName + '/' + temp_mailfile));
        if(info->shouldTerminate()) return;
    }

    /** If there are subfolders, we import them one by one */
    QDir subfolders(dirName);
    QStringList subDirs = subfolders.entryList(QStringList("[^\\.]*"), QDir::Dirs , QDir::Name);
    for(QStringList::Iterator filename = subDirs.begin() ; filename != subDirs.end() ; ++filename) {
        importDirContents(info, subfolders.filePath(*filename));
        if(info->shouldTerminate()) return;
    }
}

/**
 * Import the files within a Folder.
 * @param info Information storage for the operation.
 * @param dirName The name of the directory to import.
 */
void FilterTheBat::importFiles( FilterInfo *info, const QString& FileName)
{

    // Format of a tbb-file from The Bat! 3.x
    // ----------------------------------------
    // First comes a header of 3K (3128 byte/ 0x00000c38), which we can forget.
    // The byte 3129 is the first character of the first message.
    //
    // The end of a message is marked trough "! p 0" and 43 following characters.
    // (within: "_UB", blanks and some other chars.) Together are 48 characters as
    // separator.
    // ----------------------------------------

    long l = 0;
    QByteArray input(50,'\0');
    QRegExp regexp("!.p.0");
    QFile tbb(FileName);
    int iFound = 0;
    int count = 0;
    long endOfEmail = 0;
    QList<long> offsets;

    if (!tbb.open(QIODevice::ReadOnly)) {
        info->alert(i18n("Unable to open %1, skipping", FileName));
    } else {
        // BUILD the index of messages :
        // We need this really ugly way, because read with tbb.readLine()
        // does not work correct. Maybe in come in a continuous loop !!!
        // Reason:
        //      if you use readLine() to read from a file with binary data
        //      QFile::at() and QFile::atEnd() return wrong value. So we
        //      never get QFile::atEnd() == true in some cases. This looks
        //      like a bug in Qt3 maybe fixed in Qt4.
        //
        while((l = tbb.read(input.data(),50)) ) {
            if(info->shouldTerminate()) {
                tbb.close();
                return;
            }
            QString _tmp = input.data();

	    if (tbb.atEnd())
                break;

            iFound = _tmp.count(regexp);
            if(!iFound) {
                iFound = _tmp.lastIndexOf("!");
                if (iFound >= 0 && ((l-iFound) < 5) ) {
                    int _i = tbb.pos();
                    tbb.seek((_i - iFound));
                }
            } else {
                ++count;
                endOfEmail = (tbb.pos() - l + _tmp.indexOf(regexp));
                offsets.append(endOfEmail);
            }
        }
        // info->addLog(i18n("--COUNTED: %1").arg(count));

        // IMPORT the messages:
        if(!offsets.empty() || (offsets.empty() && (tbb.size() > 3128))) {
            offsets.append(tbb.size());
            tbb.seek(3128);
            long lastPos = 3128;
            long endPos = 0;

            QString _path = "TheBat-Import/";
            QString _tmp = FileName;
            _tmp = _tmp.remove(_tmp.length() - 13, 13);
            _path += _tmp.remove( mailDir, Qt::CaseSensitive );
            QString _info = _path;
            info->addLog(i18n("Import folder %1...", _info.remove(0,14)));
            info->setTo(_path);
            info->setFrom("../" + _info + "/messages.tbb");

            for(QList<long>::Iterator it = offsets.begin() ; it != offsets.end() ; ++it) {
                if(info->shouldTerminate()) {
                    tbb.close();
                    return;
                }
                endPos = *it;
                QByteArray input(endPos-lastPos,'\0');
                tbb.read(input.data(), endPos-lastPos);

                KTemporaryFile tmp;
                tmp.open();
                tmp.write( input, endPos-lastPos );
                tmp.flush();

                //KMessageBox::warningContinueCancel(info->parent(), "");
                if(info->removeDupMsg)
                    addMessage( info, _path, tmp.fileName() );
                else
                    addMessage_fastImport( info, _path, tmp.fileName() );

                lastPos = endPos + 48;
                tbb.seek(lastPos);
                info->setCurrent( (int) ( ( (float) tbb.pos() / tbb.size() ) * 100 ));
            }

        }
    }
    tbb.close();
}
