//=============================================================================
// File:       nntp.h
// Contents:   Declarations for DwNntpClient
// Maintainer: Doug Sauder <dwsauder@fwb.gulf.net>
// WWW:        http://www.fwb.gulf.net/~dwsauder/mimepp.html
//
// Copyright (c) 1996, 1997 Douglas W. Sauder
// All rights reserved.
// 
// IN NO EVENT SHALL DOUGLAS W. SAUDER BE LIABLE TO ANY PARTY FOR DIRECT,
// INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT OF
// THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF DOUGLAS W. SAUDER
// HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// DOUGLAS W. SAUDER SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT
// NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
// PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS ON AN "AS IS"
// BASIS, AND DOUGLAS W. SAUDER HAS NO OBLIGATION TO PROVIDE MAINTENANCE,
// SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
//
//=============================================================================

#ifndef DW_NNTP_H
#define DW_NNTP_H

#include <stdio.h>

#ifndef DW_CONFIG_H
#include <mimelib/config.h>
#endif

#ifndef DW_PROTOCOL_H
#include <mimelib/protocol.h>
#endif

#ifndef DW_STRING_H
#include <mimelib/string.h>
#endif


//=============================================================================
//+ Name DwNntpClient -- Class for handling the client side of an NNTP session
//+ Description
//. {\tt DwNntpClient} is a class that handles the client side of an NNTP
//. session.  Specifically, {\tt DwNntpClient} provides facilities for
//. opening a connection to an NNTP server, sending commands and data to
//. the server, receiving responses and data from the server, and closing
//. the connection.  The protocol implemented is the Network News Transport
//. Protocol, as specified in RFC-977.
//.
//. {\tt DwNntpClient} is derived from {\tt DwProtocolClient}.  For information
//. about inherited member functions, especially member functions for detecting
//. failures or errors, see the man page for {\tt DwProtocolClient}.
//.
//. In an NNTP session, the client sends commands to the server and receives
//. responses from the server.  A client command consists of a command word
//. and zero or more argument words.  A server response consists of a status
//. line and possibly some additional lines of text.  The status line consists
//. of a three-digit numeric reply code followed by additional information.
//. The reply code indicates a success or failure condition.  In some cases,
//. the server sends lines of text immediately after the status line.
//. {\tt DwNntpClient} provides facilities for you to send commands to the
//. server and receive responses from the server.
//.
//. {\tt DwNntpClient} has only a default constructor. On Win32 platforms,
//. it is possible for the constructor to fail. (It calls WSAStartup().)
//. You should verify that the constructor succeeded by calling the inherited
//. member function {\tt DwProtocolClient::LastError()} and checking for a zero
//. return value.
//.
//. To open a connection to the server, call the member function {\tt Open()}
//. with the name of the server as an argument. {\tt Open()} accepts an
//. optional argument that specifies the TCP port that the server listens to.
//. The default port is the standard NNTP port (119). {\tt Open()} may fail,
//. so you should check the return value to verify that it succeeded. To
//. close the connection, call the inherited member function
//. {\tt DwProtocolClient::Close()}. To check if a connection is open, call
//. the inherited member function {\tt DwProtocolClient::IsOpen()}.
//. {\tt IsOpen()} returns a boolean value that indicates whether or not
//. a call to {\tt Open()} was successful; it will not detect failure in
//. the network or a close operation by the remote host.
//.
//. For each NNTP command, {\tt DwNntpClient} has a member function that sends
//. that command and receives the server's response. If the command takes any
//. arguments, then those arguments are passed as function arguments to the
//. command function. The command functions return the numeric value of the
//. three-digit reply code returned by the server. Your program must check
//. the reply code to determine whether or not the command was accepted and
//. performed by the server.
//. In some cases, because of a communications error or some other error,
//. it is not possible for the command function to send the command or
//. receive the response.  When this happens, the command function will
//. return 0.  You can determine the precise error or failure by calling
//. the inherited member functions {\tt DwProtocolClient::LastError()} or
//. {\tt DwProtocolClient::LastFailure()}.
//.
//. After each command is sent, {\tt DwNntpClient} receives the server's
//. response and remembers it. The member function {\tt ReplyCode()}
//. returns the numeric value of the reply code received in response to
//. the last command. {\tt StatusResponse()} returns the entire status
//. response from the server, including the reply code. If no status
//. response is received, possibly because of a communications error
//. or failure, {\tt ReplyCode()} returns zero and {\tt StatusResponse()}
//. returns an empty string.
//.
//. The server sends a status response, including a reply code, for all
//. all NNTP commands. For some commands, such as when the client requests
//. an article body, the server sends a multi-line text response immediately
//. following the status response. Multi-line text responses
//. can be received in either of two ways. The simplest way is to call the
//. member function {\tt TextResponse()} after a command completes
//. successfully. This simple method works fine for non-interactive
//. applications. It can be a problem in interactive applications, however,
//. because there is no data to display to a user until the entire text
//. response is retrieved. An alternative method allows your program to
//. retrieve the text response one line at a time as it is received.
//. To use this method, you must define a subclass of {\tt DwObserver}
//. and assign an object of that class to the {\tt DwNntpClient} object
//. using the member function {\tt SetObserver()}. {\tt DwObserver} is an
//. abstract class, declared in protocol.h, that has just one pure virtual
//. member function {\tt Notify()}. After each line of the text response
//. is received, {\tt DwNntpClient} will call the {\tt Notify()} member
//. function of its assigned {\tt DwObserver} object. Each invocation of
//. {\tt Notify()} should call the {\tt DwNntpClient} member function
//. {\tt TextResponse()} to retrieve the next line of the text response.
//. Note that you cannot use both of these methods at the same time: if
//. an observer is assigned, {\tt TextResponse()} returns only the last
//. line received, not the entire multi-line text response.
//.
//. Certain NNTP commands, such as the POST command, require the NNTP client
//. to send multiple lines of text to the server. To perform this bulk data
//. transfer, {\tt DwNntpClient} provides the member function
//. {\tt SendData()}.  In the current implementation, {\tt SendData()} does
//. not convert end of line characters, so it is your responsibility to
//. convert the end of line characters to CR LF, if necessary.  (You may
//. use the utility function {\tt DwToCrLfEol()} to do the conversion.)
//. {\tt SendData()} will perform the character stuffing to protect '.' at
//. the beginning of a line, and it will append the final [CR LF] '.' CR LF.
//. It is possible to divide data and make multiple calls to {\tt SendData()};
//. however, if you do so, please note the following paragraph.
//.
//. Note: Because of a feature (some might say bug) in the current
//. implementation, {\tt SendData()} will not detect a '.' at the beginning
//. of a line if the CR LF '.' sequence is split between two calls to
//. {\tt SendData()}.  This problem will probably be resolved in a future
//. version, but be aware that such a change will require a change in
//. {\tt DwNntpClient}'s interface.
//=============================================================================

