/***************************************************************************
                          sieve.cpp  -  description
                             -------------------
    begin                : Thu Dec 20 18:47:08 EST 2001
    copyright            : (C) 2001 by Hamish Rodda
    email                : meddie@yoyo.cc.monash.edu.au
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License version 2 as     *
 *   published by the Free Software Foundation.                            *
 *                                                                         *
 ***************************************************************************/

/** 
 * Portions adapted from the SMTP ioslave.
 * Copyright (c) 2000, 2001 Alex Zepeda <jazepeda@pacbell.net>
 * Copyright (c) 2001 Michael Häckel <Michael@Haeckel.Net>
 * All rights reserved.
 *
 * Policy: the function where the error occurs calls error(). A result of
 * false, where it signifies an error, thus doesn't need to call error() itself.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

extern "C" {
#include <sasl/sasl.h>
}
#include "sieve.h"

#include <kdebug.h>
#include <kinstance.h>
#include <klocale.h>
#include <kurl.h>
#include <kmdcodec.h>

#include <qcstring.h>

#include <cstdlib>
using std::exit;
#include <sys/stat.h>

static const int debugArea = 7122;

static inline
#ifdef NDEBUG
  kndbgstream ksDebug() { return kdDebug( debugArea ); }
  kndbgstream ksDebug( bool cond ) { return kdDebug( cond, debugArea ); }
#else
  kdbgstream ksDebug() { return kdDebug( debugArea ); }
  kdbgstream ksDebug( bool cond ) { return kdDebug( cond, debugArea ); }
#endif

#define SIEVE_DEFAULT_PORT 2000
  
static sasl_callback_t callbacks[] = {
    { SASL_CB_ECHOPROMPT, NULL, NULL },
    { SASL_CB_NOECHOPROMPT, NULL, NULL },
    { SASL_CB_GETREALM, NULL, NULL },
    { SASL_CB_USER, NULL, NULL },
    { SASL_CB_AUTHNAME, NULL, NULL },
    { SASL_CB_PASS, NULL, NULL },
    { SASL_CB_GETOPT, NULL, NULL },
    { SASL_CB_CANON_USER, NULL, NULL },
    { SASL_CB_LIST_END, NULL, NULL }
};

static const unsigned int SIEVE_DEFAULT_RECIEVE_BUFFER = 512;

using namespace KIO;
extern "C"
{
	int kdemain(int argc, char **argv)
	{
		KInstance instance("kio_sieve" );
		
		ksDebug() << "*** Starting kio_sieve " << endl;

		if (argc != 4) {
			ksDebug() << "Usage: kio_sieve protocol domain-socket1 domain-socket2" << endl;
			exit(-1);
		}

    if ( sasl_client_init( callbacks ) != SASL_OK ) {
      fprintf(stderr, "SASL library initialization failed!\n");
      ::exit (-1);
    }

		kio_sieveProtocol slave(argv[2], argv[3]);
		slave.dispatchLoop();

    sasl_done();

		ksDebug() << "*** kio_sieve Done" << endl;
		return 0;
	}
}

/* ---------------------------------------------------------------------------------- */
kio_sieveResponse::kio_sieveResponse()
{
	clear();
}

/* ---------------------------------------------------------------------------------- */
const uint& kio_sieveResponse::getType() const
{
	return rType;
}

/* ---------------------------------------------------------------------------------- */
const uint kio_sieveResponse::getQuantity() const
{
	return quantity;
}

/* ---------------------------------------------------------------------------------- */
const QCString& kio_sieveResponse::getAction() const
{
	return key;
}

/* ---------------------------------------------------------------------------------- */
const QCString& kio_sieveResponse::getKey() const
{
	return key;
}

/* ---------------------------------------------------------------------------------- */
const QCString& kio_sieveResponse::getVal() const
{
	return val;
}

/* ---------------------------------------------------------------------------------- */
const QCString& kio_sieveResponse::getExtra() const
{
	return extra;
}

/* ---------------------------------------------------------------------------------- */
void kio_sieveResponse::setQuantity(const uint& newQty)
{
	rType = QUANTITY;
	quantity = newQty;
}

/* ---------------------------------------------------------------------------------- */
void kio_sieveResponse::setAction(const QCString& newAction)
{
	rType = ACTION;
	key = newAction.copy();
}

