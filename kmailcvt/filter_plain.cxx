/***************************************************************************
                          FilterPlain.cxx  -  Plain mail import
                             -------------------
    begin                : Fri Jun 14 2002
    copyright            : (C) 2002 by Laurence Anderson
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

#include <config.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kdirlister.h>
#include <qregexp.h>
#include <libgen.h>
#include <qdir.h>

#include "filter_plain.hxx"


FilterPlain::FilterPlain() :
   Filter(i18n("Import Plain Text Emails"),
               "Laurence Anderson",
              i18n("<p>Select the directory containing the emails on your system. "
              "The emails are placed in a folder with the same name as the "
              "directory they were in.</p>"
              "<p>This filter will import all .msg, .eml and .txt emails. "
              "If this causes problems for you (ie. you have KMail folders "
              "with that name, or there are some non-email txt files), "
              "cancel this import function (the next dialog "
              "will allow you to do that) and rename the existing KMail "
              "folders, or move the non-email txt files.</p>"))
{
}



FilterPlain::~FilterPlain()
{
}



void FilterPlain::import(FilterInfo *info)
{
   inf = info;

   // Select directory containing plain text emails
   mailDir = KFileDialog::getExistingDirectory(QDir::homeDirPath(),inf->parent());
   if (mailDir.isEmpty()) { // No directory selected
   	info->alert(i18n("No directory selected"));
   	return;
   }

   // Count total number of files to be processed
   inf->log(i18n("Counting files..."));
   totalFiles = countFiles("*.msg; *.eml; *.txt");
   currentFile = 0;

   inf->log(i18n("Importing new mail files..."));
   processFiles("*.msg; *.eml; *.txt");
}


/** counts all files which match filter in mail directory */
int FilterPlain::countFiles(QString filter)
{
   QDir dir (mailDir);
   QStringList files = dir.entryList(filter, QDir::Files, QDir::Name);
   return files.count();
}


/** process files that match filter */
void FilterPlain::processFiles(QString filter)
{
   QDir dir (mailDir);
   QStringList files = dir.entryList(filter, QDir::Files, QDir::Name);
   
   for ( QStringList::Iterator mailFile = files.begin(); mailFile != files.end(); ++mailFile ) {
	inf->from(i18n("From: %1").arg(*mailFile));
	inf->to(i18n("To: %1").arg(dir.dirName()));
	addMessage(inf, dir.dirName(), dir.filePath(*mailFile));
        
	inf->overall(100 * ++currentFile/ totalFiles);
   }
}
