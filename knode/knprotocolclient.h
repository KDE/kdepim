/*
    knprotocolclient.h

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

#ifndef KNPROTOCOLCLIENT_H
#define KNPROTOCOLCLIENT_H

#include <qdatetime.h>
#include <qthread.h>
#include <qcstring.h>

#include <knserverinfo.h>

class QStrList;
class KNJobData;
struct in_addr;


class KNProtocolClient : public QThread  {

  public:
    enum threadSignal { TSworkDone=0, TSjobStarted=1, TSconnect=2, TSloadGrouplist=3,
                        TSwriteGrouplist=4, TSdownloadGrouplist=5, TSdownloadNew=6,
                        TSsortNew=7, TSdownloadArticle=8, TSsendArticle=9, TSsendMail=10,
                        TSprogressUpdate=11, TSdownloadDesc=12, TSdownloadNewGroups=13 };

    KNProtocolClient(int NfdPipeIn, int NfdPipeOut);
    ~KNProtocolClient();
  
    virtual void run();

    void insertJob(KNJobData *newJob);
    void removeJob();

    void updatePercentage(int percent);

    int getProgressValue() const { return progressValue; };
    /** bytes in&out for the current connection */
    int getByteCount() const { return byteCount; };
    bool isInByteCountMode() const { return byteCountMode; };

    void terminateClient() { mTerminate = true; }
  protected:

    /** main loop, maintains connection and waits for next job */
    void waitForWork();
    /** examines the job and calls the suitable handling method */
    virtual void processJob();

    /** connect, handshake and authorization */
    virtual bool openConnection();
    bool isConnected()   { return (tcpSocket!=-1); };
    /** sends QUIT-command and closes the socket */
    virtual void closeConnection();

    /** sends a command (one line), return code is written to rep */
    virtual bool sendCommand(const QCString &cmd, int &rep);
    /** checks return code and calls handleErrors() if necessary */
    bool sendCommandWCheck(const QCString &cmd, int rep);
    /** sends a message (multiple lines) */
    bool sendMsg(const QCString &msg);

    /** reads next complete line of input */
    bool getNextLine();
    /** returns pointer to current line of input */
    char* getCurrentLine()  { return thisLine; };
    /** receives a message (multiple lines) */
    bool getMsg(QStrList &msg);
    /** reads next line and returns the response code */
    bool getNextResponse(int &rep);
    /** checks return code and calls handleErrors() if necessary */
    bool checkNextResponse(int rep);

    /** interprets error code, generates error message and closes the connection */
    virtual void handleErrors();

    void sendSignal(threadSignal s);

    KNJobData *job;
    KNServerInfo account;
    /** handleErrors() adds this string to the error message */
    QString errorPrefix;
    int progressValue, predictedLines, doneLines;
    bool byteCountMode;

  private:
    /** waits until socket is readable */
    bool waitForRead();
    /** waits until socket is writeable */
    bool waitForWrite();
    void closeSocket();
    /** sends str to the server */
    bool sendStr(const QCString &str);
    /** removes start/stop signal */
    void clearPipe();

    char *input;
    char *thisLine, *nextLine, *inputEnd;
    unsigned int inputSize;
    /** IPC-Pipes to/from async thread */
    int fdPipeIn,fdPipeOut;
    int tcpSocket;
    /** bytes in&out for the current connection */
    int byteCount;
    QTime timer;
    bool mTerminate;

};

#endif
