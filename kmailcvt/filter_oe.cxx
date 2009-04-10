/***************************************************************************
                          filter_oe.cxx  -  Outlook Express mail import
                             -------------------
    begin                : Sat Feb 1 2003
    copyright            : (C) 2003 by Laurence Anderson
                           (C) 2005 by Danny Kukawka
    email                : l.d.anderson@warwick.ac.uk
                           danny.Kukawka@web.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// This filter was created by looking at libdbx & liboe

#include <klocale.h>
#include <kfiledialog.h>
#include <ktemporaryfile.h>
#include <kdebug.h>

#include "filter_oe.hxx"

#define OE4_SIG_1 0x36464d4a
#define OE4_SIG_2 0x00010003
#define OE5_SIG_1 0xfe12adcf
#define OE5_EMAIL_SIG_2 0x6f74fdc5
#define OE5_FOLDER_SIG_2 0x6f74fdc6
#define OE5_SIG_3 0x11d1e366
#define OE5_SIG_4 0xc0004e9a
#define MBX_MAILMAGIC 0x7F007F00

FilterOE::FilterOE() :
        Filter( i18n("Import Outlook Express Emails"),
                "Laurence Anderson <br>( Filter enhanced by Danny Kukawka )</p>",
                i18n("<p><b>Outlook Express 4/5/6 import filter</b></p>"
                     "<p>You will need to locate the folder where the mailbox has been "
                     "stored by searching for .dbx or .mbx files under "
                     "<ul><li><i>C:\\Windows\\Application Data</i> in Windows 9x</li>"
                     "<li><i>Documents and Settings</i> in Windows 2000 or later</li></ul></p>"
                     "<p><b>Note:</b> Since it is possible to recreate the folder structure, the folders from "
                      "Outlook Express 5 and 6 will be stored under: \"OE-Import\" in your local folder.</p>" ))
{}

FilterOE::~FilterOE()
{
}

void FilterOE::import(FilterInfo *info)
{
    // Select directory containing plain text emails
    mailDir = KFileDialog::getExistingDirectory(QDir::homePath(),info->parent());
    if (mailDir.isEmpty()) { // No directory selected
        info->alert(i18n("No directory selected."));
        return;
    }

    QDir dir (mailDir);
    QStringList files = dir.entryList(QStringList("*.[dDmM][bB][xX]"), QDir::Files, QDir::Name);
    if (files.isEmpty()) {
        info->alert(i18n("No Outlook Express mailboxes found in directory %1.", mailDir));
        return;
    }

    totalFiles = files.count();
    currentFile = 0;
    count0x04 = 0;
    count0x84 = 0;
    parsedFolder = false;

    info->setOverall(0);

    /** search the folderfile to recreate folder struct */
    for ( QStringList::Iterator mailFile = files.begin(); mailFile != files.end(); ++mailFile ) {
        if(*mailFile == "Folders.dbx") {
            info->addLog(i18n("Import folder structure..."));
            importMailBox(info, dir.filePath(*mailFile));
            if(!folderStructure.isEmpty()) parsedFolder = true;
            // remove file from QStringList::files, no longer needed
            files.erase(mailFile);
            currentIsFolderFile = false;
            break;
        }
    }

    int n=0;
    for ( QStringList::Iterator mailFile = files.begin(); mailFile != files.end(); ++mailFile ) {
        if ( info->shouldTerminate() ) break;
        importMailBox(info, dir.filePath(*mailFile));
        info->setOverall(100 * ++n  / files.count());
    }

    info->setOverall(100);
    info->setCurrent(100);
    info->addLog(i18n("Finished importing Outlook Express emails"));
    if (info->shouldTerminate()) info->addLog( i18n("Finished import, canceled by user."));

    kDebug() <<"total emails in current file:" << totalEmails;
    kDebug() <<"0x84 Mails:" << count0x84;
    kDebug() <<"0x04 Mails:" << count0x04;
}