/* ---------------------------------------------------------------------------------- */
void kio_sieveResponse::setKey(const QCString& newKey)
{
	rType = KEY_VAL_PAIR;
	key = newKey.copy();
}

/* ---------------------------------------------------------------------------------- */
void kio_sieveResponse::setVal(const QCString& newVal)
{
	val = newVal.copy();
}

/* ---------------------------------------------------------------------------------- */
void kio_sieveResponse::setExtra(const QCString& newExtra)
{
	extra = newExtra.copy();
}

/* ---------------------------------------------------------------------------------- */
void kio_sieveResponse::clear()
{
	rType = NONE;
	extra = key = val = QCString("");
	quantity = 0;
}

/* ---------------------------------------------------------------------------------- */
kio_sieveProtocol::kio_sieveProtocol(const QCString &pool_socket, const QCString &app_socket)
	: TCPSlaveBase( SIEVE_DEFAULT_PORT, "sieve", pool_socket, app_socket, false)
	, m_connMode(NORMAL)
	, m_supportsTLS(false)
	, m_shouldBeConnected(false)
{
}

/* ---------------------------------------------------------------------------------- */
kio_sieveProtocol::~kio_sieveProtocol()
{
	if ( isConnectionValid() )
		disconnect();
}

/* ---------------------------------------------------------------------------------- */
void kio_sieveProtocol::setHost (const QString &host, int port, const QString &user, const QString &pass)
{
	if ( isConnectionValid() &&
			( m_sServer != host ||
				m_iPort != port ||
				m_sUser != user ||
				m_sPass != pass ) ) {
		disconnect();
	}
	m_sServer = host;
	m_iPort = port ? port : m_iDefaultPort;
	m_sUser = user;
	m_sPass = pass;
	m_supportsTLS = false;
}

/* ---------------------------------------------------------------------------------- */
void kio_sieveProtocol::openConnection()
{
	m_connMode = CONNECTION_ORIENTED;
	connect();
}

bool kio_sieveProtocol::parseCapabilities(bool requestCapabilities/* = false*/)
{
	ksDebug() << k_funcinfo << endl;
	
	// Setup...
	bool ret = false;
	
	if (requestCapabilities) {
		sendData("CAPABILITY");
	}

	while (receiveData()) {
		ksDebug() << "Looping receive" << endl;
		
		if (r.getType() == kio_sieveResponse::ACTION) {
			if ( r.getAction().contains("ok", false) != -1 ) {
				ksDebug() << "Sieve server ready & awaiting authentication." << endl;
				break;
			} else
				ksDebug() << "Unknown action " << r.getAction() << "." << endl;

		} else if (r.getKey() == "IMPLEMENTATION") {
			if (r.getVal().contains("sieve", false) != -1) {
				ksDebug() << "Connected to Sieve server: " << r.getVal() << endl;
				ret = true;
				setMetaData("implementation", r.getVal());
			}

		} else if (r.getKey() == "SASL") {
			// Save list of available SASL methods
			m_sasl_caps = QStringList::split(' ', r.getVal());
			ksDebug() << "Server SASL authentication methods: " << m_sasl_caps.join(", ") << endl;
			setMetaData("saslMethods", r.getVal());

		} else if (r.getKey() == "SIEVE") {
			// Save script capabilities; report back as meta data:
			ksDebug() << "Server script capabilities: " << QStringList::split(' ', r.getVal()).join(", ") << endl;
			setMetaData("sieveExtensions", r.getVal());

		} else if (r.getKey() == "STARTTLS") {
			// The server supports TLS
			ksDebug() << "Server supports TLS" << endl;
			m_supportsTLS = true;
			setMetaData("tlsSupported", "true");
			
		} else {
			ksDebug() << "Unrecognised key." << endl;
		}
	}
	
	if (!m_supportsTLS) {
		setMetaData("tlsSupported", "false");
	}

	return ret;
}

/* ---------------------------------------------------------------------------------- */
/**
 * Connects to the server.
 * returns false and calls error() if an error occurred.
 */
