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

#include <qstring.h>
#include <qcstring.h>
#include <qstringlist.h>

class KDESasl;
class KURL;


class kio_sieveResponse
{
public:
	enum responses { NONE, KEY_VAL_PAIR, ACTION, QUANTITY };

	kio_sieveResponse();

	const uint& getType() const;

	const QCString& getAction() const;
	const uint getQuantity() const;
	const QCString& getKey() const;
	const QCString& getVal() const;
	const QCString& getExtra() const;

	void setQuantity(const uint& quantity);
	void setAction(const QCString& newAction);
	void setKey(const QCString& newKey);
	void setVal(const QCString& newVal);
	void setExtra(const QCString& newExtra);

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

	kio_sieveProtocol(const QCString &pool_socket, const QCString &app_socket);
	virtual ~kio_sieveProtocol();

	virtual void mimetype(const KURL& url);
	virtual void get(const KURL& url);
	virtual void put(const KURL& url, int permissions, bool overwrite, bool resume);
	virtual void del(const KURL &url, bool isfile);

	virtual void listDir(const KURL& url);
	virtual void chmod(const KURL& url, int permissions);
	virtual void stat(const KURL& url);

	virtual void setHost(const QString &host, int port, const QString &user, const QString &pass);
	virtual void openConnection();
	virtual void closeConnection();
	//virtual void slave_status();

	/**
	 * Special commands supported by this slave:
	 * 1 - activate script
	 * 2 - deactivate (all - only one active at any one time) scripts
	 * 3 - request capabilities, returned as metadata
	 */
	virtual void special(const QByteArray &data);
	bool activate(const KURL& url);
	bool deactivate();

protected:
	bool connect(bool useTLSIfAvailable = true);
	bool authenticate();
	void disconnect(bool forcibly = false);
	void changeCheck( const KURL &url );

	bool sendData(const QCString &data);
	bool receiveData(bool waitForData = true, QCString *reparse = 0);
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
};

#endif
