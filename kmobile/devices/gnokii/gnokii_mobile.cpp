/*  This file is part of the KDE mobile library.
    Copyright (C) 2003-2005 Helge Deller <deller@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qstring.h>
#include <qstringlist.h>
#include <qdir.h>

#include <klibloader.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <kio/global.h>
#include <kdebug.h>
#include <klocale.h>

#include <gnokii.h>

#include "gnokii_mobile.h"
#include "gnokiiconfig.h"


#define KGNOKII_DEBUG_AREA 5730
#define PRINT_DEBUG kdDebug(KGNOKII_DEBUG_AREA) << "KMobileGnokii: "
#define GNOKII_DEBUG(x)   PRINT_DEBUG << x


#define APP "KMobileGnokii"
#define GNOKII_CHECK_ERROR(error) \
	do { \
		if (error) \
			PRINT_DEBUG << QString("ERROR %1: %2\n").arg(error).arg(gn_error_print(error));\
	} while (0)


/* This is a loaded library, which is initialized with the line below */
K_EXPORT_COMPONENT_FACTORY( libkmobile_gnokii, KMobileGnokii() )

/* createObject needs to be reimplemented by every KMobileDevice driver */
QObject *KMobileGnokii::createObject( QObject *parent, const char *name,
        const char *, const QStringList &args )
{
  return new KMobileGnokii( parent, name, args );
}


static char *BinDir;
static char *lockfile = NULL;
static char model[GN_MODEL_MAX_LENGTH+1], revision[GN_REVISION_MAX_LENGTH+1], imei[GN_IMEI_MAX_LENGTH+1];
static QString PhoneProductId;

static gn_statemachine state;
static gn_data data;
static gn_calnote_list calnote_list;


/**
 *  The KDE gnokii mobile device driver.
 */

KMobileGnokii::KMobileGnokii(QObject *obj, const char *name, const QStringList &args )
	: KMobileDevice(obj, name, args)
{
  // set initial device info
  setClassType( Phone );
  m_deviceName = i18n("Mobile Phone accessed via GNOKII");
  m_deviceRevision = "";
  m_connectionName = "/dev/ircomm0";
  setCapabilities( hasAddressBook | hasNotes );

  m_numAddresses = -1;

  // now initialize the configuration based on the
  // given config file from args[0]
  loadDeviceConfiguration();
  if (m_modelnr.isEmpty())
	loadGnokiiConfiguration();

  if (m_modelnr.isEmpty()) {
	// default communcation values
	m_modelnr = "6310";
	m_connection = "infrared";
	m_port = "/dev/ircomm0";
	m_baud = "9600";
  }

  PRINT_DEBUG << QString("Using GNOKII configuration: %1 %2 %3 %4\n").arg(m_modelnr)
		.arg(m_connection).arg(m_port).arg(m_baud);

  saveDeviceConfiguration();
  saveGnokiiConfiguration();
}

KMobileGnokii::~KMobileGnokii()
{
}


/******************************************************************************************
 *	GNOKII lowlevel interface
 ******************************************************************************************/

static gn_connection_type connectionToValue( QString connectionName )
{
  if (connectionName == "serial")
	return GN_CT_Serial;
  if (connectionName == "dau9p")
	return GN_CT_DAU9P;
  if (connectionName == "dlr3p")
	return GN_CT_DLR3P;
  if (connectionName == "infrared")
	return GN_CT_Infrared;
  if (connectionName == "m2bus")
	return GN_CT_M2BUS;
  if (connectionName == "irda")
	return GN_CT_Irda;
  if (connectionName == "bluetooth")
	return GN_CT_Bluetooth;
//#ifndef WIN32
  if (connectionName == "tcp")
	return GN_CT_TCP;
//#endif
  if (connectionName == "tekram")
	return GN_CT_Tekram;
  return GN_CT_Serial; /* default */
}

bool KMobileGnokii::setGnokiiStateMachine()
{
  // set the state machine to our configuration
  qstrncpy( state.config.model, m_modelnr.utf8(), sizeof(state.config.model)-1 );
  qstrncpy( state.config.port_device, m_port.utf8(), sizeof(state.config.port_device)-1 );
  state.config.connection_type = connectionToValue(m_connection);
  state.config.serial_baudrate = m_baud.toUInt();
  return true;
}

