#ifndef MK_POP3S_PROTO_H
#define MK_POP3S_PROTO_H

//Looks very simulay to Pop3_Protocol, so I inherit it, and only overload the difference.

#include "pop3_proto.h"

class Pop3s_Protocol : public Pop3_Protocol
{
public:
	Pop3s_Protocol()  {}
	virtual ~Pop3s_Protocol() {}

	virtual KIO_Protocol * clone() const { return new Pop3s_Protocol; }
	
	virtual QString protocol() const { return "pop3s"; }
	virtual QString configName() const { return "pop3s"; }

	virtual unsigned short defaultPort() const { return 995; }
};

#endif
