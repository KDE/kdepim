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
#include <QPointer>
#include <ktemporaryfile.h>
#include <qdebug.h>

#include "filter_pmail.h"

using namespace MailImporter;

FilterPMail::FilterPMail() :
    Filter(i18n("Import Folders From Pegasus-Mail"),
           i18n("Holger Schurig <br>( rewritten by Danny Kukawka )"),
           i18n("<p>Select the Pegasus-Mail directory on your system (containing *.CNM, *.PMM and *.MBX files). "
                "On many systems this is stored in C:\\pmail\\mail or C:\\pmail\\mail\\admin</p>"
                "<p><b>Note:</b> Since it is possible to recreate the folder structure, the folders "
                "will be stored under: \"PegasusMail-Import\".</p>"))
{}

FilterPMail::~FilterPMail()
{
}

void FilterPMail::import()
{
    // Select directory from where I have to import files
    QPointer<KFileDialog> kfd = new KFileDialog( QDir::homePath(), QString(), 0 );
    kfd->setMode(KFile::Directory | KFile::LocalOnly);
    if (kfd->exec()) {
        const QString maildir = kfd->selectedFile();

        importMails( maildir );
    }
    delete kfd;
}

void FilterPMail::importMails( const QString  &chosenDir )
{
    setMailDir( chosenDir );
    if (mailDir().isEmpty()) {
        filterInfo()->alert(i18n("No directory selected."));
        return;
    }

    // Count total number of files to be processed
    filterInfo()->addInfoLogEntry(i18n("Counting files..."));
    dir.setPath (mailDir());
    const QStringList files = dir.entryList(QStringList()<<QLatin1String("*.[cC][nN][mM]")<<QLatin1String("*.[pP][mM][mM]")<<QLatin1String("*.[mM][bB][xX]"), QDir::Files, QDir::Name);
    totalFiles = files.count();
    currentFile = 0;
    qDebug() <<"Count is" << totalFiles;

    if(!(folderParsed = parseFolderMatrix(mailDir()))) {
        filterInfo()->addErrorLogEntry(i18n("Cannot parse the folder structure; continuing import without subfolder support."));
    }

    filterInfo()->addInfoLogEntry(i18n("Importing new mail files ('.cnm')..."));
    processFiles(QLatin1String("*.[cC][nN][mM]"), &FilterPMail::importNewMessage);
    filterInfo()->addInfoLogEntry(i18n("Importing mail folders ('.pmm')..."));
    processFiles(QLatin1String("*.[pP][mM][mM]"), &FilterPMail::importMailFolder);
    filterInfo()->addInfoLogEntry(i18n("Importing 'UNIX' mail folders ('.mbx')..."));
    processFiles(QLatin1String("*.[mM][bB][xX]"), &FilterPMail::importUnixMailFolder);

    filterInfo()->addInfoLogEntry( i18n("Finished importing emails from %1", mailDir() ));
    filterInfo()->setCurrent(100);
    filterInfo()->setOverall(100);
}

/** this looks for all files with the filemask 'mask' and calls the 'workFunc' on each of them */
void FilterPMail::processFiles(const QString &mask, void(FilterPMail::* workFunc)(const QString&) )
{
    if (filterInfo()->shouldTerminate())
        return;

    const QStringList files = dir.entryList(QStringList(mask), QDir::Files, QDir::Name);
    //qDebug() <<"Mask is" << mask <<" count is" << files.count();
    QStringList::ConstIterator end = files.constEnd();
    for ( QStringList::ConstIterator mailFile = files.constBegin(); mailFile != end; ++mailFile ) {
        // Notify current file
        QFileInfo mailfilem_filterInfoo(*mailFile);
        filterInfo()->setFrom(mailfilem_filterInfoo.fileName());

        // Clear the other fields
        filterInfo()->setTo(QString());
        filterInfo()->setCurrent(QString());
        filterInfo()->setCurrent(-1);

        // call worker function, increase progressbar
        (this->*workFunc)(dir.filePath(*mailFile));
        ++currentFile;
        filterInfo()->setOverall( (int) ((float) currentFile / totalFiles * 100));
        filterInfo()->setCurrent( 100 );
        if (filterInfo()->shouldTerminate()) return;
    }
}


/** this function imports one *.CNM message */
void FilterPMail::importNewMessage(const QString &file)
{
    QString destFolder(QLatin1String("PegasusMail-Import/New Messages"));
    filterInfo()->setTo(destFolder);

    /* comment by Danny Kukawka:
   * addMessage() == old function, need more time and check for duplicates
   * addMessage_fastImport == new function, faster and no check for duplicates
   */
    if(filterInfo()->removeDupMessage())
        addMessage( destFolder, file );
    else
        addMessage_fastImport( destFolder, file );
}


