/*
    knprotocolclient.cpp

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include <klocale.h>
#include <kextsock.h>
#include <ksocks.h>

#include "knjobdata.h"
#include "knprotocolclient.h"


KNProtocolClient::KNProtocolClient(int NfdPipeIn, int NfdPipeOut)
:  job(0L), inputSize(10000), fdPipeIn(NfdPipeIn), fdPipeOut(NfdPipeOut), tcpSocket(-1)
{
  input = new char[inputSize];
}


KNProtocolClient::~KNProtocolClient()
{
  if (isConnected())
    closeConnection();
  delete [] input;
}


void KNProtocolClient::run()
{
  if (0!=pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL))
    qWarning("pthread_setcancelstate failed!");
  if (0!= pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL))
    qWarning("pthread_setcanceltype failed!");

  signal(SIGPIPE,SIG_IGN);   // ignore sigpipe
  waitForWork();
}


void KNProtocolClient::insertJob(KNJobData *newJob)
{
  job = newJob;
}


void KNProtocolClient::removeJob()
{
  job = 0L;
}


void KNProtocolClient::updatePercentage(int percent)
{
  byteCountMode=false;
  progressValue = percent*10;
  sendSignal(TSprogressUpdate);
}


// main loop, maintains connection and waits for next job
void KNProtocolClient::waitForWork()
{
  fd_set fdsR,fdsE;
  timeval tv;
  int selectRet;

  while (true) {
    if (isConnected()) {  // we are connected, hold the connection for xx secs
      FD_ZERO(&fdsR);
      FD_SET(fdPipeIn, &fdsR);
      FD_SET(tcpSocket, &fdsR);
      FD_ZERO(&fdsE);
      FD_SET(tcpSocket, &fdsE);
      tv.tv_sec = account.hold();
      tv.tv_usec = 0;
      selectRet = KSocks::self()->select(FD_SETSIZE, &fdsR, NULL, &fdsE, &tv);
      if (selectRet == 0) {
#ifndef NDEBUG
        qDebug("knode: KNProtocolClient::waitForWork(): hold time elapsed, closing connection.");
#endif
        closeConnection();               // nothing happend...
      } else {
        if (((selectRet > 0)&&(!FD_ISSET(fdPipeIn,&fdsR)))||(selectRet == -1)) {
#ifndef NDEBUG
          qDebug("knode: KNProtocolClient::waitForWork(): connection broken, closing it");
#endif
          closeSocket();
        }
      }
    }

    do {
      FD_ZERO(&fdsR);
      FD_SET(fdPipeIn, &fdsR);
    } while (select(FD_SETSIZE, &fdsR, NULL, NULL, NULL)<0);  // don't get tricked by signals
    
    clearPipe();      // remove start signal

    timer.start();

    sendSignal(TSjobStarted);
    if (job) {
    //  qDebug("knode: KNProtocolClient::waitForWork(): got job");

      if (job->net()&&!(account == *job->account())) {     // server changed
        account = *job->account();
        if (isConnected())
          closeConnection();
      }

      input[0] = 0;                 //terminate string
      thisLine = input;
      nextLine = input;
      inputEnd = input;
      progressValue = 10;
      predictedLines = -1;
      doneLines = 0;
      byteCount = 0;
      byteCountMode = true;

      if (!job->net())    // job needs no net access
        processJob();
      else {
        if (!isConnected())
          openConnection();

        if (isConnected())         // connection is ready
          processJob();
      }
      errorPrefix = QString::null;

      clearPipe();
    }
    sendSignal(TSworkDone);      // emit stopped signal
  }
}


void KNProtocolClient::processJob()
{}


// connect, handshake and authorization
bool KNProtocolClient::openConnection()
{
  sendSignal(TSconnect);

#ifndef NDEBUG
  qDebug("knode: KNProtocolClient::openConnection(): opening connection");
#endif

  if (account.server().isEmpty()) {
    job->setErrorString(i18n("Unable to resolve hostname"));
    return false;
  }

  KExtendedSocket ks;

  ks.setAddress(account.server(), account.port());
  ks.setTimeout(account.timeout());
  if (ks.connect() < 0) {
    if (ks.status() == IO_LookupError) {
      job->setErrorString(i18n("Unable to resolve hostname"));
    } else if (ks.status() == IO_ConnectError) {
      job->setErrorString(i18n("Unable to connect:\n%1").arg(KExtendedSocket::strError(ks.status(), errno)));
    } else if (ks.status() == IO_TimeOutError)
      job->setErrorString(i18n("A delay occurred which exceeded the\ncurrent timeout limit."));
    else
      job->setErrorString(i18n("Unable to connect:\n%1").arg(KExtendedSocket::strError(ks.status(), errno)));

    closeSocket();
    return false;
  }

  tcpSocket = ks.fd();
  ks.release();

  return true;
}


// sends QUIT-command and closes the socket
void KNProtocolClient::closeConnection()
{
  fd_set fdsW;
  timeval tv;

#ifndef NDEBUG
  qDebug("knode: KNProtocolClient::closeConnection(): closing connection");
#endif

  FD_ZERO(&fdsW);
  FD_SET(tcpSocket, &fdsW);
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  int ret = KSocks::self()->select(FD_SETSIZE, NULL, &fdsW, NULL, &tv);

  if (ret > 0) {    // we can write...
    QCString cmd = "QUIT\r\n";
    int todo = cmd.length();
    KSocks::self()->write(tcpSocket,&cmd.data()[0],todo);
  }
  closeSocket();
}


// sends a command (one line), return code is written to rep
bool KNProtocolClient::sendCommand(const QCString &cmd, int &rep)
{
  if (!sendStr(cmd + "\r\n"))
    return false;
  if (!getNextResponse(rep))
    return false;
  return true;
}


// checks return code and calls handleErrors() if necessary
bool KNProtocolClient::sendCommandWCheck(const QCString &cmd, int rep)
{
  int code;

  if (!sendCommand(cmd,code))
    return false;
  if (code!=rep) {
    handleErrors();
    return false;
  }
  return true;
}


// sends a message (multiple lines)
bool KNProtocolClient::sendMsg(const QCString &msg)
{
  const char *line = msg.data();
  const char *end;
  QCString buffer;
  size_t length;
  char inter[10000];

  progressValue = 100;
  predictedLines = msg.length()/80;   // rule of thumb

  while ((end = ::strstr(line,"\r\n"))) {
    if (line[0]=='.')                     // expand one period to double period...
      buffer.append(".");
    length = end-line+2;
    if ((buffer.length()>1)&&((buffer.length()+length)>1024)) {    // artifical limit, because I don't want to generate too large blocks
      if (!sendStr(buffer))
        return false;
      buffer = "";
    }
    if (length > 9500) {
      job->setErrorString(i18n("Message size exceeded the size of the internal buffer."));
      closeSocket();
      return false;
    }
    memcpy(inter,line,length);
    inter[length]=0;             // terminate string
    buffer += inter;
    line = end+2;
    doneLines++;
  }
  buffer += ".\r\n";
  if (!sendStr(buffer))
    return false;

  return true;
}


// reads next complete line of input
bool KNProtocolClient::getNextLine()
{
  thisLine = nextLine;
  nextLine = strstr(thisLine,"\r\n");
  if (nextLine) {                           // there is another full line in the inputbuffer
    nextLine[0] = 0;  // terminate string
    nextLine[1] = 0;
    nextLine+=2;
    return true;
  }
  unsigned int div = inputEnd-thisLine+1;   // hmmm, I need to fetch more input from the server...
  memmove(input,thisLine,div);      // save last, incomplete line
  thisLine = input;
  inputEnd = input+div-1;
  do {
    div = inputEnd-thisLine+1;
    if ((div) > inputSize-100) {
      inputSize += 10000;
      char *newInput = new char[inputSize];
      memmove(newInput,input,div);
      delete [] input;
      input = newInput;
      thisLine = input;
      inputEnd = input+div-1;
#ifndef NDEBUG
      qDebug("knode: KNProtocolClient::getNextLine(): input buffer enlarged");
#endif
    }
    if (!waitForRead())
      return false;

    int received;
    do {
      received = KSocks::self()->read(tcpSocket, inputEnd, inputSize-(inputEnd-input)-1);
    } while ((received<0)&&(errno==EINTR));       // don't get tricked by signals

    if (received <= 0) {
      job->setErrorString(i18n("The connection is broken."));
      closeSocket();
      return false;
    }

    // remove null characters that some stupid servers return...
    for (int i=0; i<received; i++)
      if (inputEnd[i] == 0) {
         memmove(inputEnd+i,inputEnd+i+1,received-i-1);
         received--;
         i--;
      }

    inputEnd += received;
    inputEnd[0] = 0;  // terminate *char

    byteCount += received;

  } while (!(nextLine = strstr(thisLine,"\r\n")));

  if (timer.elapsed()>50) {   // reduce framerate to 20 f/s
    timer.start();
    if (predictedLines > 0)
      progressValue = 100 + (doneLines*900/predictedLines);
    sendSignal(TSprogressUpdate);
  }

  nextLine[0] = 0;  // terminate string
  nextLine[1] = 0;
  nextLine+=2;
  return true;
}


// receives a message (multiple lines)
bool KNProtocolClient::getMsg(QStrList &msg)
{
  char *line;

  while (getNextLine()) {
    line = getCurrentLine();
    if (line[0]=='.') {
      if (line[1]=='.')
        line++;        // collapse double period into one
      else
        if (line[1]==0)
          return true;   // message complete
    }
    msg.append(line);
    doneLines++;
  }

  return false;        // getNextLine() failed
}


// reads next line and returns the response code
bool KNProtocolClient::getNextResponse(int &code)
{
  if (!getNextLine())
    return false;
  code = -1;
  code = atoi(thisLine);
  return true;
}


// checks return code and calls handleErrors() if necessary
bool KNProtocolClient::checkNextResponse(int code)
{
  if (!getNextLine())
    return false;
  if (atoi(thisLine)!=code) {
    handleErrors();
    return false;
  }
  return true;
}



// interprets error code, generates error message and closes the connection
void KNProtocolClient::handleErrors()
{
  if (errorPrefix.isEmpty())
    job->setErrorString(i18n("An error occurred:\n%1").arg(thisLine));
  else
    job->setErrorString(errorPrefix + thisLine);

  closeConnection();
}


void KNProtocolClient::sendSignal(threadSignal s)
{
  int signal=(int)s;
  // qDebug("knode: KNProtcolClient::sendSignal() : sending signal to main thread");
  write(fdPipeOut, &signal, sizeof(int));
}


// waits until socket is readable
bool KNProtocolClient::waitForRead()
{
  fd_set fdsR,fdsE;
  timeval tv;

  int ret;
  do {
    FD_ZERO(&fdsR);
    FD_SET(fdPipeIn, &fdsR);
    FD_SET(tcpSocket, &fdsR);
    FD_ZERO(&fdsE);
    FD_SET(tcpSocket, &fdsE);
    FD_SET(fdPipeIn, &fdsE);
    tv.tv_sec = account.timeout();
    tv.tv_usec = 0;
    ret = KSocks::self()->select(FD_SETSIZE, &fdsR, NULL, &fdsE, &tv);
  } while ((ret<0)&&(errno==EINTR));             // don't get tricked by signals

  if (ret == -1) {     // select failed
    if (job) {
      QString str = i18n("Communication error:\n");
      str += strerror(errno);
      job->setErrorString(str);
    }
    closeSocket();
    return false;
  }
  if (ret == 0) {      // Nothing happend, timeout
    if (job)
      job->setErrorString(i18n("A delay occurred which exceeded the\ncurrent timeout limit."));
    closeConnection();
    return false;
  }
  if (ret > 0) {
    if (FD_ISSET(fdPipeIn,&fdsR)) {  // stop signal
#ifndef NDEBUG
      qDebug("knode: KNProtocolClient::waitForRead(): got stop signal");
#endif
      closeConnection();
      return false;
    }
    if (FD_ISSET(tcpSocket,&fdsE)||FD_ISSET(fdPipeIn,&fdsE)) {  // broken pipe, etc
      if (job)
        job->setErrorString(i18n("The connection is broken."));
      closeSocket();
      return false;
    }
    if (FD_ISSET(tcpSocket,&fdsR))  // all ok
      return true;
  }

  if (job)
    job->setErrorString(i18n("Communication error"));
  closeSocket();
  return false;
}


// used by sendBuffer() & connect()
bool KNProtocolClient::waitForWrite()
{
  fd_set fdsR,fdsW,fdsE;
  timeval tv;

  int ret;
  do {
    FD_ZERO(&fdsR);
    FD_SET(fdPipeIn, &fdsR);
    FD_SET(tcpSocket, &fdsR);
    FD_ZERO(&fdsW);
    FD_SET(tcpSocket, &fdsW);
    FD_ZERO(&fdsE);
    FD_SET(tcpSocket, &fdsE);
    FD_SET(fdPipeIn, &fdsE);
    tv.tv_sec = account.timeout();
    tv.tv_usec = 0;
    ret = KSocks::self()->select(FD_SETSIZE, &fdsR, &fdsW, &fdsE, &tv);
  } while ((ret<0)&&(errno==EINTR));             // don't get tricked by signals


  if (ret == -1) {     // select failed
    if (job) {
      QString str = i18n("Communication error:\n");
      str += strerror(errno);
      job->setErrorString(str);
    }
    closeSocket();
    return false;
  }
  if (ret == 0) {      // nothing happend, timeout
    if (job)
      job->setErrorString(i18n("A delay occurred which exceeded the\ncurrent timeout limit."));
    closeConnection();
    return false;
  }
  if (ret > 0) {
    if (FD_ISSET(fdPipeIn,&fdsR)) {  // stop signal
#ifndef NDEBUG
      qDebug("knode: KNProtocolClient::waitForWrite(): got stop signal");
#endif
      closeConnection();
      return false;
    }
    if (FD_ISSET(tcpSocket,&fdsR)||FD_ISSET(tcpSocket,&fdsE)||FD_ISSET(fdPipeIn,&fdsE)) {  // broken pipe, etc
      if (job)
        job->setErrorString(i18n("The connection is broken."));
      closeSocket();
      return false;
    }
    if (FD_ISSET(tcpSocket,&fdsW))  // all ok
      return true;
  }

  if (job)
    job->setErrorString(i18n("Communication error"));
  closeSocket();
  return false;
}


void KNProtocolClient::closeSocket()
{
  if (-1 != tcpSocket) {
    close(tcpSocket);
    tcpSocket = -1;
  }
}


// sends str to the server
bool KNProtocolClient::sendStr(const QCString &str)
{
  int ret;
  int todo = str.length();
  int done = 0;

  while (todo > 0) {
    if (!waitForWrite())
      return false;
    ret = KSocks::self()->write(tcpSocket,&str.data()[done],todo);
    if (ret <= 0) {
      if (job) {
        QString str = i18n("Communication error:\n");
        str += strerror(errno);
        job->setErrorString(str);
      }
      closeSocket();
      return false;
    } else {
      done += ret;
      todo -= ret;
    }
    byteCount += ret;
  }
  if (timer.elapsed()>50) {   // reduce framerate to 20 f/s
    timer.start();
    if (predictedLines > 0)
      progressValue = 100 + (doneLines/predictedLines)*900;
    sendSignal(TSprogressUpdate);
  }
  return true;
}


// removes start/stop signal
void KNProtocolClient::clearPipe()
{
  fd_set fdsR;
  timeval tv;
  int selectRet;
  char buf;

  tv.tv_sec = 0;
  tv.tv_usec = 0;
  do {
    FD_ZERO(&fdsR);
    FD_SET(fdPipeIn,&fdsR);
    if (1==(selectRet=select(FD_SETSIZE,&fdsR,NULL,NULL,&tv)))
      if ( read(fdPipeIn, &buf, 1 ) == -1 )
  ::perror( "clearPipe()" );
  } while (selectRet == 1);
}

