/***************************************************************************
                          filter_oe.cxx  -  Outlook Express mail import
                             -------------------
    begin                : Sat Feb 1 2003
    copyright            : (C) 2003 by Laurence Anderson
    email                : l.d.anderson@warwick.ac.uk
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
Filter(	i18n("Import Outlook Express Emails"),
    "Laurence Anderson <p>( Filter accelerated by Danny Kukawka )</p>",
    i18n("<p><b>Outlook Express 4/5/6 import filter</b></p>"
      "<p>You will need to locate the folder where the mailbox has been stored by searching for .dbx or .mbx files under "
      "<ul><li><i>C:\\Windows\\Application Data</i> in Windows 9x"
      "<li><i>Documents and Settings</i> in Windows 2000 or later</ul></p>"
      "<p><b>Note:</b> Emails will be imported into folders named after the file they came from, prefixed with OE-</p>"
      ))
{
}

FilterOE::~FilterOE()
{
   endImport();
}

void FilterOE::import(FilterInfo *info)
{
  inf = info;

  // Select directory containing plain text emails
  QString mailDir = KFileDialog::getExistingDirectory(QDir::homeDirPath(),inf->parent());
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

  inf->setOverall(0);

  int n=0;
  for ( QStringList::Iterator mailFile = files.begin(); mailFile != files.end(); ++mailFile ) {
    importMailBox(dir.filePath(*mailFile));
    inf->setOverall(100 * ++n  / files.count());
  }

  inf->setOverall(100);
  inf->setCurrent(100);
  inf->addLog(i18n("Finished importing Outlook Express emails"));
}

void FilterOE::importMailBox(const QString& fileName)
{
  QFile mailfile(fileName);
  QFileInfo mailfileinfo(fileName);
  folderName = "OE-" + mailfileinfo.baseName(TRUE);
  inf->setFrom(mailfileinfo.fileName());
  inf->setTo(folderName);

  if (!mailfile.open(IO_ReadOnly)) {
    inf->addLog(i18n("Unable to open mailbox %1").arg(fileName));
    return;
  }
  QDataStream mailbox(&mailfile);
  mailbox.setByteOrder(QDataStream::LittleEndian);

  // Parse magic
  Q_UINT32 sig_block1, sig_block2;
  mailbox >> sig_block1 >> sig_block2;
  if (sig_block1 == OE4_SIG_1 && sig_block2 == OE4_SIG_2) {
    inf->addLog(i18n("Importing OE4 Mailbox %1").arg(fileName));
    mbxImport(mailbox);
    return;
  } else {
    Q_UINT32 sig_block3, sig_block4;
    mailbox >> sig_block3 >> sig_block4;
    if (sig_block1 == OE5_SIG_1 && sig_block3 == OE5_SIG_3 && sig_block4 == OE5_SIG_4) {
      if (sig_block2 == OE5_EMAIL_SIG_2) {
        inf->addLog(i18n("Importing OE5+ Mailbox %1").arg(fileName));
        dbxImport(mailbox);
        return;
      } else if (sig_block2 == OE5_FOLDER_SIG_2) {
        inf->addLog(i18n("Ignoring OE5+ Folder file %1").arg(fileName));
        return;
      }
    }
  }
  inf->addLog(i18n("File %1 does not seem to be an Outlook Express mailbox").arg(fileName));
}

/* ------------------- MBX support ------------------- */

void FilterOE::mbxImport(QDataStream& ds)
{
  Q_UINT32 msgCount, lastMsgNum, fileSize;

  // Read the header
  ds >> msgCount >> lastMsgNum >> fileSize;
  ds.device()->at( ds.device()->at() + 64 ); // Skip 0's
  kdDebug() << "This mailbox has " << msgCount << " messages" << endl;
  if (msgCount == 0) return; // Don't import empty mailbox

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
      if (msgMagic != MBX_MAILMAGIC) *tmp.dataStream() << msgMagic;
      else break;
    } while ( !ds.atEnd() );

    tmp.close();
    /* comment by Danny Kukawka:
     * addMessage() == old function, need more time and check for duplicates
     * addMessage_fastImport == new function, faster and no check for duplicates
     */
    if(inf->removeDupMsg) addMessage( inf, folderName, tmp.name() );
    else addMessage_fastImport( inf, folderName, tmp.name() );
    
    tmp.unlink();
  }
}

