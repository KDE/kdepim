/***************************************************************************
                          filter_pmail.cxx  -  Pegasus-Mail import
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
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kdirlister.h>
#include <qregexp.h>

#include "filter_pmail.hxx"


filter_pmail::filter_pmail() :
   filter(i18n("Import folders from Pegasus-Mail (*.CNM, *.PMM, *.MBX)"),"Holger Schurig")
{
  CAP=i18n("Import Pegasus-Mail");
}



filter_pmail::~filter_pmail()
{
}



void filter_pmail::import(filterInfo *info)
{
   QString  choosen;
   QString  msg;

   if (!kmailStart(info)) { return; }
   inf = info;
   par = info->parent();

   msg = i18n("Select the Pegasus-Mail directory on your system.\n\n"
              "This import filter will import your folders, but not\n"
              "the folder structure. But you'll probably only do\n"
              "this one time ;-).\n\n"
              "NOTE: Kmailcvt creates folders with the prefix 'pmail-'.\n"
              "If this causes trouble to you (you've got kmail folders\n"
              "with that prefix) cancel this import function (next dialog\n"
              "will let you do that and rename the existing kmail\n"
              "folders."
             );
   info->alert(CAP,msg);

   // Select directory from where I have to import files
   sprintf(dir,getenv("HOME"));
   choosen=KFileDialog::getExistingDirectory(dir,par);
   if (choosen.length()==0) { return; } // No directory choosen here!
   strcpy(dir,choosen.latin1());

   // Count total number of files to be processed
   info->log(i18n("Couting files ..."));
   totalFiles = countFiles(".cnm");
   totalFiles += countFiles(".pmm");
   totalFiles += countFiles(".mbx");

   //msg=i18n("Searching for distribution lists ('.pml') ...");
   //info->log(msg);
   //totalFiles += countFiles(dir, "*.pml");

   if (!kmailStart(info))
      return;
   info->log(i18n("Importing new mail files ('.cnm') ..."));
   processFiles(".cnm", &importNewMessage);
   info->log(i18n("Importing mail folders ('.pmm') ..."));
   processFiles(".pmm", &importMailFolder);
   info->log(i18n("Importing 'unix' mail folders ('.mbx') ..."));
   processFiles(".mbx", &importUnixMailFolder);
   kmailStop(info);
}


/** counts all files with mask (e.g. '*.cnm') in
in a directory */
int filter_pmail::countFiles(const char *mask)
{
   DIR *d;
   struct dirent *entry;
   d = opendir(dir);
   int n = 0;

   entry=readdir(d);
   while (entry!=NULL) {
      char *file=entry->d_name;
      if (strlen(file)>4 && strcasecmp(&file[strlen(file)-4],mask)==0)
         n++;
      entry=readdir(d);
   }
   closedir(d);

   return n;
}


/** updates currentFile and the progress bar */
void filter_pmail::nextFile()
{
   float perc;

   currentFile++;
   perc=(((float) currentFile)/((float) totalFiles))*100.0;
   inf->overall(perc);
}


/** this looks for all files with the filemask 'mask' and calls the 'workFunc' on each of them */
void filter_pmail::processFiles(const char *mask, void(filter_pmail::* workFunc)(const char*) )
{
   DIR *d;
   struct dirent *entry;
   d = opendir(dir);

   entry=readdir(d);
   while (entry!=NULL) {
      char *file=entry->d_name;
      if (strlen(file)>4 && strcasecmp(&file[strlen(file)-4],mask)==0) {

         // Notify current file
         QString msg;
         msg = i18n("From")+": "+file;
         inf->from(msg);

         // Clear the other fields
         msg = "";
         inf->to(msg);
         inf->current(msg);
         inf->current(-1);

         // combine dir and filename into a path
         QString path = dir;
         path.append("/");
         path.append(file);

         // call worker function, increase progressbar
         (this->*workFunc)(path.latin1());
         nextFile();
      }
      entry=readdir(d);
   }
   closedir(d);
}


/** this function imports one *.CNM message */
void filter_pmail::importNewMessage(const char *file)
{
   unsigned long added;
   const char* destFolder = "PMail-New Messages";
   QString msg;

   msg = i18n("To")+": "+destFolder;
   inf->to(msg);

   kmailMessage((filterInfo *) inf, (char *)destFolder, (char *)file, added);
}