bool kio_sieveProtocol::connect(bool useTLSIfAvailable)
{
	ksDebug() << k_funcinfo << endl;
	
	if (isConnectionValid()) return true;

	infoMessage(i18n("Connecting to %1...").arg( m_sServer));

	if (m_connMode == CONNECTION_ORIENTED && m_shouldBeConnected) {
		error(ERR_CONNECTION_BROKEN, i18n("The connection to the server was lost."));
		return false;
	}
	
	setBlockConnection(true);

	if (!connectToHost(m_sServer, m_iPort, true)) {
		return false;
	}

	if (!parseCapabilities()) {
		closeDescriptor();
		error(ERR_UNSUPPORTED_PROTOCOL, i18n("Server identification failed."));
		return false;
	}

	// Attempt to start TLS
	// FIXME find a test server and test that this works
	if (useTLSIfAvailable && m_supportsTLS && canUseTLS()) {
		sendData("STARTTLS");
		if (operationSuccessful()) {
			ksDebug() << "TLS has been accepted. Starting TLS..." << endl
				  << "WARNING this is untested and may fail." << endl;
			int retval = startTLS();
			if (retval == 1) {
				ksDebug() << "TLS enabled successfully." << endl;
				// reparse capabilities:
				parseCapabilities(true);
			} else {
				ksDebug() << "TLS initiation failed, code " << retval << endl;
				disconnect(true);
				return connect(false);
				// error(ERR_INTERNAL, i18n("TLS initiation failed."));
			}
		} else
			ksDebug() << "Server incapable of TLS. Transmitted documents will be unencrypted." << endl;
	} else
		ksDebug() << "We are incapable of TLS. Transmitted documents will be unencrypted." << endl;

	infoMessage(i18n("Authenticating user..."));
	if (!authenticate()) {
		disconnect();
		error(ERR_COULD_NOT_AUTHENTICATE, i18n("Authentication failed."));
		return false;
	}

	m_shouldBeConnected = true;
	return true;
}

/* ---------------------------------------------------------------------------------- */
void kio_sieveProtocol::closeConnection()
{
	m_connMode = CONNECTION_ORIENTED;
	disconnect();
}

/* ---------------------------------------------------------------------------------- */
void kio_sieveProtocol::disconnect(bool forcibly)
{
	if (!forcibly) {
		sendData("LOGOUT");

		if (!operationSuccessful())
			ksDebug() << "Server did not logout cleanly." << endl;
	}

	closeDescriptor();
	m_shouldBeConnected = false;
}

/* ---------------------------------------------------------------------------------- */
/*void kio_sieveProtocol::slave_status()
{
	slaveStatus(isConnectionValid() ? m_sServer : "", isConnectionValid());
	
	finished();
}*/

/* ---------------------------------------------------------------------------------- */
void kio_sieveProtocol::special(const QByteArray &data)
{
	int tmp;
	QDataStream stream(data, IO_ReadOnly);
	KURL url;

	stream >> tmp;

	switch (tmp) {
		case 1:
			stream >> url;
			if (!activate(url))
				return;
			break;
		case 2:
			if (!deactivate())
				return;
			break;
		case 3:
			parseCapabilities(true);
			break;
	}

	infoMessage(i18n("Done."));

	finished();
}

/* ---------------------------------------------------------------------------------- */
bool kio_sieveProtocol::activate(const KURL& url)
{
	if (!connect())
		return false;

	infoMessage(i18n("Activating script..."));

	QString filename = url.fileName(false);

	if (filename.isEmpty()) {
		error(ERR_DOES_NOT_EXIST, url.prettyURL());
		return false;
	}

	if (!sendData("SETACTIVE \"" + filename.utf8() + "\""))
		return false;

	if (operationSuccessful()) {
		ksDebug() << "Script activation complete." << endl;
		return true;
	} else {
		error(ERR_INTERNAL_SERVER, i18n("There was an error activating the script."));
		return false;
	}
}

/* ---------------------------------------------------------------------------------- */
bool kio_sieveProtocol::deactivate()
{
	if (!connect())
		return false;

	if (!sendData("SETACTIVE \"\""))
		return false;

	if (operationSuccessful()) {
		ksDebug() << "Script deactivation complete." << endl;
		return true;
	} else {
		error(ERR_INTERNAL_SERVER, i18n("There was an error deactivating the script."));
		return false;
	}
}

