/*
    knsmtpclient.cpp

    KNode, the KDE newsreader
    Copyright (c) 1999-2001 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#include <unistd.h>

#include <klocale.h>

#include "knarticle.h"
#include "knsmtpclient.h"


KNSmtpClient::KNSmtpClient(int NfdPipeIn, int NfdPipeOut)
: KNProtocolClient(NfdPipeIn,NfdPipeOut)
{}


KNSmtpClient::~KNSmtpClient()
{}


// examines the job and calls the suitable handling method
void KNSmtpClient::processJob()
{
  switch (job->type()) {
    case KNJobData::JTmail :
      doMail();
      break;
    default:
#ifndef NDEBUG
      qDebug("knode: KNSmtpClient::processJob(): mismatched job");
#endif
      break;
  }
}
  

void KNSmtpClient::doMail()
{
  KNLocalArticle *art=static_cast<KNLocalArticle*>(job->data());
  
  sendSignal(TSsendMail); 

  QCString cmd = "MAIL FROM:<";
  cmd += art->from()->email();
  cmd += ">";
  if(!sendCommandWCheck(cmd, 250))
    return;
    
  progressValue = 80;

  QStrList emails;
  art->to()->emails(&emails);
  bool rcptOK=false;

  for(char *e=emails.first() ; e; e=emails.next()) {
    cmd="RCPT TO:<" + QCString(e) + ">";
    if(sendCommandWCheck(cmd, 250))
      rcptOK=true;
  }

  if(!rcptOK) // mail has not been accepted by the smtp-host
    return;
    
  progressValue = 90;

  if(!sendCommandWCheck("DATA", 354))
    return;
    
  progressValue = 100;
  
  if(!sendMsg(art->encodedContent(true)))
    return;
    
  if(!checkNextResponse(250))
    return;
}


bool KNSmtpClient::openConnection()
{
  QString oldPrefix = errorPrefix;
  errorPrefix=i18n("Unable to connect.\nThe following error occurred:\n");

  if (!KNProtocolClient::openConnection())
    return false;
    
  progressValue = 30;
    
  if (!checkNextResponse(220))
    return false;
    
  progressValue = 50;

  char hostName[500];

  QCString cmd = "HELO ";
    
  if (gethostname(hostName,490)==0) {
    cmd += hostName;
    //qDebug("knode: KNSmtpClient::openConnection(): %s",cmd.data());
  } else {
    cmd += "foo";
    //qDebug("knode: KNSmtpClient::openConnection(): can't detect hostname, using foo");
  }

  int rep;
  if (!sendCommand(cmd,rep))
    return false;

  while (rep == 220) {  // some smtp servers send multiple "220 xxx" lines...
    if (!getNextResponse(rep))
      return false;
  }

  if (rep!=250) {
    handleErrors();
    return false;
  }

  progressValue = 70;
  
  errorPrefix = oldPrefix;
  return true;
}



//--------------------------------

