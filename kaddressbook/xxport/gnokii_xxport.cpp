/*
    This file is part of KAddressbook.
    Copyright (c) 2003 - 2003 Helge Deller <deller@kde.org>

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

/* 
    Description:
    This filter allows you to import and export the KDE addressbook entries
    to/from a mobile phone, which is accessible via gnokii.
    Gnokii homepage is at: http://www.gnokii.org

    TODO:
	- handle callergroup value (Friend, VIP, Family, ...)
	- export to phone
	- modal mode (show dialog during up/download)
*/

#include "config.h"

#include <klocale.h>
#include <kmessagebox.h>
#include <kabc/addressbook.h>

#include <kdebug.h>

#ifdef HAVE_GNOKII_H
extern "C" {
#include <gnokii.h>
}
#endif

#include "gnokii_xxport.h"

#define APP "GNOKII_XXPORT"

class GNOKIIXXPortFactory : public XXPortFactory
{
  public:
    XXPortObject *xxportObject( KABCore *core, QObject *parent, const char *name )
    {
      return new GNOKIIXXPort( core, parent, name );
    }
};

extern "C"
{
  void *init_libkaddrbk_gnokii_xxport()
  {
    return ( new GNOKIIXXPortFactory() );
  }
}


GNOKIIXXPort::GNOKIIXXPort( KABCore *core, QObject *parent, const char *name )
  : XXPortObject( core, parent, name )
{
  createImportAction( i18n( "Import from Mobile Phone..." ) );
  createExportAction( i18n( "Export to Mobile Phone..." ) );
}

/* import */

#ifdef HAVE_GNOKII_H
static char *BinDir;
static char *lockfile = NULL;

static struct gn_statemachine state;
static gn_data data;

static QString businit(void)
{
	gn_error error;
	char *aux;

	gn_data_clear(&data);

	aux = gn_cfg_get(gn_cfg_info, "global", "use_locking");
	/* Defaults to 'no' */
	if (aux && !strcmp(aux, "yes")) {
		lockfile = gn_device_lock(state.config.port_device);
		if (lockfile == NULL) {
			return i18n("Lock file error.\n "
			"Please exit all other running instances of gnokii and try again.");
		}
	}

	/* Initialise the code for the GSM interface. */
	error = gn_gsm_initialise(&state);
	if (error != GN_ERR_NONE) {
		return i18n("Telephone interface init failed: %1").arg(gn_error_print(error));
	}

	return QString::null;
}

static void busterminate(void)
{
	gn_sm_functions(GN_OP_Terminate, NULL, &state);
	if (lockfile) gn_device_unlock(lockfile);
}

static gn_error phone_memory_entries( gn_memory_type memtype, gn_memory_status *memstat )
{
	gn_error error;

	gn_data_clear(&data);
	memset(memstat, 0, sizeof(*memstat));
	memstat->memory_type = memtype;
	data.memory_status = memstat;
	error = gn_sm_functions(GN_OP_GetMemoryStatus, &data, &state);
	if (error != GN_ERR_NONE) {
		switch (memtype) {
		  case GN_MT_SM:
			// use at least 100 entries
			memstat->used = 0;
			memstat->free = 100;
			break;
		  default:
		  case GN_MT_ME:
			// Phone doesn't support ME (5110)
			memstat->used = memstat->free = 0;
			break;
		}
	}
	return error;
}