static void append_lf2crlf( QByteArray & out, const QByteArray & in ) {
  if ( in.isEmpty() )
    return;
  const unsigned int oldOutSize = out.size();
  out.resize( oldOutSize + 2 * in.size() );
  const char * s = in.begin();
  const char * const end = in.end();
  char * d = out.begin() + oldOutSize;
  char last = '\0';
  while ( s < end ) {
    if ( *s == '\n' && last != '\r' )
      *d++ = '\r';
    *d++ = last = *s++;
  }
  out.resize( d - out.begin() );
}

void kio_sieveProtocol::put(const KURL& url, int /*permissions*/, bool /*overwrite*/, bool /*resume*/)
{
	if (!connect())
		return;

	infoMessage(i18n("Sending data..."));

	QString filename = url.fileName(false);

	if (filename.isEmpty()) {
		error(ERR_MALFORMED_URL, url.prettyURL());
		return;
	}

	QByteArray data;
	for (;;) {
		dataReq();
		QByteArray buffer;
		const int newSize = readData(buffer);
		append_lf2crlf( data, buffer );
		if ( newSize < 0 ) {
		  // read error: network in unknown state so disconnect
		  error(ERR_COULD_NOT_READ, i18n("KIO data supply error."));
		  return;
		}
		if ( newSize == 0 )
		  break;
	}

	// script size
	int bufLen = (int)data.size() - 1;
	totalSize(bufLen);

	// timsieved 1.1.0:
	// C: HAVESPACE "rejected" 74
	// S: NO "Number expected"
	// C: HAVESPACE 74
	// S: NO "Missing script name"
	// S: HAVESPACE "rejected" "74"
	// C: NO "Number expected"
	// => broken, we can't use it :-(
	// (will be fixed in Cyrus 2.1.10)
#ifndef HAVE_BROKEN_TIMSIEVED
	// first, check quota (it's a SHOULD in draft std)
	if (!sendData("HAVESPACE \"" + filename.utf8() + "\" "
		      + QCString().setNum( bufLen )))
		return;

	if (!operationSuccessful()) {
		error(ERR_DISK_FULL, i18n("Quota exceeded"));
		return;
	}
#endif

	if (!sendData("PUTSCRIPT \"" + filename.utf8() + "\" {"
		      + QCString().setNum( bufLen ) + "+}"))
		return;

	// atEnd() lies so the code below doesn't work.
	/*if (!atEnd()) {
		// We are not expecting any data here, so if the server has responded
		// with anything but OK we treat it as an error.
		char * buf = new char[2];
		while (!atEnd()) {
			ksDebug() << "Reading..." << endl;
			read(buf, 1);
			ksDebug() << "Trailing [" << buf[0] << "]" << endl;
		}
		ksDebug() << "End of data." << endl;
		delete[] buf;

		if (!operationSuccessful()) {
			error(ERR_UNSUPPORTED_PROTOCOL, i18n("A protocol error occurred "
						"while trying to negotiate script uploading.\n"
						"The server responded:\n%1")
							.arg(r.getAction().right(r.getAction().length() - 3)));
			return;
		}
	}*/

	// upload data to the server
	if (write(data, bufLen) != bufLen) {
		error(ERR_COULD_NOT_WRITE, i18n("Network error."));
		disconnect(true);
		return;
	}

	// finishing CR/LF
	if (!sendData(""))
		return;

	processedSize(bufLen);

	infoMessage(i18n("Verifying upload completion..."));

	if (operationSuccessful())
		ksDebug() << "Script upload complete." << endl;
	
	else {
		/* The managesieve server parses received scripts and rejects
		 * scripts which are not syntactically correct. Here we expect
		 * to receive a message detailing the error (only the first
		 * error is reported. */
		if (r.getAction().length() > 3) {
			// make a copy of the extra info
			QCString extra = r.getAction().right(r.getAction().length() - 3);

			// send the extra message off for re-processing
			receiveData(false, &extra);

			if (r.getAction() == kio_sieveResponse::QUANTITY) {
				// length of the error message
				uint len = r.getQuantity();

				QCString errmsg(len + 1);

				read(errmsg.data(), len);

				error(ERR_INTERNAL_SERVER,
						i18n("The script did not upload successfully.\n"
							"This is probably due to errors in the script.\n"
							"The server responded:\n%1").arg(errmsg));

				// clear the rest of the incoming data
				receiveData();
			} else
				error(ERR_INTERNAL_SERVER,
					i18n("The script did not upload successfully.\n"
						"The script may contain errors."));
		} else
			error(ERR_INTERNAL_SERVER,
				i18n("The script did not upload successfully.\n"
					"The script may contain errors."));
	}

	//if ( permissions != -1 )
	//	chmod( url, permissions );

	infoMessage(i18n("Done."));

	finished();
}

