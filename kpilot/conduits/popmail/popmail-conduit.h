// popmail-conduit.h
//
// Copyright (C) 1998,1999 Dan Pilone
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING. 
//
// $Revision$



#ifndef _POPMAIL_CONDUIT_H
#define _POPMAIL_CONDUIT_H

#include "baseConduit.h"
#include "pi-mail.h"

class PilotRecord;

class PopMailConduit : public BaseConduit
{
public:
  PopMailConduit(eConduitMode mode);
  virtual ~PopMailConduit();
  
  virtual void doSync();
  virtual QWidget* aboutAndSetup();

  virtual const char* dbInfo() { return "MailDB"; }

	virtual QPixmap *icon() const;

	/**
	* Returns a string describing the version of
	* the popmail conduit being used. This is used
	* in mail headers and in window captions.
	*/
	static const char *version();


	typedef enum RetrievalMode { NONE=0, POP=1, UNIXMailbox=2 } ;
	typedef enum SendMode { SEND_NONE=0,
		SEND_SENDMAIL=7,
		SEND_SMTP=12
		} ;


	static PilotRecord *readMessage(FILE *mailbox,
	        char *buffer,int bufferSize);
  
protected:
	// Pilot -> Sendmail
	//
	//
	int sendPendingMail(int mode /* unused */);
	int sendViaSendmail();
	int sendViaSMTP();
	void sendMessage(FILE* sendf, struct Mail& theMail);


	// Local mail -> Pilot
	//
	//
	int retrieveIncoming(int mode);
	int doPopQuery();
	int doUnixStyle();


  // Taken from pilot-mail.c in pilot-link.0.8.7 by Kenneth Albanowski
  int getpopchar(int socket);
  int getpopstring(int socket, char * buf);
  int getpopresult(int socket, char * buf);

	// Helper functions for the POP mail handler
	//
	//
	typedef enum { POP_DELE=16 } retrieveFlags ;
	void retrievePOPMessages(KSocket *,
		int const msgcount,
		int const flags,
		char *buffer, int const buffer_size);

	static char* skipspace(char * c);
	static void header(struct Mail * m, char * t);

	/**
	* This function skips blank lines in the
	* input file; the first non-blank line
	* is returned in buf. size is the size of
	* buf in bytes; lines longer than size will
	* be truncated (this is like fgets()).
	* @return Number of blank lines skipped
	*/
	static int skipBlanks(FILE *,char *buf,int size);

	/**
	* Reads a file, using a buffer buf of size
	* size, and interprets this as mail headers.
	* The headers are placed in the struct Mail *.
	* If expectFrom is non-zero, requires a leading
	* "From address" line, as per standard UNIX mailbox
	* style.
	* @return 0 if the headers are bad (specifically,
	* if a required From line is missing)
	* @return <0 the number of header lines read,
	* if the message ends during the headers
	* return >0 the number of header lines read if
	* the message continues normally.
	*/
	static int readHeaders(FILE *,char *buf,int size,
		struct Mail *,int expectFrom);
	static int readBody(FILE *,char *buf,int size);

private:
};

#endif
