#ifndef MK_POP3_PROTO_H
#define MK_POP3_PROTO_H

#include "kio_proto.h"
#include <kurl.h>

class Pop3_Protocol : public KIO_Protocol
{
public:
	Pop3_Protocol()  {}
	virtual ~Pop3_Protocol() {}

	virtual KIO_Protocol * clone() const { return new Pop3_Protocol; }

	virtual bool connectionBased() const { return true; }
	
	virtual QString protocol() const { return "pop3"; }
	virtual QString configName() const { return "pop3"; }
	virtual bool canReadSubjects() const { return true; }
	virtual bool canDeleteMail() const { return true; }
	virtual bool canReadMail() const { return true; }

	virtual bool hasMailbox() const { return false; }
	virtual bool hasAuth() const { return true; }
	virtual unsigned short defaultPort() const { return 110; }

	virtual DeleteTypeEnum deleteFunction() const { return get; }

	virtual QStringList authList() const { return QStringList::split( '|', "Plain|APOP", false ); }
	
	virtual void readSubjectKURL( KURL & kurl, KIO::MetaData & ) { kurl.setPath( kurl.path().replace( "/download/", "/headers/" ) ); }
	virtual void deleteMailKURL ( KURL & kurl, KIO::MetaData & ) { kurl.setPath( kurl.path().replace( "/download/", "/remove/" ) ); }
	virtual bool commitDelete () { return true; }
	virtual void deleteCommitKURL(KURL & kurl, KIO::MetaData & ) { kurl.setPath( "commit" ); }
};

#endif
