/***************************************************************************
                          FilterEudoraAb.cxx  -  description
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

#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>

#include <qdir.h>

#include <klocale.h>
#include <kfiledialog.h>

#define CTRL_C	3


FilterEudoraAb::FilterEudoraAb() : Filter(i18n("Import Filter for Eudora Light Addressbook"),"Hans Dijkema")
, LINES(0)
{
}

FilterEudoraAb::~FilterEudoraAb()
{
}

void FilterEudoraAb::import(FilterInfo *info)
{
  QString file;
  QWidget *parent=info->parent();
  FILE   *F;

  if (!kabStart(info)) return;

  file=KFileDialog::getOpenFileName(QDir::homeDirPath() ,"*.txt *.TXT *.Txt",parent);
  if (file.length()==0) 
  {
    info->alert(i18n("No address book chosen"));
    return;
  }

  F=fopen(file.latin1(),"rt");
  if (F==NULL) {
    info->alert(i18n("Unable to open file '%1'").arg(file));
    return;
  }

  info->from(file);
  info->to(i18n("KAddressBook"));
  info->current(i18n("Currently converting Eudora Light addresses to address book"));
  convert(F,info);
  {int i,N;
    LINES=keys.size();
    for(i=0,N=keys.size();i<N;i++) {
      /*   printf("%s %s %s %s %s %s\n",keys[i].latin1(),emails[i].latin1(),
           names[i].latin1(),adr[i].latin1(),
           phones[i].latin1(),comments[i].latin1()
           );
       */
      {
        QString msg=i18n("Adding/Merging '%1' email '%2'").arg(keys[i]).arg(emails[i]);
        info->log(msg);
      }

      QString comment;
      if(adr[i].isEmpty())
        comment=comments[i];
      else 
        comment=adr[i]+"\n"+comments[i]; 

      kabAddress(info,"Eudora Light",
          keys[i],(emails[i].isEmpty()) ? QString::null : emails[i],
          QString::null,QString::null,QString::null,
          (names[i]=="") ? QString::null : names[i],
          QString::null,
          QString::null,QString::null,
          QString::null,QString::null,
          QString::null,
          QString::null,QString::null,
          QString::null,QString::null,
          (phones[i].isNull()) ? QString::null : phones[i],QString::null,
          QString::null,QString::null,
          QString::null,QString::null,
          (comments[i].isNull()) ? QString::null : comments[i], QString::null
          );

      { 
        info->overall(100*i/LINES);
      }
    }
    {
      info->log(i18n("Added %1 keys").arg(keys.size()));
    }
  }

  kabStop(info);
  info->current(i18n("Finished converting Eudora Light addresses to KAddressBook"));
  fclose(F);
  info->overall(100);
}

#define LINELEN 10240

void FilterEudoraAb::convert(FILE *f,FilterInfo *info)
{
  QString line;
  char _line[LINELEN+1];
  int  i,e;
  int LINE=0;
  while(fgets(_line,LINELEN,f)!=NULL) { LINES+=1; }
  rewind(f);
  while(fgets(_line,LINELEN,f)!=NULL) {

    LINE+=1;
    info->current(100 * LINE / LINES );

    for(i=0;_line[i]!='\n' && _line[i]!='\0';i++);
    _line[i]='\0';
    line=_line;

    if (line.startsWith("alias")) {
      QString k = key(line); 
      e=find(k);
      keys[e]=k;
      emails[e]=email(line);
      {
        QString msg=i18n("Reading '%1', email '%2'").arg(k).arg(emails[e]);
        info->log(msg);
      }
    }
    else if (line.startsWith("note")) { 
      QString k = key(line); 
      e=find(k);
      keys[e]=k;
      comments[e]=comment(line);
      names[e]=get(line,"name");
      adr[e]=get(line, "address");
      phones[e]=get(line, "phone");
    }
  }
  info->current(100);
}

QString FilterEudoraAb::key(const QString& line) const
{
  int b,e;
  QString result;
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

QString FilterEudoraAb::email(const QString& line) const
{
  int b;
  QString result;
  b=line.findRev('\"');if (b==-1) { 
    b=line.findRev(' ');if(b==-1) { return result; }
  }
  result=line.mid(b+1);
  return result;
}

QString FilterEudoraAb::comment(const QString& line) const
{
  int b;
  QString result;
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

QString FilterEudoraAb::get(const QString& line,const QString& key) const
{
  QString result;
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

int FilterEudoraAb::find(const QString& key) const
{
  return keys.findIndex(key);
}

// vim: ts=2 sw=2 et