//+ Noentry ~DwNntpClient


class DW_EXPORT DwNntpClient : public DwProtocolClient {

friend class NNTP;
friend class NNTPObserver;
    
public:

    enum {
        kCmdNoCommand=0,
        kCmdArticle,
        kCmdBody,
        kCmdHead,
        kCmdStat,
        kCmdGroup,
        kCmdHelp,
        kCmdIhave,
        kCmdLast,
        kCmdList,
        kCmdNewgroups,
        kCmdNewnews,
        kCmdNext,
        kCmdPost,
        kCmdQuit,
        kCmdSlave
    };

    DwNntpClient();
    //. Initializes the {\tt DwNntpClient} object.
    //. It is possible for the constructor to fail.  To verify that the
    //. constructor succeeded, call the member function {\tt LastError()}
    //. and check that it returns zero.  (In the Win32 implementation, the
    //. constructor calls the Winsock function {\tt WSAStartup()}, which
    //. may fail.)

    virtual ~DwNntpClient();

    virtual int Open(const char* aServer, DwUint16 aPort=119);
    //. Opens a TCP connection to the server {\tt aServer} at port {\tt aPort}.
    //. {\tt aServer} may be either a host name, such as "news.acme.com" or
    //. an IP number in dotted decimal format, such as "147.81.64.60".  The
    //. default value for {\tt aPort} is 119, the well-known port for NNTP
    //. assigned by the Internet Assigned Numbers Authority (IANA).
    //.
    //. If the connection attempt succeeds, the server sends a response.
    //. {\tt Open()} returns the server's numeric reply code.  The full
    //. response from the server can be retrieved by calling
    //. {\tt StatusResponse()}.
    //.
    //. If the connection attempt fails, {\tt Open()} returns 0.  To determine
    //. what error occurred when a connection attempt fails, call the inherited
    //. member function {\tt DwProtocolClient::LastError()}.  To determine if
    //. a failure also occurred, call the inherited member function
    //. {\tt DwProtocolClient::LastFailure()}.

