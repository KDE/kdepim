/***************************************************************************
                          knprotocolclient.cpp  -  description
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

#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <klocale.h>

#include "knprotocolclient.h"

KNProtocolClient::KNProtocolClient(int NfdPipeIn, int NfdPipeOut, QObject *parent, const char *name)
:  QObject(parent,name), job(0L), inputSize(10000), fdPipeIn(NfdPipeIn), fdPipeOut(NfdPipeOut), tcpSocket(-1)
{
  input = new char[inputSize];
}

KNProtocolClient::~KNProtocolClient()
{
  if (isConnected())
    closeConnection();
  delete input;
}


void* KNProtocolClient::startThread(void* pseudoThis)
{
  KNProtocolClient* newthis = (KNProtocolClient*) (pseudoThis);

  if (0!=pthread_setcanceltype(PTHREAD_CANCEL_ENABLE,NULL))
    qDebug("pthread_setcanceltype failed!");
  if (0!= pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL))
    qDebug("pthread_setcanceltype failed!");

  signal(SIGPIPE,SIG_IGN);   // ignore sigpipe

  newthis->waitForWork();
  return NULL;
}


void KNProtocolClient::insertJob(KNJobData *newJob)
{
  job = newJob;
}


void KNProtocolClient::removeJob()
{
  job = 0L;
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
      selectRet = select(FD_SETSIZE, &fdsR, NULL, &fdsE, &tv);
      if (selectRet == 0) {
      	qDebug("KNProtocolClient::waitForWork(): hold time elapsed, closing connection.");
				closeConnection();               // nothing happend...
      } else {
				if (((selectRet > 0)&&(!FD_ISSET(fdPipeIn,&fdsR)))||(selectRet == -1)) {
        	qDebug("KNProtocolClient::waitForWork(): connection broken, closing it");
          closeSocket();
        }
      }
    }

    do {
      FD_ZERO(&fdsR);
      FD_SET(fdPipeIn, &fdsR);
    } while (select(FD_SETSIZE, &fdsR, NULL, NULL, NULL)<0);  // don't get tricked by signals

    clearPipe();      // remove start signal

    sendSignal(TSjobStarted);
    if (job) {
    // 	qDebug("KNProtocolClient::waitForWork(): got job");  too verbose

      if (!account.isEqual(job->account())) {     // server changed
      	account.copy(job->account());
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

      if (!isConnected())
				openConnection();

      if (isConnected())         // connection is ready
				processJob();
			errorPrefix = "";
				
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

 	qDebug("KNProtocolClient::openConnection(): opening connection");
	
  if (account.server().isEmpty()) {
    job->setErrorString(i18n("Unable to resolve hostname"));
    return false;
  }

  tcpSocket = socket(PF_INET,SOCK_STREAM,0);
  if (-1 == tcpSocket) {
    QString str = i18n("Communication error:\n");
    str += strerror(errno);
    job->setErrorString(str);
    return false;
  }

#if (__GNU_LIBRARY__ != 1)        // non-blocking operation doesn't work on libc5
  if (-1 == fcntl(tcpSocket,F_SETFL,O_NONBLOCK)) {  // make socket non-blocking
    QString str = i18n("Communication error:\n");
    str += strerror(errno);
    job->setErrorString(str);
    closeSocket();
    return false;
  }
#endif

  in_addr address;

  if (inet_aton(account.server(),&address)) {
    if (!conRawIP(&address)) {
      closeSocket();
      return false;
    }
  } else {               // host name lookup....
    struct hostent* hostData = gethostbyname(account.server());

    if (NULL==hostData) {
      herror("connect(): ");
	    job->setErrorString(i18n("Unable to resolve hostname"));
      closeSocket();
      return false;
    }

    char** addr_list = hostData->h_addr_list;  // Try every IP
    while ((*addr_list)&&(!conRawIP((struct in_addr*)*addr_list)))
      ++addr_list;
    if (!(*addr_list)) {
      closeSocket();
      return false;
    }
  }
  return true;
}


// sends QUIT-command and closes the socket
void KNProtocolClient::closeConnection()
{
  fd_set fdsW;
  timeval tv;

 	qDebug("KNProtocolClient::closeConnection(): closing connection");

  FD_ZERO(&fdsW);
  FD_SET(tcpSocket, &fdsW);
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  int ret = select(FD_SETSIZE, NULL, &fdsW, NULL, &tv);

  if (ret > 0) {    // we can write...
    QCString cmd = "QUIT\r\n";
    int todo = cmd.length();
    send(tcpSocket,&cmd.data()[0],todo, MSG_NOSIGNAL);
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
bool KNProtocolClient::sendMsg(const DwString &msg)
{
	const char *line = msg.c_str();
	char *end;
	QCString buffer;
	size_t length;
	char inter[10000];
	
	progressValue = 100;
	predictedLines = msg.length()/80;	  // rule of thumb
	
	while ((end = strstr(line,"\r\n"))) {
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
    if ((inputEnd-input) > 9500) {
      job->setErrorString(i18n("Message size exceeded the size of the internal buffer."));
      closeSocket();
      return false;
    }
    if (!waitForRead())
      return false;

    int received;
    do {
      received = recv(tcpSocket, inputEnd, inputSize-(inputEnd-input)-1, MSG_NOSIGNAL);
    } while ((received<0)&&(errno==EINTR));       // don't get tricked by signals

    if (received <= 0) {
      job->setErrorString(i18n("The connection is broken."));
      closeSocket();
      return false;
    }
    inputEnd += received;
    inputEnd[0] = 0;  // terminate *char

    byteCount += received;
    if (predictedLines != -1)
      progressValue = 100 + (doneLines*900/predictedLines);

    sendSignal(TSprogressUpdate);

  } while (!(nextLine = strstr(thisLine,"\r\n")));
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
    job->setErrorString(i18n("An error occoured:\n"));
  else
    job->setErrorString(errorPrefix + thisLine);

  closeConnection();
}


void KNProtocolClient::sendSignal(threadSignal s)
{
	int signal=(int)s;
	// qDebug("KNProtcolClient::sendSignal() : sending signal to main thread"); too verbose
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
    ret = select(FD_SETSIZE, &fdsR, NULL, &fdsE, &tv);
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
      job->setErrorString(i18n("A delay occured which exceeded the\ncurrent timeout limit."));
    closeConnection();
    return false;
  }
  if (ret > 0) {
    if (FD_ISSET(fdPipeIn,&fdsR)) {  // stop signal
     	qDebug("KNProtocolClient::waitForRead(): got stop signal");
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
    ret = select(FD_SETSIZE, &fdsR, &fdsW, &fdsE, &tv);
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
      job->setErrorString(i18n("A delay occured which exceeded the\ncurrent timeout limit."));
    closeConnection();
    return false;
  }
  if (ret > 0) {
    if (FD_ISSET(fdPipeIn,&fdsR)) {  // stop signal
     	qDebug("KNProtocolClient::waitForWrite(): got stop signal");
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


// used by openConnection()
bool KNProtocolClient::conRawIP(in_addr* ip)
{
  fd_set fdsR,fdsW;
  timeval tv;

	job->setErrorString("");
  sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_port = htons(account.port());
  address.sin_addr = *ip;
  if (-1 == ::connect(tcpSocket,(struct sockaddr*)&address,sizeof(struct sockaddr_in))) {
    if (errno == EINPROGRESS) {
      int ret;
      do {
        FD_ZERO(&fdsR);
        FD_SET(fdPipeIn, &fdsR);
        FD_ZERO(&fdsW);
        FD_SET(tcpSocket, &fdsW);
        tv.tv_sec = account.timeout();
        tv.tv_usec = 0;
        ret = select(FD_SETSIZE, &fdsR, &fdsW, NULL, &tv);
      } while ((ret<0)&&(errno==EINTR));       // don't get tricked by signals

      if (ret == -1) {     // select failed
				QString str = i18n("Communication error:\n");
    		str += strerror(errno);
    		job->setErrorString(str);
				return false;				
      }

      if (ret == 0) {      // nothing happend, timeout
      	job->setErrorString(i18n("A delay occured which exceeded the\ncurrent timeout limit."));
				return false;
      }

      if ((ret > 0)&&(FD_ISSET(fdPipeIn,&fdsR)))   // stop signal
				return false;

      int err = 0;
      unsigned int len = 1;
      if (getsockopt(tcpSocket,SOL_SOCKET,SO_ERROR,&err,&len)!=0) {
       	QString str = i18n("Communication error:\n");
    		str += strerror(errno);
    		job->setErrorString(str);
				return false;
      } else {
        QCString str;
				switch (err) {
					case 0: return true;   // success!!
					case ECONNREFUSED:
					  job->setErrorString(i18n("The server refused the connection."));
					  return false;
  				case ETIMEDOUT:
	  				job->setErrorString(i18n("A delay occured which exceeded the\ncurrent timeout limit."));
					  return false;
					default:
                      job->setErrorString(i18n("Unable to connect:\n%1").arg(strerror(err)));
					  return false;
				}
      }
    } else {
      QCString str;
			switch (errno) {
				case 0: return true;   // success!!
				case ECONNREFUSED:
				  job->setErrorString(i18n("The server refused the connection."));
				  return false;
  			case ETIMEDOUT:
	  			job->setErrorString(i18n("A delay occured which exceeded the\ncurrent timeout limit."));
				  return false;
				default:
    			  job->setErrorString(i18n("Unable to connect:\n%1").arg(strerror(errno)));
				  return false;
			}
    }
  }
  return true;
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
    ret = send(tcpSocket,&str.data()[done],todo, MSG_NOSIGNAL);
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
    if (predictedLines != -1)
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
      if ( ::read(fdPipeIn, &buf, 1 ) == -1 )
	::perror( "clearPipe()" );
  } while (selectRet == 1);
}


// -----------------------------------------------------------------------------

#include "knprotocolclient.moc"
