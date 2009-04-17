/***************************************************************************
                          FilterPMail.cxx  -  Pegasus-Mail import
                             -------------------
    begin                : Sat Jan 6 2001
    copyright            : (C) 2001 by Holger Schurig <holgerschurig@gmx.de>
                           (C) 2005 by Danny Kukawka <danny.kukawka@web.de>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <klocale.h>
#include <kfiledialog.h>
#include <QRegExp>
#include <ktemporaryfile.h>
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
}

void FilterPMail::import(FilterInfo *info)
{
    inf = info;

    // Select directory from where I have to import files
    KFileDialog *kfd;
    kfd = new KFileDialog( QDir::homePath(), "", 0 );
    kfd->setMode(KFile::Directory | KFile::LocalOnly);
    kfd->exec();
    chosenDir = kfd->selectedFile();
    delete kfd;
    if (chosenDir.isEmpty()) {
        info->alert(i18n("No directory selected."));
        return;
    }

    // Count total number of files to be processed
    info->addLog(i18n("Counting files..."));
    dir.setPath (chosenDir);
    const QStringList files = dir.entryList(QStringList("*.[cC][nN][mM]")<<"*.[pP][mM][mM]"<<"*.[mM][bB][xX]", QDir::Files, QDir::Name);
    totalFiles = files.count();
    currentFile = 0;
    kDebug() <<"Count is" << totalFiles;

    if(!(folderParsed = parseFolderMatrix())) {
        info->addLog(i18n("Cannot parse the folder structure; continuing import without subfolder support."));
    }

    info->addLog(i18n("Importing new mail files ('.cnm')..."));
    processFiles("*.[cC][nN][mM]", &FilterPMail::importNewMessage);
    info->addLog(i18n("Importing mail folders ('.pmm')..."));
    processFiles("*.[pP][mM][mM]", &FilterPMail::importMailFolder);
    info->addLog(i18n("Importing 'UNIX' mail folders ('.mbx')..."));
    processFiles("*.[mM][bB][xX]", &FilterPMail::importUnixMailFolder);

    info->addLog( i18n("Finished importing emails from %1", chosenDir ));
    info->setCurrent(100);
    info->setOverall(100);
    delete kfd;
}

/** this looks for all files with the filemask 'mask' and calls the 'workFunc' on each of them */
void FilterPMail::processFiles(const QString& mask, void(FilterPMail::* workFunc)(const QString&) )
{
    if (inf->shouldTerminate()) return;

    const QStringList files = dir.entryList(QStringList(mask), QDir::Files, QDir::Name);
    //kDebug() <<"Mask is" << mask <<" count is" << files.count();
    for ( QStringList::ConstIterator mailFile = files.constBegin(); mailFile != files.constEnd(); ++mailFile ) {
        // Notify current file
        QFileInfo mailfileinfo(*mailFile);
        inf->setFrom(mailfileinfo.fileName());

        // Clear the other fields
        inf->setTo(QString());
        inf->setCurrent(QString());
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
    if (!f.open(QIODevice::ReadOnly)) {
        inf->alert(i18n("Unable to open %1, skipping", file));
    } else {
        // Get folder name
        l = f.read((char *) &pmm_head, sizeof(pmm_head));
        QString folder("PegasusMail-Import/");
        if(folderParsed)
            folder.append(getFolderName((QString)pmm_head.id));
        else
            folder.append(pmm_head.folder);
        inf->setTo(folder);
        inf->addLog(i18n("Importing %1", QString("../") + QString(pmm_head.folder)));

        QByteArray input(MAX_LINE,'\0');
        bool first_msg = true;

        while (!f.atEnd()) {
            KTemporaryFile tempfile;
            tempfile.open();
            inf->setCurrent( (int) ( ( (float) f.pos() / f.size() ) * 100 ) );

            if(!first_msg) {
                // set the filepos back to last line minus the separate char (0x1a)
                f.seek(f.pos() - l + 1);
            }

            // no problem to loose the last line in file. This only contains a separate char
            while ( ! f.atEnd() &&  (l = f.readLine(input.data(),MAX_LINE))) {
                    if (inf->shouldTerminate()){
                        return;
                    }
                    if ( input.at( 0 ) == 0x1a ) {
                        break;
                    } else {
                        tempfile.write( input, l );
                    }
            }
            tempfile.flush();

            if(inf->removeDupMsg)
                addMessage( inf, folder, tempfile.fileName() );
            else
                addMessage_fastImport( inf, folder, tempfile.fileName() );

            first_msg = false;
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
    QString folder("PegasusMail-Import/"), s(file), separate;
    QByteArray line(MAX_LINE,'\0');
    int n = 0, l = 0;

    /** Get the folder name */
    s.replace( QRegExp("mbx$"), "pmg");
    s.replace( QRegExp("MBX$"), "PMG");
    f.setFileName(s);
    if (! f.open( QIODevice::ReadOnly ) ) {
        inf->alert( i18n("Unable to open %1, skipping", s ) );
        return;
    } else {
        f.read((char *) &pmg_head, sizeof(pmg_head));
        f.close();

         if(folderParsed)
            folder.append(getFolderName((QString)pmg_head.id));
        else
            folder.append(pmg_head.folder);

        inf->setTo(folder);
        inf->setTo(folder);
    }

    /** Read in the mbox */
    f.setFileName(file);
    if (! f.open( QIODevice::ReadOnly ) ) {
        inf->alert( i18n("Unable to open %1, skipping", s ) );
    } else {
        inf->addLog(i18n("Importing %1", QString("../") + QString(pmg_head.folder)));
        l = f.readLine( line.data(),MAX_LINE); // read the first line which is unneeded
        while ( ! f.atEnd() ) {
            KTemporaryFile tempfile;
            tempfile.open();

            // we lost the last line, which is the first line of the new message in
            // this lopp, but this is ok, because this is the separate line with
            // "From ???@???" and we can forget them
            while ( ! f.atEnd() &&  (l = f.readLine(line.data(),MAX_LINE)) && ((separate = line.data()).left(5) != "From ")) {
                tempfile.write(line.data(), l);
                if (inf->shouldTerminate()){
                    return;
                }
            }
            tempfile.flush();
            if(inf->removeDupMsg)
                addMessage( inf, folder, tempfile.fileName() );
            else
                addMessage_fastImport( inf, folder, tempfile.fileName() );

            n++;
            inf->setCurrent(i18n("Message %1", n));
            inf->setCurrent( (int) ( ( (float) f.pos() / f.size() ) * 100 ) );
        }
    }
    f.close();
}

/** Parse the information about folderstructure to folderMatrix */
bool FilterPMail::parseFolderMatrix()
{
    kDebug() <<"Start parsing the foldermatrix.";
    inf->addLog(i18n("Parsing the folder structure..."));

    QFile hierarch(chosenDir + "/hierarch.pm");
    if (! hierarch.open( QIODevice::ReadOnly ) ) {
        inf->alert( i18n("Unable to open %1, skipping", chosenDir + "hierarch.pm" ) );
        return false;
    } else {
        QStringList tmpList;
        QByteArray tmpRead;
        while ( !hierarch.atEnd() ) {
            tmpRead = hierarch.readLine();
            if ( tmpRead.isEmpty() )
                break;
            QString tmpArray[5];
            tmpRead.remove(tmpRead.length() -2,2);
            QStringList tmpList = QString(tmpRead).split(',', QString::SkipEmptyParts);
            int i = 0;
            for ( QStringList::Iterator it = tmpList.begin(); it != tmpList.end(); ++it, i++) {
                QString _tmp = *it;
                if(i < 5) tmpArray[i] = _tmp.remove('\"');
                else {
                    hierarch.close();
                    return false;
                }
            }
            folderMatrix.append(FolderStructure(tmpArray));
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
        for ( FolderStructureIterator it = folderMatrix.begin(); it != folderMatrix.end(); ++it) {
            FolderStructure tmp = *it;

            QString _ID = tmp[2];
            if(_ID == search) {
                QString _type = tmp[0] + tmp[1];
                if(( _type == "21")) {
                    found = true;
                    break;
                }
                else {
                    folder.prepend((tmp[4] + '/'));
                    search = tmp[3];
                }
            }
        }
    }
    return folder;
}
