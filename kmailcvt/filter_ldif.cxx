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


#include "filter_ldif.hxx"

#include <kfiledialog.h>
#include <klocale.h>
#include <qtextstream.h>
#include <stdlib.h>


filter_ldif::filter_ldif() : filter(i18n("Import Netscape LDIF Adress Book (.LDIF)"),"Oliver Strutynski")
{}

filter_ldif::~filter_ldif()
{}

void filter_ldif::import(filterInfo *info) {
   const char* file;

   // Removed file selection!!

   file="/home/olistrut/test1.ldif";

   QString from=i18n("from: "),to=i18n("to: ");
   from+="\t"; from+=file;
   to+="\t"; to+=i18n("the K Address Book");
   info->from(from);
   info->to(to);
   info->current(i18n("Currently converting .LDIF address file to Kab"));

   convert(file, info);

   info->current(100.0);
   info->overall(100.0);
   info->current(i18n("Done converting .LDIF address file to Kab"));
}


#define STRCPY(a,b)	a=b
#define STRLEN(a)	a.length()

bool filter_ldif::convert(const char *filename, filterInfo *info) {
   //bool ret;
   QString caption;
   caption=i18n("Import Netscape LDIF Personal Adressbook (.LDIF)");

   if (!kabStart(info)) {
     info->alert(caption,"Error starting KAB");
     return false;
   }

   // cout << "Converting file..." << filename << "\n";
   QFile f(filename);

   QString firstName="", email="", lastName="";
   QString title=""; QString givenName=""; QString comment="";
   QString organization=""; QString homepage=""; QString locality="";
   QString street=""; QString zipCode=""; QString phone="";
   QString fax=""; QString country=""; QString mobile=""; QString state="";
   QString department=""; QString empty="";

   // Initializing code table for base64 decoding
   initCodeTable();

   if ( f.open(IO_ReadOnly) ) {
       QTextStream t( &f );
       QString s;
       QString fieldname;

       // We need this for calculating progress
       uint fileSize = f.size();
       uint bytesProcessed = 0;

       // Set to true if data currently read is part of
       // a list of names. Lists will be ignored.
       bool isGroup=false;

       while ( !t.eof() ) {
	   s = t.readLine();
	   bytesProcessed += s.length();
	   if (s.isEmpty()) {
		// Newline: Write data
		if (!isGroup) {
			kabAddress(info,i18n("Netscape Addressbook"), givenName, email, title,firstName,empty,lastName,
		           	street, locality, state, zipCode, country, organization, department,
			   	empty, empty, phone, fax, mobile, empty, homepage, empty,
			   	comment,empty);

			STRCPY(firstName,""); STRCPY(email,""); STRCPY(lastName,""); STRCPY(title,"");
			STRCPY(givenName,""); STRCPY(comment,""); STRCPY(organization,""); STRCPY(homepage,"");
			STRCPY(locality,""); STRCPY(street,""); STRCPY(zipCode,""); STRCPY(phone,"");
			STRCPY(fax,""); STRCPY(country,""); STRCPY(mobile,""); STRCPY(state,""); STRCPY(department,"");
   		} else {
			info->log("Warning: List data is being ignored.");
   		}
		isGroup=false;
	   } else {

		int position = s.find("::");
		if (position != -1) {
			// String is BASE64 encoded
			fieldname = s.left(position);
			s = decodeBase64(s.mid(position+3, s.length()-position));
		} else {
			position = s.find(":");
			fieldname = s.left(position);
			// Convert Utf8 string to unicode so special characters are preserved
			//s = QString::fromUtf8(s.mid(position+2, s.length()-position));
                        // This seems not neccesary because s is already QString
			s = s.mid(position+2); //, s.length()-position));
		}

		if (s.length()>1023) {
	          s=s.left(1023);
                  info->log("Warning: Truncating entry: "+fieldname+" - "+s);
		}

		if (fieldname == "givenname") {
			STRCPY(firstName,s);
   		} else if (fieldname == "sn") {
			STRCPY(lastName, s);
   		} else if (fieldname == "mail") {
			STRCPY(email, s);
   		} else if (fieldname == "title") {
			STRCPY(title, s);
   		} else if (fieldname == "cn") {
			STRCPY(givenName, s);
   		} else if (fieldname == "o") {
			STRCPY(organization, s);
   		} else if (fieldname == "description") {
			STRCPY(comment, s);
   		} else if (fieldname == "homeurl") {
			STRCPY(homepage, s);
   		} else if (fieldname == "homephone") {
			if (STRLEN(phone) > 0) { info->log("Discarding Phone Number "+s); }
			STRCPY(phone, s);
   		} else if (fieldname == "telephonenumber") {
			if (STRLEN(phone) > 0) { info->log("Discarding Phone Number "+s); }
			STRCPY(phone, s);
   		} else if (fieldname == "postalcode") {
			STRCPY(zipCode, s);
   		} else if (fieldname == "facsimiletelephonenumber") {
			STRCPY(fax, s);
   		} else if (fieldname == "streetaddress") {
			STRCPY(street, s);
   		} else if (fieldname == "locality") {
			STRCPY(locality, s);
   		} else if (fieldname == "countryname") {
			STRCPY(country, s);
   		} else if (fieldname == "cellphone") {
			STRCPY(mobile, s);
   		} else if (fieldname == "st") {
			STRCPY(state, s);
   		} else if (fieldname == "ou") {
			STRCPY(department, s);
   		} else if (fieldname == "objectclass") {
			if (s == "groupOfNames") { isGroup = true; }
   		}
		// update progress information
		info->current((float)bytesProcessed/fileSize*100);
		info->overall((float)bytesProcessed/fileSize*100);
	   }
       }
       f.close();
   } else {
     char msg[1024];
     sprintf(msg,i18n("Can't open '%s' for reading").latin1(),filename);
     info->alert(caption,msg);
     return false;
   }

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

    // cout << "  Trying to decode base64 string: " << input << "\n";
    QString result;

    int tempLen = input.length();
    for(unsigned int i=0; i<input.length(); i++) {
        if(codes[ input[i].latin1() ] < 0) {
	   // cout << "Invalid character in base64 string: " << input[i].latin1() << "\n";
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
    //result = QString::fromUtf8(result.simplifyWhiteSpace());
    result = result.simplifyWhiteSpace();
    return result;
}


/* Initialize lookup */
void filter_ldif::initCodeTable() {
    // chars for 0..63
    //const char* alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
    for (int i=0; i<256; i++) codes[i] = -1;
    for (int i = 'A'; i <= 'Z'; i++) codes[i] = (int)(i - 'A');
    for (int i = 'a'; i <= 'z'; i++) codes[i] = (int)(26 + i - 'a');
    for (int i = '0'; i <= '9'; i++) codes[i] = (int)(52 + i - '0');
    codes['+'] = 62;
    codes['/'] = 63;
}

