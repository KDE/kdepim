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

  info->setFrom(file);
  info->setTo(i18n("KAddressBook"));
  info->setCurrent(i18n("Currently converting Eudora Light addresses to address book"));
  info->setCurrent(0);
  
  QString line;
  QTextStream stream(&F);
  KABC::Addressee *a = 0;
  int count = 0;
  int bytesRead = 0;

  while(!stream.eof()) {
    line = stream.readLine();
    bytesRead += line.length();

    if (line.left(5) == "alias") {
      if (a) { // Write it out
        addContact( *a );
        delete a;
        a = 0;
        count++;
        info->setOverall(100*bytesRead/F.size());
        a = new KABC::Addressee();
      } else a = new KABC::Addressee();
      a->setFormattedName(key(line));
      a->insertEmail(email(line));
      info->addLog(i18n("Reading '%1'").arg(a->formattedName()));
    }
    else if (line.left(4) == "note") { 
      if (!a) break; // Must have an alias before a note
      a->setNote(comment(line));
      a->setFamilyName(get(line, "name"));
      KABC::Address addr;
      addr.setLabel(get(line, "address"));
      a->insertAddress(addr);
      a->insertPhoneNumber( KABC::PhoneNumber( get(line,"phone"), KABC::PhoneNumber::Voice ) );
    }
  }
  if (a) { // Write out address
    addContact( *a );
    delete a;
    a = 0;
    count++;
  }
  info->setCurrent(100);
  info->addLog(i18n("Added %1 keys").arg(count));

  closeAddressBook( );
  info->setCurrent(i18n("Finished converting Eudora Light addresses to KAddressBook"));
  F.close();
  info->setOverall(100);
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

// vim: ts=2 sw=2 et
