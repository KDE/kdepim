/*
    This file is part of KAddressbook.
    Copyright (c) 2003-2004 Helge Deller <deller@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.

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
    Gnokii homepage: http://www.gnokii.org

    TODO:
	- create a log file and give user possibility to see it afterwards
	- handle callergroup value (Friend, VIP, Family, ...) better
*/

#include "config.h"

#include <qcursor.h>

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprogress.h>
#include <kguiitem.h>

#ifdef HAVE_GNOKII_H
extern "C" {
#include <gnokii.h>
}
#else
#ifdef __GNUC__
# warning "Please install the gnokii development headers and libraries !"
#endif
#endif

#include "gnokii_xxport.h"

#define APP "GNOKII_XXPORT"

#if 1 // !defined(NDEBUG)
 #define GNOKII_DEBUG(x)	do { kdWarning() << (x); } while (0)
#else
 #define GNOKII_DEBUG(x)	do { } while (0)
#endif
#define GNOKII_CHECK_ERROR(error) \
	do { \
		if (error) \
			kdError() << QString("ERROR %1: %2\n").arg(error).arg(gn_error_print(error));\
	} while (0)

// Locale conversion routines:
// Gnokii uses the local 8 Bit encoding (based on LC_ALL), kaddressbook uses Unicode
#define GN_FROM(x)	QString::fromLocal8Bit(x)
#define GN_TO(x)	(x).local8Bit()

// static variables for GUI updates
static GNOKIIXXPort *this_filter;
static KProgressDialog *m_progressDlg;

K_EXPORT_KADDRESSBOOK_XXFILTER( libkaddrbk_gnokii_xxport, GNOKIIXXPort )

GNOKIIXXPort::GNOKIIXXPort( KABC::AddressBook *ab, QWidget *parent, const char *name )
  : KAB::XXPort( ab, parent, name )
{
	this_filter = this;
	m_progressDlg = NULL;
	createImportAction( i18n( "Import From Mobile Phone..." ) );
	createExportAction( i18n( "Export to Mobile Phone..." ) );
}

/* import */

#ifdef HAVE_GNOKII_H
static char *lockfile = NULL;
static char manufacturer[64], model[GN_MODEL_MAX_LENGTH+1],
            revision[GN_REVISION_MAX_LENGTH+1], imei[GN_IMEI_MAX_LENGTH+1];
static QString PhoneProductId;

static struct gn_statemachine state;
static gn_data data;

static void busterminate(void)
{
	gn_sm_functions(GN_OP_Terminate, NULL, &state);
	if (lockfile) gn_device_unlock(lockfile);
}

static QString businit(void)
{
	gn_error error;
	char *aux;

#if defined(LIBGNOKII_VERSION)
	if (gn_cfg_read_default()<0)
#else
	static char *BinDir;
	if (gn_cfg_read(&BinDir)<0)
#endif
		return i18n("Failed to initialize the gnokii library.");

	if (!gn_cfg_phone_load("", &state))
		return i18n("Gnokii is not yet configured.");

	// uncomment to debug all gnokii communication on stderr.
	// gn_log_debug_mask = GN_LOG_T_STDERR;

	gn_data_clear(&data);

	aux = gn_cfg_get(gn_cfg_info, "global", "use_locking");
	// Defaults to 'no'
	if (aux && !strcmp(aux, "yes")) {
		lockfile = gn_device_lock(state.config.port_device);
		if (lockfile == NULL) {
			return i18n("Gnokii reports a 'Lock File Error'.\n "
			"Please exit all other running instances of gnokii, check if you have "
			"write permissions in the /var/lock directory and try again.");
		}
	}

	// Initialise the code for the GSM interface.
	int old_dcd = state.config.require_dcd; // work-around for older gnokii versions
	state.config.require_dcd = false;
	error = gn_gsm_initialise(&state);
	GNOKII_CHECK_ERROR(error);
	state.config.require_dcd = old_dcd;
	if (error != GN_ERR_NONE) {
		busterminate();
		return i18n("<qt><center>Mobile Phone interface initialization failed.<br><br>"
			"The returned error message was:<br><b>%1</b><br><br>"
			"You might try to run \"gnokii --identify\" on the command line to "
			"check any cable/transport issues and to verify if your gnokii "
			"configuration is correct.</center></qt>")
			.arg(gn_error_print(error));
	}

	// identify phone
	gn_data_clear(&data);
	data.manufacturer = manufacturer;
	data.model = model;
	data.revision = revision;
	data.imei = imei;

	QCString unknown(GN_TO(i18n("Unknown")));
	qstrncpy(manufacturer, unknown, sizeof(manufacturer)-1);
	qstrncpy(model, unknown, sizeof(model)-1);
	qstrncpy(revision, unknown, sizeof(revision)-1);
	qstrncpy(imei, unknown, sizeof(imei)-1);

	if (m_progressDlg->wasCancelled())
		return QString::null;
	else
		error = gn_sm_functions(GN_OP_Identify, &data, &state);
	GNOKII_CHECK_ERROR(error);

	GNOKII_DEBUG( QString("Found mobile phone: %1 %2, Revision: %3, IMEI: %4\n")
				.arg(manufacturer, model, revision, imei) );

	PhoneProductId = QString("%1-%2-%3-%4").arg(APP).arg(model).arg(revision).arg(imei);

	return QString::null;
}


