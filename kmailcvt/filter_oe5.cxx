/***************************************************************************
                          filter_oe5.cxx  -  description
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
#include "harray.hxx"

#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>

#include <qdir.h>

#include <klocale.h>
#include <kfiledialog.h>

filter_oe5::filter_oe5() : filter(i18n("Import Folders From Outlook Express 5"),"Stephan B. Nedregard/Hans Dijkema")
{
  CAP=i18n("Import Outlook Express 5");
}

filter_oe5::~filter_oe5()
{}

void filter_oe5::import(filterInfo *info)
{
QString  choosen;
QString  msg;
QWidget *parent=info->parent();

   if (!kmailStart(info)) { return; }

   msg=i18n("Stephan B. Nedregard kindly contributed the Outlook Express 4/5\n"
	    "import code.\n\n"
	    "Select the Outlook Express 5 directory on your system.\n\n"
	    "This import filter will search for folders (the '.mbx' files).\n\n"
	    "NOTE: You will not be able to revert to your original folder\n"
	    "structure, only the folders themselves are imported. But you will\n"
	    "probably only do this one time.\n\n"
	    "NOTE: Kmailcvt takes the same folder names as the Outlook Express\n"
	    "folder names, but precedes them with 'OE5-'. If this causes\n"
	    "problems for you (you have KMail folders beginning with 'OE5-'),\n"
	    "cancel this import function (the next dialog will allow you to do\n"
	    "that) and rename the existing KMail folders."
           );
   info->alert(CAP,msg);

   choosen=KFileDialog::getExistingDirectory(QDir::homeDirPath(),parent,"ImportOE5");
   if (choosen.isEmpty()) { return; } // No directory choosen here!

   msg=i18n("Searching for Outlook Express 5 '.dbx' folders in directory %1").arg(choosen);
   info->log(msg);

   {DIR *d;
    struct dirent *entry;
      d=opendir(QFile::encodeName(choosen));
      if (d==NULL) {QString msg;
        msg=i18n("Can't open directory %1").arg(choosen);
        info->alert(CAP,msg);
      }
      else {int   N=0,n=0;
            float perc;

        entry=readdir(d);
        while (entry!=NULL) {char *file=entry->d_name;
          if (strlen(file)>4 && strcasecmp(&file[strlen(file)-4],".dbx")==0) {
            N+=1;
          }
          entry=readdir(d);
        }
        if (N==0) {
          info->alert(CAP,i18n("No '.dbx' folders found!"));
        }
        rewinddir(d);

        info->overall();

        entry=readdir(d);
        while (entry!=NULL) {char *file=entry->d_name;

          n+=1;
          perc=(((float) n)/((float) N))*100.0;
          info->overall(perc);

          if (strlen(file)>4 && strcasecmp(&file[strlen(file)-4],".dbx")==0) {
            {
              char fldr[PATH_MAX],name[256];

              snprintf(fldr, sizeof(fldr), "%s/%s",QFile::encodeName(choosen).data(),file);
              snprintf(name, sizeof(name), "%s",file);name[strlen(name)-4]='\0';

              {QString s;
                 s.sprintf("\t%s",fldr);
                 s=i18n("Source:")+s;
                 info->from(s);
                 s.sprintf("\tOE5-%s",name);
                 s=i18n("Destination:")+s;
                 info->to(s);
              }

              {QString f,n;
                 msg=i18n("  importing folder '%1' to KMail 'OE5-%2'...").arg(file).arg(name);
                 info->log(msg);
              }

              {oe5_2mbox m(fldr,name,this,info);
                m.convert();
              }
            }
          }
          entry=readdir(d);
        }
        closedir(d);

        if (N!=0) {
          info->log(i18n("done."));
          info->current();info->current(100.0);
          info->overall();info->overall(100.0);
          info->alert(CAP,i18n("All '.dbx' folders are imported"));
        }
      }
   }

   kmailStop(info);
}