bool KMobileGnokii::saveConfig( KConfig &conf, QString group )
{
  conf.setGroup(group);
  conf.writeEntry("model", m_modelnr );
  conf.writeEntry("port", m_port );
  conf.writeEntry("connection", m_connection );
//  conf.writeEntry("initlength", "default" );
  conf.writeEntry("serial_baudrate", m_baud );
//  conf.writeEntry("serial_write_usleep", "0" );
//  conf.writeEntry("handshake", "" ); // software (or:rtscts), hardware (or:xonxoff)
  conf.writeEntry("require_dcd", "1" ); 
//  conf.writeEntry("smsc_timeout", "1" ); 
  conf.sync();
  return true;
}

bool KMobileGnokii::loadConfig( KConfig &conf, QString group )
{
  conf.setGroup(group);
  m_modelnr    = conf.readEntry("model", m_modelnr );
  m_port       = conf.readEntry("port", m_port );
  m_connection = conf.readEntry("connection", m_connection );
  m_baud       = conf.readEntry("serial_baudrate", m_baud );
  return true;
}

bool KMobileGnokii::saveGnokiiConfiguration()
{
  KConfig conf( QDir::homeDirPath() + "/.gnokiirc", false, false, "" );
  return saveConfig( conf, "global" );
}

bool KMobileGnokii::loadGnokiiConfiguration()
{
  KConfig conf( QDir::homeDirPath() + "/.gnokiirc", true, false, "" );
  return loadConfig( conf, "global" );
}

bool KMobileGnokii::saveDeviceConfiguration()
{
  return saveConfig( *config(), "global" );
}

bool KMobileGnokii::loadDeviceConfiguration()
{
  return loadConfig( *config(), "global" );
}


static void busterminate(void)
{
	gn_sm_functions(GN_OP_Terminate, NULL, &state);
	if (lockfile) gn_device_unlock(lockfile);
}

