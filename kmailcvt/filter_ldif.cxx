/***************************************************************************
                          filter_ldif.cxx  -  description
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

#include <qdir.h>

#include <kfiledialog.h>
#include <klocale.h>

#include "filter_ldif.hxx"

filter_ldif::filter_ldif() : filter(i18n("Import Netscape LDIF Address Book 1(.LDIF)"),"Oliver Strutynski")
{}

filter_ldif::~filter_ldif()
{}

void filter_ldif::import(filterInfo *info) {
   QWidget *parent=info->parent();

   QString filename = KFileDialog::getOpenFileName( QDir::homeDirPath(), 
		   	"*.ldif *.LDIF *.Ldif", parent);
   if (filename.isEmpty()) {
     info->alert(name(),i18n("No Addressbook chosen"));
     return;
   }

   QString from( i18n("Source: ") + "\t" + filename );
   QString to( i18n("Destination: ") + "\t" + i18n("the KAddressBook") );

   info->from(from);
   info->to(to);
   info->current(i18n("Currently converting .LDIF address file to Kab"));

   convert(filename, info);

   info->current(100.0);
   info->overall(100.0);
   info->current(i18n("Finished converting .LDIF address file to Kab"));
}


bool filter_ldif::convert(const QString &filename, filterInfo *info) {
   if (!kabStart(info))
	return false;

   QString caption( i18n("Import Netscape LDIF Personal Addressbook (.LDIF)") );

   QFile f(filename);
   if ( !f.open(IO_ReadOnly) ) {
	QString msg =  i18n("Can't open '%1' for reading").arg(filename);
	info->alert(caption,msg);
	return false;
   }

   QString empty;
   QString givenName, email, title, firstName, lastName, nickName,
	street, locality, state, zipCode, country, organization,
	department, phone, fax, mobile, homepage, comment;

   // Initializing code table for base64 decoding
   initCodeTable();

   QTextStream t( &f );
   QString s, fieldname;

   // We need this for calculating progress
   uint fileSize = f.size();
   uint bytesProcessed = 0;

   // Set to true if data currently read is part of
   // a list of names. Lists will be ignored.
   bool isGroup = false;

   while ( !t.eof() ) {
	s = t.readLine();
	bytesProcessed += s.length();
	
	// update progress information
    	info->current((float)bytesProcessed/fileSize*100);
    	info->overall((float)bytesProcessed/fileSize*100);
	
	if (s.isEmpty()) {
		// Newline: Write data
		if (!isGroup) {
  			kabAddress( info, i18n("Netscape Addressbook"),
				givenName, email, title, firstName, empty, lastName, nickName,
				street, locality, state, zipCode, country, organization, 
				department, empty, empty, phone, fax, mobile, empty, homepage, 
				empty, comment, empty);
			givenName = email = title = firstName = lastName = nickName =
			 street = locality = state = zipCode = country = organization =
			 department = phone = fax = mobile = homepage = comment = "";

   		} else {
			info->log(i18n("Warning: List data is being ignored."));
   		}

		isGroup = false;
		continue;
	}
	
	int position = s.find("::");
    	if (position != -1) {
    		// String is BASE64 encoded
    		fieldname = s.left(position).lower();
    		s = decodeBase64(s.mid(position+3, s.length()-position-2));
    	} else {
    		position = s.find(":");
    		fieldname = s.left(position).lower();
    		// Convert Utf8 string to unicode so special characters are preserved
		// We need this since we are reading normal strings from the file
		// which are not converted automatically
		s = QString::fromUtf8(s.mid(position+2, s.length()-position-2).latin1());
	}

	if (s.stripWhiteSpace().isEmpty())
		continue;

	if (fieldname == "givenname")
		{ firstName = s; continue; }

	if (fieldname == "xmozillanickname")
		{ nickName = s; continue; }
	
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
		{ comment = s; continue; }

	if (fieldname == "homeurl")
		{ homepage = s;	continue; }

	if (fieldname == "homephone" || fieldname == "telephonenumber") {
		if (!phone.isEmpty()) info->log(i18n("Discarding Phone Number %1").arg(s));
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
  	  	
    } /* while !eof(f) */

    f.close();

    kabStop(info);
    return true;
}


/*
 * Decodes a BASE-64 encoded stream to recover the original data and compacts white space.
 * Code heavily based on java code written by Kevin Kelley (kelley@ruralnet.net)
 * published unter the GNU Library Public License
*/
QString filter_ldif::decodeBase64(QString input)
{
    QCString result;

    int tempLen = input.length();
    for(unsigned int i=0; i<input.length(); i++) {
        if(codes[ input[i].latin1() ] < 0) {
	   // std::cout << "Invalid character in base64 string: " <<
	   // input[i].latin1() << std::endl;
	   --tempLen; // ignore non-valid chars and padding
        }
    }

    // calculate required length:
    //  -- 3 bytes for every 4 valid base64 chars
    //  -- plus 2 bytes if there are 3 extra base64 chars,
    //     or plus 1 byte if there are 2 extra.
    int len = (tempLen / 4) * 3;
    if ((tempLen % 4) == 3) len += 2;
    if ((tempLen % 4) == 2) len += 1;

    int shift = 0; // # of excess bits stored in accum
    int accum = 0; // excess bits

    // we now loop over through the entire string
    for (unsigned int i=0; i<input.length(); i++) {
        int value = codes[ input[i].latin1() ];

        if ( value >= 0 ) {         // skip over non-code
            accum <<= 6;            // bits shift up by 6 each time thru
            shift += 6;             // loop, with new bits being put in
            accum |= value;         // at the bottom.
            if ( shift >= 8 ) {      // whenever there are 8 or more shifted in,
                shift -= 8;          // write them out (from the top, leaving any
                                    // excess at the bottom for next iteration.
                result += (char) ((accum >> shift) & 0xff);
            }
        }
    }

    // Remove any linefeeds, tabs and multiple space from decoded string and
    // convert to unicode.
    return QString::fromUtf8(result).simplifyWhiteSpace();
}


/* Initialize lookup */
void filter_ldif::initCodeTable() {
    // chars for 0..63
    for (int i=0; i<256; i++) codes[i] = -1;
    for (int i = 'A'; i <= 'Z'; i++) codes[i] = (int)(i - 'A');
    for (int i = 'a'; i <= 'z'; i++) codes[i] = (int)(26 + i - 'a');
    for (int i = '0'; i <= '9'; i++) codes[i] = (int)(52 + i - '0');
    codes['+'] = 62;
    codes['/'] = 63;
}

