/***************************************************************************
                          knprotocolclient.h  -  description
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


#ifndef KNPROTOCOLCLIENT_H
#define KNPROTOCOLCLIENT_H

#include <qobject.h>
#include <qstrlist.h>
#include <mimelib/string.h>

#include "knjobdata.h"

struct in_addr;

class KNProtocolClient : public QObject  {

Q_OBJECT

public:
	enum threadSignal	{ TSworkDone=0, TSjobStarted=1, TSconnect=2,
	                    TSdownloadGrouplist=3, TSdownloadNew=4, TSsortNew=5,
	                    TSdownloadArticle=6, TSsendArticle=7, TSsendMail=8,
	                    TSprogressUpdate=9 };

	KNProtocolClient(int NfdPipeIn, int NfdPipeOut, QObject *parent=0, const char *name=0);
	~KNProtocolClient();
	
	static void* startThread(void* pseudoThis);

  void insertJob(KNJobData *newJob);
  void removeJob();

  int getProgressValue() const { return progressValue; };
  int getByteCount() const { return byteCount; };       // bytes in&out for the current connection
	
protected:

  void waitForWork();         // main loop, maintains connection and waits for next job
  virtual void processJob();  // examines the job and calls the suitable handling method

  virtual bool openConnection();   // connect, handshake and authorization
  bool isConnected()   { return (tcpSocket!=-1); };
  virtual void closeConnection();  // sends QUIT-command and closes the socket

  virtual bool sendCommand(const QCString &cmd, int &rep);  // sends a command (one line), return code is written to rep
  bool sendCommandWCheck(const QCString &cmd, int rep);  // checks return code and calls handleErrors() if necessary
  bool sendMsg(const DwString &msg);     // sends a message (multiple lines)

  bool getNextLine();                 // reads next complete line of input
  char* getCurrentLine()  { return thisLine; };  // returns pointer to current line of input	
  bool getMsg(QStrList &msg);         // receives a message (multiple lines)
  bool getNextResponse(int &rep);     // reads next line and returns the response code
  bool checkNextResponse(int rep);  	// checks return code and calls handleErrors() if necessary

  virtual void handleErrors();         // interprets error code, generates error message and closes the connection

  void sendSignal(threadSignal s);

  KNJobData *job;
  KNServerInfo account;
  QString errorPrefix;     // handleErrors() adds this string to the error message
  int progressValue,predictedLines;

private:

  bool waitForRead();          // waits until socket is readable
  bool waitForWrite();         // waits until socket is writeable
  bool conRawIP(in_addr* ip);  // used by openConnection()
  void closeSocket();
  bool sendStr(const QCString &str);   // sends str to the server
  void clearPipe();            // removes start/stop signal

  char *input;
  char *thisLine, *nextLine, *inputEnd;
  const unsigned int inputSize;
  int fdPipeIn,fdPipeOut;      //IPC-Pipes to/from async thread
  int tcpSocket;
  int byteCount,doneLines;     // bytes in&out for the current connection
};

#endif
