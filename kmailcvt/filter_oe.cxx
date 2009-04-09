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

#include <config.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <ktempfile.h>
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
                     "<ul><li><i>C:\\Windows\\Application Data</i> in Windows 9x"
                     "<li><i>Documents and Settings</i> in Windows 2000 or later</ul></p>"
                     "<p><b>Note:</b> Since it is possible to recreate the folder structure, the folders from "
                      "Outlook Express 5 and 6 will be stored under: \"OE-Import\" in your local folder.</p>" ))
{}

FilterOE::~FilterOE()
{
}

void FilterOE::import(FilterInfo *info)
{
    // Select directory containing plain text emails
    mailDir = KFileDialog::getExistingDirectory(QDir::homeDirPath(),info->parent());
    if (mailDir.isEmpty()) { // No directory selected
        info->alert(i18n("No directory selected."));
        return;
    }

    QDir dir (mailDir);
    QStringList files = dir.entryList("*.[dDmM][bB][xX]", QDir::Files, QDir::Name);
    if (files.isEmpty()) {
        info->alert(i18n("No Outlook Express mailboxes found in directory %1.").arg(mailDir));
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
            files.remove(mailFile);
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

    kdDebug() << "\n" << "total emails in current file: " << totalEmails << endl;
    kdDebug() << "0x84 Mails: " << count0x84 << endl;
    kdDebug() << "0x04 Mails: " << count0x04 << endl;
}

void FilterOE::importMailBox( FilterInfo *info, const QString& fileName)
{
    QFile mailfile(fileName);
    QFileInfo mailfileinfo(fileName);
    QString _nameOfFile = fileName;
    _nameOfFile.remove( mailDir );
    _nameOfFile.remove( "/" );
    info->setFrom(mailfileinfo.fileName());
    
    if (!mailfile.open(IO_ReadOnly)) {
        info->addLog(i18n("Unable to open mailbox %1").arg(fileName));
        return;
    }
    QDataStream mailbox(&mailfile);
    mailbox.setByteOrder(QDataStream::LittleEndian);

    // Parse magic
    Q_UINT32 sig_block1, sig_block2;
    mailbox >> sig_block1 >> sig_block2;
    if (sig_block1 == OE4_SIG_1 && sig_block2 == OE4_SIG_2) {
        folderName = "OE-Import/" + mailfileinfo.baseName(TRUE);
        info->addLog(i18n("Importing OE4 Mailbox %1").arg( "../" + _nameOfFile));
        info->setTo(folderName);
        mbxImport(info, mailbox);
        return;
    } else {
        Q_UINT32 sig_block3, sig_block4;
        mailbox >> sig_block3 >> sig_block4;
        if (sig_block1 == OE5_SIG_1 && sig_block3 == OE5_SIG_3 && sig_block4 == OE5_SIG_4) {
            if (sig_block2 == OE5_EMAIL_SIG_2) {
                folderName = "OE-Import/" + mailfileinfo.baseName(TRUE);
                if(parsedFolder) {
                    QString _tmpFolder = getFolderName(_nameOfFile);
                    if(!_tmpFolder.isEmpty()) folderName = "OE-Import/" + _tmpFolder;
                }
                info->addLog(i18n("Importing OE5+ Mailbox %1").arg( "../" + _nameOfFile));
                info->setTo(folderName);
                dbxImport(info, mailbox);
                return;
            } else if (sig_block2 == OE5_FOLDER_SIG_2) {
                if(!parsedFolder) {
                    info->addLog(i18n("Importing OE5+ Folder file %1").arg( "../" + _nameOfFile));
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
    Q_UINT32 msgCount, lastMsgNum, fileSize;

    // Read the header
    ds >> msgCount >> lastMsgNum >> fileSize;
    ds.device()->at( ds.device()->at() + 64 ); // Skip 0's
    kdDebug() << "This mailbox has " << msgCount << " messages" << endl;
    if (msgCount == 0)
        return; // Don't import empty mailbox

    Q_UINT32 msgMagic;
    ds >> msgMagic; // Read first magic

    while (!ds.atEnd()) {
        Q_UINT32 msgNumber, msgSize, msgTextSize;
        KTempFile tmp;
        tmp.dataStream()->setByteOrder(QDataStream::LittleEndian);

        // Read the messages
        ds >> msgNumber >> msgSize >> msgTextSize; // All seem to be lies...?

        do {
            ds >> msgMagic;
            if (msgMagic != MBX_MAILMAGIC)
                *tmp.dataStream() << msgMagic;
            else
                break;
        } while ( !ds.atEnd() );

        tmp.close();
        /* comment by Danny Kukawka:
         * addMessage() == old function, need more time and check for duplicates
         * addMessage_fastImport == new function, faster and no check for duplicates
         */
        if(info->removeDupMsg)
            addMessage( info, folderName, tmp.name() );
        else
            addMessage_fastImport( info, folderName, tmp.name() );

        tmp.unlink();
        if(info->shouldTerminate()) return;
    }
}

/* ------------------- DBX support ------------------- */

void FilterOE::dbxImport( FilterInfo *info, QDataStream& ds)
{
    // Get item count & offset of index
    Q_UINT32 itemCount, indexPtr;
    ds.device()->at(0xc4);
    ds >> itemCount;
    ds.device()->at(0xe4);
    ds >> indexPtr;
    kdDebug() << "Item count is " << itemCount << ", Index at " << indexPtr << endl;

    if (itemCount == 0)
        return; // Empty file
    totalEmails = itemCount;
    currentEmail = 0;
    // Parse the indexes
    ds.device()->at(indexPtr);
    dbxReadIndex(info, ds, indexPtr);
}

void FilterOE::dbxReadIndex( FilterInfo *info, QDataStream& ds, int filePos)
{
    
    if(info->shouldTerminate()) return;
    Q_UINT32 self, unknown, nextIndexPtr, parent, indexCount;
    Q_UINT8 unknown2, ptrCount;
    Q_UINT16 unknown3;
    int wasAt = ds.device()->at();
    ds.device()->at(filePos);


    kdDebug() << "Reading index of file " << folderName << endl;
    ds >> self >> unknown >> nextIndexPtr >> parent >> unknown2 >> ptrCount >> unknown3 >> indexCount; // _dbx_tableindexstruct

    kdDebug() << "This index has " << (int) ptrCount << " data pointers" << endl;
    for (int count = 0; count < ptrCount; count++) {
        if(info->shouldTerminate()) return;
        Q_UINT32 dataIndexPtr, anotherIndexPtr, anotherIndexCount; // _dbx_indexstruct
        ds >> dataIndexPtr >> anotherIndexPtr >> anotherIndexCount;

        if (anotherIndexCount > 0) {
            kdDebug() << "Recursing to another table @ " << anotherIndexPtr << endl;
            dbxReadIndex(info, ds, anotherIndexPtr);
        }
        kdDebug() << "Data index @ " << dataIndexPtr << endl;
        dbxReadDataBlock(info, ds, dataIndexPtr);
    }

    if (indexCount > 0) { // deal with nextTablePtr
        kdDebug() << "Recuring to next table @ " << nextIndexPtr << endl;
        dbxReadIndex(info, ds, nextIndexPtr);
    }

    ds.device()->at(wasAt); // Restore file position to same as when function called
}

void FilterOE::dbxReadDataBlock( FilterInfo *info, QDataStream& ds, int filePos)
{
    Q_UINT32 curOffset, blockSize;
    Q_UINT16 unknown;
    Q_UINT8 count, unknown2;
    int wasAt = ds.device()->at();
    
    QString folderEntry[4];
    
    ds.device()->at(filePos);

    ds >> curOffset >> blockSize >> unknown >> count >> unknown2; // _dbx_email_headerstruct
    kdDebug() << "Data block has " << (int) count << " elements" << endl;

    for (int c = 0; c < count; c++) {
        if(info->shouldTerminate()) return;
        Q_UINT8 type;  // _dbx_email_pointerstruct
        Q_UINT32 value; // Actually 24 bit

        ds >> type >> value;
        value &= 0xffffff;
        ds.device()->at(ds.device()->at() - 1); // We only wanted 3 bytes

        if(!currentIsFolderFile) {
            if (type == 0x84) { // It's an email!
                kdDebug() << "**** Offset of emaildata (0x84) " << value << " ****" << endl;
                dbxReadEmail(info, ds, value);
                ++count0x84;
            } else if( type == 0x04) {
                int currentFilePos = ds.device()->at();
                ds.device()->at(filePos + 12 + value + (count*4) );
                Q_UINT32 newOFF;
                ds >> newOFF;
                kdDebug() << "**** Offset of emaildata (0x04) " <<  newOFF << endl;
                ds.device()->at(currentFilePos);
                dbxReadEmail(info, ds, newOFF);
                ++count0x04;
            }
        }
        else {
            // this is a folderfile
            if(type == 0x02) {
                // kdDebug() << "**** FOLDER: descriptive name ****" << endl;
                folderEntry[0] = parseFolderString(ds, filePos + 12 + value + (count*4) );
            } else if (type == 0x03) {
                // kdDebug() << "**** FOLDER: filename ****" << endl;
                folderEntry[1] = parseFolderString(ds, filePos + 12 + value + (count*4) );
                
            } else if (type == 0x80) {
                // kdDebug() << "**** FOLDER: current ID ****" << endl;
                folderEntry[2] = QString::number(value); 
                
            } else if (type == 0x81) {
                // kdDebug() << "**** FOLDER: parent ID ****" << endl;
                folderEntry[3] =  QString::number(value);
            }
        }
    }
    if(currentIsFolderFile) {
        folderStructure.append(folderEntry);
    }
    ds.device()->at(wasAt); // Restore file position to same as when function called
}

void FilterOE::dbxReadEmail( FilterInfo *info, QDataStream& ds, int filePos)
{
    if(info->shouldTerminate()) return;
    Q_UINT32 self, nextAddressOffset, nextAddress=0;
    Q_UINT16 blockSize;
    Q_UINT8 intCount, unknown;
    KTempFile tmp;
    bool _break = false;
    int wasAt = ds.device()->at();
    ds.device()->at(filePos);

    do {
        ds >> self >> nextAddressOffset >> blockSize >> intCount >> unknown >> nextAddress; // _dbx_block_hdrstruct
        QByteArray blockBuffer(blockSize);
        ds.readRawBytes(blockBuffer.data(), blockSize);
        tmp.dataStream()->writeRawBytes(blockBuffer.data(), blockSize);
        // to detect incomplete mails or corrupted archives. See Bug #86119
        if(ds.atEnd()) {
            _break = true;
            break;
        }
        ds.device()->at(nextAddress);
    } while (nextAddress != 0);
    tmp.close();

    if(!_break) {
        if(info->removeDupMsg)
            addMessage( info, folderName, tmp.name() );
        else
            addMessage_fastImport( info, folderName, tmp.name() );

        currentEmail++;
        int currentPercentage = (int) ( ( (float) currentEmail / totalEmails ) * 100 );
        info->setCurrent(currentPercentage);
        ds.device()->at(wasAt);
    }
    tmp.unlink();
}

/* ------------------- FolderFile support ------------------- */
QString FilterOE::parseFolderString( QDataStream& ds, int filePos )
{
    char tmp;
    QString returnString;
    int wasAt = ds.device()->at();
    ds.device()->at(filePos);
    
    // read while != 0x00
    while( !ds.device()->atEnd() ) {
        tmp = ds.device()->getch();
        if( tmp != 0x00) {
            returnString += tmp;
        }
        else break;
    }
    ds.device()->at(wasAt);
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
    QString search = filename.lower();
    
    while (!found)
    {
        for ( FolderStructureIterator it = folderStructure.begin(); it != folderStructure.end(); it++) {
            FolderStructure tmp = *it;
            if(foundFilename == false) {
                QString _tmpFileName = tmp[1];
                _tmpFileName = _tmpFileName.lower();
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
