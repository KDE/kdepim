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


#include <klocale.h>
#include <kfiledialog.h>

#define CTRL_C	3


FilterEudoraAb::FilterEudoraAb() : Filter(i18n("Import Filter for Eudora Light Addressbook"),"Hans Dijkema")
{
}

FilterEudoraAb::~FilterEudoraAb()
{
}

void FilterEudoraAb::import(FilterInfo *info)
{
  QString file;
  QWidget *parent=info->parent();

  if (!openAddressBook(info)) return;

  file=KFileDialog::getOpenFileName(QDir::homeDirPath() ,"*.[tT][xX][tT]",parent);
  if (file.length()==0) 
  {
    info->alert(i18n("No address book chosen"));
    return;
  }

  QFile F(file);
  if (! F.open(IO_ReadOnly)) {
    info->alert(i18n("Unable to open file '%1'").arg(file));
    return;
  }

  info->from(file);
  info->to(i18n("KAddressBook"));
  info->current(i18n("Currently converting Eudora Light addresses to address book"));
  convert(F,info);
  {
    int i;
    int lines=keys.size();
    for(i=0;i<lines;i++) 
    {
      QString msg=i18n("Adding/Merging '%1' email '%2'").arg(keys[i]).arg(emails[i]);
      info->log(msg);

      QString comment;
      if(adr[i].isEmpty())
        comment=comments[i];
      else 
        comment=adr[i]+"\n"+comments[i]; 

      KABC::Addressee a;
      a.setFormattedName(keys[i]);
      if (!emails[i].isEmpty()) a.insertEmail(emails[i]);
      if (!names[i].isEmpty()) a.setFamilyName(names[i]);
      if (!phones[i].isNull()) a.insertPhoneNumber( KABC::PhoneNumber( phones[i], KABC::PhoneNumber::Home | KABC::PhoneNumber::Voice ) );
      if (!comments[i].isNull()) a.setNote(comments[i]);

      addContact( a );
      { 
        info->overall(100*i/lines);
      }
    }
    {
      info->log(i18n("Added %1 keys").arg(keys.size()));
    }
  }

  closeAddressBook( );
  info->current(i18n("Finished converting Eudora Light addresses to KAddressBook"));
  F.close();
  info->overall(100);
}

void FilterEudoraAb::convert(QFile& f,FilterInfo *info)
{
  info->current(0);
  
  QString line;
  int e;
  QTextStream stream(&f);
  
  // Do the actual convert
  while(!stream.eof()) {
    line = stream.readLine();

    if (line.left(5) == "alias") {
      QString k = key(line);
      e=find(k);
      if (keys.at(e) == keys.end()) keys.append(k);
      else keys[e]=k;
      if (emails.at(e) == emails.end()) emails.append(email(line));
      else emails[e]=email(line);
      {
        QString msg=i18n("Reading '%1', email '%2'").arg(k).arg(emails[e]);
        info->log(msg);
      }
    }
    else if (line.left(4) == "note") { 
      QString k = key(line); 
      e=find(k);
      if (keys.at(e) == keys.end()) keys.append(k);
      else keys[e]=k;
      if (comments.at(e) == comments.end()) comments.append(comment(line));
      else comments[e]=comment(line);
      if (names.at(e) == names.end()) names.append(get(line,"name"));
      else names[e]=get(line,"name");
      if (adr.at(e) == adr.end()) adr.append(get(line, "address"));
      else adr[e]=get(line, "address");
      if (phones.at(e) == adr.end()) phones.append(get(line, "phone"));
      else phones[e]=get(line, "phone");
    }
  }
  info->current(100);
}

QString FilterEudoraAb::key(const QString& line) const
{
  int b,e;
  QString result;
  b=line.find('\"',0);
  if (b==-1) { 
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
  QString fd="<"+key+":";
  int b,e;
  uint i;

  // Find formatted key, return on error
  b=line.find(fd);
  if (b==-1) { return QString(); }

  b+=fd.length();
  e=line.find('>',b);
  if (e==-1) { return QString(); }
  
  e--;
  QString result=line.mid(b,e-b+1);
  for(i=0;i<result.length();i++) {
    if (result[i]==CTRL_C) { result[i]='\n'; }
  }
  return result;
}

int FilterEudoraAb::find(const QString& key) const
{ // Either return the found position, or the next free one
  int n = keys.findIndex(key);
  return n == -1 ? keys.count() : n;
}

// vim: ts=2 sw=2 et
