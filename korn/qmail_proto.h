#ifndef QMAIL_PROTO_H
#define QMAIL_PROTO_H

#include "kio_proto.h"

class QMail_Protocol : public KIO_Protocol
{
public:
	QMail_Protocol() {}
	virtual ~QMail_Protocol() {}

	virtual KIO_Protocol * clone() const { return new QMail_Protocol; }

	virtual bool connectionBased() const { return true; }

	virtual QString protocol() const { return "file"; }
	virtual QString configName() const { return "qmail"; }

	virtual bool canReadSubjects() const { return true; }
	virtual bool canDeleteMail() const { return true; }
	virtual bool canReadMail() const { return true; }
	virtual bool fullMessage() const { return true; }

	virtual bool hasServer() const { return false; }
	virtual bool hasPort() const { return false; }
	virtual bool hasUsername() const { return false; }
	virtual bool hasMailbox() const { return true; }
	virtual bool hasPassword() const { return false; }
	
	virtual QString mailboxName() const { return i18n( "Path:" ); }
};

#endif