/** this function imports one mail folder file (*.PMM) */
void FilterPMail::importMailFolder(const QString &file)
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
        filterInfo()->alert(i18n("Unable to open %1, skipping", file));
    } else {
        // Get folder name
        l = f.read((char *) &pmm_head, sizeof(pmm_head));
        QString folder(i18nc("define folder name when we will import pegasus mail","PegasusMail-Import") + QLatin1Char('/'));
        if(folderParsed)
            folder.append(getFolderName(QString::fromLatin1(pmm_head.id)));
        else
            folder.append(QString::fromLatin1(pmm_head.folder));
        filterInfo()->setTo(folder);
        filterInfo()->addInfoLogEntry(i18n("Importing %1", QString::fromLatin1("../") + QString::fromLatin1(pmm_head.folder)));

        QByteArray input(MAX_LINE,'\0');
        bool first_msg = true;

        while (!f.atEnd()) {
            QTemporaryFile tempfile;
            tempfile.open();
            filterInfo()->setCurrent( (int) ( ( (float) f.pos() / f.size() ) * 100 ) );

            if(!first_msg) {
                // set the filepos back to last line minus the separate char (0x1a)
                f.seek(f.pos() - l + 1);
            }

            // no problem to loose the last line in file. This only contains a separate char
            while ( ! f.atEnd() && (l = f.readLine(input.data(),MAX_LINE))) {
                if (filterInfo()->shouldTerminate()){
                    return;
                }
                if ( input.at( 0 ) == 0x1a ) {
                    break;
                } else {
                    tempfile.write( input, l );
                }
            }
            tempfile.flush();

            if(filterInfo()->removeDupMessage())
                addMessage( folder, tempfile.fileName() );
            else
                addMessage_fastImport( folder, tempfile.fileName() );

            first_msg = false;
        }
    }
    f.close();
}


/** imports a 'unix' format mail folder (*.MBX) */
void FilterPMail::importUnixMailFolder(const QString &file)
{
    struct {
        char folder[58];
        char id[31];
    } pmg_head;

    QFile f;
    QString folder(QLatin1String("PegasusMail-Import/")), s(file), separate;
    QByteArray line(MAX_LINE,'\0');
    int n = 0, l = 0;

    /** Get the folder name */
    s.replace( QRegExp(QLatin1String("mbx$")), QLatin1String("pmg"));
    s.replace( QRegExp(QLatin1String("MBX$")), QLatin1String("PMG"));
    f.setFileName(s);
    if (! f.open( QIODevice::ReadOnly ) ) {
        filterInfo()->alert( i18n("Unable to open %1, skipping", s ) );
        return;
    } else {
        f.read((char *) &pmg_head, sizeof(pmg_head));
        f.close();

        if(folderParsed)
            folder.append(getFolderName(QString::fromLatin1(pmg_head.id)));
        else
            folder.append(QString::fromLatin1(pmg_head.folder));

        filterInfo()->setTo(folder);
        filterInfo()->setTo(folder);
    }

    /** Read in the mbox */
    f.setFileName(file);
    if (! f.open( QIODevice::ReadOnly ) ) {
        filterInfo()->alert( i18n("Unable to open %1, skipping", s ) );
    } else {
        filterInfo()->addInfoLogEntry(i18n("Importing %1", QLatin1String("../") + QString::fromLatin1(pmg_head.folder)));
        l = f.readLine( line.data(),MAX_LINE); // read the first line which is unneeded
        while ( ! f.atEnd() ) {
            QTemporaryFile tempfile;
            tempfile.open();

            // we lost the last line, which is the first line of the new message in
            // this lopp, but this is ok, because this is the separate line with
            // "From ???@???" and we can forget them
            while ( ! f.atEnd() && (l = f.readLine(line.data(),MAX_LINE)) &&((separate = QString::fromLatin1(line.data())).left(5) != QLatin1String("From "))) {
                tempfile.write(line.data(), l);
                if (filterInfo()->shouldTerminate()){
                    return;
                }
            }
            tempfile.flush();
            if(filterInfo()->removeDupMessage())
                addMessage( folder, tempfile.fileName() );
            else
                addMessage_fastImport( folder, tempfile.fileName() );

            n++;
            filterInfo()->setCurrent(i18n("Message %1", n));
            filterInfo()->setCurrent( (int) ( ( (float) f.pos() / f.size() ) * 100 ) );
        }
    }
    f.close();
}

/** Parse the m_filterInfoormation about folderstructure to folderMatrix */
bool FilterPMail::parseFolderMatrix( const QString  &chosendir )
{
    qDebug() <<"Start parsing the foldermatrix.";
    filterInfo()->addInfoLogEntry(i18n("Parsing the folder structure..."));

    QFile hierarch(chosendir + QLatin1String("/hierarch.pm"));
    if (! hierarch.open( QIODevice::ReadOnly ) ) {
        filterInfo()->alert( i18n("Unable to open %1, skipping",chosendir + QLatin1String("hierarch.pm") ) );
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
            QStringList tmpList = QString::fromLatin1(tmpRead).split(QLatin1Char(','), QString::SkipEmptyParts);
            int i = 0;
            QStringList::ConstIterator end( tmpList.constEnd() );
            for ( QStringList::ConstIterator it = tmpList.constBegin(); it != end; ++it, ++i) {
                QString _tmp = *it;
                if(i < 5) tmpArray[i] = _tmp.remove(QLatin1Char('\"'));
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
QString FilterPMail::getFolderName(const QString &ID)
{
    bool found = false;
    QString folder;
    QString search = ID;

    while (!found)
    {
        FolderStructureIterator end( folderMatrix.end() );
        for ( FolderStructureIterator it = folderMatrix.begin(); it != end; ++it) {
            FolderStructure tmp = *it;

            QString _ID = tmp[2];
            if(_ID == search) {
                QString _type = tmp[0] + tmp[1];
                if(( _type == QLatin1String("21"))) {
                    found = true;
                    break;
                }
                else {
                    folder.prepend((tmp[4] + QLatin1Char('/')));
                    search = tmp[3];
                }
            }
        }
    }
    return folder;
}
