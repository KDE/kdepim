/***************************************************************************
                          FilterLDIF.cxx  -  description
                             -------------------
    begin                : Fri Dec 1, 2000
    copyright            : (C) 2000 by Oliver Strutynski
    email                : olistrut@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <iostream>
#include <stdlib.h>


#include <kfiledialog.h>
#include <klocale.h>
#include <kmdcodec.h>

#include "filter_ldif.hxx"

FilterLDIF::FilterLDIF() 
  : Filter(i18n("Import Netscape LDIF Personal Address Book"), 
	"Oliver Strutynski",
	i18n("<p>The LDAP Data Interchange Format (LDIF) is a common ASCII-text based "
		"Internet interchange format. Most programs allow you to export data in "
		"LDIF format and e.g. Netscape and Mozilla store by default their "
		"Personal Address Book files in this format.</p>"
		"This import filter reads any LDIF version 1 file and imports the " 
		"entries into your KDE Addressbook.</p>")
    )
{}

FilterLDIF::~FilterLDIF()
{}

void FilterLDIF::import(FilterInfo *info) {
   QWidget *parent = info->parent();

   QString filename = KFileDialog::getOpenFileName( QDir::homeDirPath(), 
		   	"*.ldif *.LDIF *.Ldif", parent);
   if (filename.isEmpty()) {
     info->alert(i18n("No Addressbook chosen"));
     return;
   }

   info->setFrom(filename);
   info->setTo(i18n("KAddressBook"));
   info->setCurrent(i18n("Currently converting .LDIF address file to KAddressBook"));

   convert(filename, info);

   info->setCurrent(100);
   info->setOverall(100);
   info->setCurrent(i18n("Finished converting .LDIF address file to KAddressBook"));
}


bool FilterLDIF::convert(const QString &filename, FilterInfo *info) {
   if (!openAddressBook(info))
	return false;

   QString caption( i18n("Import Netscape LDIF Personal Addressbook (.LDIF)") );

   QFile f(filename);
   if ( !f.open(IO_ReadOnly) ) {
	QString msg =  i18n("Can't open '%1' for reading").arg(filename);
	info->alert(msg);
	return false;
   }

   QString empty;

   QTextStream t( &f );
   t.setEncoding(QTextStream::Latin1);
   QString s, completeline, fieldname;

   // We need this for calculating progress
   uint fileSize = f.size();
   uint bytesProcessed = 0;

   // Set to true if data currently read is part of
   // a list of names. Lists will be ignored.
   bool isGroup = false;
   bool lastWasComment = false;
   int numEntries = 0;

   KABC::Addressee *a = new KABC::Addressee();
   KABC::Address *addr = new KABC::Address();
   addr->setType(KABC::Address::Home);
   while ( !t.eof() ) {
	s = t.readLine();
	completeline = s;
	bytesProcessed += s.length();
	
	// update progress information
    	info->setCurrent(100 * bytesProcessed / fileSize);
    	info->setOverall(100 * bytesProcessed / fileSize);
	
	if (s.isEmpty() && t.eof()) {
		// Newline: Write data
writeData:
		if (!isGroup) {
			if (!a->formattedName().isEmpty() && a->emails().count() > 0) {
				numEntries++;
				a->insertAddress(*addr);
				addContact(*a);
			}
   		} else {
			info->addLog(i18n("Warning: List data is being ignored."));
   		}

		// delete old and create a new empty entry
		delete a;
		delete addr;
		a = new KABC::Addressee();
		addr = new KABC::Address();
 		addr->setType(KABC::Address::Home);

		isGroup = false;
		lastWasComment = false;
		continue;
	}
	
	int position = s.find("::");
    	if (position != -1) {
    		// String is BASE64 encoded
    		fieldname = s.left(position).lower();
    		s = QString::fromUtf8(KCodecs::base64Decode(
						s.mid(position+3, s.length()-position-2).latin1()))
						.simplifyWhiteSpace();
    	} else {
    		position = s.find(":");
		if (position != -1) {
    			fieldname = s.left(position).lower();
    			// Convert Utf8 string to unicode so special characters are preserved
			// We need this since we are reading normal strings from the file
			// which are not converted automatically
			s = QString::fromUtf8(s.mid(position+2, s.length()-position-2).latin1());
		} else {
			fieldname = QString::null;
			s = QString::fromUtf8(s);
		}
	}

	if (s.stripWhiteSpace().isEmpty())
		continue;

	if (fieldname.isEmpty()) {
		if (lastWasComment) {
			// if the last entry was a comment, add this one too, since
			// we might have a multi-line comment entry.
			if (!a->note().isEmpty())
				a->setNote(a->note() + "\n");
			a->setNote(a->note() + s);
		}
		continue;
	}
	lastWasComment = false;

	if (fieldname == "givenname")
		{ a->setGivenName(s); continue; }

	if (fieldname == "xmozillanickname")
		{ a->setNickName(s); continue; }

	if (fieldname == "dn")	/* ignore */
		{ goto writeData; }
	
	if (fieldname == "sn")
		{ a->setFamilyName(s); continue; }
	
	if (fieldname == "mail")
		{ a->insertEmail(s); continue; }

	if (fieldname == "title")
		{ a->setTitle(s); continue; }

	if (fieldname == "cn")
		{ a->setFormattedName(s); continue; }

	if (fieldname == "o")
		{ a->setOrganization(s); continue; }

	if (fieldname == "description")
		{ a->setNote(s); lastWasComment = true; continue; }

	if (fieldname == "homeurl")
		{ a->setUrl(s); continue; }

	if (fieldname == "homephone")
		{ a->insertPhoneNumber( KABC::PhoneNumber (s, KABC::PhoneNumber::Home ) ); continue; }
	
	if (fieldname == "telephonenumber")
		{ a->insertPhoneNumber( KABC::PhoneNumber (s, KABC::PhoneNumber::Voice ) ); continue; }

	if (fieldname == "postalcode")
		{ addr->setPostalCode(s); continue; }

	if (fieldname == "facsimiletelephonenumber")
		{ a->insertPhoneNumber( KABC::PhoneNumber (s, KABC::PhoneNumber::Fax ) ); continue; }
	
	if (fieldname == "streetaddress")
		{ addr->setStreet(s); continue; }

	if (fieldname == "locality")
		{ addr->setLocality(s); continue; }
	
	if (fieldname == "countryname")
		{ addr->setCountry(s); continue; }
		
	if (fieldname == "cellphone")
		{ a->insertPhoneNumber( KABC::PhoneNumber (s, KABC::PhoneNumber::Cell ) ); continue; }

	if (fieldname == "st")
		{ addr->setRegion(s); continue; }

	if (fieldname == "ou")
		{ a->setRole(s); continue; }

	if (fieldname == "objectclass" && s == "groupOfNames")
			isGroup = true;

	info->addLog(i18n("Unable to handle line: %1").arg(completeline));
  	  	
    } /* while !eof(f) */
    delete a;
    delete addr;

    f.close();
    
    info->addLog(i18n("%1 phonebook entries sucessfully imported.").arg(numEntries));

    closeAddressBook();
    return true;
}
