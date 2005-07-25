/***************************************************************************
                          FilterPMail.cxx  -  Pegasus-Mail import
                             -------------------
    begin                : Sat Jan 6 2001
    copyright            : (C) 2001 by Holger Schurig
                           (C) 2005 by Danny Kukawka
    email                : holgerschurig@gmx.de
                           danny.kukawka@web.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <config.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <qregexp.h>
#include <ktempfile.h>
#include <kdebug.h>

#include "filter_pmail.hxx"


FilterPMail::FilterPMail() :
        Filter(i18n("Import Folders From Pegasus-Mail"),
               "Holger Schurig <br>( rewritten by Danny Kukawka )",
               i18n("<p>Select the Pegasus-Mail directory on your system (containing *.CNM, *.PMM and *.MBX files). "
                    "On many systems this is stored in C:\\pmail\\mail or C:\\pmail\\mail\\admin</p>"
                    "<p><b>Note:</b> Since it is possible to recreate the folder structure, the folders "
                    "will be stored under: \"PegasusMail-Import\".</p>"))
{}

FilterPMail::~FilterPMail()
{
    endImport();
}

void FilterPMail::import(FilterInfo *info)
{
    inf = info;
    
    // Select directory from where I have to import files
    KFileDialog *kfd;
    kfd = new KFileDialog( QDir::homeDirPath(), "", 0, "kfiledialog", true );
    kfd->setMode(KFile::Directory | KFile::LocalOnly);
    kfd->exec();
    chosenDir = kfd->selectedFile();

    if (chosenDir.isEmpty()) {
        info->alert(i18n("No directory selected."));
        return;
    }

    // Count total number of files to be processed
    info->addLog(i18n("Counting files..."));
    dir.setPath (chosenDir);
    QStringList files = dir.entryList("*.[cC][nN][mM]; *.[pP][mM][mM]; *.[mM][bB][xX]", QDir::Files, QDir::Name);
    totalFiles = files.count();
    currentFile = 0;
    kdDebug() << "Count is " << totalFiles << endl;
    
    if(!(folderParsed = parseFolderMatrix())) {
        info->addLog(i18n("Cannot parse the folder structure; continuing import without subfolder support."));
    }

    info->addLog(i18n("Importing new mail files ('.cnm')..."));
    processFiles("*.[cC][nN][mM]", &FilterPMail::importNewMessage);
    info->addLog(i18n("Importing mail folders ('.pmm')..."));
    processFiles("*.[pP][mM][mM]", &FilterPMail::importMailFolder);
    info->addLog(i18n("Importing 'UNIX' mail folders ('.mbx')..."));
    processFiles("*.[mM][bB][xX]", &FilterPMail::importUnixMailFolder);

    info->addLog( i18n("Finished importing emails from %1").arg( chosenDir ));
    info->setCurrent(100);
    info->setOverall(100);
}

/** this looks for all files with the filemask 'mask' and calls the 'workFunc' on each of them */
void FilterPMail::processFiles(const QString& mask, void(FilterPMail::* workFunc)(const QString&) )
{
    if (inf->shouldTerminate()) return;
    
    QStringList files = dir.entryList(mask, QDir::Files, QDir::Name);
    //kdDebug() << "Mask is " << mask << " count is " << files.count() << endl;
    for ( QStringList::Iterator mailFile = files.begin(); mailFile != files.end(); ++mailFile ) {
        // Notify current file
        QFileInfo mailfileinfo(*mailFile);
        inf->setFrom(mailfileinfo.fileName());

        // Clear the other fields
        inf->setTo(QString::null);
        inf->setCurrent(QString::null);
        inf->setCurrent(-1);

        // call worker function, increase progressbar
        (this->*workFunc)(dir.filePath(*mailFile));
        ++currentFile;
        inf->setOverall( (int) ((float) currentFile / totalFiles * 100));
        inf->setCurrent( 100 );
        if (inf->shouldTerminate()) return;
    }
}