// get number of entries in this phone memory type (internal/SIM-card)
static gn_error read_phone_memstat( const gn_memory_type memtype, gn_memory_status *memstat )
{
	gn_error error;

	gn_data_clear(&data);
	memset(memstat, 0, sizeof(*memstat));
	memstat->memory_type = memtype;
	data.memory_status = memstat;
	error = gn_sm_functions(GN_OP_GetMemoryStatus, &data, &state);
	GNOKII_CHECK_ERROR(error);
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
	GNOKII_DEBUG( QString("\n\nMobile phone memory status: Type: %1, used=%2, free=%3, total=%4\n\n")
					.arg(memtype).arg(memstat->used).arg(memstat->free).arg(memstat->used+memstat->free) );
	return error;
}


// read phone entry #index from memory #memtype
static gn_error read_phone_entry( const int index, const gn_memory_type memtype, gn_phonebook_entry *entry )
{
	gn_error error;
	entry->memory_type = memtype;
	entry->location = index;
	data.phonebook_entry = entry;
	error = gn_sm_functions(GN_OP_ReadPhonebook, &data, &state);
	GNOKII_CHECK_ERROR(error);
	return error;
}

static bool phone_entry_empty( const int index, const gn_memory_type memtype )
{
	gn_error error;
	gn_phonebook_entry entry;
	entry.memory_type = memtype;
	entry.location = index;
	data.phonebook_entry = &entry;
	error = gn_sm_functions(GN_OP_ReadPhonebook, &data, &state);
	if (error == GN_ERR_EMPTYLOCATION)
		return true;
	GNOKII_CHECK_ERROR(error);
	if (error == GN_ERR_NONE && entry.empty)
		return true;
	return false;
}

static QString buildPhoneInfoString( const gn_memory_status &memstat )
{
	QString format = QString::fromLatin1("<tr><td><b>%1</b></td><td>%2</td></tr>");

	return QString::fromLatin1("<b>%1</b><br><table>%2%3%4%5%6</table><br>")
		.arg(i18n("Mobile Phone information:"))
		.arg(format.arg(i18n("Manufacturer")).arg(GN_FROM(manufacturer)))
		.arg(format.arg(i18n("Phone model")).arg(GN_FROM(model)))
		.arg(format.arg(i18n("Revision")).arg(GN_FROM(revision)))
		.arg(format.arg(i18n("IMEI")).arg(GN_FROM(imei)))
		.arg(format.arg(i18n("Phonebook status"))
			   .arg(i18n("%1 out of %2 contacts used").arg(memstat.used).arg(memstat.used+memstat.free)));
}

static QString buildMemoryTypeString( gn_memory_type memtype )
{
	switch (memtype) {
	case GN_MT_ME:	return i18n("internal memory");
	case GN_MT_SM:	return i18n("SIM-card memory");
	default:	return i18n("unknown memory");
	}
}

