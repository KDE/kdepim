/*
    ksslsocket.cpp - KDE SSL Socket

    Copyright (c) 2004      by Jason Keirstead <jason@keirstead.org>

    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    stolen from kopete, but this is a modified version.

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <qsocketnotifier.h>

#include <dcopclient.h>
#include <klocale.h>
#include <kdebug.h>
#include <kssl.h>
#include <ksslinfodlg.h>
#include <ksslpeerinfo.h>
#include <ksslcertchain.h>
#include <ksslcertificatecache.h>
#include <kapplication.h>
#include <kmessagebox.h>

#include "ksslsocket.h"

struct KSSLSocketPrivate
{
	mutable KSSL *kssl;
	KSSLCertificateCache *cc;
	DCOPClient *dcc;
	QMap<QString,QString> metaData;
	QSocketNotifier *socketNotifier;
};

KSSLSocket::KSSLSocket() : KExtendedSocket()
{
  kdDebug() << "KSSLSocket() " << (void*)this << endl;

	d = new KSSLSocketPrivate;
	d->kssl = 0L;
	d->dcc = KApplication::kApplication()->dcopClient();
	d->cc = new KSSLCertificateCache;
	d->cc->reload();

	//No blocking
	setBlockingMode(false);

	//Connect internal slots
	QObject::connect( this, SIGNAL(connectionSuccess()), this, SLOT(slotConnected()) );
	QObject::connect( this, SIGNAL(closed(int)), this, SLOT(slotDisconnected()) );
	QObject::connect( this, SIGNAL(connectionFailed(int)), this, SLOT(slotDisconnected()));
}

KSSLSocket::~KSSLSocket()
{
  kdDebug() << "KSSLSocket()::~KSSLSocket() " << (void*)this << endl;

	//Close connection
	closeNow();

	if( d->kssl )
	{
		d->kssl->close();
		delete d->kssl;
	}

	delete d->cc;
	delete d;
}

Q_LONG KSSLSocket::readBlock( char* data, Q_ULONG maxLen )
{
	return d->kssl->read( data, maxLen );
}

Q_LONG KSSLSocket::writeBlock( const char* data, Q_ULONG len )
{
  kdDebug() << "KSSLSocket::writeBlock() " << (void*)this  << endl;
  kdDebug() << "  d->kssl: " << (void*)d->kssl << endl;
	return d->kssl->write( data, len );
}

void KSSLSocket::slotConnected()
{
  kdDebug() << "KSSLSocket::slotConnected() " << (void*)this << endl;
	if( KSSL::doesSSLWork() )
	{
		kdDebug(0) << k_funcinfo << "Trying SSL connection..." << endl;
		if( !d->kssl )
		{
			d->kssl = new KSSL();
		}
		else
		{
			d->kssl->reInitialize();
		}
		d->kssl->setPeerHost(host());
                kdDebug() << "SOCKET STATUS: " << socketStatus() << endl;
		int rc = d->kssl->connect( sockfd );
                if ( rc <= 0 ) {
                  kdError() << "Error connecting KSSL: " << rc << endl;
                  kdDebug() << "SYSTEM ERROR: " << systemError() << endl;
                  emit sslFailure();
                  closeNow();
                } else {
		  readNotifier()->setEnabled(true);

		  if( verifyCertificate() != 1 )
		  {
			  closeNow();
		  }
                }
	}
	else
	{
		kdError(0) << k_funcinfo << "SSL not functional!" << endl;

		d->kssl = 0L;
		emit sslFailure();
		closeNow();
	}
}

void KSSLSocket::slotDisconnected()
{
  kdDebug() << "KSSLSocket::slotDisconnected() " << (void*)this << endl;
	if( readNotifier() )
		readNotifier()->setEnabled(false);
}

void KSSLSocket::setMetaData( const QString &key, const QVariant &data )
{
	QVariant v = data;
	d->metaData[key] = v.asString();
}

bool KSSLSocket::hasMetaData( const QString &key )
{
	return d->metaData.contains(key);
}

QString KSSLSocket::metaData( const QString &key )
{
	if( d->metaData.contains(key) )
		return d->metaData[key];
	return QString::null;
}

/*
I basically copied the below from tcpKIO::SlaveBase.hpp, with some modificaions and formatting.

 * Copyright (C) 2000 Alex Zepeda <zipzippy@sonic.net
 * Copyright (C) 2001-2003 George Staikos <staikos@kde.org>
 * Copyright (C) 2001 Dawit Alemayehu <adawit@kde.org>
*/