static void inplace_crlf2lf( QByteArray & in ) {
  if ( in.isEmpty() )
    return;
  QByteArray & out = in; // inplace
  const char * s = in.begin();
  const char * const end = in.end();
  char * d = out.begin();
  char last = '\0';
  while ( s < end ) {
    if ( *s == '\n' && last == '\r' )
      --d;
    *d++ = last = *s++;
  }
  out.resize( d - out.begin() );
}

/* ---------------------------------------------------------------------------------- */
void kio_sieveProtocol::get(const KURL& url)
{
	if (!connect())
		return;

	infoMessage(i18n("Retrieving data..."));

	QString filename = url.fileName(false);

	if (filename.isEmpty()) {
		error(ERR_MALFORMED_URL, url.prettyURL());
		return;
	}

	//SlaveBase::mimetype( QString("text/plain") ); // "application/sieve");

	if (!sendData("GETSCRIPT \"" + filename.utf8() + "\""))
		return;

	if (receiveData() && r.getType() == kio_sieveResponse::QUANTITY) {
		// determine script size
		ssize_t bufLen = r.getQuantity();
		totalSize( bufLen );

		QByteArray dat( bufLen );
		ssize_t readLen = read( dat.data(), dat.size() );
		
		if (readLen != bufLen) {
			error(ERR_COULD_NOT_READ, i18n("Network error."));
			disconnect(true);
			return;
		}

		inplace_crlf2lf( dat );
		// send data to slaveinterface
		data(dat);
		processedSize(readLen);
		data(QByteArray());

		infoMessage(i18n("Finishing up...") );

		if (operationSuccessful())
			ksDebug() << "Script retrieval complete." << endl;
		else
			ksDebug() << "Script retrieval failed." << endl;
	} else {
		error(ERR_UNSUPPORTED_PROTOCOL, i18n("A protocol error occurred "
							"while trying to negotiate script downloading."));
		return;
	}

	infoMessage(i18n("Done."));
	finished();
}

void kio_sieveProtocol::del(const KURL &url, bool isfile)
{
	if (!isfile) {
		error(ERR_INTERNAL, i18n("Folders are not supported."));
		return;
	}

	if (!connect())
		return;

	infoMessage(i18n("Deleting file..."));

	QString filename = url.fileName(false);

	if (filename.isEmpty()) {
		error(ERR_MALFORMED_URL, url.prettyURL());
		return;
	}

	if (!sendData("DELETESCRIPT \"" + filename.utf8() + "\""))
		return;

	if (operationSuccessful())
		ksDebug() << "Script deletion successful." << endl;
	else {
		error(ERR_INTERNAL_SERVER, i18n("The server would not delete the file."));
		return;
	}

	infoMessage(i18n("Done."));
	
	finished();
}

void kio_sieveProtocol::chmod(const KURL& url, int permissions)
{
  switch ( permissions ) {
  case 0700: // activate
    activate(url);
    break;
  case 0600: // deactivate
    deactivate();
    break;
  default: // unsupported
    error(ERR_CANNOT_CHMOD, i18n("Cannot chmod to anything but 0700 (active) or 0600 (inactive script)."));
    return;
  }
  
  finished();
}

#if defined(_AIX) && defined(stat)
#undef stat
#endif