/** this function imports one mail folder file (*.PMM) */
void filter_pmail::importMailFolder(const char *file)
{
   struct {
      char folder[86];
      char id[42];
   } pmm_head;

   FILE *f;
   QString msg;
   QString folder;
   int ch = 0;
   int state = 0;
   int n = 0;
   FILE *temp = NULL;
   char tempname[128];
   unsigned long added;


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

   // open the message
   f = fopen(file, "rb");

   // Get folder name
   fread(&pmm_head, sizeof(pmm_head), 1, f);
   folder = "PMail-";
   folder.append(pmm_head.folder);
   msg = i18n("To")+": "+folder;
   inf->to(msg);
   // The folder name might contain weird characters ...
   folder.replace(QRegExp("[^a-zA-Z0-9:.-]"), ":");

   // State machine to read the data in. The fgetc usage is probably terribly slow ...
   while ((ch = fgetc(f)) >= 0) {
      switch (state) {

      // new message state
      case 0:
         // open temp output file

         // XXX this is UGLY. Some people have the stuff in /var/tmp ...
         // However, KTempFile is also ugly, because it does not return
         // the full path of the created file (and creates it in the
         // current directory) :-(
         snprintf(tempname, sizeof(tempname), "/tmp/pmail_pmm.%d",getpid());
         temp = fopen(tempname,"wb");
         state = 1;
         n++;
         msg.sprintf("Message %d", n);
         inf->current(msg);
         // fall throught

      // inside a message state
      case 1:
         if (ch == 0x1a) {
            // close file, send it
            fclose(temp);
            kmailMessage((filterInfo *) inf, (char *)folder.latin1(), tempname, added);
            unlink(tempname);
            state = 0;
            break;
         }
         if (ch == 0x0d) {
             break;
         }
         fputc(ch, temp);
         break;
      }
   }

   // did Folder end without 0x1a at the end?
   if (state != 0) {
      fclose(temp);
      kmailMessage((filterInfo *) inf, (char *)folder.latin1(), tempname, added);
      unlink(tempname);
   }

   fclose(f);
}


/** imports a 'unix' format mail folder (*.MBX) */
void filter_pmail::importUnixMailFolder(const char *file)
{
   #define MAX_LINE 4096
   #define MSG_SEPERATOR_START "From "
   #define MSG_SEPERATOR_REGEX "^From .*..:...*$"
   static short msgSepLen = strlen(MSG_SEPERATOR_START);

   struct {
      char folder[58];
   } pmg_head;
   FILE *f;
   QString s(file);
   QString folder;

   char line[MAX_LINE];
   FILE *temp = NULL;
   char tempname[128];
   int n = 0;
   QRegExp regexp(MSG_SEPERATOR_REGEX);

   unsigned long added;

   // Get folder name
   s.replace( QRegExp("mbx$"), "pmg");
   s.replace( QRegExp("MBX$"), "PMG");
   f = fopen(s.latin1(), "rb");
   fread(&pmg_head, sizeof(pmg_head), 1, f);
   fclose(f);
   folder = "PMail-";
   folder.append(pmg_head.folder);
   inf->to(i18n("To")+": "+folder);
   // The folder name might contain weird characters ...
   folder.replace(QRegExp("[^a-zA-Z0-9:.-]"), ":");

   // Read in the folder
   //
   // Messages are separated by "From " lines. We use a logic similar
   // to KMail's kmfolder.cpp to find the boundaries.

   snprintf(tempname, sizeof(tempname), "/tmp/pmail_mbx.%d",getpid());
   f = fopen(file, "rb");
   while (fgets(line, MAX_LINE, f)) {
      // Look for separator
      if (temp &&                                             // when we wrote to outfile
         (strncmp(line,MSG_SEPERATOR_START, msgSepLen)==0 &&  // quick compar
         regexp.match(line) >= 0))                            // slower regexp
      {
         fclose(temp);
         kmailMessage((filterInfo *) inf, (char *)folder.latin1(), tempname, added);
         unlink(tempname);
         temp = NULL;
      }

      // Do we need to open/reopen output file?
      if (!temp) {
         // XXX again, direct usage of '/tmp' is ugly, but again KTempFile is not good
         // enought for us -- as of early Januar 2001
         temp = fopen(tempname,"w");
         // Notify progress
         n++;
         s.sprintf("Message %d", n);
         inf->current(s);
      }

      fputs(line, temp);
   }

   if (temp) {
      fclose(temp);
      kmailMessage((filterInfo *) inf, (char *)folder.latin1(), tempname, added);
      unlink(tempname);
   }

   fclose(f);
}
