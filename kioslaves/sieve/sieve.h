/***************************************************************************
                          sieve.h  -  description
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
#ifndef __sieve_h__
#define __sieve_h__

#include <kio/tcpslavebase.h>
#include <kio/authinfo.h>

#include <tqstring.h>
#include <tqcstring.h>
#include <tqstringlist.h>

class KDESasl;
class KURL;


class kio_sieveResponse
{
public:
	enum responses { NONE, KEY_VAL_PAIR, ACTION, QUANTITY };

	kio_sieveResponse();

	const uint& getType() const;

	const TQCString& getAction() const;
	const uint getQuantity() const;
	const TQCString& getKey() const;
	const TQCString& getVal() const;
	const TQCString& getExtra() const;

	void setQuantity(const uint& quantity);
	void setAction(const TQCString& newAction);
	void setKey(const TQCString& newKey);
	void setVal(const TQCString& newVal);
	void setExtra(const TQCString& newExtra);

	void clear();

protected:
	uint		rType;
	uint		quantity;
	QCString	key;
	QCString	val;
	QCString	extra;
};

class kio_sieveProtocol : public KIO::TCPSlaveBase
{

public:
	enum connectionModes { NORMAL, CONNECTION_ORIENTED };
	enum Results { OK, NO, BYE, OTHER };

	kio_sieveProtocol(const TQCString &pool_socket, const TQCString &app_socket);
	virtual ~kio_sieveProtocol();

	virtual void mimetype(const KURL& url);
	virtual void get(const KURL& url);
	virtual void put(const KURL& url, int permissions, bool overwrite, bool resume);
	virtual void del(const KURL &url, bool isfile);

	virtual void listDir(const KURL& url);
	virtual void chmod(const KURL& url, int permissions);
	virtual void stat(const KURL& url);

	virtual void setHost(const TQString &host, int port, const TQString &user, const TQString &pass);
	virtual void openConnection();
	virtual void closeConnection();
	//virtual void slave_status();

	/**
	 * Special commands supported by this slave:
	 * 1 - activate script
	 * 2 - deactivate (all - only one active at any one time) scripts
	 * 3 - request capabilities, returned as metadata
	 */
	virtual void special(const TQByteArray &data);
	bool activate(const KURL& url);
	bool deactivate();

protected:
	bool connect(bool useTLSIfAvailable = true);
	bool authenticate();
	void disconnect(bool forcibly = false);
	void changeCheck( const KURL &url );

	bool sendData(const TQCString &data);
	bool receiveData(bool waitForData = true, TQCString *reparse = 0);
	bool operationSuccessful();
	int operationResult();

	bool parseCapabilities(bool requestCapabilities = false);
  bool saslInteract( void *in, KIO::AuthInfo &ai );

	// IOSlave global data
	uint				m_connMode;

	// Host-specific data
	QStringList			m_sasl_caps;
	bool				m_supportsTLS;

	// Global server respose class
	kio_sieveResponse	r;

	// connection details
	QString				m_sServer;
	QString				m_sUser;
	QString				m_sPass;
	QString				m_sAuth;
	bool				m_shouldBeConnected;
        bool m_allowUnencrypted;

private:
	bool requestCapabilitiesAfterStartTLS() const;

	TQString m_implementation;
};

#endif