void FilterOE::importMailBox( FilterInfo *info, const QString& fileName)
{
    QFile mailfile(fileName);
    QFileInfo mailfileinfo(fileName);
    QString _nameOfFile = fileName;
    _nameOfFile.remove( mailDir );
    _nameOfFile.remove( '/' );
    info->setFrom(mailfileinfo.fileName());

    if (!mailfile.open(QIODevice::ReadOnly)) {
        info->addLog(i18n("Unable to open mailbox %1", fileName));
        return;
    }
    QDataStream mailbox(&mailfile);
    mailbox.setByteOrder(QDataStream::LittleEndian);

    // Parse magic
    quint32 sig_block1, sig_block2;
    mailbox >> sig_block1 >> sig_block2;
    if (sig_block1 == OE4_SIG_1 && sig_block2 == OE4_SIG_2) {
        folderName = "OE-Import/" + mailfileinfo.completeBaseName();
        info->addLog(i18n("Importing OE4 Mailbox %1", QString("../") + _nameOfFile));
        info->setTo(folderName);
        mbxImport(info, mailbox);
        return;
    } else {
        quint32 sig_block3, sig_block4;
        mailbox >> sig_block3 >> sig_block4;
        if (sig_block1 == OE5_SIG_1 && sig_block3 == OE5_SIG_3 && sig_block4 == OE5_SIG_4) {
            if (sig_block2 == OE5_EMAIL_SIG_2) {
                folderName = "OE-Import/" + mailfileinfo.completeBaseName();
                if(parsedFolder) {
                    QString _tmpFolder = getFolderName(_nameOfFile);
                    if(!_tmpFolder.isEmpty()) folderName = "OE-Import/" + _tmpFolder;
                }
                info->addLog(i18n("Importing OE5+ Mailbox %1", QString("../") + _nameOfFile));
                info->setTo(folderName);
                dbxImport(info, mailbox);
                return;
            } else if (sig_block2 == OE5_FOLDER_SIG_2) {
                if(!parsedFolder) {
                    info->addLog(i18n("Importing OE5+ Folder file %1", QString("../") + _nameOfFile));
                    currentIsFolderFile = true;
                    dbxImport(info, mailbox);
                    currentIsFolderFile = false;
                }
                return;
            }
        }
    }
    // info->addLog(i18n("File %1 does not seem to be an Outlook Express mailbox").arg("../" + _nameOfFile));
}

/* ------------------- MBX support ------------------- */

void FilterOE::mbxImport( FilterInfo *info, QDataStream& ds)
{
    quint32 msgCount, lastMsgNum, fileSize;

    // Read the header
    ds >> msgCount >> lastMsgNum >> fileSize;
    ds.device()->seek( ds.device()->pos() + 64 ); // Skip 0's
    kDebug() <<"This mailbox has" << msgCount <<" messages";
    if (msgCount == 0)
        return; // Don't import empty mailbox

    quint32 msgMagic;
    ds >> msgMagic; // Read first magic

    while (!ds.atEnd()) {
        quint32 msgNumber, msgSize, msgTextSize;
        KTemporaryFile tmp;
        tmp.open();
        QDataStream ds(&tmp);
        ds.setByteOrder(QDataStream::LittleEndian);

        // Read the messages
        ds >> msgNumber >> msgSize >> msgTextSize; // All seem to be lies...?

        do {
            ds >> msgMagic;
            if (msgMagic != MBX_MAILMAGIC)
                ds << msgMagic;
            else
                break;
        } while ( !ds.atEnd() );

        tmp.flush();
        /* comment by Danny Kukawka:
         * addMessage() == old function, need more time and check for duplicates
         * addMessage_fastImport == new function, faster and no check for duplicates
         */
        if(info->removeDupMsg)
            addMessage( info, folderName, tmp.fileName() );
        else
            addMessage_fastImport( info, folderName, tmp.fileName() );

        if(info->shouldTerminate()) return;
    }
}

/* ------------------- DBX support ------------------- */