static gn_error read_phone_entries( const char *memtypestr, gn_memory_type memtype, KABC::AddresseeList *addrList )
{
  gn_error error;
  gn_memory_status memstat;
  
  // get number of entries in this phone memory type (internal/SIM-card)
  error = phone_memory_entries(memtype, &memstat);

  if (error != GN_ERR_NONE)
	return error;

  kdWarning() << QString("Memory-Type: %1, used=%2, free=%3\n")
			.arg(memtypestr).arg(memstat.used).arg(memstat.free);

  gn_phonebook_entry entry;
  QStringList addrlist;
  KABC::Address *addr;
  QString s;

  int num_read = 0;

  for (int i = 1; i <= (memstat.used + memstat.free); i++) {
	entry.memory_type = memtype;
	entry.location = i;
	data.phonebook_entry = &entry;
	error = gn_sm_functions(GN_OP_ReadPhonebook, &data, &state);
	if (error)
	   kdWarning() << QString("entry=%1: ERROR: %2 (%3)\n").arg(i).arg(gn_error_print(error)).arg(error);
	if (error == GN_ERR_EMPTYLOCATION)
		continue;
	if (error == GN_ERR_INVALIDLOCATION)
		break;
	if (error == GN_ERR_INVALIDMEMORYTYPE)
		break;
	if (error == GN_ERR_NONE) {
		kdWarning() << QString("%1: %2, num=%3, location=%4, group=%5, count=%6\n").arg(i).arg(entry.name)
			.arg(entry.number).arg(entry.location).arg(entry.caller_group).arg(entry.subentries_count);
		KABC::Addressee *a = new KABC::Addressee();
		
		/* try to split Name into FamilyName and GivenName */
		s = QString(entry.name).stripWhiteSpace();
		if (s.find(',')!=-1) {
		  addrlist = QStringList::split(',', s);
		  if (addrlist.count()==2) {
			a->setFamilyName(addrlist[0].stripWhiteSpace());
			a->setGivenName(addrlist[1].stripWhiteSpace());
		  } else
			a->setGivenName(s);
		} else {
		  addrlist = QStringList::split(' ', s);
		  if (addrlist.count()==2) {
			a->setFamilyName(addrlist[1].stripWhiteSpace());
			a->setGivenName(addrlist[0].stripWhiteSpace());
		  } else
			a->setGivenName(s);
		}

		a->insertCustom(APP, "X_GSM_CALLERGROUP", s.setNum(entry.caller_group));
		a->insertCustom(APP, "X_GSM_STORE_AT:", QString("%1%2").arg(memtypestr).arg(entry.location));

		if (!entry.subentries_count)
		  a->insertPhoneNumber(KABC::PhoneNumber(entry.number, KABC::PhoneNumber::Work | KABC::PhoneNumber::Pref));

                /* scan sub-entries */
		if (entry.subentries_count)
		 for (int n=0; n<entry.subentries_count; n++) {
		  QString s = QString(entry.subentries[n].data.number).stripWhiteSpace();
		  kdWarning() << QString(" Subentry#%1, entry_type=%2, number_type=%3, number=%4\n")
				.arg(n).arg(entry.subentries[n].entry_type)
				.arg(entry.subentries[n].number_type).arg(s); 
		  if (s.isEmpty())
			continue;
		  switch(entry.subentries[n].entry_type) {
		   case GN_PHONEBOOK_ENTRY_Name:
			a->setName(s);
			break;
		   case GN_PHONEBOOK_ENTRY_Email:
			a->insertEmail(s);
			break;
		   case GN_PHONEBOOK_ENTRY_Postal:
			addrlist = QStringList::split(',', s, true);
			addr = new KABC::Address(KABC::Address::Work);
			if (addrlist.count() == 3) {
			  addr->setLocality(addrlist[0].stripWhiteSpace());
			  addr->setPostalCode(addrlist[1].stripWhiteSpace());
			  addr->setCountry(i18n(addrlist[2].stripWhiteSpace()));
			} else {
			  addr->setStreet(s.stripWhiteSpace());
			}
			a->insertAddress(*addr);
			delete addr;
			break;
		   case GN_PHONEBOOK_ENTRY_Note:
			if (!a->note().isEmpty())
				s = "\n" + s;
			a->setNote(a->note()+s);
			break;
		   case GN_PHONEBOOK_ENTRY_Number:
			enum KABC::PhoneNumber::Types phonetype;
			switch (entry.subentries[n].number_type) {
			 case GN_PHONEBOOK_NUMBER_Mobile: phonetype = KABC::PhoneNumber::Cell; break;
			 case GN_PHONEBOOK_NUMBER_Fax:    phonetype = KABC::PhoneNumber::Fax;  break;
			 case GN_PHONEBOOK_NUMBER_General:
			 case GN_PHONEBOOK_NUMBER_Work:   phonetype = KABC::PhoneNumber::Work; break;
			 default:
			 case GN_PHONEBOOK_NUMBER_Home:   phonetype = KABC::PhoneNumber::Home; break;
			}
			//if (s == entry.number)
			//  type = (KABC::PhoneNumber::Types) (phonetype | KABC::PhoneNumber::Pref);
			a->insertPhoneNumber(KABC::PhoneNumber(s, phonetype));
			break;
		   case GN_PHONEBOOK_ENTRY_URL:
			a->setUrl(s);
			break;
		   case GN_PHONEBOOK_ENTRY_Group:
			a->insertCategory(s);
			break;
		   default:
			kdWarning() << QString(" Not handled id=%1, entry=%2\n")
				.arg(entry.subentries[n].entry_type).arg(s);
			break;
		  } // switch()
		} // if(subentry)
		
		addrList->append(*a);
		// did we read all valid phonebook-entries ?
		num_read++;
		if (num_read >= memstat.used)
			break;	// yes, all were read
		else
			continue; // no, we are still missing some.
	}
	kdWarning() << QString("GNOKII Error: %1\n").arg(gn_error_print(error));
  }

  return GN_ERR_NONE;
}
#endif




KABC::AddresseeList GNOKIIXXPort::importContacts( const QString& ) const
{
  KABC::AddresseeList addrList;

#ifndef HAVE_GNOKII_H

  KMessageBox::error(core(), i18n("Gnokii interface is not available.\n"
		"Please ask your distributor to add gnokii during compile time."));

#else

  if (gn_cfg_read(&BinDir)<0 || !gn_cfg_phone_load("", &state)) {
	KMessageBox::error(core(), i18n("GNOKII isn't yet configured."));
	return addrList;
  }

  QString errStr = businit();
  if (!errStr.isEmpty()) {
	KMessageBox::error(core(), errStr);
	return addrList;
  }

  kdWarning() << "GNOKII import filter started.\n";
  
  read_phone_entries("ME", GN_MT_ME, &addrList); // internal phone memory
  read_phone_entries("SM", GN_MT_SM, &addrList); // SIM card

  kdWarning() << "GNOKII import filter finished.\n";

  busterminate();

#endif

  return addrList;
}


/* export */

bool GNOKIIXXPort::exportContacts( const KABC::AddresseeList &list, const QString& )
{
  Q_UNUSED(list);

#ifndef HAVE_GNOKII_H

  KMessageBox::error(core(), i18n("Gnokii interface is not available.\n"
		"Please ask your distributor to add gnokii during compile time."));

#else

  KMessageBox::information(core(), i18n("Export to phone not yet implemented.\n"));

#endif

  return true;
}

#include "gnokii_xxport.moc"
