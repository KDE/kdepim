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
  : Filter(i18n("Import Netscape LDIF Personal Address Book (.LDIF)"), 
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

   info->from(filename);
   info->to(i18n("KAddressBook"));
   info->current(i18n("Currently converting .LDIF address file to KAddressBook"));

   convert(filename, info);

   info->current(100);
   info->overall(100);
   info->current(i18n("Finished converting .LDIF address file to KAddressBook"));
}


bool FilterLDIF::convert(const QString &filename, FilterInfo *info) {
   if (!kabStart(info))
	return false;

   QString caption( i18n("Import Netscape LDIF Personal Addressbook (.LDIF)") );

   QFile f(filename);
   if ( !f.open(IO_ReadOnly) ) {
	QString msg =  i18n("Can't open '%1' for reading").arg(filename);
	info->alert(msg);
	return false;
   }

   QString empty;
   QString givenName, email, title, firstName, lastName, nickName,
	street, locality, state, zipCode, country, organization,
	department, phone, fax, mobile, homepage, comment;

   QTextStream t( &f );
   QString s, completeline, fieldname;

   // We need this for calculating progress
   uint fileSize = f.size();
   uint bytesProcessed = 0;

   // Set to true if data currently read is part of
   // a list of names. Lists will be ignored.
   bool isGroup = false;
   bool lastWasComment = false;
   int numEntries = 0;

   while ( !t.eof() ) {
	s = t.readLine();
	completeline = s;
	bytesProcessed += s.length();
	
	// update progress information
    	info->current(100 * bytesProcessed / fileSize);
    	info->overall(100 * bytesProcessed / fileSize);
	
	if (s.isEmpty() && t.eof()) {
		// Newline: Write data
writeData:
		if (!isGroup) {
			if (!firstName.isEmpty() || !lastName.isEmpty() || !email.isEmpty()) {
				numEntries++;
  				kabAddress( info, i18n("Netscape Addressbook"),
				  givenName, email, title, firstName, empty, lastName, nickName,
				  street, locality, state, zipCode, country, organization, 
				  department, empty, empty, phone, fax, mobile, empty, homepage, 
				  empty, comment, empty);
			}
			givenName = email = title = firstName = lastName = nickName =
			 street = locality = state = zipCode = country = organization =
			 department = phone = fax = mobile = homepage = comment = QString::null;

   		} else {
			info->log(i18n("Warning: List data is being ignored."));
   		}

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
			if (!comment.isEmpty())
				comment += "\n";
			comment += s;
		}
		continue;
	}
	lastWasComment = false;

	if (fieldname == "givenname")
		{ firstName = s; continue; }

	if (fieldname == "xmozillanickname")
		{ nickName = s; continue; }

	if (fieldname == "dn")	/* ignore */
		{ goto writeData; }
	
	if (fieldname == "sn")
		{ lastName = s;	continue; }
	
	if (fieldname == "mail")
		{ email = s; continue; }

	if (fieldname == "title")
		{ title = s; continue; }

	if (fieldname == "cn")
		{ givenName = s; continue; }

	if (fieldname == "o")
		{ organization = s; continue; }

	if (fieldname == "description")
		{ comment = s; lastWasComment = true; continue; }

	if (fieldname == "homeurl")
		{ homepage = s;	continue; }

	if (fieldname == "homephone" || fieldname == "telephonenumber") {
		if (!phone.isEmpty())
			info->log(i18n("Discarding phone number %1 from entry of %2 %3")
					.arg(s)
					.arg(firstName)
					.arg(lastName));
		else
  			phone = s;
		continue; 
	}

	if (fieldname == "postalcode")
		{ zipCode = s;	continue; }

	if (fieldname == "facsimiletelephonenumber")
		{ fax = s; continue; }
	
	if (fieldname == "streetaddress")
		{ street = s; continue; }

	if (fieldname == "locality")
		{ locality = s; continue; }
	
	if (fieldname == "countryname")
		{ country = s; continue; }
		
	if (fieldname == "cellphone")
		{ mobile = s; continue; }

	if (fieldname == "st")
		{ state = s; continue; }

	if (fieldname == "ou")
		{ department = s; continue; }

	if (fieldname == "objectclass" && s == "groupOfNames")
			isGroup = true;

	info->log(i18n("Unable to handle line: %1").arg(completeline));
  	  	
    } /* while !eof(f) */

    f.close();
    
    info->log(i18n("%1 phonebook entries sucessfully imported.").arg(numEntries));

    kabStop(info);
    return true;
}