void FilterOE::dbxImport( FilterInfo *info, QDataStream& ds)
{
    // Get item count & offset of index
    quint32 itemCount, indexPtr;
    ds.device()->seek(0xc4);
    ds >> itemCount;
    ds.device()->seek(0xe4);
    ds >> indexPtr;
    kDebug() <<"Item count is" << itemCount <<", Index at" << indexPtr;

    if (itemCount == 0)
        return; // Empty file
    totalEmails = itemCount;
    currentEmail = 0;
    // Parse the indexes
    ds.device()->seek(indexPtr);
    dbxReadIndex(info, ds, indexPtr);
}

void FilterOE::dbxReadIndex( FilterInfo *info, QDataStream& ds, int filePos)
{

    if(info->shouldTerminate()) return;
    quint32 self, unknown, nextIndexPtr, parent, indexCount;
    quint8 unknown2, ptrCount;
    quint16 unknown3;
    int wasAt = ds.device()->pos();
    ds.device()->seek(filePos);


    kDebug() <<"Reading index of file" << folderName;
    ds >> self >> unknown >> nextIndexPtr >> parent >> unknown2 >> ptrCount >> unknown3 >> indexCount; // _dbx_tableindexstruct

    kDebug() <<"This index has" << (int) ptrCount <<" data pointers";
    for (int count = 0; count < ptrCount; count++) {
        if(info->shouldTerminate()) return;
        quint32 dataIndexPtr, anotherIndexPtr, anotherIndexCount; // _dbx_indexstruct
        ds >> dataIndexPtr >> anotherIndexPtr >> anotherIndexCount;

        if (anotherIndexCount > 0) {
            kDebug() <<"Recursing to another table @" << anotherIndexPtr;
            dbxReadIndex(info, ds, anotherIndexPtr);
        }
        kDebug() <<"Data index @" << dataIndexPtr;
        dbxReadDataBlock(info, ds, dataIndexPtr);
    }

    if (indexCount > 0) { // deal with nextTablePtr
        kDebug() <<"Recuring to next table @" << nextIndexPtr;
        dbxReadIndex(info, ds, nextIndexPtr);
    }

    ds.device()->seek(wasAt); // Restore file position to same as when function called
}

void FilterOE::dbxReadDataBlock( FilterInfo *info, QDataStream& ds, int filePos)
{
    quint32 curOffset, blockSize;
    quint16 unknown;
    quint8 count, unknown2;
    int wasAt = ds.device()->pos();

    QString folderEntry[4];

    ds.device()->seek(filePos);

    ds >> curOffset >> blockSize >> unknown >> count >> unknown2; // _dbx_email_headerstruct
    kDebug() <<"Data block has" << (int) count <<" elements";

    for (int c = 0; c < count; c++) {
        if(info->shouldTerminate()) return;
        quint8 type;  // _dbx_email_pointerstruct
        quint32 value; // Actually 24 bit

        ds >> type >> value;
        value &= 0xffffff;
        ds.device()->seek(ds.device()->pos() - 1); // We only wanted 3 bytes

        if(!currentIsFolderFile) {
            if (type == 0x84) { // It's an email!
                kDebug() <<"**** Offset of emaildata (0x84)" << value <<" ****";
                dbxReadEmail(info, ds, value);
                ++count0x84;
            } else if( type == 0x04) {
                int currentFilePos = ds.device()->pos();
                ds.device()->seek(filePos + 12 + value + (count*4) );
                quint32 newOFF;
                ds >> newOFF;
                kDebug() <<"**** Offset of emaildata (0x04)" <<  newOFF;
                ds.device()->seek(currentFilePos);
                dbxReadEmail(info, ds, newOFF);
                ++count0x04;
            }
        }
        else {
            // this is a folderfile
            if(type == 0x02) {
                // kDebug() <<"**** FOLDER: descriptive name ****";
                folderEntry[0] = parseFolderString(ds, filePos + 12 + value + (count*4) );
            } else if (type == 0x03) {
                // kDebug() <<"**** FOLDER: filename ****";
                folderEntry[1] = parseFolderString(ds, filePos + 12 + value + (count*4) );

            } else if (type == 0x80) {
                // kDebug() <<"**** FOLDER: current ID ****";
                folderEntry[2] = QString::number(value);

            } else if (type == 0x81) {
                // kDebug() <<"**** FOLDER: parent ID ****";
                folderEntry[3] =  QString::number(value);
            }
        }
    }
    if(currentIsFolderFile) {
        folderStructure.append(FolderStructure(folderEntry));
    }
    ds.device()->seek(wasAt); // Restore file position to same as when function called
}

