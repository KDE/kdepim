/***************************************************************************
                          filter_eudora_ab.cxx  -  description
                          ------------------------------------
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

#include "filter_eudora_ab.hxx"
#include "harray.hxx"

#include <kfiledialog.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <klocale.h>

#define CTRL_C	3


filter_eudora_ab::filter_eudora_ab() : filter(i18n("Import filter for the Eudora Light Addressbook"),"Hans Dijkema")
{
  CAP=i18n("Import filter for the Eudora Light Addressbook");
  LINES=0;
  printf("name\n");
}

filter_eudora_ab::~filter_eudora_ab()
{}

void filter_eudora_ab::import(filterInfo *info)
{
QString file;
char    dir[1024];
QWidget *parent=info->parent();
FILE   *F;

   if (!kabStart(info)) { 
     info->alert(name(),i18n("Can't open K Addressbook"));
   }

   sprintf(dir,getenv("HOME"));
 
   file=KFileDialog::getOpenFileName(dir,"*.txt *.TXT *.Txt",parent);
   if (file.length()==0) {
     info->alert(name(),i18n("No Addressbook choosen"));
     return;
   }
   F=fopen(file.latin1(),"rt");
   if (F==NULL) {QString msg=i18n("Can't open file '%1'").arg(file);
     info->alert(name(),msg);
     return;
   }

   QString from=i18n("Source: "),to=i18n("Destination: ");
     from+="\t";from+=file;
     to+="\t";to+=i18n("the K Addressbook");
     info->from(from);
     info->to(to);
     info->current(i18n("Currently converting Eudora Light addresses to Kab"));
     convert(F,info);
     {int i,N;
       LINES=keys.len();
       for(i=0,N=keys.len();i<N;i++) {
      /*   printf("%s %s %s %s %s %s\n",keys[i].latin1(),emails[i].latin1(),
                                   names[i].latin1(),adr[i].latin1(),
                                   phones[i].latin1(),comments[i].latin1()
               );
      */
         {QString msg=i18n("Adding/Merging '%1' email '%2'").arg(keys[i]).arg(emails[i]);
           info->log(msg);
         }

         QString comment;
           if(adr[i]=="") { comment=comments[i]; }
           else { comment=adr[i]+"\n"+comments[i]; }
 
           kabAddress(info,"Eudora Light",
                      keys[i],(emails[i]=="") ? KAB_NIL : emails[i],
                      KAB_NIL,KAB_NIL,KAB_NIL,
                      (names[i]=="") ? KAB_NIL : names[i],
                      KAB_NIL,KAB_NIL,
                      KAB_NIL,KAB_NIL,
                      KAB_NIL,
                      KAB_NIL,KAB_NIL,
                      KAB_NIL,KAB_NIL,
                      (phones[i]=="") ? KAB_NIL : phones[i],KAB_NIL,
                      KAB_NIL,KAB_NIL,
                      KAB_NIL,KAB_NIL,
                      (comments[i]=="") ? KAB_NIL : comments[i], KAB_NIL
                     );

         { float perc=((float) i)/((float) LINES)*100.0;
             info->overall(perc);
         }
       }
       {QString msg;
          msg=i18n("Added %1 keys").arg(keys.len());
          info->log(msg);
       }
     }

   kabStop(info);
   info->current(i18n("Done converting Eudora Light addresses to Kab"));
   fclose(F);
   info->overall(100.0);
}

#define LINELEN 10240

void filter_eudora_ab::convert(FILE *f,filterInfo *info)
{
QString line;
char _line[LINELEN+1];
int  i,e;
int LINE=0;
float perc;
  while(fgets(_line,LINELEN,f)!=NULL) { LINES+=1; }
  rewind(f);
  while(fgets(_line,LINELEN,f)!=NULL) {

    LINE+=1;
    perc=((float) LINE)/((float) LINES)*100.0;
    info->current(perc);

    for(i=0;_line[i]!='\n' && _line[i]!='\0';i++);
    _line[i]='\0';
    line=_line;

    if (strncmp(line.latin1(),"alias",5)==0) {QString key,email;
      key=getkey(line);
      email=getemail(line);
      e=find(key);
      keys[e]=key;emails[e]=email;
      {QString msg=i18n("Reading '%1', email '%2'").arg(key).arg(email);
        info->log(msg);
      }
    }
    else if (strncmp(line.latin1(),"note",4)==0) { 
     QString key,name,address,phone,comment;
      key=getkey(line);
      comment=getcomment(line);
      name=get(line,"name");
      address=get(line,"address");
      phone=get(line,"phone");
      
      e=find(key);
      keys[e]=key;comments[e]=comment;names[e]=name;
      adr[e]=address;phones[e]=phone;
    }
  }
  info->current(100.0);
}

QString filter_eudora_ab::getkey(QString line)
{
int b,e;
QString result="";
 b=line.find('\"',0);if (b==-1) { 
   b=line.find(' ');
   if (b==-1) { return result; }
   b+=1;
   e=line.find(' ',b);
   result=line.mid(b,e-b);
   return result;
 }
 b+=1;
 e=line.find('\"',b);if (e==-1) { return result; }
 result=line.mid(b,e-b);
return result;
}

QString filter_eudora_ab::getemail(QString line)
{
int b;
QString result="";
  b=line.findRev('\"');if (b==-1) { 
    b=line.findRev(' ');if(b==-1) { return result; }
  }
  result=line.mid(b+1);
return result;
}

QString filter_eudora_ab::getcomment(QString line)
{
int b;
QString result="";
unsigned int i;
  b=line.findRev('>');if (b==-1) {
    b=line.findRev('\"');if (b==-1) { return result; }
  }
  result=line.mid(b+1);
  for(i=0;i<result.length();i++) {
    if (result[i]==CTRL_C) { result[i]='\n'; }
  }
return result;
}

QString filter_eudora_ab::get(QString line,QString key)
{
QString result="";
QString fd="<"+key+":";
int b,e;
unsigned int i;
  b=line.find(fd);if (b==-1) { return result; }
  b+=fd.length();
  e=line.find('>',b);if (e==-1) { return result; }
  e-=1;
  result=line.mid(b,e-b+1);
  for(i=0;i<result.length();i++) {
    if (result[i]==CTRL_C) { result[i]='\n'; }
  }
return result;
}

int filter_eudora_ab::find(QString key)
{
int i,N;
  for(i=0,N=keys.len();i<N && key!=keys[i];i++);
return i;
}