static QString businit(void)
{
	gn_error error;
	char *aux;

	if (gn_cfg_read(&BinDir)<0 || !gn_cfg_phone_load("", &state))
		return i18n("GNOKII isn't yet configured.");

	gn_data_clear(&data);

	aux = gn_cfg_get(gn_cfg_info, "global", "use_locking");
	// Defaults to 'no'
	if (aux && !strcmp(aux, "yes")) {
		lockfile = gn_device_lock(state.config.port_device);
		if (lockfile == NULL) {
			return i18n("Lock file error.\n "
			"Please exit all other running instances of gnokii and try again.");
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
		return i18n("Mobile phone interface initialization failed:\n%1").arg(gn_error_print(error));
	}

	// model
	gn_data_clear(&data);
	data.model = model;
	model[0] = 0;
	error = gn_sm_functions(GN_OP_GetModel, &data, &state);
	GNOKII_CHECK_ERROR(error);
	if (model[0] == 0)
		strcpy(model, i18n("unknown").utf8());
	data.model = NULL;

	// revision
	data.revision = revision;
	revision[0] = 0;
	error = gn_sm_functions(GN_OP_GetRevision, &data, &state);
	GNOKII_CHECK_ERROR(error);
	data.revision = NULL;

	// imei	
	data.imei = imei;
	imei[0] = 0;
	error = gn_sm_functions(GN_OP_GetImei, &data, &state);
	GNOKII_CHECK_ERROR(error);
	data.imei = NULL;

	GNOKII_DEBUG( QString("Found mobile phone: Model: %1, Revision: %2, IMEI: %3\n")
				.arg(model).arg(revision).arg(imei) ); 

	PhoneProductId = QString("%1-%2-%3-%4").arg(APP).arg(model).arg(revision).arg(imei);

	return QString::null;
}


// get number of entries in this phone memory type (internal/SIM-card)
static gn_error read_phone_memstat( gn_memory_type memtype, gn_memory_status *memstat )
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
static gn_error read_phone_entry( int index, gn_memory_type memtype, gn_phonebook_entry *entry )
{
	gn_error error;
	entry->memory_type = memtype;
	entry->location = index;
	data.phonebook_entry = entry;
	error = gn_sm_functions(GN_OP_ReadPhonebook, &data, &state);
	GNOKII_CHECK_ERROR(error);
	return error;
}

#if 0
static bool phone_entry_empty( int index, gn_memory_type memtype )
{
	gn_phonebook_entry entry;
	gn_error error;
	error = read_phone_entry( index, memtype, &entry );
	if (error == GN_ERR_EMPTYLOCATION)
		return true;
	if (error == GN_ERR_NONE && entry.empty)
		return true;
	return false;
}
#endif

static int gn_error2kio_error( gn_error err )
{
  if (err != GN_ERR_NONE)
	GNOKII_CHECK_ERROR(err);

  switch (err) {
	case GN_ERR_NONE:
		return 0;
	case GN_ERR_INVALIDMEMORYTYPE:
	case GN_ERR_INVALIDLOCATION:
	case GN_ERR_EMPTYLOCATION:
		return KIO::ERR_DOES_NOT_EXIST;
	case GN_ERR_MEMORYFULL:
		return KIO::ERR_OUT_OF_MEMORY;
	case GN_ERR_NOLINK:
		return KIO::ERR_COULD_NOT_CONNECT;
	case GN_ERR_TIMEOUT:
		return KIO::ERR_SERVER_TIMEOUT;
	case GN_ERR_ENTRYTOOLONG:
	case GN_ERR_WRONGDATAFORMAT:
	case GN_ERR_INVALIDSIZE:
		return KIO::ERR_COULD_NOT_WRITE;
	default:
		return KIO::ERR_INTERNAL;
  }
}

static gn_error read_phone_entry_highlevel( int index, const gn_memory_type memtype, KABC::Addressee *a )
{
  gn_phonebook_entry entry;
  QStringList addrlist;
  QString s, country;
  KABC::Address *addr;
  gn_error error;

//  if (index > (memstat.used + memstat.free))
//	return GN_ERR_INVALIDLOCATION;

  error = read_phone_entry( index, memtype, &entry );
  if (error != GN_ERR_NONE)
      return error;

  GNOKII_DEBUG(QString("%1: %2, num=%3, location=%4, group=%5, count=%6\n").arg(index).arg(entry.name)
	.arg(entry.number).arg(entry.location).arg(entry.caller_group).arg(entry.subentries_count));

  // try to split Name into FamilyName and GivenName
  s = QString(entry.name).simplifyWhiteSpace();
  a->setFormattedName(s);
  if (s.find(',')!=-1) {
  	addrlist = QStringList::split(',', s);
	if (addrlist.count()==2) {
		a->setFamilyName(addrlist[0].simplifyWhiteSpace());
		a->setGivenName(addrlist[1].simplifyWhiteSpace());
	  } else
		a->setGivenName(s);
  } else {
  	addrlist = QStringList::split(' ', s);
	  if (addrlist.count()==2) {
		a->setFamilyName(addrlist[1].simplifyWhiteSpace());
		a->setGivenName(addrlist[0].simplifyWhiteSpace());
	  } else
		a->setGivenName(s);
  }

  a->insertCustom(APP, "X_GSM_CALLERGROUP", s.setNum(entry.caller_group));
  a->insertCustom(APP, "X_GSM_STORE_AT", QString("%1_%2").arg(GN_MT_ME).arg(entry.location));

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
		QString s = QString(entry.subentries[n].data.number).simplifyWhiteSpace();
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
				addrlist = QStringList::split(',', s, true);
				addr = new KABC::Address(KABC::Address::Work);
				switch (addrlist.count()) {
				 case 4:	addr->setStreet(addrlist[0].simplifyWhiteSpace());
						addr->setLocality(addrlist[1].simplifyWhiteSpace());
						addr->setPostalCode(addrlist[2].simplifyWhiteSpace());
						country = addrlist[3].simplifyWhiteSpace();
						if (!country.isEmpty())
							addr->setCountry(i18n(country.utf8()));
						break;
				 case 3:	addr->setLocality(addrlist[0].simplifyWhiteSpace());
						addr->setPostalCode(addrlist[1].simplifyWhiteSpace());
						country = addrlist[2].simplifyWhiteSpace();
						if (!country.isEmpty())
							addr->setCountry(i18n(country.utf8()));
						break;
				 default:	addr->setStreet(s.simplifyWhiteSpace());
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
	} // for(subentry)

  GNOKII_CHECK_ERROR(error);
  return error;
}

/******************************************************************************************
 *	
 ******************************************************************************************/



// connect the device and ask user to turn device on (if necessary)
bool KMobileGnokii::connectDevice(QWidget * /*parent*/)
{
  if (connected())
	return true;

  QString err = businit();
  m_connected = err.isEmpty();
  PRINT_DEBUG << QString("connectDevice() : %1\n").arg(m_connected ? "Ok" : err);
  return m_connected;
}

// disconnect the device and return true, if sucessful
bool KMobileGnokii::disconnectDevice(QWidget * /*parent*/)
{
  if (!connected())
	return true;
  busterminate();
  m_connected = false;
  PRINT_DEBUG << QString("disconnectDevice() : %1\n").arg("done");
  return true;
}

// provice the own configuration dialog
bool KMobileGnokii::configDialog(QWidget *parent)
{
  QString model, connection, port, baud;
  int ok = 0;
  GnokiiConfig *dialog = new GnokiiConfig(parent);
  if (dialog) {
	dialog->setValues(m_modelnr, m_connection, m_port, m_baud);
	ok = dialog->exec();
  }
  dialog->getValues(model, connection, port, baud);
  delete dialog;
  if (ok == QDialog::Accepted) {
	m_modelnr = model;
	m_connection = connection;
	m_port = port;
	m_baud = baud;
	saveDeviceConfiguration();
  };
  return true;
}

QString KMobileGnokii::iconFileName() const
{
  return "mobile_phone";
}

// return a unique ID, e.g. the IMEI number of phones, or a serial number
// this String is used to have a unique identification for syncronisation.
QString KMobileGnokii::deviceUniqueID()
{ 
  return QString("GNOKII-IMEI-%1").arg(QString::fromLocal8Bit(imei));
}



/**********************************************************************************
 * Addressbook / Phonebook support                                                *
 **********************************************************************************/

int KMobileGnokii::numAddresses()
{
  if (!connectDevice(NULL))
	return 0;

  gn_memory_status memstat;
  gn_error error;

  if (m_numAddresses>=0)
	return m_numAddresses;

  error = read_phone_memstat( GN_MT_ME, &memstat );
  GNOKII_CHECK_ERROR(error);
  if (error)
	memstat.used = -1;

  m_numAddresses = memstat.used;

  if (m_numAddresses>0) {
	// initialize the addrList array
	m_addrList.clear();
	KABC::Addressee addr;
	for (int i=0; i<=m_numAddresses; i++)
		m_addrList.append(addr);
  }

  return m_numAddresses;
}  

int KMobileGnokii::readAddress( int index, KABC::Addressee &addr )
{
  PRINT_DEBUG << QString("############   GET ADDRESS #%1\n").arg(index);
  // index is zero-based, but in gnokii the first address starts at 1
  if (index<0 || index>=numAddresses())
    return KIO::ERR_DOES_NOT_EXIST;

  // now get our addressbook entry

  // do we have this entry in the cache already ?
  if (m_addrList.count() > (unsigned)index && !m_addrList[index].isEmpty()) {
	addr = m_addrList[index];
	return 0;
  }

  gn_error err = read_phone_entry_highlevel(index+1, GN_MT_ME, &addr );
  if (!err)
	m_addrList[index] = addr;

  return gn_error2kio_error(err);
}

int KMobileGnokii::storeAddress( int, const KABC::Addressee &, bool )
{
  /* XXX: this is a read-only device */
  return KIO::ERR_WRITE_ACCESS_DENIED;
}



/**********************************************************************************
 * Calendar support                                                               *
 **********************************************************************************/

static void QDateTime_2_timestamp( const QDateTime &qdt, gn_timestamp &ts )
{
  ts.year = qdt.date().year();
  ts.month = qdt.date().month();
  ts.day = qdt.date().day();
  ts.hour = qdt.time().hour();
  ts.minute = qdt.time().minute();
  ts.second = qdt.time().second();
  ts.timezone = 0;
}

static QDateTime timestamp_2_QDateTime( const gn_timestamp &ts )
{
  return QDateTime(	QDate(ts.year, ts.month, ts.day),
			QTime(ts.hour, ts.minute, ts.second)    );
}

static void print_calnote( const gn_calnote &entry )
{
  gn_timestamp ts = entry.time;
  kdWarning()	<< "location=" << entry.location       /* The number of the note in the phone memory */
		<< "  type=" <<  entry.type                /* The type of the note */
		<< "  gn_timestamp=" <<         /* The time of the note */  
		" year=" << ts.year <<
		" month=" << ts.month <<
		" day=" << ts.day <<
		" hour=" << ts.hour <<
		" minute=" << ts.minute <<
		" second=" << ts.second <<
		" TZ=" << ts.timezone 
		<< "  gn_calnote_alarm="<< entry.alarm.enabled   /* The alarm of the note */
		<< "  text=" << entry.text		   /* The text of the note */
		<< "  number=" << entry.phone_number	/* For Call only: the phone number */
		<< "  recurr=" << entry.recurrence << endl << endl;
}




int KMobileGnokii::numCalendarEntries()
{
  gn_data_clear(&data);
  gn_calnote entry;

  memset(&entry, 0, sizeof(entry));
  data.calnote = &entry;
  entry.location = 1;
  data.calnote_list = &calnote_list;

  gn_error error = gn_sm_functions(GN_OP_GetCalendarNote, &data, &state);
  switch (error) {
	case GN_ERR_NONE:
	case GN_ERR_INVALIDLOCATION:
        case GN_ERR_EMPTYLOCATION:
		return calnote_list.number;
	default:
		GNOKII_CHECK_ERROR(error);
        	return 0;
  }
}

int KMobileGnokii::readCalendarEntry( int index, KCal::Event &event )
{
  if (index < 0 || index >= GN_CALNOTE_MAX_NUMBER)
	return KIO::ERR_DOES_NOT_EXIST;

  gn_data_clear(&data);
  gn_calnote entry;

  memset(&entry, 0, sizeof(entry));
  entry.location = index+1;
  data.calnote = &entry;
  data.calnote_list = &calnote_list;
  
  gn_error error = gn_sm_functions(GN_OP_GetCalendarNote, &data, &state);
  GNOKII_CHECK_ERROR(error);
  if (error != GN_ERR_NONE)
	return gn_error2kio_error(error);

  print_calnote( entry );

  QDateTime dt_start = timestamp_2_QDateTime(entry.time);
  QDateTime dt_end = dt_start.addSecs( 60*60 ); // XXX: assume one hour
  event.setDtStart( dt_start );
  event.setDtEnd( dt_end );
  event.setSummary( QString::fromUtf8(entry.text) );

  // type:
  switch (entry.type) {
  case GN_CALNOTE_MEETING:
		event.setCategories(i18n("MEETING"));
		break;
  case GN_CALNOTE_CALL:
		event.setCategories(i18n("PHONE CALL"));
		event.setDescription(QString::fromUtf8(entry.phone_number));
		break;
  case GN_CALNOTE_BIRTHDAY:
		event.setCategories(i18n("BIRTHDAY"));
		break;
  case GN_CALNOTE_REMINDER:
		event.setCategories(i18n("REMINDER"));
		break;
  default:
		kdWarning() << "unknown calendar GN_CALNOTE_XXXX type #" << entry.type << endl;
  }

  // alarm:
  if (entry.alarm.enabled) {
    QDateTime at = timestamp_2_QDateTime(entry.alarm.timestamp);
    if (at.isValid() && dt_start.isValid()) {
      int seconds = abs(at.secsTo(dt_start));
      seconds %= 60*60*24; /* max. 1 day in advance... */
      KCal::Alarm *eventalarm = event.newAlarm();
      eventalarm->setStartOffset(KCal::Duration(seconds));
    }
  }

  // recurrence:
  switch (entry.recurrence) {
  case GN_CALNOTE_NEVER:
		break;
  case GN_CALNOTE_DAILY:
		event.recurrence()->setDaily(1,-1);
		break;
  case GN_CALNOTE_WEEKLY:
  case GN_CALNOTE_2WEEKLY:
		event.recurrence()->setDaily( 7 + (entry.recurrence==GN_CALNOTE_2WEEKLY ? 7:0) , -1);
		break;
  case GN_CALNOTE_MONTHLY:
		event.recurrence()->setMonthly(KCal::Recurrence::rMonthlyPos, 1, -1);
		break;
  case GN_CALNOTE_YEARLY:
		event.recurrence()->setYearly(KCal::Recurrence::rYearlyPos, 1, -1);
		break;
  default: // hourly
		event.recurrence()->setHourly(entry.recurrence, -1);
		break;
  }

  return 0;
}

int KMobileGnokii::storeCalendarEntry( int index, const KCal::Event &event )
{
  if (index < 0 || index >= GN_CALNOTE_MAX_NUMBER)
	return KIO::ERR_DOES_NOT_EXIST;

  gn_error error;
  gn_calnote entry;

  gn_data_clear(&data);
  memset(&entry, 0, sizeof(entry));
  entry.location = index+1;
  data.calnote = &entry;
  data.calnote_list = &calnote_list;

  // read first
  error = gn_sm_functions(GN_OP_GetCalendarNote, &data, &state);
  // GNOKII_CHECK_ERROR(error);

  QDateTime_2_timestamp( event.dtStart(), entry.time );
  strncpy(entry.text, event.summary().utf8(), sizeof(entry.text)-1);

  // type:
  entry.type = GN_CALNOTE_MEETING;
  if (event.categories().findIndex(i18n("MEETING")) != -1) {
	entry.type = GN_CALNOTE_MEETING;
  } else if (event.categories().findIndex(i18n("PHONE CALL")) != -1) {
	entry.type = GN_CALNOTE_CALL;
  	strncpy(entry.phone_number, event.description().utf8(), sizeof(entry.phone_number)-1);
  } else if (event.categories().findIndex(i18n("BIRTHDAY")) != -1) {
	entry.type = GN_CALNOTE_BIRTHDAY;
  } else { // assume i18n("REMINDER")
	entry.type = GN_CALNOTE_REMINDER;
  }

  // alarm:
  entry.alarm.enabled = 0;
  if (event.isAlarmEnabled()) {
	const KCal::Alarm *eventalarm = *event.alarms().at(0);
	if (eventalarm) {
		if (eventalarm->hasTime()) {

		   QDateTime_2_timestamp( eventalarm->time(), entry.alarm.timestamp );
		} else
		if (eventalarm->hasStartOffset()) {
		   QDateTime dt = event.dtStart();
		   dt = dt.addSecs(-eventalarm->startOffset().asSeconds());
		   QDateTime_2_timestamp( dt, entry.alarm.timestamp );
		}
	}
  }

  // recurrence:
  switch (event.recurrence()->recurrenceType()) {
  case KCal::Recurrence::rNone:
  default:
		entry.recurrence = GN_CALNOTE_NEVER;
		break;
  case KCal::Recurrence::rHourly:
		entry.recurrence = (gn_calnote_recurrence) (event.recurrence()->frequency());
		break;
  case KCal::Recurrence::rDaily:
		entry.recurrence = GN_CALNOTE_DAILY;
		break;
  case KCal::Recurrence::rWeekly:
		entry.recurrence = (gn_calnote_recurrence) (GN_CALNOTE_WEEKLY * event.recurrence()->frequency());
		break;
  case KCal::Recurrence::rMonthlyPos:
  case KCal::Recurrence::rMonthlyDay:
		entry.recurrence = GN_CALNOTE_MONTHLY;
		break;
  case KCal::Recurrence::rYearlyMonth:
  case KCal::Recurrence::rYearlyDay:
  case KCal::Recurrence::rYearlyPos:
		entry.recurrence = GN_CALNOTE_YEARLY;
		break;
  }

  print_calnote( entry );

return 0; // XXX

  error = gn_sm_functions(GN_OP_WriteCalendarNote, &data, &state);
  GNOKII_CHECK_ERROR(error);

  return 0;
}


/**********************************************************************************
 * Notes support                                                                  *
 **********************************************************************************/

int KMobileGnokii::numNotes()
{
  return 100; /* we simulate one address */
}

int KMobileGnokii::readNote( int index, QString &note )
{
  // index is zero-based, and we only have one simulated note
  if (index<0 || index>=numNotes())
    return KIO::ERR_DOES_NOT_EXIST;

  note = QString("NOTE #%1\n"
		 "--------\n"
	"This is a sample note #%2\n\n"
	"DeviceClassName: %3\n"
	"Device Driver  : %4\n"
	"Device Revision: %5\n")
	.arg(index).arg(index)
	.arg(deviceClassName()).arg(deviceName()).arg(revision()); 
  return 0;
}

#include "gnokii_mobile.moc"