void FilterOE::dbxReadEmail( FilterInfo *info, QDataStream& ds, int filePos)
{
    if(info->shouldTerminate()) return;
    quint32 self, nextAddressOffset, nextAddress=0;
    quint16 blockSize;
    quint8 intCount, unknown;
    KTemporaryFile tmp;
    tmp.open();
    bool _break = false;
    int wasAt = ds.device()->pos();
    ds.device()->seek(filePos);
    QDataStream tempDs (&tmp);

    do {
        ds >> self >> nextAddressOffset >> blockSize >> intCount >> unknown >> nextAddress; // _dbx_block_hdrstruct
        QByteArray blockBuffer(blockSize,'\0');
        ds.readRawData(blockBuffer.data(), blockSize);
        tempDs.writeRawData(blockBuffer.data(), blockSize);
        // to detect incomplete mails or corrupted archives. See Bug #86119
        if(ds.atEnd()) {
            _break = true;
            break;
        }
        ds.device()->seek(nextAddress);
    } while (nextAddress != 0);
    tmp.flush();

    if(!_break) {
        if(info->removeDupMsg)
            addMessage( info, folderName, tmp.fileName() );
        else
            addMessage_fastImport( info, folderName, tmp.fileName() );

        currentEmail++;
        int currentPercentage = (int) ( ( (float) currentEmail / totalEmails ) * 100 );
        info->setCurrent(currentPercentage);
        ds.device()->seek(wasAt);
    }
}

/* ------------------- FolderFile support ------------------- */
QString FilterOE::parseFolderString( QDataStream& ds, int filePos )
{
    char tmp;
    QString returnString;
    int wasAt = ds.device()->pos();
    ds.device()->seek(filePos);

    // read while != 0x00
    while( !ds.device()->atEnd() ) {
        ds.device()->getChar(&tmp);
        if( tmp != 0x00) {
            returnString += tmp;
        }
        else break;
    }
    ds.device()->seek(wasAt);
    return returnString;
}

/** get the foldername for a given file ID from folderMatrix */
QString FilterOE::getFolderName(QString filename)
{
    bool found = false;
    bool foundFilename = false;
    QString folder;
    // we must do this because folder with more than one upper letter
    // at start have maybe not a file named like the folder !!!
    QString search = filename.toLower();

    while (!found)
    {
        for ( FolderStructureIterator it = folderStructure.begin(); it != folderStructure.end(); it++) {
            FolderStructure tmp = *it;
            if(foundFilename == false) {
                QString _tmpFileName = tmp[1];
                _tmpFileName = _tmpFileName.toLower();
                if(_tmpFileName == search) {
                    folder.prepend( tmp[0] + QString::fromLatin1("/") );
                    search = tmp[3];
                    foundFilename = true;
                }
            } else {
                QString _currentID = tmp[2];
                QString _parentID = tmp[3];
                if(_currentID == search) {
                    if(_parentID.isEmpty()) { // this is the root of the folder
                        found = true;
                        break;
                    } else {
                        folder.prepend( tmp[0] + QString::fromLatin1("/") );
                        search = tmp[3];
                    }
                }
            }
        }
        // need to break the while loop maybe in some cases
        if((foundFilename == false) && (folder.isEmpty())) return folder;
    }
    return folder;
}