    DwObserver* SetObserver(DwObserver* aObserver);
    //. Sets the observer object that interacts with the {\tt DwNntpClient}
    //. object to retrieve a multi-line text response.  If an observer is set,
    //. {\tt DwNntpClient} will call the observer's {\tt Notify()} method
    //. after each line of the text response is received.  To remove
    //. an observer, call {\tt SetObserver()} with a NULL argument.
    //. {\tt SetObserver()} returns the previously set observer, or NULL if
    //. no observer was previously set.

    int ReplyCode() const;
    //. Returns the numeric value of the three-digit reply code received
    //. from the server in response to the last client command.  If no
    //. response was received, {\tt ReplyCode()} returns zero.

    const DwString& StatusResponse() const;
    //. Returns the entire status response last received from the server.
    //. If no response was received, perhaps because of a communications
    //. failure, {\tt StatusResponse()} returns an empty string.

    const DwString& TextResponse() const;
    //. If no observer is set for this object, {\tt TextResponse()} returns
    //. a string that comprises the entire sequence of lines received from
    //. the server.  Otherwise, if an observer {\tt is} set for this object,
    //. {\tt TextResponse()} returns only the most recent line received.

    int Article(int aNumber=(-1));
    int Article(const char* aMsgid);
    //. Sends the NNTP ARTICLE command and returns the reply code received
    //. from the server. If no response is received, the function returns
    //. zero.
    //. The optional argument {\tt aNumber} specifies the number of an
    //. article to retrieve. If {\tt Article()} is called with the default
    //. argument, the ARTICLE command is sent to the server with no argument.
    //. {\tt aMsgId} specifies the message id of an article to retrieve.

    int Body(int aNumber=(-1));
    int Body(const char* aMsgid);
    //. Sends the NNTP BODY command and returns the reply code received
    //. from the server. If no response is received, the function returns
    //. zero.
    //. The optional argument {\tt aNumber} specifies the number of an
    //. article whose body should be retrieved. If {\tt Body()} is called
    //. with the default argument, the BODY command is sent to the server
    //. with no argument. {\tt aMsgId} specifies the message id of the
    //. article to access.

    int Head(int aNumber=(-1));
    int Head(const char* aMsgid);
    //. Sends the NNTP HEAD command and returns the reply code received
    //. from the server. If no response is received, the function returns
    //. zero.
    //. The optional argument {\tt aNumber} specifies the number of an
    //. article whose header lines should be retrieved. If {\tt Head()}
    //. is called with the default argument, the HEAD command is sent to
    //. the server with no argument. {\tt aMsgId} specifies the message id
    //. of the article to access.

    int Stat(int aNumber=(-1));
    int Stat(const char* aMsgid);
    //. Sends the NNTP STAT command and returns the reply code received
    //. from the server. If no response is received, the function returns
    //. zero.
    //. The optional argument {\tt aNumber} specifies the number of an
    //. article to access. If {\tt Stat()} is called with the default
    //. argument, the STAT command is sent to the server with no argument.
    //. {\tt aMsgId} specifies the message id of the article to access.

    int Group(const char* aNewsgroupName);
    //. Sends the NNTP GROUP command and returns the reply code received from
    //. the server.  The argument {\tt aNewsgroupName} specifies the newgroup
    //. to be selected. If no response is received, the function returns zero.