void kio_sieveProtocol::stat(const KURL& url)
{
	if (!connect())
		return;

	UDSEntry entry;

	QString filename = url.fileName(false);

	if (filename.isEmpty()) {
		UDSAtom atom;
		atom.m_uds = KIO::UDS_NAME;
		atom.m_str = "/";
		entry.append(atom);

		atom.m_uds = KIO::UDS_FILE_TYPE;
		atom.m_long = S_IFDIR;
		entry.append(atom);

		atom.m_uds = KIO::UDS_ACCESS;
		atom.m_long = 0700;
		entry.append(atom);

		statEntry(entry);

	} else {
		if (!sendData("LISTSCRIPTS"))
			return;

		while(receiveData()) {
			if (r.getType() == kio_sieveResponse::ACTION) {
				if (r.getAction().contains("OK", false) == 1) 
					// Script list completed
					break;

			} else
				if (filename == QString::fromUtf8(r.getKey())) {
					entry.clear();

					UDSAtom atom;
					atom.m_uds = KIO::UDS_NAME;
					atom.m_str = QString::fromUtf8(r.getKey());
					entry.append(atom);

					atom.m_uds = KIO::UDS_FILE_TYPE;
					atom.m_long = S_IFREG;
					entry.append(atom);

					atom.m_uds = KIO::UDS_ACCESS;
					if ( r.getExtra() == "ACTIVE" )
					  atom.m_long = 0700; // mark exec'able
					else
					  atom.m_long = 0600;
					entry.append(atom);

					atom.m_uds = KIO::UDS_MIME_TYPE;
					atom.m_str = "application/sieve";
					entry.append(atom);

					//setMetaData("active", (r.getExtra() == "ACTIVE") ? "yes" : "no");

					statEntry(entry);
					// cannot break here because we need to clear
					// the rest of the incoming data.
				}
		}
	}

	finished();
}

void kio_sieveProtocol::listDir(const KURL& /*url*/)
{
	if (!connect())
		return;

	if (!sendData("LISTSCRIPTS"))
		return;

	UDSEntry entry;

	while(receiveData()) {
		if (r.getType() == kio_sieveResponse::ACTION) {
			if (r.getAction().contains("OK", false) == 1)
				// Script list completed.
				break;

		} else {
			entry.clear();

			UDSAtom atom;
			atom.m_uds = KIO::UDS_NAME;
			atom.m_str = QString::fromUtf8(r.getKey());
			entry.append(atom);

			atom.m_uds = KIO::UDS_FILE_TYPE;
			atom.m_long = S_IFREG;
			entry.append(atom);

			atom.m_uds = KIO::UDS_ACCESS;
			if ( r.getExtra() == "ACTIVE" )
			  atom.m_long = 0700; // mark exec'able
			else
			  atom.m_long = 0600;
			entry.append(atom);

			atom.m_uds = KIO::UDS_MIME_TYPE;
			atom.m_str = "application/sieve";
			entry.append(atom);

			//asetMetaData("active", (r.getExtra() == "ACTIVE") ? "true" : "false");

			ksDebug() << "Listing script " << r.getKey() << endl;
			listEntry(entry , false);
		}
	}

	listEntry(entry, true);

	finished();
}

/* ---------------------------------------------------------------------------------- */
bool kio_sieveProtocol::saslInteract( void *in, AuthInfo &ai )
{
  ksDebug() << "sasl_interact" << endl;
  sasl_interact_t *interact = ( sasl_interact_t * ) in;

  //some mechanisms do not require username && pass, so it doesn't need a popup
  //window for getting this info
  for ( ; interact->id != SASL_CB_LIST_END; interact++ ) {
    if ( interact->id == SASL_CB_AUTHNAME || 
         interact->id == SASL_CB_PASS ) {

	    if (m_sUser.isEmpty() || m_sPass.isEmpty()) {
		    if (!openPassDlg(ai)) {
			    error(ERR_ABORTED, i18n("No authentication details supplied."));
			    return false;
		    }
        m_sUser = ai.username;
        m_sPass = ai.password;
	    }
      break;
    }
  }
  
  interact = ( sasl_interact_t * ) in;
  while( interact->id != SASL_CB_LIST_END ) {
    ksDebug() << "SASL_INTERACT id: " << interact->id << endl;
    switch( interact->id ) {
      case SASL_CB_USER:
      case SASL_CB_AUTHNAME:
        ksDebug() << "SASL_CB_[AUTHNAME|USER]: '" << m_sUser << "'" << endl;
        interact->result = strdup( m_sUser.utf8() );
        interact->len = strlen( (const char *) interact->result );
        break;
      case SASL_CB_PASS:
        ksDebug() << "SASL_CB_PASS: [hidden] " << endl;
        interact->result = strdup( m_sPass.utf8() );
        interact->len = strlen( (const char *) interact->result );
        break;
      default:
        interact->result = NULL; interact->len = 0;
        break;
    }
    interact++;
  }
  return true;
}

#define SASLERROR  error(ERR_COULD_NOT_AUTHENTICATE, i18n("An error occured during authentication: %1").arg( \
      QString::fromUtf8( sasl_errdetail( conn ) )));

