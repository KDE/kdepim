#ifndef MK_PROCESS_PROTOCOL
#define MK_PROCESS_PROTOCOL

#include "kio_proto.h"

class Process_Protocol : public KIO_Protocol
{
public:
	Process_Protocol() { }
	virtual ~Process_Protocol() { }

	virtual KIO_Protocol * clone() const { return new Process_Protocol; }

	virtual QString protocol() const { return "process"; }
	virtual QString configName() const { return "process"; }

	virtual bool canReadSubjects() const { return false; }
	virtual bool canDeleteMail() const { return false; }
	virtual bool canReadMail() const { return false; }
	virtual bool fullMessage() const { return false; }

	virtual bool hasServer() const { return false; }
	virtual bool hasPort() const { return false; }
	virtual bool hasUsername() const { return false; }
	virtual bool hasMailbox() const { return true; }
	virtual bool hasPassword() const { return false; }

	virtual QString mailboxName() const { return i18n("Program: "); }
};

#endif
