#ifndef MK_IMAP_PROTO_H
#define MK_IMAP_PROTO_H

#include "kio_proto.h"
#include <kurl.h>

/*
 * With deleting and IMAP4 is a small problem: messages don't looks as deleted, as they
 * apear with their full body. By deletion, kio_imap marks the message with FLAGS.SILENT as \DELETED.
 * If there is a commit-function, it should be installed in this file.
 */

class Imap_Protocol : public KIO_Protocol
{
public:
	Imap_Protocol()  {}
	virtual ~Imap_Protocol() {}

	virtual KIO_Protocol * clone() const { return new Imap_Protocol; }

	virtual bool connectionBased() const { return true; }
	
	virtual QString protocol() const { return "newimap"; }
	virtual QString configName() const { return "imap"; }
	virtual bool canReadSubjects() const { return true; }
	virtual bool canDeleteMail() const { return false; } //See comment above class: metadata expunge=auto doesn't work.
	virtual bool canReadMail() const { return true; }

	virtual bool hasAuth() const { return true; }
	virtual unsigned short defaultPort() const { return 143; }

	virtual QStringList authList() const { return QStringList::split( '|', "Plain|LOGIN|ANONYMOUS|CRAM-MD5", false); }
	//Could not test did, my server don't support other authentication methods.

	virtual void recheckKURL    ( KURL &kurl, KIO::MetaData & ) { kurl.setQuery( "unseen" ); }
	virtual void readSubjectKURL( KURL &kurl, KIO::MetaData & ) { kurl.setPath( kurl.path() + ";section=ENVELOPE" ); }
	virtual void deleteMailConnectKURL( KURL &, KIO::MetaData & metadata ) { metadata.insert( "expunge", "auto" ); }
	
};

#endif
