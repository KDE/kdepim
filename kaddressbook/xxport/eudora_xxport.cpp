/*
    This file is part of KAddressbook.
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

	As a special exception, permission is given to link this program
	with any edition of Qt, and distribute the resulting executable,
	without including the source code for Qt in the source distribution.
 */


#include <qfile.h>

#include <kfiledialog.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ktempfile.h>
#include <kurl.h>

#include <kdebug.h>

#include "importdialog.h"

#include "eudora_xxport.h"

#define CTRL_C 3

class EudoraXXPortFactory : public XXPortFactory
{
  public:
    XXPortObject *xxportObject( KABCore *core, QObject *parent, const char *name )
    {
      return new EudoraXXPort( core, parent, name );
    }
};

extern "C"
{
  void *init_libkaddrbk_eudora_xxport()
  {
    return ( new EudoraXXPortFactory() );
  }
}


  EudoraXXPort::EudoraXXPort( KABCore *core, QObject *parent, const char *name )
: XXPortObject( core, parent, name )
{
  createImportAction( i18n( "Import Eudora Addressbook .." ) );
  //createExportAction( i18n( "Export Eudora Addressbook .." ) );
}

bool EudoraXXPort::exportContacts( const KABC::AddresseeList &, const QString& )
{
  // ### implement me
  return false;
}

KABC::AddresseeList EudoraXXPort::importContacts( const QString& ) const
{

  QString file=KFileDialog::getOpenFileName(QDir::homeDirPath() ,"*.txt *.TXT",0);
  if (file.isEmpty())
    return KABC::AddresseeList();

  QFile f(file);
  if (!f.open(IO_ReadOnly))
    return KABC::AddresseeList();

  QString line;
  QTextStream stream(&f);
  KABC::Addressee *a = 0;
  int bytesRead = 0;

  KABC::AddresseeList list;

  while(!stream.eof()) {
    line = stream.readLine();
    bytesRead += line.length();

    if (line.startsWith("alias")) {
      if (a) { // Write it out
        list << *a;
        delete a;
        a = 0;
        a = new KABC::Addressee();
      } else a = new KABC::Addressee();
      a->setFormattedName(key(line));
      a->insertEmail(email(line));
    }
    else if (line.startsWith("note")) {
      if (!a) break; // Must have an alias before a note
      a->setNote(comment(line));
      a->setFamilyName(get(line, "name"));
      KABC::Address addr;
      QString addrStr = get(line, "address");
      if (!addrStr.isEmpty())
        kdDebug() << addrStr << endl; // dump complete address
      addr.setLabel(addrStr);
      a->insertAddress(addr);
      a->insertPhoneNumber( KABC::PhoneNumber( get(line,"phone"), KABC::PhoneNumber::Voice ) );
    }
  }


  if (a) { // Write out address
    list << *a;
    delete a;
    a = 0;
  }

  f.close();

  return list;
}

void EudoraXXPort::doExport( QFile *fp, const KABC::AddresseeList &list )
{
  QTextStream t( fp );

  KABC::AddresseeList::ConstIterator iter;
  KABC::Field::List fields = core()->addressBook()->fields();
  KABC::Field::List::Iterator fieldIter;
  bool first = true;

  // First output the column headings
  for ( fieldIter = fields.begin(); fieldIter != fields.end(); ++fieldIter ) {
    if ( !first )
      t << ",";

    t << "\"" << (*fieldIter)->label() << "\"";
    first = false;
  }
  t << "\n";

  // Then all the addressee objects
  KABC::Addressee addr;
  for ( iter = list.begin(); iter != list.end(); ++iter ) {
    addr = *iter;
    first = true;

    for ( fieldIter = fields.begin(); fieldIter != fields.end(); ++fieldIter ) {
      if ( !first )
        t << ",";

      t << "\"" << (*fieldIter)->value( addr ).replace( "\n", "\\n" ) << "\"";
      first = false;
    }

    t << "\n";
  }
}

QString EudoraXXPort::key(const QString& line) const
{
  int e;
  QString result;
  int b=line.find('\"',0);
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

QString EudoraXXPort::email(const QString& line) const
{
  int b;
  QString result;
  b=line.findRev('\"');if (b==-1) { 
    b=line.findRev(' ');if(b==-1) { return result; }
  }
  result=line.mid(b+1);
  return result;
}

QString EudoraXXPort::comment(const QString& line) const
{
  int b;
  QString result;
  uint i;
  b=line.findRev('>');if (b==-1) {
    b=line.findRev('\"');if (b==-1) { return result; }
  }
  result=line.mid(b+1);
  for(i=0;i<result.length();i++) {
    if (result[i]==CTRL_C) { result[i]='\n'; }
  }
  return result;
}

QString EudoraXXPort::get(const QString& line,const QString& key) const
{
  QString fd="<"+key+":";
  int b,e;
  uint i;

  // Find formatted key, return on error
  b=line.find(fd);
  if (b==-1) { return QString::null; }

  b+=fd.length();
  e=line.find('>',b);
  if (e==-1) { return QString::null; }

  e--;
  QString result=line.mid(b,e-b+1);
  for(i=0;i<result.length();i++) {
    if (result[i]==CTRL_C) { result[i]='\n'; }
  }
  return result;
}

#include "eudora_xxport.moc"

// vim: ts=2 sw=2 et
