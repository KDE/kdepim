#ifndef MK_NNTP_PROTO_H
#define MK_NNTP_PROTO_H

#include "kio_proto.h"

class Nntp_Protocol : public KIO_Protocol
{
public:
	Nntp_Protocol() { }
	virtual ~Nntp_Protocol() { }

	virtual KIO_Protocol * clone() const { return new Nntp_Protocol; }

	virtual bool connectionBased() const { return true; }

	virtual QString protocol() const { return "nntp"; }
	virtual QString configName() const { return "nntp"; }

	virtual bool canReadSubjects() const { return true; }
	virtual bool canDeleteMail() const { return false; }
	virtual bool canReadMail() const { return true; }
	virtual bool fullMessage() const { return true; }

	virtual unsigned short defaultPort() const { return 119; }
};

#endif