/** this function imports one *.CNM message */
void FilterPMail::importNewMessage(const QString& file)
{
    QString destFolder("PegasusMail-Import/New Messages");
    inf->setTo(destFolder);

    /* comment by Danny Kukawka:
     * addMessage() == old function, need more time and check for duplicates
     * addMessage_fastImport == new function, faster and no check for duplicates
     */
    if(inf->removeDupMsg)
        addMessage( inf, destFolder, file );
    else
        addMessage_fastImport( inf, destFolder, file );
}


/** this function imports one mail folder file (*.PMM) */
void FilterPMail::importMailFolder(const QString& file)
{
    // Format of a PMM file:
    // First comes a header with 128 bytes. At the beginning is the name of
    // the folder. Then there are some unknown bytes (strings). At offset 128
    // the first message starts.
    //
    // Each message is terminated by a 0x1A byte. The next message follows
    // immediately.
    //
    // The last message is followed by a 0x1A, too.
    //
    // 000000 6d 61 69 6c 73 65 72 76 65 72 2d 70 72 6f 6a 65    mailserver-proje
    // 000010 63 74 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ct..............
    // 000020 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
    // 000030 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
    // 000040 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
    // 000050 00 00 00 00 00 00 36 30 34 37 35 37 32 45 3a 36    ......6047572E:6
    // 000060 46 34 39 3a 46 4f 4c 30 31 33 35 35 00 00 00 00    F49:FOL01355....
    // 000070 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
    // 000080 52 65 74 75 72 6e 2d 50 61 74 68 3a 20 3c 75 72    Return-Path: <ur
    // ...
    // 000cb0 2d 2d 2d 2d 2d 2d 2d 2d 2d 2b 0d 0a 1a 52 65 74    ---------+...Ret
    // 000cc0 75 72 6e 2d 50 61 74 68 3a 20 3c 62 6f 75 6e 63    urn-Path: <bounc
    // ...
    // 04dc50 46 30 33 38 44 2e 36 31 35 44 37 34 44 30 2d 2d    F038D.615D74D0--
    // 04dc60 0d 0a 1a

    struct {
        char folder[86];
        char id[42];
    } pmm_head;
    
    long l = 0;
    QFile f(file);
    if (!f.open(IO_ReadOnly)) {
        inf->alert(i18n("Unable to open %1, skipping").arg(file));
    } else {
        // Get folder name
        l = f.readBlock((char *) &pmm_head, sizeof(pmm_head));
        QString folder("PegasusMail-Import/");
        if(folderParsed) 
            folder.append(getFolderName((QString)pmm_head.id));
        else 
            folder.append(pmm_head.folder);
        inf->setTo(folder);
        inf->addLog(i18n("Importing %1").arg("../" + QString(pmm_head.folder)));
        
        QByteArray input(MAX_LINE);
        bool first_msg = true;
        
        while (!f.atEnd()) {
            KTempFile tempfile;
            inf->setCurrent( (int) ( ( (float) f.at() / f.size() ) * 100 ) );
            
            if(!first_msg) {
                // set the filepos back to last line minus the seperate char (0x1a)
                f.at(f.at() - l + 1); 
            }
            
            // no problem to loose the last line in file. This only contains a seperate char
            while ( ! f.atEnd() &&  (l = f.readLine(input.data(),MAX_LINE))) {
                    if (inf->shouldTerminate()){
                        tempfile.close();
                        tempfile.unlink();
                        return;
                    }
                    if(input[0] == 0x1a ) {
                        break;
                    } else {
                        tempfile.file()->writeBlock( input, l );
                    }
            }
            tempfile.close();
            
            if(inf->removeDupMsg)
                addMessage( inf, folder, tempfile.name() );
            else
                addMessage_fastImport( inf, folder, tempfile.name() );
            
            first_msg = false;
            tempfile.unlink();
        }
    }
    f.close();
}