bool kio_sieveProtocol::authenticate()
{
  int result;
  sasl_conn_t *conn = NULL;
  sasl_interact_t *client_interact = NULL;
  const char *out = NULL;
  uint outlen;
  const char *mechusing = NULL;

	/* Retrieve authentication details from user.
	 * Note: should this require realm as well as user & pass details
	 * before it automatically skips the prompt?
	 * Note2: encoding issues with PLAIN login? */
	AuthInfo ai;
	ai.url.setProtocol("sieve");
	ai.url.setHost(m_sServer);
	ai.url.setPort(m_iPort);
	ai.username = m_sUser;
	ai.password = m_sPass;
	ai.keepPassword = true;
  ai.caption = i18n("Sieve Authentication Details");
  ai.comment = i18n("Please enter your authentication details for your sieve account "
	  "(usually the same as your email password):");

  result = sasl_client_new( "sieve",
                       m_sServer.latin1(),
                       0, 0, NULL, 0, &conn );

  if ( result != SASL_OK ) {
    ksDebug() << "sasl_client_new failed with: " << result << endl;
    SASLERROR
    return false;
  }

	QStringList strList;
//	strList.append("NTLM");

  if (!metaData("sasl").isEmpty())
    strList.append(metaData("sasl").latin1());
  else
    strList = m_sasl_caps;	

  do {
    result = sasl_client_start(conn, strList.join(" ").latin1(), &client_interact,
                       0, &outlen, &mechusing);

    if (result == SASL_INTERACT) 
      if ( !saslInteract( client_interact, ai ) ) {
        sasl_dispose( &conn );
        return false;
      };
  } while ( result == SASL_INTERACT );

  if ( result != SASL_CONTINUE && result != SASL_OK ) {
    ksDebug() << "sasl_client_start failed with: " << result << endl;
    SASLERROR
    sasl_dispose( &conn );
    return false;
  }

	ksDebug() << "Preferred authentication method is " << mechusing << "." << endl;

	if (!sendData("AUTHENTICATE \"" + QCString( mechusing ) + "\""))
		return false;
	
	QCString command;
	
	do {
		receiveData();
		
		if (operationResult() != OTHER)
			break;
		
		ksDebug() << "Challenge len  " << r.getQuantity() << endl;

		if (r.getType() != kio_sieveResponse::QUANTITY) {
      sasl_dispose( &conn );
			error(ERR_UNSUPPORTED_PROTOCOL,
					i18n("A protocol error occurred during authentication.\n"
							"Choose a different authentication method to %1.").arg(mechusing));
			return false;
		}

		uint qty = r.getQuantity();

		receiveData();

		if (r.getType() != kio_sieveResponse::ACTION && r.getAction().length() != qty) {
      sasl_dispose( &conn );
			error(ERR_UNSUPPORTED_PROTOCOL,
					i18n("A protocol error occurred during authentication.\n"
							"Choose a different authentication method to %1.").arg(mechusing));
			return false;
		}

    QByteArray challenge, tmp;
    tmp.setRawData( r.getAction().data(), qty );
    KCodecs::base64Decode( tmp, challenge );
    tmp.resetRawData( r.getAction().data(), qty );
//		ksDebug() << "S:  [" << r.getAction() << "]." << endl;
//		ksDebug() << "S-1:  [" << QCString(challenge.data(), challenge.size()+1) << "]." << endl;

    do {
      result = sasl_client_step(conn, challenge.isEmpty() ? 0 : challenge.data(),
                                  challenge.size(),
                                  &client_interact,
                                  &out, &outlen);

      if (result == SASL_INTERACT) 
        if ( !saslInteract( client_interact, ai ) ) {
          sasl_dispose( &conn );
          return false;
        };
    } while ( result == SASL_INTERACT );

    ksDebug() << "sasl_client_step: " << result << endl;
    if ( result != SASL_CONTINUE && result != SASL_OK ) {
      ksDebug() << "sasl_client_step failed with: " << result << endl;
      SASLERROR
      sasl_dispose( &conn );
      return false;
    }
   
    tmp.setRawData( out, outlen );
    KCodecs::base64Encode( tmp, challenge );
    tmp.resetRawData( out, outlen );
    sendData("\"" + QCString( challenge.data(), challenge.size()+1 ) + "\"");
//    ksDebug() << "C:  [" << QCString(challenge.data(), challenge.size()+1) << "]." << endl;
//    ksDebug() << "C-1:  [" << out << "]." << endl;
  } while ( true );

	ksDebug() << "Challenges finished." << endl;
  sasl_dispose( &conn );
    
	if (operationResult() == OK) {
		// Authentication succeeded.
		return true;
	} else {
		// Authentication failed.
		error(ERR_COULD_NOT_AUTHENTICATE, i18n("Authentication failed.\nMost likely the password is wrong.\nThe server responded:\n%1").arg( r.getAction() ) );
		return false;
	}
}

