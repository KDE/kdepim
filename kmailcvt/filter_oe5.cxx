/***************************************************************************
                          FilterOE5.cxx  -  description
                             -------------------
    begin                : Fri Jun 30 2000
    copyright            : (C) 2000 by Hans Dijkema
    email                : kmailcvt@hum.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "filter_oe5.hxx"
#include "oe5_2mbox.h"

#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>

#include <qdir.h>

#include <klocale.h>
#include <kfiledialog.h>

FilterOE5::FilterOE5() : Filter(i18n("Import Folders From Outlook Express 5/6"),
                                     "Stephan B. Nedregard/Hans Dijkema",
      i18n("<p>Select the Outlook Express directory on your system. "
      "This import filter will search for folders (the '.mbx' files).</p>"
      "<p><b>NOTE:</b> You will not be able to revert to your original folder "
      "structure, only the folders themselves are imported. But you will "
      "probably only do this one time.</p>"
      "<p><b>NOTE:</b> The folder names will be the same as the Outlook Express "
      "folder names, but they will be preceded with 'OE5-'. If this causes "
      "problems for you (you have KMail folders beginning with 'OE5-'), "
      "cancel this import function (the next dialog will allow you to do "
      "that) and rename the existing KMail folders.</p>")),
      CAP(i18n("Import Outlook Express 5/6"))
{
}

FilterOE5::~FilterOE5()
{}

void FilterOE5::import(FilterInfo *info)
{
  QString  chosen;
  QWidget *parent=info->parent();

  if (!kmailStart(info)) return;

  chosen=KFileDialog::getExistingDirectory(QDir::homeDirPath(),parent, 
                                           i18n("Select Folder"));
  if (chosen.isEmpty()) return; // No directory chosen here!

  info->log(i18n("Searching for Outlook Express '.dbx' folders in directory %1").arg(chosen));

  QDir dir(chosen);
  if (!dir.isReadable())
  {
    info->alert(CAP, i18n("Can't open directory %1").arg(chosen));
    return;
  }

  QStringList folders = dir.entryList("*.[dD][bB][xX]", QDir::Files);
  if (folders.isEmpty())
  {
    info->alert(CAP,i18n("No '.dbx' folders found!"));
    return;
  }

  info->overall(0.0f);
  int n=0;
  for (QStringList::ConstIterator it = folders.begin(); it != folders.end(); ++it)
  {
    info->overall(float(++n) / folders.count() * 100.0f);

    QString path = chosen + '/' + *it,
            name = (*it).left((*it).length() - 4);
    info->from(i18n("Source: ") + path);
    info->to(i18n("Destination: ") + "OE5-" + name);
    info->log(i18n("  importing folder '%1' to KMail 'OE5-%2'...").arg(*it).arg(name));
    OE52MBox(path,name,this,info).convert();
  }

  info->log(i18n("done."));
  info->current();info->current(100.0);
  info->overall();info->overall(100.0);
  info->alert(CAP,i18n("All '.dbx' folders are imported"));

  kmailStop(info);
}

// vim: ts=2 sw=2 et
