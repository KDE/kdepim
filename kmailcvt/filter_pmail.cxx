/***************************************************************************
                          FilterPMail.cxx  -  Pegasus-Mail import
                             -------------------
    begin                : Sat Jan 6 2001
    copyright            : (C) 2001 by Holger Schurig
    email                : holgerschurig@gmx.de
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
   "Holger Schurig <p>( Filter accelerated by Danny Kukawka )</p>",
   i18n("<p>Select the Pegasus-Mail directory on your system (containing CNM, PMM and MBX files). "
              "On many systems this is stored in C:\\PMail\\mail</p>"
              "<p><b>Note:</b> This import filter will import your folders, but not "
              "the folder structure. But you will probably only do "
              "this one time.</p>"
              "<p><b>Note:</b> Emails will be imported into folder with the prefix pmail-</p>"))
{
}

FilterPMail::~FilterPMail()
{
   endImport();
}

void FilterPMail::import(FilterInfo *info)
{
   inf = info;

   // Select directory from where I have to import files
   QString chosenDir=KFileDialog::getExistingDirectory(QDir::homeDirPath(),info->parent());
   if (chosenDir.isEmpty()) {
      info->alert(i18n("No directory selected."));
      return;
   }

   // Count total number of files to be processed
   info->addLog(i18n("Counting files..."));
   dir.setPath (chosenDir);
   QStringList files = dir.entryList("*.[cC][nN][mM]; *.[pP][mM][mM]; *.[mM][bB][xX]", QDir::Files, QDir::Name);
   totalFiles = files.count();
   kdDebug() << "Count is " << totalFiles << endl;

   info->addLog(i18n("Importing new mail files ('.cnm')..."));
   processFiles("*.[cC][nN][mM]", &FilterPMail::importNewMessage);
   info->addLog(i18n("Importing mail folders ('.pmm')..."));
   processFiles("*.[pP][mM][mM]", &FilterPMail::importMailFolder);
   info->addLog(i18n("Importing 'UNIX' mail folders ('.mbx')..."));
   processFiles("*.[mM][bB][xX]", &FilterPMail::importUnixMailFolder);
}

/** this looks for all files with the filemask 'mask' and calls the 'workFunc' on each of them */
void FilterPMail::processFiles(const QString& mask, void(FilterPMail::* workFunc)(const QString&) )
{
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
      inf->addLog(i18n("Importing %1").arg(*mailFile));
      (this->*workFunc)(dir.filePath(*mailFile));
      currentFile++;
      inf->setOverall( 100 * currentFile / totalFiles );
   }
}


/** this function imports one *.CNM message */
void FilterPMail::importNewMessage(const QString& file)
{
   QString destFolder("PMail-New Messages");
   inf->setTo(destFolder);
   
   /* comment by Danny Kukawka:
    * addMessage() == old function, need more time and check for duplicates
    * addMessage_fastImport == new function, faster and no check for duplicates
    */
   if(inf->removeDupMsg) addMessage( inf, destFolder, file );
   else addMessage_fastImport( inf, destFolder, file );
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

   int ch = 0;
   int state = 0;
   int n = 0;
   KTempFile *tempfile = 0;

   // open the message
   QFile f(file);
   f.open(IO_ReadOnly);

   // Get folder name
   f.readBlock((char *) &pmm_head, sizeof(pmm_head));
   QString folder("PMail-");
   folder.append(pmm_head.folder);
   inf->setTo(folder);
   // The folder name might contain weird characters ...
   folder.replace(QRegExp("[^a-zA-Z0-9:.-]"), ":");

   // State machine to read the data in. The fgetc usage is probably terribly slow ...
   while ((ch = f.getch()) >= 0) {
      switch (state) {

      // new message state
      case 0:
         // open temp output file
         tempfile = new KTempFile;
         state = 1;
         inf->setCurrent(i18n("Message %1").arg(n++));
         // fall through

      // inside a message state
      case 1:
         if (ch == 0x1a) {
            // close file, send it
            tempfile->close();
            
	    if(inf->removeDupMsg) addMessage( inf, folder, tempfile->name() );
	    else addMessage_fastImport( inf, folder, tempfile->name() );
            
            tempfile->unlink();
            delete tempfile;
            state = 0;
            break;
         }
         if (ch == 0x0d) {
             break;
         }
         tempfile->file()->putch(ch);
         break;
      }
   }

   // did Folder end without 0x1a at the end?
   if (state != 0) {
      tempfile->close();
      if(inf->removeDupMsg) addMessage( inf, folder, tempfile->name() );
      else addMessage_fastImport( inf, folder, tempfile->name() );
      
      tempfile->unlink();
      delete tempfile;
   }

   f.close();
}


/** imports a 'unix' format mail folder (*.MBX) */
void FilterPMail::importUnixMailFolder(const QString& file)
{
   #define MAX_LINE 4096
   #define MSG_SEPERATOR_START "From "
   #define MSG_SEPERATOR_REGEX "^From .*..:...*$"
   static short msgSepLen = strlen(MSG_SEPERATOR_START);

   struct {
      char folder[58];
   } pmg_head;
   QFile f;
   QString s(file);
   QString folder;

   QCString line(MAX_LINE);
   KTempFile *tempfile = 0;
   int n = 0;
   QRegExp regexp(MSG_SEPERATOR_REGEX);


   // Get folder name
   s.replace( QRegExp("mbx$"), "pmg");
   s.replace( QRegExp("MBX$"), "PMG");
   f.setName(s);
   f.open(IO_ReadOnly);
   f.readBlock((char *) &pmg_head, sizeof(pmg_head));
   f.close();
   folder = "PMail-";
   folder.append(pmg_head.folder);
   inf->setTo(folder);
   // The folder name might contain weird characters ...
   folder.replace(QRegExp("[^a-zA-Z0-9:.-]"), ":");

   // Read in the folder
   //
   // Messages are separated by "From " lines. We use a logic similar
   // to KMail's kmfolder.cpp to find the boundaries.

   f.setName(file);
   f.open(IO_ReadOnly);
   while (f.readLine(line.data(), MAX_LINE)) {
      // Look for separator
      if (tempfile &&                                             // when we wrote to outfile
         (strncmp(line,MSG_SEPERATOR_START, msgSepLen)==0 &&  // quick compar
         regexp.search(line) >= 0))                            // slower regexp
      {
         tempfile->close();
         
	 if(inf->removeDupMsg) addMessage( inf, folder, tempfile->name() );
      	 else addMessage_fastImport( inf, folder, tempfile->name() );
	 
         tempfile->unlink();
         delete tempfile;
         tempfile = 0;
      }

      // Do we need to open/reopen output file?
      if (!tempfile) {
         tempfile = new KTempFile;
         // Notify progress
         n++;
         inf->setCurrent(i18n("Message %1").arg(n));
      }

      tempfile->file()->writeBlock(line.data(), line.length());
   }

   if (tempfile) {
      tempfile->close();
      
      if(inf->removeDupMsg) addMessage( inf, folder, tempfile->name() );
      else addMessage_fastImport( inf, folder, tempfile->name() );
      
      tempfile->unlink();
      delete tempfile;
   }

   f.close();
}

// vim: ts=2 sw=2 et