/** imports a 'unix' format mail folder (*.MBX) */
void FilterPMail::importUnixMailFolder(const QString& file)
{
    struct {
        char folder[58];
        char id[31];
    } pmg_head;
    
    QFile f;
    QString folder("PegasusMail-Import/"), s(file), seperate;
    QByteArray line(MAX_LINE);
    int n = 0, l = 0;

    /** Get the folder name */
    s.replace( QRegExp("mbx$"), "pmg");
    s.replace( QRegExp("MBX$"), "PMG");
    f.setName(s);
    if (! f.open( IO_ReadOnly ) ) {
        inf->alert( i18n("Unable to open %1, skipping").arg( s ) );
        return;
    } else {
        f.readBlock((char *) &pmg_head, sizeof(pmg_head));
        f.close();
        
         if(folderParsed) 
            folder.append(getFolderName((QString)pmg_head.id));
        else 
            folder.append(pmg_head.folder);
        
        inf->setTo(folder);
        inf->setTo(folder);
    }
    
    /** Read in the mbox */
    f.setName(file);
    if (! f.open( IO_ReadOnly ) ) {
        inf->alert( i18n("Unable to open %1, skipping").arg( s ) );
    } else {
        inf->addLog(i18n("Importing %1").arg("../" + QString(pmg_head.folder)));
        l = f.readLine( line.data(),MAX_LINE); // read the first line which is unneeded
        while ( ! f.atEnd() ) {
            KTempFile tempfile;
            
            // we lost the last line, which is the first line of the new message in
            // this lopp, but this is ok, because this is the seperate line with
            // "From ???@???" and we can forget them
            while ( ! f.atEnd() &&  (l = f.readLine(line.data(),MAX_LINE)) && ((seperate = line.data()).left(5) != "From ")) {
                tempfile.file()->writeBlock(line.data(), l);
                if (inf->shouldTerminate()){
                    tempfile.close();
                    tempfile.unlink();
                    return;
                }
            }
            tempfile.close();
            if(inf->removeDupMsg)
                addMessage( inf, folder, tempfile.name() );
            else
                addMessage_fastImport( inf, folder, tempfile.name() );
            tempfile.unlink();   
            
            n++;
            inf->setCurrent(i18n("Message %1").arg(n));            
            inf->setCurrent( (int) ( ( (float) f.at() / f.size() ) * 100 ) );
        }
    }    
    f.close();
}

/** Parse the information about folderstructure to folderMatrix */
bool FilterPMail::parseFolderMatrix() 
{
    kdDebug() << "Start parsing the foldermatrix." << endl;
    inf->addLog(i18n("Parsing the folder structure..."));
    
    QFile hierarch(chosenDir + "/hierarch.pm");
    if (! hierarch.open( IO_ReadOnly ) ) {
        inf->alert( i18n("Unable to open %1, skipping").arg( chosenDir + "hierarch.pm" ) );
        return false;
    } else {
        QStringList tmpList;
        QString tmpRead;
        while ( !hierarch.atEnd() &&  hierarch.readLine(tmpRead,100)) {
            QString tmpArray[5];
            tmpRead.remove(tmpRead.length() -2,2);
            QStringList tmpList = QStringList::split(",", tmpRead, false);
            int i = 0;
            for ( QStringList::Iterator it = tmpList.begin(); it != tmpList.end(); ++it, i++) {
                QString _tmp = *it;
                if(i < 5) tmpArray[i] = _tmp.remove("\"");
                else {
                    hierarch.close();
                    return false; 
                }
            } 
            folderMatrix.append(tmpArray);
        }
    }
    hierarch.close();
    return true;
}

/** get the foldername for a given file ID from folderMatrix */
QString FilterPMail::getFolderName(QString ID) 
{
    bool found = false;
    QString folder;
    QString search = ID;
    
    while (!found)
    {
        for ( FolderStructureIterator it = folderMatrix.begin(); it != folderMatrix.end(); it++) {
            FolderStructure tmp = *it;
            
            QString _ID = tmp[2];
            if(_ID == search) {
                QString _type = tmp[0] + tmp[1];
                if(( _type == "21")) {
                    found = true;
                    break;
                }
                else {
                    folder.prepend((tmp[4] + "/"));
                    search = tmp[3];
                }
            }
        }  
    }
    return folder;
}