// read and evaluate all phone entries
static gn_error read_phone_entries( const char *memtypestr, gn_memory_type memtype,
			KABC::AddresseeList *addrList )
{
  gn_error error;

  if (m_progressDlg->wasCancelled())
	return GN_ERR_NONE;

  KProgress* progress = (KProgress*)m_progressDlg->progressBar();

  progress->setProgress(0);
  this_filter->processEvents();

  // get number of entries in this phone memory type (internal/SIM-card)
  gn_memory_status memstat;
  error = read_phone_memstat(memtype, &memstat);

  gn_phonebook_entry entry;
  QStringList addrlist;
  KABC::Address *addr;
  QString s, country;

  progress->setTotalSteps(memstat.used);
  m_progressDlg->setLabel(i18n("<qt>Importing <b>%1</b> contacts from <b>%2</b> of the Mobile Phone.<br><br>%3</qt>")
		.arg(memstat.used)
		.arg(buildMemoryTypeString(memtype))
		.arg(buildPhoneInfoString(memstat)) );

  int num_read = 0;

  for (int i = 1; !m_progressDlg->wasCancelled() && i <= memstat.used + memstat.free; i++) {
	error = read_phone_entry( i, memtype, &entry );

	progress->setProgress(num_read);
  	this_filter->processEvents();

	if (error == GN_ERR_EMPTYLOCATION)
		continue;
	if (error == GN_ERR_INVALIDLOCATION)
		break;
	if (error == GN_ERR_INVALIDMEMORYTYPE)
		break;
	if (error == GN_ERR_NONE) {
		GNOKII_DEBUG(QString("%1: %2, num=%3, location=%4, group=%5, count=%6\n").arg(i).arg(GN_FROM(entry.name))
			.arg(GN_FROM(entry.number)).arg(entry.location).arg(entry.caller_group).arg(entry.subentries_count));
		KABC::Addressee *a = new KABC::Addressee();

		// try to split Name into FamilyName and GivenName
		s = GN_FROM(entry.name).simplifyWhiteSpace();
		a->setFormattedName(s); // set formatted name as in Phone
		if (s.find(',') == -1) {
		  // assumed format: "givenname [... familyname]"
		  addrlist = QStringList::split(' ', s);
		  if (addrlist.count() == 1) {
			// only one string -> put it in the GivenName
			a->setGivenName(s);
		  } else {
			// multiple strings -> split them.
			a->setFamilyName(addrlist.last().simplifyWhiteSpace());
			addrlist.remove(addrlist.last());
			a->setGivenName(addrlist.join(" ").simplifyWhiteSpace());
		  }
		} else {
		  // assumed format: "familyname, ... givenname"
		  addrlist = QStringList::split(',', s);
		  a->setFamilyName(addrlist.first().simplifyWhiteSpace());
		  addrlist.remove(addrlist.first());
		  a->setGivenName(addrlist.join(" ").simplifyWhiteSpace());
		}

		a->insertCustom(APP, "X_GSM_CALLERGROUP", s.setNum(entry.caller_group));
		a->insertCustom(APP, "X_GSM_STORE_AT", QString("%1%2").arg(memtypestr).arg(entry.location));

		// set ProductId
		a->setProductId(PhoneProductId);

		// evaluate timestamp (ignore timezone)
		QDateTime datetime;
		if (entry.date.year<1998)
			datetime = QDateTime::currentDateTime();
		else
			datetime = QDateTime( QDate(entry.date.year, entry.date.month, entry.date.day),
							  QTime(entry.date.hour, entry.date.minute, entry.date.second) );
		GNOKII_DEBUG(QString(" date=%1\n").arg(datetime.toString()));
		a->setRevision(datetime);

		if (!entry.subentries_count)
		  a->insertPhoneNumber(KABC::PhoneNumber(entry.number, KABC::PhoneNumber::Work | KABC::PhoneNumber::Pref));

		/* scan sub-entries */
		if (entry.subentries_count)
		 for (int n=0; n<entry.subentries_count; n++) {
		  QString s = GN_FROM(entry.subentries[n].data.number).simplifyWhiteSpace();
		  GNOKII_DEBUG(QString(" Subentry#%1, entry_type=%2, number_type=%3, number=%4\n")
				.arg(n).arg(entry.subentries[n].entry_type)
				.arg(entry.subentries[n].number_type).arg(s));
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
			addrlist = QStringList::split(';', s, true);
			addr = new KABC::Address(KABC::Address::Work);
			if (addrlist.count() <= 1) {
				addrlist = QStringList::split(',', s, true);
				if (addrlist.count() > 1 ) {
					// assumed format: "Locality, ZIP, Country"
					addr->setLocality(addrlist[0]);
					addr->setPostalCode(addrlist[1]);
					if (!addrlist[2].isEmpty())
						addr->setCountry(i18n(GN_TO(addrlist[2])));
				} else {
					// no idea about the format, just store it.
					addr->setLocality(s);
				}
			} else {
				// assumed format: "POBox; Extended; Street; Locality; Region; ZIP [;Country]
				addr->setPostOfficeBox(addrlist[0]);
				addr->setExtended(addrlist[1]);
				addr->setStreet(addrlist[2]);
				addr->setLocality(addrlist[3]);
				addr->setRegion(addrlist[4]);
				addr->setPostalCode(addrlist[5]);
				country = addrlist[6];
				if (!country.isEmpty())
					addr->setCountry(i18n(GN_TO(country)));
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
			GNOKII_DEBUG(QString(" Not handled id=%1, entry=%2\n")
				.arg(entry.subentries[n].entry_type).arg(s));
			break;
		  } // switch()
		} // if(subentry)

		// add only if entry was valid
		if (strlen(entry.name) || strlen(entry.number) || entry.subentries_count)
			addrList->append(*a);

		// did we read all valid phonebook-entries ?
		num_read++;
		delete a;
		if (num_read >= memstat.used)
			break;	// yes, all were read
		else
			continue; // no, we are still missing some.
	}
	GNOKII_CHECK_ERROR(error);
  }

  return GN_ERR_NONE;
}
#endif



