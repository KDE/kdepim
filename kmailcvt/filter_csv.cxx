/***************************************************************************
                          FilterCSV.cxx  -  description
                             -------------------
    begin                : Tue Feb 18, 2003
    copyright            : (C) 2003 by Laurence Anderson
    email                : l.d.anderson@warwick.ac.uk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kfiledialog.h>
#include <klocale.h>
#include <kmdcodec.h>
#include <qstringlist.h>
#include <kdebug.h>

#include "filter_csv.hxx"

FilterCSV::FilterCSV() 
  : Filter(i18n("Import CSV files"), 
	"Laurence Anderson",
	i18n("<p>This filter will import CSV files exported by the Windows address book</p>")
    )
{}

FilterCSV::~FilterCSV()
{}

void FilterCSV::import(FilterInfo *info) {
   QWidget *parent = info->parent();

   QString filename = KFileDialog::getOpenFileName( QDir::homeDirPath(), 
		   	"*.[cC][sS][vV]", parent);
   if (filename.isEmpty()) {
     info->alert(i18n("No Addressbook chosen"));
     return;
   }

   info->from(filename);
   info->to(i18n("KAddressBook"));
   info->current(i18n("Currently converting .CSV address file to KAddressBook"));

   convert(filename, info);

   info->current(100);
   info->overall(100);
   info->current(i18n("Finished converting .CSV address file to KAddressBook"));
}


bool FilterCSV::convert(const QString &filename, FilterInfo *info) {
   if (!openAddressBook(info)) return false;

   QString caption( i18n("Import Netscape LDIF Personal Addressbook (.LDIF)") );

   QFile f(filename);
   f.open(IO_ReadOnly);
   QTextStream csv(&f);
   
   // We need this for calculating progress
   uint fileSize = f.size();
   uint bytesProcessed = 0;
   uint numEntries = 0;
   
   // First read away first line
   QString csvLine = csv.readLine();

   while ((csvLine = csv.readLine()) != QString::null) {
      KABC::Addressee a;

      QStringList csvEntrys = QStringList::split(",", csvLine);
      numEntries++;

      int loopIndex = 0;
      KABC::Address home;
      KABC::Address work;
      for ( QStringList::Iterator csvEntry = csvEntrys.begin(); csvEntry != csvEntrys.end(); ++csvEntry, ++loopIndex ) {
         // Name,E-mail Address,Home Street,Home City,Home Postal Code,Home State,Home Country/Region,Home Phone,Business Street,Business City,Business Postal Code,Business State,Business Country/Region,Business Phone,Company,Job Title
         switch (loopIndex) {
		case 0: a.setFormattedName(*csvEntry); break; // Name FIXME: setNameFromString?
		case 1: a.insertEmail(*csvEntry); break; // E-mail
		case 2: home.setStreet(*csvEntry); break; // Home street
		case 3: home.setLocality(*csvEntry); break; // Home city
		case 4: home.setPostalCode(*csvEntry); break; // Home postcode
		case 5: home.setRegion(*csvEntry); break; // Home state
		case 6: home.setCountry(*csvEntry); break; // Home country
		case 7: a.insertPhoneNumber( KABC::PhoneNumber( *csvEntry, KABC::PhoneNumber::Home | KABC::PhoneNumber::Voice ) ); break; // Home phone
		case 8: work.setStreet(*csvEntry); break; // Business Street
		case 9: work.setLocality(*csvEntry); break; // Business city
		case 10: work.setPostalCode(*csvEntry); break; // Busines Postal code
		case 11: work.setRegion(*csvEntry); break; // Business state
		case 12: work.setCountry(*csvEntry); break; // Business country
		case 13: a.insertPhoneNumber( KABC::PhoneNumber (*csvEntry, KABC::PhoneNumber::Work | KABC::PhoneNumber::Voice ) ); break; // Business phone
		case 14: a.setOrganization(*csvEntry); break; // Company
		case 15: a.setRole(*csvEntry); break; // Job title
         }
      }
      a.insertAddress(home);
      a.insertAddress(work);
      //kdDebug() << givenName << " " << email << endl;
      if (!a.formattedName().isEmpty() || a.emails().count() > 1) {
         addContact ( a );
      } else {
         info->log(i18n("Warning: CSV line ignored, as each line must have a valid name and email address."));
      }
      
      bytesProcessed += csvLine.length();
      // update progress information
      info->current(100 * bytesProcessed / fileSize);
      info->overall(100 * bytesProcessed / fileSize);
   }

   f.close();
    
   if (closeAddressBook()) info->log(i18n("%1 addressbook entries sucessfully imported.").arg(numEntries));
   else info->log(i18n("Couldn't import contacts"));

   return true;
}