/* ------------------- DBX support ------------------- */

void FilterOE::dbxImport(QDataStream& ds)
{
  // Get item count & offset of index
  Q_UINT32 itemCount, indexPtr;
  ds.device()->at(0xc4);
  ds >> itemCount;
  ds.device()->at(0xe4);
  ds >> indexPtr;
  kdDebug() << "Item count is " << itemCount << ", Index at " << indexPtr << endl;

  if (itemCount == 0) return; // Empty file
  totalEmails = itemCount;
  currentEmail = 0;
  // Parse the indexes
  ds.device()->at(indexPtr);
  dbxReadIndex(ds, indexPtr);
}

void FilterOE::dbxReadIndex(QDataStream& ds, int filePos)
{
  Q_UINT32 self, unknown, nextIndexPtr, parent, indexCount;
  Q_UINT8 unknown2, ptrCount;
  Q_UINT16 unknown3;
  int wasAt = ds.device()->at();
  ds.device()->at(filePos);

  kdDebug() << "Reading index of file " << folderName << endl;
  ds >> self >> unknown >> nextIndexPtr >> parent >> unknown2 >> ptrCount >> unknown3 >> indexCount; // _dbx_tableindexstruct
  if (indexCount > 0) { // deal with nextTablePtr
    kdDebug() << "Recuring to next table @ " << nextIndexPtr << endl;
    dbxReadIndex(ds, nextIndexPtr);
  }

  kdDebug() << "This index has " << (int) ptrCount << " data pointers" << endl;
  for (int count = 0; count < ptrCount; count++) {
    Q_UINT32 dataIndexPtr, anotherIndexPtr, anotherIndexCount; // _dbx_indexstruct
    ds >> dataIndexPtr >> anotherIndexPtr >> anotherIndexCount;

    if (anotherIndexCount > 0) {
      kdDebug() << "Recursing to another table @ " << anotherIndexPtr << endl;
      dbxReadIndex(ds, anotherIndexPtr);
    }
    kdDebug() << "Data index @ " << dataIndexPtr << endl;
    dbxReadDataBlock(ds, dataIndexPtr);
  }
  ds.device()->at(wasAt); // Restore file position to same as when function called
}

void FilterOE::dbxReadDataBlock(QDataStream& ds, int filePos)
{
  Q_UINT32 curOffset, blockSize;
  Q_UINT16 unknown;
  Q_UINT8 count, unknown2;
  int wasAt = ds.device()->at();
  ds.device()->at(filePos);

  ds >> curOffset >> blockSize >> unknown >> count >> unknown2; // _dbx_email_headerstruct
  kdDebug() << "Data block has " << (int) count << " elements" << endl;

  for (int c = 0; c < count; c++) {
    Q_UINT8 type;  // _dbx_email_pointerstruct
    Q_UINT32 value; // Actually 24 bit

    ds >> type >> value;
    value &= 0xffffff;
    ds.device()->at(ds.device()->at() - 1); // We only wanted 3 bytes

    if (type == 0x84) { // It's an email!
      kdDebug() << "**** Offset of emaildata " << value << " ****" << endl;
      dbxReadEmail(ds, value);
    }
  }
  ds.device()->at(wasAt); // Restore file position to same as when function called
}

void FilterOE::dbxReadEmail(QDataStream& ds, int filePos)
{
  Q_UINT32 self, nextAddressOffset, nextAddress=0;
  Q_UINT16 blockSize;
  Q_UINT8 intCount, unknown;
  KTempFile tmp;
  int wasAt = ds.device()->at();
  ds.device()->at(filePos);

  do {
    ds >> self >> nextAddressOffset >> blockSize >> intCount >> unknown >> nextAddress; // _dbx_block_hdrstruct
    QByteArray blockBuffer(blockSize);
    ds.readRawBytes(blockBuffer.data(), blockSize);
    tmp.dataStream()->writeRawBytes(blockBuffer.data(), blockSize);
    ds.device()->at(nextAddress);
  } while (nextAddress != 0);
  tmp.close();
  
  if(inf->removeDupMsg) addMessage( inf, folderName, tmp.name() );
  else addMessage_fastImport( inf, folderName, tmp.name() );
  
  inf->setCurrent( ++currentEmail / totalEmails * 100);
  tmp.unlink();

  ds.device()->at(wasAt);
}

// vim: ts=2 sw=2 et
