/***************************************************************************
                  filter_opera.cxx  -  Outlook mail import
                             -------------------
    begin                : February 01  2005
    copyright            : (C) 2005 by Danny Kukawka
    email                : danny.kukawka@web.de
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
#include <ktempfile.h>
#include <kdebug.h>

#include "filter_outlook.hxx"

FilterOutlook::FilterOutlook() :
Filter(	i18n("Import Outlook Emails"),
    "Danny Kukawka",
    i18n("<p><b>Outlook email import filter</b></p>"
      "<p>This filter will import mails from a Outlook pst-file. You will need to locate "
      "the folder where the pst-file has been stored by searching for .pst files under: "
      "<i>C:\\Documents and Settings</i> in Windows 2000 or later</p>"
      "<p><b>Note:</b> Emails will be imported into a folder named after the "
      "account they came from, prefixed with OUTLOOK-</p>"
      ))
{
}

FilterOutlook::~FilterOutlook()
{ 
   endImport();
}

void FilterOutlook::import(FilterInfo *info)
{
  info->alert(i18n("No directory selected."));
  info->addLog(i18n("Counting files..."));
  info->addLog(i18n("Counting mails..."));
  info->addLog(i18n("Counting directorys..."));
  info->addLog(i18n("Counting folder..."));
  info->addLog(i18n("Importing new mail files..."));
  info->addLog(i18n("No file found for import."));
  
  QString outlookDir = "";
  outlookDir = KFileDialog::getExistingDirectory(QDir::homeDirPath(), info->parent());
  QDir importDir (outlookDir);
  QStringList files = importDir.entryList("*.[pP][sS][tT]", QDir::Files, QDir::Name);
  for ( QStringList::Iterator pstFile = files.begin(); pstFile != files.end(); ++pstFile)
  {
    info->addLog( i18n("Importing emails from %1...").arg( *pstFile ) );
    info->addLog( i18n("%1 Duplicate messages not imported").arg(count_duplicates));
    info->alert( i18n("Unable to open %1, skipping").arg( *pstFile ) );
  }
}