KABC::AddresseeList GNOKIIXXPort::importContacts( const QString& ) const
{
	KABC::AddresseeList addrList;

#ifndef HAVE_GNOKII_H

	KMessageBox::error(parentWidget(), i18n("Gnokii interface is not available.\n"
		"Please ask your distributor to add gnokii at compile time."));

#else

	if (KMessageBox::Continue != KMessageBox::warningContinueCancel(parentWidget(),
		i18n("<qt>Please connect your Mobile Phone to your computer and press "
		     "<b>Continue</b> to start importing the personal contacts.<br><br>"
		     "Please note that if your Mobile Phone is not properly connected "
		     "the following detection phase might take up to two minutes, during which "
                     "KAddressbook will behave unresponsively.</qt>") ))
	  return addrList;

	m_progressDlg = new KProgressDialog( parentWidget(), "importwidget",
		i18n("Mobile Phone Import"),
		i18n("<qt><center>Establishing connection to the Mobile Phone.<br><br>"
		     "Please wait...</center></qt>") );
	m_progressDlg->setAllowCancel(true);
	m_progressDlg->progressBar()->setProgress(0);
	m_progressDlg->progressBar()->setCenterIndicator(true);
	m_progressDlg->setModal(true);
	m_progressDlg->setInitialSize(QSize(450,350));
	m_progressDlg->show();
  	processEvents();

#if (QT_VERSION >= 0x030300)
	m_progressDlg->setCursor( Qt::BusyCursor );
#endif
	QString errStr = businit();
	m_progressDlg->unsetCursor();

	if (!errStr.isEmpty()) {
		KMessageBox::error(parentWidget(), errStr);
		delete m_progressDlg;
		return addrList;
	}

	GNOKII_DEBUG("GNOKII import filter started.\n");
	m_progressDlg->setButtonText(i18n("&Stop Import"));

	read_phone_entries("ME", GN_MT_ME, &addrList); // internal phone memory
	read_phone_entries("SM", GN_MT_SM, &addrList); // SIM card

	GNOKII_DEBUG("GNOKII import filter finished.\n");

	busterminate();
	delete m_progressDlg;

#endif

	return addrList;
}


// export to phone

