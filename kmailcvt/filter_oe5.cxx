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

#include "filter_oe5.hxx"
#include "oe5_2mbox.h"
#include "harray.hxx"

#include <kfiledialog.h>
#include <kdirlister.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <klocale.h>


filter_oe5::filter_oe5() : filter(i18n("Import folders from Outlook Express 5"),"Stephan B. Nedregard/Hans Dijkema")
{
  CAP=i18n("Import Outlook Express 5");
}

filter_oe5::~filter_oe5()
{}

void filter_oe5::import(filterInfo *info)
{
char     dir[1024];
QString  choosen;
QString  msg;
QWidget *parent=info->parent();

   if (!kmailStart(info)) { return; }

   sprintf(dir,getenv("HOME"));

   {QString m;
     m=i18n("Stephan B. Nedregard kindly contributed the OE4/5 import code!\n\n"
            "Select the Outlook Express 5 directory on "
            "your system.\n\n"
            "This import filter wil search for folders\n"
            "(the '.dbx' files)\n\n"
            "NOTE: You won't get back your original folder-\n"
            "structure, only the folders themselves are\n"
            "imported. But you'll probably only do this\n"
            "one time ;-).\n\n"
            "NOTE: Kmailcvt takes the same foldernames\n"
            "as the Outlook Express foldernames, but precedes\n"
            "them with 'OE5-'. If this causes trouble to you\n"
            "(you've got kmail folders beginning with 'OE5-')\n"
            "cancel this import function (next dialog will\n"
            "let you do that) and rename the existing kmail\n"
            "folders."
           );
     info->alert(CAP,m);
   }

   choosen=KFileDialog::getExistingDirectory(dir,parent,"ImportOE5");
   if (choosen.length()==0) { return; } // No directory choosen here!
   strcpy(dir,choosen.latin1());

   msg=i18n("Searching for Outlook Express 5 '.dbx' folders in directory %1").arg(dir);
   info->log(msg);

   {DIR *d;
    struct dirent *entry;
      d=opendir(dir);
      if (d==NULL) {QString msg;
        msg=i18n("Can't open directory %1").arg(dir);
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
              char fldr[1024],name[256];

              sprintf(fldr,"%s/%s",dir,file);
              sprintf(name,"%s",file);name[strlen(name)-4]='\0';

              {QString s;
                 s.sprintf("\t%s",fldr);
                 s=i18n("from:")+s;
                 info->from(s);
                 s.sprintf("\tOE5-%s",name);
                 s=i18n("to:")+s;
                 info->to(s);
              }

              {QString f,n;
                 f.sprintf(" '%s' ",file);
                 n.sprintf(" 'OE5-%s'...",name);
                 msg=i18n("  importing folder %1 to kmail %2").arg(f).arg(n);
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