int KSSLSocket::messageBox( KIO::SlaveBase::MessageBoxType type, const QString &text, const QString &caption,
	const QString &buttonYes, const QString &buttonNo )
{
	kdDebug(0) << "messageBox " << type << " " << text << " - " << caption << buttonYes << buttonNo << endl;
	QByteArray data, result;
	QCString returnType;
	QDataStream arg(data, IO_WriteOnly);
	arg << (int)1 << (int)type << text << caption << buttonYes << buttonNo;

	if (!d->dcc->isApplicationRegistered("kio_uiserver"))
	{
		KApplication::startServiceByDesktopPath("kio_uiserver.desktop",QStringList());
	}

	d->dcc->call("kio_uiserver", "UIServer",
			"messageBox(int,int,QString,QString,QString,QString)", data, returnType, result);

	if( returnType == "int" )
	{
		int res;
		QDataStream r(result, IO_ReadOnly);
		r >> res;
		return res;
	}
	else
		return 0; // communication failure
}


//  Returns 0 for failed verification, -1 for rejected cert and 1 for ok
int KSSLSocket::verifyCertificate()
{
	int rc = 0;
	bool permacache = false;
	bool _IPmatchesCN = false;
	int result;
	bool doAddHost = false;
	QString ourHost = host();
	QString ourIp = peerAddress()->pretty();

	QString theurl = "https://" + ourHost + ":" + port();

	if (!d->cc)
		d->cc = new KSSLCertificateCache;

	KSSLCertificate& pc = d->kssl->peerInfo().getPeerCertificate();

	KSSLCertificate::KSSLValidationList ksvl = pc.validateVerbose(KSSLCertificate::SSLServer);

        if ( ksvl.count() == 1 && ksvl.first() == KSSLCertificate::Unknown ) {
          kdDebug() << "Unknown validation error" << endl;
          return 0;
        }

	_IPmatchesCN = d->kssl->peerInfo().certMatchesAddress();

	if (!_IPmatchesCN && (metaData("ssl_militant") == "TRUE") )
	{
		ksvl << KSSLCertificate::InvalidHost;
	}

	KSSLCertificate::KSSLValidation ksv = KSSLCertificate::Ok;
	if (!ksvl.isEmpty())
		ksv = ksvl.first();

	/* Setting the various bits of meta-info that will be needed. */
	setMetaData("ssl_cipher", d->kssl->connectionInfo().getCipher());
	setMetaData("ssl_cipher_desc", d->kssl->connectionInfo().getCipherDescription());
	setMetaData("ssl_cipher_version", d->kssl->connectionInfo().getCipherVersion());
	setMetaData("ssl_cipher_used_bits", QString::number(d->kssl->connectionInfo().getCipherUsedBits()));
	setMetaData("ssl_cipher_bits", QString::number(d->kssl->connectionInfo().getCipherBits()));
	setMetaData("ssl_peer_ip", ourIp );

	QString errorStr;
	for(KSSLCertificate::KSSLValidationList::ConstIterator it = ksvl.begin();
		it != ksvl.end(); ++it)
	{
		errorStr += QString::number(*it)+":";
	}

	setMetaData("ssl_cert_errors", errorStr);
	setMetaData("ssl_peer_certificate", pc.toString());

	if (pc.chain().isValid() && pc.chain().depth() > 1)
	{
		QString theChain;
		QPtrList<KSSLCertificate> chain = pc.chain().getChain();
		for (KSSLCertificate *c = chain.first(); c; c = chain.next())
		{
			theChain += c->toString();
			theChain += "\n";
		}
		setMetaData("ssl_peer_chain", theChain);
	}
	else
	{
		setMetaData("ssl_peer_chain", "");
	}

	setMetaData("ssl_cert_state", QString::number(ksv));

	if (ksv == KSSLCertificate::Ok)
	{
		rc = 1;
		setMetaData("ssl_action", "accept");
	}

	// Since we're the parent, we need to teach the child.
	setMetaData("ssl_parent_ip", ourIp );
	setMetaData("ssl_parent_cert", pc.toString());

	//  - Read from cache and see if there is a policy for this
	KSSLCertificateCache::KSSLCertificatePolicy cp = d->cc->getPolicyByCertificate(pc);
	//  - validation code
	if (ksv != KSSLCertificate::Ok)
	{
		if( cp == KSSLCertificateCache::Unknown || cp == KSSLCertificateCache::Ambiguous)
		{
			cp = KSSLCertificateCache::Prompt;
		}
		else
		{
			// A policy was already set so let's honor that.
			permacache = d->cc->isPermanent(pc);
		}

		if (!_IPmatchesCN && (metaData("ssl_militant") == "TRUE") 
 		    && cp == KSSLCertificateCache::Accept)
		{
			cp = KSSLCertificateCache::Prompt;
		}

		// Precondition: cp is one of Reject, Accept or Prompt
		switch (cp)
		{
			case KSSLCertificateCache::Accept:
				rc = 1;
				break;

			case KSSLCertificateCache::Reject:
				rc = -1;
				break;

			case KSSLCertificateCache::Prompt:
			{
				do
				{
					if (ksv == KSSLCertificate::InvalidHost)
					{
						QString msg = i18n("The IP address of the host %1 "
								"does not match the one the "
								"certificate was issued to.");
						result = messageBox( KIO::SlaveBase::WarningYesNoCancel,
						msg.arg(ourHost),
						i18n("Server Authentication"),
						i18n("&Details"),
						i18n("Co&ntinue") );
					}
					else
					{
						QString msg = i18n("The server certificate failed the "
							"authenticity test (%1).");
						result = messageBox( KIO::SlaveBase::WarningYesNoCancel,
						msg.arg(ourHost),
						i18n("Server Authentication"),
						i18n("&Details"),
						i18n("Co&ntinue") );
					}
				}
				while (result == KMessageBox::Yes);

				if (result == KMessageBox::No)
				{
					rc = 1;
					cp = KSSLCertificateCache::Accept;
					doAddHost = true;
					result = messageBox( KIO::SlaveBase::WarningYesNo,
							i18n("Would you like to accept this "
							"certificate forever without "
							"being prompted?"),
							i18n("Server Authentication"),
								i18n("&Forever"),
								i18n("&Current Sessions Only"));
					if (result == KMessageBox::Yes)
						permacache = true;
					else
						permacache = false;
				}
				else
				{
					rc = -1;
					cp = KSSLCertificateCache::Prompt;
				}

				break;
		}
		default:
		kdDebug(0) << "SSL error in cert code."
				<< endl;
		break;
		}
	}

	//  - cache the results
	d->cc->addCertificate(pc, cp, permacache);
	if (doAddHost)
		d->cc->addHost(pc, ourHost);


	if (rc == -1)
		return rc;


	kdDebug(0) << "SSL connection information follows:" << endl
		<< "+-----------------------------------------------" << endl
		<< "| Cipher: " << d->kssl->connectionInfo().getCipher() << endl
		<< "| Description: " << d->kssl->connectionInfo().getCipherDescription() << endl
		<< "| Version: " << d->kssl->connectionInfo().getCipherVersion() << endl
		<< "| Strength: " << d->kssl->connectionInfo().getCipherUsedBits()
		<< " of " << d->kssl->connectionInfo().getCipherBits()
		<< " bits used." << endl
		<< "| PEER:" << endl
		<< "| Subject: " << d->kssl->peerInfo().getPeerCertificate().getSubject() << endl
		<< "| Issuer: " << d->kssl->peerInfo().getPeerCertificate().getIssuer() << endl
		<< "| Validation: " << (int)ksv << endl
		<< "| Certificate matches IP: " << _IPmatchesCN << endl
		<< "+-----------------------------------------------"
		<< endl;

	return rc;
}


#include "ksslsocket.moc"
