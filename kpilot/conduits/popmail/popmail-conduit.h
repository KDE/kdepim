// Conduit for KPilot <--> POP3
// (c) 1998 Dan Pilone

#ifndef _POPMAIL_CONDUIT_H
#define _POPMAIL_CONDUIT_H

#include "baseConduit.h"
#include <kmsgbox.h>
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
  
protected:
  void sendPendingMail();
  void sendViaSendmail();
  void sendViaSMTP();
  void sendMessage(FILE* sendf, struct Mail& theMail);
  void doPopQuery();

  // Just a convienience function
  void showMessage(char* message)
    { KMsgBox::message(0L, "Error retrieving mail", message, KMsgBox::STOP); }

  // Taken from pilot-mail.c in pilot-link.0.8.7 by Kenneth Albanowski
  int getpopchar(int socket);
  int getpopstring(int socket, char * buf);
  int getpopresult(int socket, char * buf);
  char* skipspace(char * c);
  void header(struct Mail * m, char * t);

private:
};

#endif
