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
#include <klocale.h>
#include <kfiledialog.h>
#include <libgen.h>

#include "filter_plain.hxx"


FilterPlain::FilterPlain() :
   Filter(i18n("Import Plain Text Emails"),
               "Laurence Anderson",
              i18n("<p>Select the directory containing the emails on your system. "
              "The emails are placed in a folder with the same name as the "
              "directory they were in, prefixed by PLAIN-</p>"
              "<p>This filter will import all .msg, .eml and .txt emails.</p>"))
{
}

FilterPlain::~FilterPlain()
{
}

void FilterPlain::import(FilterInfo *info)
{
   // Select directory containing plain text emails
   QString mailDir = KFileDialog::getExistingDirectory(QDir::homeDirPath(),info->parent());
   if (mailDir.isEmpty()) { // No directory selected
   	info->alert(i18n("No directory selected"));
   	return;
   }
   QDir dir (mailDir);
   QStringList files = dir.entryList("*.[eE][mM][lL]; *.[tT][xX][tT]; *.[mM][sS][gG]", QDir::Files, QDir::Name);

   // Count total number of files to be processed
   info->addLog(i18n("Counting files..."));
   int totalFiles = files.count();
   int currentFile = 0;

   info->addLog(i18n("Importing new mail files..."));
   for ( QStringList::Iterator mailFile = files.begin(); mailFile != files.end(); ++mailFile ) {
      info->setFrom(*mailFile);
      info->setTo(dir.dirName());
      addMessage(info, "PLAIN-" + dir.dirName(), dir.filePath(*mailFile));
      info->setOverall(100 * ++currentFile/ totalFiles);
      if ( info->shouldTerminate() ) return;
   }
}