#ifdef HAVE_GNOKII_H

static QString makeValidPhone( const QString &number )
{
	// allowed chars: 0-9, *, #, p, w, +
	QString num = number.simplifyWhiteSpace();
	QString allowed("0123456789*+#pw");
	for (unsigned int i=num.length(); i>=1; i--)
		if (allowed.find(num[i-1])==-1)
			num.remove(i-1,1);
	if (num.isEmpty())
		num = "0";
	return num;
}

static gn_error xxport_phone_write_entry( int phone_location, gn_memory_type memtype,
			const KABC::Addressee *addr)
{
	gn_phonebook_entry entry;
	QString s;

	memset(&entry, 0, sizeof(entry));
	strncpy(entry.name, GN_TO(addr->realName()), sizeof(entry.name)-1);
	s = addr->phoneNumber(KABC::PhoneNumber::Pref).number();
	if (s.isEmpty())
		s = addr->phoneNumber(KABC::PhoneNumber::Work).number();
	if (s.isEmpty())
		s = addr->phoneNumber(KABC::PhoneNumber::Home).number();
	if (s.isEmpty())
		s = addr->phoneNumber(KABC::PhoneNumber::Cell).number();
	if (s.isEmpty() && addr->phoneNumbers().count()>0)
		s = (*addr->phoneNumbers().at(0)).number();
	s = makeValidPhone(s);
	strncpy(entry.number, s.ascii(), sizeof(entry.number)-1);
	entry.memory_type = memtype;
	QString cg = addr->custom(APP, "X_GSM_CALLERGROUP");
	if (cg.isEmpty())
		entry.caller_group = 5;		// default group
	else
		entry.caller_group = cg.toInt();
	entry.location = phone_location;

	// set date/revision
	QDateTime datetime = addr->revision();
	QDate date(datetime.date());
	QTime time(datetime.time());
	entry.date.year = date.year();
	entry.date.month = date.month();
	entry.date.day = date.day();
	entry.date.hour = time.hour();
	entry.date.minute = time.minute();
	entry.date.second = time.second();

	GNOKII_DEBUG(QString("Write #%1: name=%2, number=%3\n").arg(phone_location)
					.arg(GN_FROM(entry.name)).arg(GN_FROM(entry.number)));

	const KABC::Address homeAddr = addr->address(KABC::Address::Home);
	const KABC::Address workAddr = addr->address(KABC::Address::Work);

	entry.subentries_count = 0;
	gn_phonebook_subentry *subentry = &entry.subentries[0];
	// add all phone numbers
	const KABC::PhoneNumber::List phoneList = addr->phoneNumbers();
	KABC::PhoneNumber::List::ConstIterator it;
	for ( it = phoneList.begin(); it != phoneList.end(); ++it ) {
		const KABC::PhoneNumber *phonenumber = &(*it);
		s = phonenumber->number();
		if (s.isEmpty()) continue;
		subentry->entry_type  = GN_PHONEBOOK_ENTRY_Number;
		gn_phonebook_number_type type;
		switch (phonenumber->type() & ~KABC::PhoneNumber::Pref) {
			case KABC::PhoneNumber::Home:	type = GN_PHONEBOOK_NUMBER_Home;	break;
			case KABC::PhoneNumber::Voice:
			case KABC::PhoneNumber::Work:	type = GN_PHONEBOOK_NUMBER_Work;	break;
			case KABC::PhoneNumber::Pager:
			case KABC::PhoneNumber::Cell:	type = GN_PHONEBOOK_NUMBER_Mobile;	break;
			case KABC::PhoneNumber::Fax:	type = GN_PHONEBOOK_NUMBER_Fax;		break;
			default:			type = GN_PHONEBOOK_NUMBER_General;	break;
		}
		subentry->number_type = type;
		strncpy(subentry->data.number, makeValidPhone(s).ascii(), sizeof(subentry->data.number)-1);
		subentry->id = phone_location<<8+entry.subentries_count;
		entry.subentries_count++;
		subentry++;
		if (entry.subentries_count >= GN_PHONEBOOK_SUBENTRIES_MAX_NUMBER)
			break; // Phonebook full
	}
	// add URL
	s = addr->url().prettyURL();
	if (!s.isEmpty() && (entry.subentries_count<GN_PHONEBOOK_SUBENTRIES_MAX_NUMBER)) {
		subentry->entry_type = GN_PHONEBOOK_ENTRY_URL;
		strncpy(subentry->data.number, GN_TO(s), sizeof(subentry->data.number)-1);
		entry.subentries_count++;
		subentry++;
	}
	// add E-Mails
	QStringList emails = addr->emails();
	for (unsigned int n=0; n<emails.count(); n++) {
		if (entry.subentries_count >= GN_PHONEBOOK_SUBENTRIES_MAX_NUMBER)
			break; // Phonebook full
		s = emails[n].simplifyWhiteSpace();
		if (s.isEmpty()) continue;
		// only one email allowed if we have URLS, notes, addresses (to avoid phone limitations)
		if (n && !addr->url().isEmpty() && !addr->note().isEmpty() && addr->addresses().count()) {
			GNOKII_DEBUG(QString(" DROPPED email %1 in favor of URLs, notes and addresses.\n")
					.arg(s));
			continue;
		}
		subentry->entry_type  = GN_PHONEBOOK_ENTRY_Email;
		strncpy(subentry->data.number, GN_TO(s), sizeof(subentry->data.number)-1);
		entry.subentries_count++;
		subentry++;
	}
	// add Adresses
	const KABC::Address::List addresses = addr->addresses();
	KABC::Address::List::ConstIterator it2;
	for ( it2 = addresses.begin(); it2 != addresses.end(); ++it2 ) {
		if (entry.subentries_count >= GN_PHONEBOOK_SUBENTRIES_MAX_NUMBER)
			break; // Phonebook full
		const KABC::Address *Addr = &(*it2);
		if (Addr->isEmpty()) continue;
		subentry->entry_type  = GN_PHONEBOOK_ENTRY_Postal;
		QStringList a;
		QChar sem(';');
		QString sem_repl(QString::fromLatin1(","));
      		a.append( Addr->postOfficeBox().replace( sem, sem_repl ) );
		a.append( Addr->extended()     .replace( sem, sem_repl ) );
		a.append( Addr->street()       .replace( sem, sem_repl ) );
		a.append( Addr->locality()     .replace( sem, sem_repl ) );
		a.append( Addr->region()       .replace( sem, sem_repl ) );
		a.append( Addr->postalCode()   .replace( sem, sem_repl ) );
		a.append( Addr->country()      .replace( sem, sem_repl ) );
		s = a.join(sem);
		strncpy(subentry->data.number, GN_TO(s), sizeof(subentry->data.number)-1);
		entry.subentries_count++;
		subentry++;
	}
	// add Note
	s = addr->note().simplifyWhiteSpace();
	if (!s.isEmpty() && (entry.subentries_count<GN_PHONEBOOK_SUBENTRIES_MAX_NUMBER)) {
		subentry->entry_type = GN_PHONEBOOK_ENTRY_Note;
		strncpy(subentry->data.number, GN_TO(s), sizeof(subentry->data.number)-1);
		entry.subentries_count++;
		subentry++;
	}

	// debug output
	for (int st=0; st<entry.subentries_count; st++) {
		gn_phonebook_subentry *subentry = &entry.subentries[st];
		GNOKII_DEBUG(QString(" SubTel #%1: entry_type=%2, number_type=%3, number=%4\n")
						.arg(st).arg(subentry->entry_type)
						.arg(subentry->number_type).arg(GN_FROM(subentry->data.number)));
	}

	data.phonebook_entry = &entry;
	gn_error error = gn_sm_functions(GN_OP_WritePhonebook, &data, &state);
	GNOKII_CHECK_ERROR(error);

	return error;
}