    int Help();
    //. Sends the NNTP HELP command and returns the reply code received from
    //. the server.  If no response is received, the function returns zero.

    int Ihave(const char* aMsgId);
    //. Sends the NNTP IHAVE command and returns the reply code received from
    //. the server.  {\tt aMsgId} specifies the message id of the article
    //. to be sent.  If no response is received, the function returns zero.

    int Last();
    //. Sends the NNTP LAST command and returns the reply code received from
    //. the server.  If no response is received, the function returns zero.

    int List();
    //. Sends the NNTP LIST command and returns the reply code received from
    //. the server.  If no response is received, the function returns zero.

    int Newgroups(const char* aDate, const char* aTime,
        DwBool aIsGmt=DwFalse, const char* aDistributions=0);
    //. Sends the NNTP NEWGROUPS command and returns the reply code received
    //. from the server.  If no response is received, the function returns
    //. zero.
    //. {\tt aDate} is the date in the form YYMMDD, where YY is the two
    //. digit year, MM is the month, and DD is the day of the month.
    //. {\tt aTime} is the time in the form HHMMSS, where HH is hours,
    //. MM is minutes, and SS is seconds. If {\tt aIsGmt} is true,
    //. the optional GMT argument will be sent. {\tt aDistributions}
    //. specifies the optional list of distribution groups.

    int Newnews(const char* aNewsgroups, const char* aDate,
        const char* aTime, DwBool aIsGmt=DwFalse, const char* aDistribution=0);
    //. Sends the NNTP NEWNEWS command and returns the reply code received
    //. from the server.  If no response is received, the function returns
    //. zero.
    //. {\tt aNewsgroups} is the newsgroups argument for the command.
    //. {\tt aDate} is the date in the form YYMMDD, where YY is the two
    //. digit year, MM is the month, and DD is the day of the month.
    //. {\tt aTime} is the time in the form HHMMSS, where HH is hours,
    //. MM is minutes, and SS is seconds. If {\tt aIsGmt} is true,
    //. the optional GMT argument will be sent. {\tt aDistributions}
    //. specifies the optional list of distribution groups.

    int Next();
    //. Sends the NNTP NEXT command and returns the reply code received from
    //. the server.  If no response is received, perhaps because of an error,
    //. the function returns zero.

    int Post();
    //. Sends the NNTP POST command and returns the reply code received from
    //. the server.  If no response is received, perhaps because of an error,
    //. the function returns zero.

    int Quit();
    //. Sends the NNTP QUIT command and returns the reply code received from
    //. the server.  If no response is received, perhaps because of an error,
    //. the function returns zero.

    int Slave();
    //. Sends the NNTP SLAVE command and returns the reply code received from
    //. the server.  If no response is received, perhaps because of an error,
    //. the function returns zero.

    int SendData(const DwString& aStr);
    int SendData(const char* aBuf, int aBufLen);
    //. Sends bulk data to the server and returns the reply code received.
    //. A bulk data transfer follows a POST or IHAVE command and is used to
    //. send a complete article to the server.
    //.
    //. In the current implementation, {\tt SendData()} does not convert end
    //. of line characters, so it is your responsibility to convert the end
    //. of line characters to CR LF, if necessary.  (You may use the utility
    //. function {\tt DwToCrLfEol()} to do the conversion.)  {\tt SendData()}
    //. will perform the character stuffing to protect '.' at the beginning of
    //. a line, and it will append the final [CR LF] '.' CR LF.  It is possible
    //. to divide the data and make multiple calls to {\tt SendData()}; however,
    //. this may cause problems in the current implementation if a CR LF '.'
    //. sequence is split between calls.

private:

    char*       mSendBuffer;
    char*       mRecvBuffer;
    int         mLastChar;
    int         mLastLastChar;
    int         mNumRecvBufferChars;
    int         mRecvBufferPos;
    int         mReplyCode;
    DwString    mStatusResponse;
    DwString    mTextResponse;
    DwObserver* mObserver;

    virtual int PGetLine(char** aPtr, int* aLen);
    virtual void PGetStatusResponse();
    virtual void PGetTextResponse();

};

#endif
