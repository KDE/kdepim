#ifndef MK_IMAPS_PROTO_H
#define MK_IMAPS_PROTO_H

#include "kio_proto.h"

//Looks very simular to Imap_Protocol, so I inheritanced it.

class Imaps_Protocol : public Imap_Protocol
{
public:
	Imaps_Protocol()  {}
	virtual ~Imaps_Protocol() {}

	virtual KIO_Protocol * clone() const { return new Imaps_Protocol; }
	
	virtual QString protocol() const { return "newimaps"; }
	virtual QString configName() const { return "imaps"; }

	virtual unsigned short defaultPort() const { return 993; }
};

#endif