static gn_error xxport_phone_delete_entry( int phone_location, gn_memory_type memtype )
{
	gn_phonebook_entry entry;
	memset(&entry, 0, sizeof(entry));
	entry.empty = 1;
	entry.memory_type = memtype;
	entry.location = phone_location;
	data.phonebook_entry = &entry;
	GNOKII_DEBUG(QString("Deleting entry %1\n").arg(phone_location));
	gn_error error = gn_sm_functions(GN_OP_WritePhonebook, &data, &state);
	GNOKII_CHECK_ERROR(error);
	return error;
}

#endif

bool GNOKIIXXPort::exportContacts( const KABC::AddresseeList &list, const QString & )
{
#ifndef HAVE_GNOKII_H

	Q_UNUSED(list);
	KMessageBox::error(parentWidget(), i18n("Gnokii interface is not available.\n"
		"Please ask your distributor to add gnokii at compile time."));

#else
	if (KMessageBox::Continue != KMessageBox::warningContinueCancel(parentWidget(),
		i18n("<qt>Please connect your Mobile Phone to your computer and press "
		     "<b>Continue</b> to start exporting the selected personal contacts.<br><br>"
		     "Please note that if your Mobile Phone is not properly connected "
		     "the following detection phase might take up to two minutes, during which "
		     "KAddressbook will behave unresponsively.</qt>") ))
	  return false;

	m_progressDlg = new KProgressDialog( parentWidget(), "importwidget",
		i18n("Mobile Phone Export"),
		i18n("<qt><center>Establishing connection to the Mobile Phone.<br><br>"
		     "Please wait...</center></qt>") );
	m_progressDlg->setAllowCancel(true);
	m_progressDlg->progressBar()->setProgress(0);
	m_progressDlg->progressBar()->setCenterIndicator(true);
	m_progressDlg->setModal(true);
	m_progressDlg->setInitialSize(QSize(450,350));
	m_progressDlg->show();
  	processEvents();

	KProgress* progress = (KProgress*)m_progressDlg->progressBar();

	KABC::AddresseeList::ConstIterator it;
	QStringList failedList;

	gn_error error;
	bool deleteLabelInitialized = false;

#if (QT_VERSION >= 0x030300)
	m_progressDlg->setCursor( Qt::BusyCursor );
#endif
	QString errStr = businit();
	m_progressDlg->unsetCursor();

	if (!errStr.isEmpty()) {
		KMessageBox::error(parentWidget(), errStr);
		delete m_progressDlg;
		return false;
	}

	GNOKII_DEBUG("GNOKII export filter started.\n");

	gn_memory_type memtype = GN_MT_ME;	// internal phone memory

	int phone_count;	// num entries in phone
	bool overwrite_phone_entries = false;
	int phone_entry_no, entries_written;
	bool entry_empty;

	// get number of entries in this phone memory
	gn_memory_status memstat;
	error = read_phone_memstat(memtype, &memstat);
	if (error == GN_ERR_NONE) {
		GNOKII_DEBUG("Writing to internal phone memory.\n");
	} else {
		memtype = GN_MT_SM;	// try SIM card instead
		error = read_phone_memstat(memtype, &memstat);
		if (error != GN_ERR_NONE)
			goto finish;
		GNOKII_DEBUG("Writing to SIM card memory.\n");
	}
	phone_count = memstat.used;

	if (memstat.free >= (int) list.count()) {
		if (KMessageBox::No == KMessageBox::questionYesNo(parentWidget(),
			i18n("<qt>Do you want the selected contacts to be <b>appended</b> to "
			     "the current mobile phonebook or should they <b>replace</b> all "
			     "currently existing phonebook entries ?<br><br>"
			     "Please note, that in case you choose to replace the phonebook "
			     "entries, every contact in your phone will be deleted and only "
			     "the newly exported contacts will be available from inside your phone.</qt>"),
			i18n("Export to Mobile Phone"),
			KGuiItem(i18n("&Append to Current Phonebook")),
			KGuiItem(i18n("&Replace Current Phonebook with New Contacts")) ) )
				overwrite_phone_entries = true;
	}

  	progress->setTotalSteps(list.count());
	entries_written = 0;
	progress->setProgress(entries_written);
	m_progressDlg->setButtonText(i18n("&Stop Export"));
	m_progressDlg->setLabel(i18n("<qt>Exporting <b>%1</b> contacts to the <b>%2</b> "
			"of the Mobile Phone.<br><br>%3</qt>")
		.arg(list.count())
		.arg(buildMemoryTypeString(memtype))
		.arg(buildPhoneInfoString(memstat)) );

	// Now run the loop...
	phone_entry_no = 1;
	for ( it = list.begin(); it != list.end(); ++it ) {
		const KABC::Addressee *addr = &(*it);
		if (addr->isEmpty())
			continue;
		// don't write back SIM-card entries !
		if (addr->custom(APP, "X_GSM_STORE_AT").startsWith("SM"))
			continue;

		progress->setProgress(entries_written++);

try_next_phone_entry:
  		this_filter->processEvents();
		if (m_progressDlg->wasCancelled())
			break;

		// End of phone memory reached ?
		if (phone_entry_no > (memstat.used + memstat.free))
			break;

		GNOKII_DEBUG(QString("Try to write entry '%1' at phone_entry_no=%2, phone_count=%3\n")
				.arg(addr->realName()).arg(phone_entry_no).arg(phone_count));

		error = GN_ERR_NONE;

		// is this phone entry empty ?
		entry_empty = phone_entry_empty(phone_entry_no, memtype);
		if (overwrite_phone_entries) {
			// overwrite this phonebook entry ...
			if (!entry_empty)
				phone_count--;
			error = xxport_phone_write_entry( phone_entry_no, memtype, addr);
			phone_entry_no++;
		} else {
			// add this phonebook entry if possible ...
			if (entry_empty) {
				error = xxport_phone_write_entry( phone_entry_no, memtype, addr);
				phone_entry_no++;
			} else {
				phone_entry_no++;
				goto try_next_phone_entry;
			}
		}

		if (error != GN_ERR_NONE)
			failedList.append(addr->realName());

		// break if we got an error on the first entry
		if (error != GN_ERR_NONE && it==list.begin())
			break;

	} // for()

	// if we wanted to overwrite all entries, make sure, that we also
	// delete all remaining entries in the mobile phone.
	while (overwrite_phone_entries && error==GN_ERR_NONE && phone_count>0) {
		if (m_progressDlg->wasCancelled())
			break;
		if (!deleteLabelInitialized) {
			m_progressDlg->setLabel(
				i18n("<qt><center>"
				     "All selected contacts have been sucessfully copied to "
				     "the Mobile Phone.<br><br>"
				     "Please wait until all remaining orphaned contacts from "
				     "the Mobile Phone have been deleted.</center></qt>") );
			m_progressDlg->setButtonText(i18n("&Stop Delete"));
			deleteLabelInitialized = true;
  			progress->setTotalSteps(phone_count);
			entries_written = 0;
			progress->setProgress(entries_written);
  			this_filter->processEvents();
		}
		if (phone_entry_no > (memstat.used + memstat.free))
			break;
		entry_empty = phone_entry_empty(phone_entry_no, memtype);
		if (!entry_empty) {
			error = xxport_phone_delete_entry(phone_entry_no, memtype);
			phone_count--;
			progress->setProgress(++entries_written);
  			this_filter->processEvents();
		}
		phone_entry_no++;
	}

finish:
	m_progressDlg->setLabel(i18n("Export to phone finished."));
	this_filter->processEvents();

	GNOKII_DEBUG("GNOKII export filter finished.\n");

	busterminate();
	delete m_progressDlg;

	if (!failedList.isEmpty()) {
		GNOKII_DEBUG(QString("Failed to export: %1\n").arg(failedList.join(", ")));
		KMessageBox::informationList(parentWidget(),
                        i18n("<qt>The following contacts could not be exported to the Mobile Phone. "
			     "Possible Reasons for this problem could be:<br><ul>"
			     "<li>The contacts contain more information per entry than the phone can store.</li>"
			     "<li>Your phone does not allow to store multiple addresses, emails, homepages, ...</li>"
			     "<li>other storage size related problems.</li>"
			     "</ul>"
			     "To avoid those kind of problems in the future please reduce the amount of different "
			     "fields in the above contacts.</qt>"),
			failedList,
			i18n("Mobile Phone Export") );
	}

#endif

	return true;
}

#include "gnokii_xxport.moc"
/* vim: set sts=4 ts=4 sw=4: */

