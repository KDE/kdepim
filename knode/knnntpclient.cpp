/***************************************************************************
                          knnntpclient.cpp  -  description
                             -------------------

    copyright            : (C) 2000 by Christian Thurner
    email                : cthurner@freepage.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stdlib.h>
#include <qstrlist.h>
//#include <kapp.h>
#include <klocale.h>

#include "knsavedarticle.h"
#include "kngroup.h"
#include "knjobdata.h"
#include "knnntpclient.h"

KNNntpClient::KNNntpClient(int NfdPipeIn, int NfdPipeOut, QObject *parent, const char *name)
: KNProtocolClient(NfdPipeIn,NfdPipeOut,parent,name)
{}


KNNntpClient::~KNNntpClient()
{}
	

// examines the job and calls the suitable handling method
void KNNntpClient::processJob()
{
	switch (job->type()) {
		case KNJobData::JTlistGroups :
			doListGroups();
			break;
		case KNJobData::JTfetchNewHeaders :
			doFetchNewHeaders();
			break;
		case KNJobData::JTfetchArticle :
			doFetchArticle();
			break;
		case KNJobData::JTpostArticle :
			doPostArticle();
			break;
		default:
			qDebug("KNNntpClient::processJob(): mismatched job");		
	}		
}
	

void KNNntpClient::doListGroups()
{
	QStrList *target=(QStrList*)job->data();
	char *s;
	
	sendSignal(TSdownloadGrouplist);
	errorPrefix = i18n("The grouplist could not be retrieved.\nThe following error occured:\n");
	
	progressValue = 100;
	predictedLines = 30000;     // rule of thumb ;-)
	
	if (!sendCommandWCheck("LIST",215))       // 215 list of newsgroups follows
		return;
			
	if (!getMsg(*target))
		return;
		
  progressValue = 1000;		
  sendSignal(TSprogressUpdate);
	
	char *line=target->first();	
	while (line) {
		s = strchr(line,' ');
		if(!s) {
			qDebug("retrieved broken group-line - ignoring !!");
			if (line==target->getLast()) {     // if it's the last, after remove() the
				target->remove();                // next to last line would get current.
				line = 0L;
			} else {
				target->remove();
				line = target->current();
			}
		} else {
			s[0] = 0;    // cut string
			line = target->next();
		}
	}
}


void KNNntpClient::doFetchNewHeaders()
{
	KNGroup* target=(KNGroup*)job->data();
	char* s;
	int first, last, oldlast, toFetch;
	QCString cmd;
	
	sendSignal(TSdownloadNew);
	errorPrefix=i18n("No new articles could have been retrieved!\nThe following error ocurred:\n");
	
	cmd="GROUP ";
	cmd+=target->groupname();
	if (!sendCommandWCheck(cmd,211))       // 211 n f l s group selected
		return;
		
  progressValue = 90;	
		
	s = strchr(getCurrentLine(),' ');	
	if (s) {
	  s++;
		s = strchr(s,' ');	
	}
	if (s) {
	  s++;
		first=atoi(s);
		s = strchr(s,' ');
	}
	if (s) {
		last=atoi(s);
	} else {
		QString tmp=i18n("No new articles could have been retrieved!\nThe server sent a malformated response:\n");
		tmp+=getCurrentLine();
		job->setErrorString(tmp);
		return;
	}
	
	if(target->lastNr()==0) {   //first fetch
		if(first>0)
			oldlast=first-1;
		else
			oldlast=first;
	}	else
		oldlast=target->lastNr();
		
	toFetch=last-oldlast;
	qDebug("last %d  oldlast %d  toFetch %d\n",last,oldlast,toFetch);
		
	if(toFetch==0) {
		qDebug("No new Articles in group\n");
		return;
	}
	
	if(toFetch>target->maxFetch()) {
		toFetch=target->maxFetch();
		qDebug("Fetching only %d articles\n",toFetch);
	}

  progressValue = 100;	
 	predictedLines = toFetch;
		
	qDebug("KNNntpClient::doFetchNewHeaders() : xover %d-%d", last-toFetch+1, last);
	cmd.sprintf("xover %d-%d",last-toFetch+1,last);
	if (!sendCommandWCheck(cmd,224))       // 224 success
		return;
  	
	QStrList headers;
	if (!getMsg(headers))
		return;
	
	progressValue = 1000;		
  sendSignal(TSprogressUpdate);
  	
	sendSignal(TSsortNew);
	target->insortNewHeaders(&headers);
	target->setLastNr(last);
}


void KNNntpClient::doFetchArticle()
{
	KNFetchArticle *target=(KNFetchArticle*)job->data();
	
	sendSignal(TSdownloadArticle);
	errorPrefix = i18n("Article could not been retrieved.\nThe following error occured:\n");

	progressValue = 100;
	predictedLines = target->lines()+10;
		
	QCString cmd = "ARTICLE " + target->messageId();
	if (!sendCommandWCheck(cmd,220))       // 220 n <a> article retrieved - head and body follow
		return;
	
	QStrList msg;
	if (!getMsg(msg))
		return;
		
	progressValue = 1000;		
  sendSignal(TSprogressUpdate);
		
  target->setData(&msg,false);
	target->parse();
}


void KNNntpClient::doPostArticle()
{
	KNSavedArticle *art =(KNSavedArticle*)job->data();
	
	sendSignal(TSsendArticle);	
	
	if (!sendCommandWCheck("POST",340))       // 340 send article to be posted. End with <CR-LF>.<CR-LF>
		return;

	if (!sendMsg(art->encodedData()))
		return;
		
	if (!checkNextResponse(240))           	// 240 article posted ok
		return;
}


bool KNNntpClient::openConnection()
{
	if (!KNProtocolClient::openConnection())
		return false;
		
	progressValue = 30;
		
	int rep;
	if (!getNextResponse(rep))
		return false;
		
	if ((rep!=200)&&(rep!=201)) {  // 200 server ready - posting allowed
	  handleErrors();              // 201 server ready - no posting allowed
	  return false;
	}
	
	progressValue = 50;

	if (!sendCommand("MODE READER",rep))
		return false;

	if ((rep!=200)&&(rep!=201)) { // 200 Hello, you can post
	  handleErrors();             // 201 Hello, you can't post
	  return false;
	}
	
	progressValue = 70;
	
	return true;
}


// authentication on demand
bool KNNntpClient::sendCommand(const QCString &cmd, int &rep)
{
	if (!KNProtocolClient::sendCommand(cmd,rep))
		return false;
	
	if (rep==480) {            // 480 requesting authorization
		qDebug("Authorization requested");
		
		if (!account.user().length()) {
			job->setErrorString(i18n("Authentication failed !!\nCheck your username and password."));
			return false;
		}

		qDebug("user: %s",account.user().data());
				
		QCString command = "AUTHINFO USER ";
		command += account.user();
		if (!KNProtocolClient::sendCommand(command,rep))
			return false;
		
		if (rep==381) {          // 381 PASS required
			qDebug("Password required");
			
			if (!account.pass().length()) {
				job->setErrorString(i18n("Authentication failed !!\nCheck your username and password."));
				return false;
			}	
					
			qDebug("pass: %s",account.pass().data());
			
			command = "AUTHINFO PASS ";
			command += account.pass();
			if (!KNProtocolClient::sendCommand(command,rep))
				return false;	
		}
		
		if (rep==281) {         // 281 authorization success
			qDebug("Authorization successful");			
			if (!KNProtocolClient::sendCommand(cmd,rep))    // retry the original command
				return false;
		} else {
			qDebug("Authorization failed");
			handleErrors();
			return false;
		}
	}
	return true;			
}
