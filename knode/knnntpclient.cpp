/***************************************************************************
                          knnntpclient.cpp  -  description
                             -------------------

    copyright            : (C) 2000 by Christian Gebauer
    email                : gebauer@bigfoot.com
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
#include <klocale.h>

#include "knsavedarticle.h"
#include "knfetcharticle.h"
#include "kngroup.h"
#include "kngroupmanager.h"
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
    case KNJobData::JTLoadGroups :
      doLoadGroups();
      break;
    case KNJobData::JTFetchGroups :
      doFetchGroups();
      break;      
    case KNJobData::JTCheckNewGroups :
      doCheckNewGroups();
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
      qDebug("knode: KNNntpClient::processJob(): mismatched job");
  }   
}
  

void KNNntpClient::doLoadGroups()
{
  KNGroupListData *target = static_cast<KNGroupListData *>(job->data());
  sendSignal(TSloadGrouplist);

  if (!target->readIn())
    job->setErrorString(i18n("Unable to read the group list file"));
}


void KNNntpClient::doFetchGroups()
{
  KNGroupListData *target = static_cast<KNGroupListData *>(job->data());
  
  sendSignal(TSdownloadGrouplist);
  errorPrefix = i18n("The group list could not be retrieved.\nThe following error occured:\n");
  
  progressValue = 100;
  predictedLines = 30000;     // rule of thumb ;-)
  
  if (!sendCommandWCheck("LIST",215))       // 215 list of newsgroups follows
    return;
    
  char *s, *line;
  QStrList tmpList;
  
  while (getNextLine()) {
    line = getCurrentLine();
    if (line[0]=='.') {
      if (line[1]=='.')
        line++;        // collapse double period into one
      else
        if (line[1]==0)
          break;   // message complete
    }
    s = strchr(line,' ');
    if(!s)
      qDebug("knode: retrieved broken group-line - ignoring");
    else {
      s[0] = 0;    // cut string
      tmpList.append(line);
    }
    doneLines++;
  }
  if (!job->success() || job->canceled())
    return;     // stopped...

  if (target->getDescriptions) {
    errorPrefix = i18n("The group descriptions could not be retrieved.\nThe following error occured:\n");
    progressValue = 100;
    doneLines = 0;
    predictedLines = tmpList.count();

    sendSignal(TSdownloadDesc);
    sendSignal(TSprogressUpdate);

    if (!sendCommandWCheck("LIST NEWSGROUPS",215))       // 215 informations follows
      return; 

    while (getNextLine()) {
      line = getCurrentLine();
      if (line[0]=='.') {
        if (line[1]=='.')
          line++;        // collapse double period into one
        else
          if (line[1]==0)
            break;   // message complete
      }
      s = line;
      while (*s != '\0' && *s != '\t' && *s != ' ') s++;
      if (*s == '\0') {
        qDebug("knode: retrieved broken group-description - ignoring");
      } else {
        s[0] = 0;         // terminate groupname
        s++;
        while (*s == ' ' || *s == '\t') s++;    // go on to the description

        if (target->subscribed.contains(line)) {
          target->subscribed.remove(line);    // group names are unique, we wont find it again anyway...
          target->groups->append(new KNGroupInfo(line,s,false,true));
        } else {
          target->groups->append(new KNGroupInfo(line,s,false,false));
        }

        tmpList.remove(line);
      }
      doneLines++;
    }
    if (!job->success() || job->canceled())
      return;     // stopped...
  }
  
  // now add all remaining groups in tmpList (those without description)
    
  while ((line=tmpList.getFirst())) {

    if (target->subscribed.contains(line)) {
      target->subscribed.remove(line);    // group names are unique, we wont find it again anyway...
      target->groups->append(new KNGroupInfo(line,"",false,true));
    } else {
      target->groups->append(new KNGroupInfo(line,"",false,false));
    }

    tmpList.removeFirst();
  }

  target->groups->sort();

  sendSignal(TSwriteGrouplist);
  if (!target->writeOut())
    job->setErrorString(i18n("Unable to write the group list file"));

}


void KNNntpClient::doCheckNewGroups()
{
  KNGroupListData *target = static_cast<KNGroupListData *>(job->data());
  
  sendSignal(TSdownloadNewGroups);
  errorPrefix = i18n("New groups could not be retrieved.\nThe following error occured:\n");
  
  progressValue = 100;
  predictedLines = 30;     // rule of thumb ;-)
  
  QCString cmd;
  cmd.sprintf("NEWGROUPS %.2d%.2d%.2d 000000",target->fetchSince.year()%100,target->fetchSince.month(),target->fetchSince.day());
  if (!sendCommandWCheck(cmd,231))      // 231 list of new newsgroups follows
    return;

  QStrList tmpList;     
  char *s, *line;

  while (getNextLine()) {
    line = getCurrentLine();
    if (line[0]=='.') {
      if (line[1]=='.')
        line++;        // collapse double period into one
      else
        if (line[1]==0)
          break;   // message complete
    }
    s = strchr(line,' ');
    if(!s)
      qDebug("knode: retrieved broken group-line - ignoring");
    else {
      s[0] = 0;    // cut string
      tmpList.append(line);
    }
    doneLines++;
  }
  if (!job->success() || job->canceled())
    return;     // stopped...

  QSortedList<KNGroupInfo> tempList;
  tempList.setAutoDelete(true);
  
  if (target->getDescriptions) {
    errorPrefix = i18n("The group descriptions could not be retrieved.\nThe following error occured:\n");
    progressValue = 100;
    doneLines = 0;
    predictedLines = tmpList.count()*3;

    sendSignal(TSdownloadDesc);
    sendSignal(TSprogressUpdate);

    cmd = "LIST NEWSGROUPS ";
    QStrList desList;
    char *s;

    for (char *group=tmpList.first(); group; group=tmpList.next()) {
      if (!sendCommandWCheck(cmd+group,215))       // 215 informations follows
        return;
      desList.clear();
      if (!getMsg(desList))
        return;

      if (desList.count()>0) {        // group has a description
        s = desList.first();
        while (*s != '\0' && *s != '\t' && *s != ' ') s++;
        if (*s == '\0') {
          qDebug("knode: retrieved broken group-description - ignoring");
          tempList.append(new KNGroupInfo(group,"",true));
        } else {
          while (*s == ' ' || *s == '\t') s++;    // go on to the description
          tempList.append(new KNGroupInfo(group,s,true));
        }
      } else {
        tempList.append(new KNGroupInfo(group,"",true));
      }
    }
  } else {    // don't fetch descriptions...
    for (char *group=tmpList.first(); group; group=tmpList.next())
      tempList.append(new KNGroupInfo(group,"",true));
  }

  sendSignal(TSloadGrouplist);

  if (!target->readIn()) {
    job->setErrorString(i18n("Unable to read the group list file"));
    return;
  }
  target->merge(&tempList);
  sendSignal(TSwriteGrouplist);
  if (!target->writeOut()) {
    job->setErrorString(i18n("Unable to write the group list file"));
    return;
  }
}


void KNNntpClient::doFetchNewHeaders()
{
  KNGroup* target=static_cast<KNGroup*>(job->data());
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
  } else
    oldlast=target->lastNr();
    
  toFetch=last-oldlast;
  //qDebug("knode: last %d  oldlast %d  toFetch %d\n",last,oldlast,toFetch);
    
  if(toFetch==0) {
    //qDebug("knode: No new Articles in group\n");
    return;
  }
  
  if(toFetch>target->maxFetch()) {
    toFetch=target->maxFetch();
    //qDebug("knode: Fetching only %d articles\n",toFetch);
  }

  progressValue = 100;  
  predictedLines = toFetch;
    
  //qDebug("knode: KNNntpClient::doFetchNewHeaders() : xover %d-%d", last-toFetch+1, last);
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
  KNFetchArticle *target = static_cast<KNFetchArticle*>(job->data());
  
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
  KNSavedArticle *art = static_cast<KNSavedArticle*>(job->data());
  
  sendSignal(TSsendArticle);  
  
  if (!sendCommandWCheck("POST",340))       // 340 send article to be posted. End with <CR-LF>.<CR-LF>
    return;

  if (!sendMsg(art->encodedData()))
    return;
    
  if (!checkNextResponse(240))            // 240 article posted ok
    return;
}


bool KNNntpClient::openConnection()
{
  QString oldPrefix = errorPrefix;
  errorPrefix=i18n("Unable to connect.\nThe following error ocurred:\n");

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

  if (rep==500) {
    qDebug("knode: \"MODE READER\" command not recognized.");
  } else
    if ((rep!=200)&&(rep!=201)) { // 200 Hello, you can post
      handleErrors();             // 201 Hello, you can't post
      return false;
    }
  
  progressValue = 70;
  
  errorPrefix = oldPrefix;
  return true;
}


// authentication on demand
bool KNNntpClient::sendCommand(const QCString &cmd, int &rep)
{
  if (!KNProtocolClient::sendCommand(cmd,rep))
    return false;
  
  if (rep==480) {            // 480 requesting authorization
    //qDebug("knode: Authorization requested");
    
    if (!account.user().length()) {
      job->setErrorString(i18n("Authentication failed!\nCheck your username and password."));
      return false;
    }

    //qDebug("knode: user: %s",account.user().data());
        
    QCString command = "AUTHINFO USER ";
    command += account.user().local8Bit();
    if (!KNProtocolClient::sendCommand(command,rep))
      return false;
    
    if (rep==381) {          // 381 PASS required
      //qDebug("knode: Password required");
      
      if (!account.pass().length()) {
        job->setErrorString(i18n("Authentication failed!\nCheck your username and password."));
        return false;
      } 
          
      //qDebug("knode: pass: %s",account.pass().data());
      
      command = "AUTHINFO PASS ";
      command += account.pass().local8Bit();
      if (!KNProtocolClient::sendCommand(command,rep))
        return false; 
    }
    
    if (rep==281) {         // 281 authorization success
      //qDebug("knode: Authorization successful");
      if (!KNProtocolClient::sendCommand(cmd,rep))    // retry the original command
        return false;
    } else {
      job->setErrorString(i18n("Authentication failed!\nCheck your username and password."));
      return false;
    }
  }
  return true;      
}


void KNNntpClient::handleErrors()
{
  if (errorPrefix.isEmpty())
    job->setErrorString(i18n("An error occured:\n%1").arg(getCurrentLine()));
  else
    job->setErrorString(errorPrefix + getCurrentLine());

  int code = atoi(getCurrentLine());

  // close the connection only when necessary:
  if ((code != 430)&&(code != 411))  // 430 no such article found / 411 no such news group
    closeConnection();
}


//--------------------------------

#include "knnntpclient.moc"
