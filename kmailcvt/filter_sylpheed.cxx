/***************************************************************************
            filter_sylpheed.hxx  -  Sylpheed maildir mail import
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

#include "filter_sylpheed.hxx"

#include <config.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kdebug.h>

/** Default constructor. */
FilterSylpheed::FilterSylpheed( void ) :
        Filter( i18n( "Import Sylpheed Maildirs and Folder Structure" ),
                "Danny Kukawka",
                i18n( "<p><b>Sylpheed import filter</b></p>"
                      "<p>Select the base directory of the Sylpheed mailfolder you want to import "
                      "(usually: ~/Mail ).</p>"
                      "<p>Since it is possible to recreate the folder structure, the folders "
                      "will be stored under: \"Sylpheed-Import\" in your local folder.</p>" 
                      "<p>This filter also recreates the status of message, e.g. new or forwarded.") )
{}

/** Destructor. */
FilterSylpheed::~FilterSylpheed( void )
{
}

/** Recursive import of Sylpheed maildir. */
void FilterSylpheed::import( FilterInfo *info )
{

    QString _homeDir = QDir::homeDirPath();

    KFileDialog *kfd;
    kfd = new KFileDialog( _homeDir, "", 0, "kfiledialog", true );
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
    else if ( mailDir == QDir::homeDirPath() || mailDir == ( QDir::homeDirPath() + "/" ) ) {
        info->addLog( i18n( "No files found for import." ) );
    } else {
        info->setOverall(0);

        /** Recursive import of the MailFolders */
        QDir dir(mailDir);
        QStringList rootSubDirs = dir.entryList("[^\\.]*", QDir::Dirs , QDir::Name);
        int currentDir = 1, numSubDirs = rootSubDirs.size();
        for(QStringList::Iterator filename = rootSubDirs.begin() ; filename != rootSubDirs.end() ; ++filename, ++currentDir) {
            if(info->shouldTerminate()) break;
            importDirContents(info, dir.filePath(*filename));
            info->setOverall((int) ((float) currentDir / numSubDirs * 100));
        }
    }

    info->addLog( i18n("Finished importing emails from %1").arg( mailDir ));
    if (count_duplicates > 0) {
        info->addLog( i18n("1 duplicate message not imported", "%n duplicate messages not imported", count_duplicates));
    }
    if (info->shouldTerminate()) info->addLog( i18n("Finished import, canceled by user."));
    count_duplicates = 0;
    info->setCurrent(100);
    info->setOverall(100);
}

/**
 * Import of a directory contents.
 * @param info Information storage for the operation.
 * @param dirName The name of the directory to import.
 */
void FilterSylpheed::importDirContents( FilterInfo *info, const QString& dirName)
{
    if(info->shouldTerminate()) return;
    
    /** Here Import all archives in the current dir */
    importFiles(info, dirName);

    /** If there are subfolders, we import them one by one */
    QDir subfolders(dirName);
    QStringList subDirs = subfolders.entryList("[^\\.]*", QDir::Dirs , QDir::Name);
    for(QStringList::Iterator filename = subDirs.begin() ; filename != subDirs.end() ; ++filename) {
        if(info->shouldTerminate()) return;
        importDirContents(info, subfolders.filePath(*filename));
    }
}


/**
 * Import the files within a Folder.
 * @param info Information storage for the operation.
 * @param dirName The name of the directory to import.
 */
void FilterSylpheed::importFiles( FilterInfo *info, const QString& dirName)
{
    QDir dir(dirName);
    QString _path;
    bool generatedPath = false;

    QDict<unsigned long> msgflags;
    msgflags.setAutoDelete(true);

    QDir importDir (dirName);
    QStringList files = importDir.entryList("[^\\.]*", QDir::Files, QDir::Name);
    int currentFile = 1, numFiles = files.size();
    
    readMarkFile(info, dir.filePath(".sylpheed_mark"), msgflags);
    
    for ( QStringList::Iterator mailFile = files.begin(); mailFile != files.end(); ++mailFile, ++currentFile) {
        if(info->shouldTerminate()) return;
        QString _mfile = *mailFile;
        if (!(_mfile.endsWith(".sylpheed_cache") || _mfile.endsWith(".sylpheed_mark") || _mfile.endsWith(".mh_sequences") )) {
            if(!generatedPath) {
                _path = "Sylpheed-Import/";
                QString _tmp = dir.filePath(*mailFile);
                _tmp = _tmp.remove(_tmp.length() - _mfile.length() -1, _mfile.length()+1);
                _path += _tmp.remove( mailDir ,TRUE);
                QString _info = _path;
                info->addLog(i18n("Import folder %1...").arg(_info.remove(0,15)));

                info->setFrom(_info);
                info->setTo(_path);
                generatedPath = true;
            }

            QString flags;
            if (msgflags[_mfile])
                flags = msgFlagsToString(*(msgflags[_mfile]));
             
            if(info->removeDupMsg) {
                if(! addMessage( info, _path, dir.filePath(*mailFile), flags )) {
                    info->addLog( i18n("Could not import %1").arg( *mailFile ) );
                }
                info->setCurrent((int) ((float) currentFile / numFiles * 100));
            } else {
                if(! addMessage_fastImport( info, _path, dir.filePath(*mailFile), flags )) {
                    info->addLog( i18n("Could not import %1").arg( *mailFile ) );
                }
                info->setCurrent((int) ((float) currentFile / numFiles * 100));
            }
        }
    }
}


void FilterSylpheed::readMarkFile( FilterInfo *info, const QString &path, QDict<unsigned long> &dict )
{
    /* Each sylpheed mail directory contains a .sylpheed_mark file which
     * contains all the flags for each messages. The layout of this file
     * is documented in the source code of sylpheed: in procmsg.h for
     * the flag bits, and procmsg.c.
     *
     * Note that the mark file stores 32 bit unsigned integers in the
     * platform's native "endianness". 
     *
     * The mark file starts with a 32 bit unsigned integer with a version
     * number. It is then followed by pairs of 32 bit unsigned integers,
     * the first one with the message file name (which is a number), 
     * and the second one with the actual message flags */

    Q_UINT32 in, flags;
    QFile file(path);

    if (!file.open(IO_ReadOnly)) 
        return;
    
    QDataStream stream(&file);

    if (Q_BYTE_ORDER == Q_LITTLE_ENDIAN)
        stream.setByteOrder(QDataStream::LittleEndian);



    /* Read version; if the value is reasonably too big, we're looking
     * at a file created on another platform. I don't have any test 
     * marks/folders, so just ignoring this case */
    stream >> in;
    if (in > (Q_UINT32) 0xffff) 
        return;

    while (!stream.atEnd()) {
        if(info->shouldTerminate()){
            file.close();
            return;
        }
        stream >> in;
        stream >> flags;
        QString s;
        s.setNum((uint) in);
        dict.insert(s, new unsigned long(flags));
    }
}

QString FilterSylpheed::msgFlagsToString(unsigned long flags)
{
    QString status;
    
    /* see sylpheed's procmsg.h */
    if (flags & 1UL) status += 'N';
    if (flags & 2UL) status += 'U';
    if ((flags & 3UL) == 0UL) status += 'R';
    if (flags & 8UL) status += 'D';
    if (flags & 16UL) status += 'A';
    if (flags & 32UL) status += 'F';
    
    return status;
}