/* --------------------------------------------------------------------------- */
void kio_sieveProtocol::mimetype(const KURL & url)
{
	ksDebug() << "Requesting mimetype for " << url.prettyURL() << endl;

	if (url.fileName(false).isEmpty())
		mimeType( "inode/directory" );
	else
		mimeType( "application/sieve" );

	finished();
}


/* --------------------------------------------------------------------------- */
bool kio_sieveProtocol::sendData(const QCString &data)
{
	QCString write_buf = data + "\r\n";

	//ksDebug() << "C: " << data << endl;

	// Write the command
	ssize_t write_buf_len = write_buf.length();
	if (write(write_buf.data(), write_buf_len) != write_buf_len) {
		error(ERR_COULD_NOT_WRITE, i18n("Network error."));
		disconnect(true);
		return false;
	}

	return true;
}

/* --------------------------------------------------------------------------- */
bool kio_sieveProtocol::receiveData(bool waitForData, QCString *reparse)
{
	QCString interpret;
	int start, end;

	if (!reparse) {
		if (!waitForData)
			// is there data waiting?
			if (atEnd()) return false;

		// read data from the server
		char buffer[SIEVE_DEFAULT_RECIEVE_BUFFER];
		readLine(buffer, SIEVE_DEFAULT_RECIEVE_BUFFER - 1);
		buffer[SIEVE_DEFAULT_RECIEVE_BUFFER-1] = '\0';

		// strip LF/CR
		interpret = QCString(buffer).left(qstrlen(buffer) - 2);

	} else {
		interpret = reparse->copy();
	}

	r.clear();

	//ksDebug() << "S: " << interpret << endl;

	switch(interpret[0]) {
		case '{':
		  {
			// expecting {quantity}
			start = 0;
			end = interpret.find('}', start + 1);

			bool ok = false;
			r.setQuantity(interpret.mid(start + 1, end - start - 1).toUInt( &ok ));
			if (!ok) {
				disconnect();
				error(ERR_INTERNAL_SERVER, i18n("A protocol error occurred."));
				return false;
			}

			return true;
		  }
		case '"':
			// expecting "key" "value" pairs
			break;
		default:
			// expecting single string
			r.setAction(interpret);
			return true;
	}

	start = 0;

	end = interpret.find(34, start + 1);
	if (end == -1) {
		ksDebug() << "Possible insufficient buffer size." << endl;
		r.setKey(interpret.right(interpret.length() - start));
		return true;
	}

	r.setKey(interpret.mid(start + 1, end - start - 1));

	start = interpret.find(34, end + 1);
	if (start == -1) {
		if ((int)interpret.length() > end)
			// skip " and space
			r.setExtra(interpret.right(interpret.length() - end - 2));

		return true;
	}

	end = interpret.find(34, start + 1);
	if (end == -1) {
		ksDebug() << "Possible insufficient buffer size." << endl;
		r.setVal(interpret.right(interpret.length() - start));
		return true;
	}

	r.setVal(interpret.mid(start + 1, end - start - 1));
	return true;
}

bool kio_sieveProtocol::operationSuccessful()
{
	while (receiveData(false)) {
		if (r.getType() == kio_sieveResponse::ACTION) {
			QCString response = r.getAction().left(2);
			if (response == "OK") {
				return true;
			} else if (response == "NO") {
				return false;
			}
		}
	}
	return false;
}

int kio_sieveProtocol::operationResult()
{
	if (r.getType() == kio_sieveResponse::ACTION) {
		QCString response = r.getAction().left(2);
		if (response == "OK") {
			return OK;
		} else if (response == "NO") {
			return NO;
		} else if (response == "BY"/*E*/) {
			return BYE;
		}
	}
	
	return OTHER;
}
