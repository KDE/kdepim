//=============================================================================
// File:       pop.h
// Contents:   Declarations for DwPopClient
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

#ifndef DW_POP_H
#define DW_POP_H

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
//+ Name DwPopClient -- Class for handling the client side of a POP session
//+ Description
//. {\tt DwPopClient} is a class that handles the client side of a POP
//. session.  Specifically, {\tt DwPopClient} provides facilities for
//. opening a connection to a POP server, sending commands to the server,
//. receiving responses from the server, and closing the connection. The
//. protocol implemented is the Post Office Protocol version 3, as specified
//. in RFC-1939.
//.
//. {\tt DwPopClient} is derived from {\tt DwProtocolClient}. For information
//. about inherited member functions, especially member functions for detecting
//. failures or errors, see the man page for {\tt DwProtocolClient}.
//.
//. In a POP session, the client sends commands to the server and receives
//. responses from the server.  A client command consists of a command word
//. and zero or more argument words.  A server response consists of a single
//. line status response, which may be followed immediately by a multi-line
//. response. The first word of the status response is either +OK or -ERR,
//. indicating the success or failure of the command. The status line may
//. also contain other information requested by the client.
//.
//. {\tt DwPopClient} has only a default constructor. On Win32 platforms,
//. it is possible for the constructor to fail. (It calls WSAStartup().)
//. You should verify that the constructor succeeded by calling the inherited
//. member function {\tt DwProtocolClient::LastError()} and checking for a zero
//. return value.
//.
//. To open a connection to the server, call the member function {\tt Open()}
//. with the name of the server as an argument. {\tt Open()} accepts an
//. optional argument that specifies the TCP port that the server listens to.
//. The default port is the standard POP port (110). {\tt Open()} may fail,
//. so you should check the return value to verify that it succeeded. To
//. close the connection, call the inherited member function
//. {\tt DwProtocolClient::Close()}. To check if a connection is open, call
//. the inherited member function {\tt DwProtocolClient::IsOpen()}.
//. {\tt IsOpen()} returns a boolean value that indicates whether or not
//. a call to {\tt Open()} was successful; it will not detect failure in
//. the network or a close operation by the remote host.
//.
//. For each POP command, {\tt DwPopClient} has a member function that sends
//. that command and receives the server's response. If the command takes any
//. arguments, then those arguments are passed as function arguments to the
//. command function. The command functions return the first character
//. of the server's response, which will be '+' if the command succeeded
//. or '-' if the command failed.
//. In some cases, because of a communications error or some other error,
//. it is not possible for the command function to send the command or
//. receive the response.  When this happens, the command function will
//. return 0.  You can determine the precise error or failure by calling
//. the inherited member functions {\tt DwProtocolClient::LastError()} or
//. {\tt DwProtocolClient::LastFailure()}.
//.
//. After each command is sent, {\tt DwPopClient} receives the server's
//. response and remembers it. The member function {\tt StatusCode()}
//. returns the first character of the server's status response; it will be
//. '+' or '-', indicating success or failure, or zero if no response was
//. received from the server. {\tt SingleLineResponse()} returns the entire
//. single line status response from the server, including the initial
//. "+OK" or "-ERR" status word.
//.
//. The server sends a single-line response, including a status code, for all
//. POP commands. For some commands, such as when the client requests a
//. mail message, the server sends a multi-line text response immediately
//. following the single-line status response. Multi-line text responses
//. can be received in either of two ways. The simplest way is to call
//. the member function {\tt MultiLineResponse()} after a command completes
//. successfully. This simple method works fine for non-interactive
//. applications. It can be a problem in interactive applications, however,
//. because there is no data to display to a user until the entire multi-line
//. response is retrieved. An alternative method allows your program to
//. retrieve the multi-line response one line at a time as it is received.
//. To use this method, you must define a subclass of {\tt DwObserver}
//. and assign an object of that class to the {\tt DwPopClient} object
//. using the member function {\tt SetObserver()}. {\tt DwObserver} is an
//. abstract class, declared in protocol.h, that has just one pure virtual
//. member function {\tt Notify()}. After each line of the multi-line response
//. is received, {\tt DwPopClient} will call the {\tt Notify()} member
//. function of its assigned {\tt DwObserver} object. Each invocation of
//. {\tt Notify()} should call the {\tt DwPopClient} member function
//. {\tt MultiLineResponse()} to retrieve the next line of the text response.
//. Note that you cannot use both of these methods at the same time: if
//. an observer is assigned, {\tt MultiLineResponse()} returns only the last
//. line received, not the entire multi-line response.
//=============================================================================

//+ Noentry ~DwPopClient


class DW_EXPORT DwPopClient : public DwProtocolClient {

public:

    enum {
        kCmdNoCommand=0,
        kCmdUser,
        kCmdPass,
        kCmdQuit,
        kCmdStat,
        kCmdList,
        kCmdRetr,
        kCmdDele,
        kCmdNoop,
        kCmdRset,
        kCmdApop,
        kCmdTop,
        kCmdUidl
    };

    DwPopClient();
    //. Initializes the {\tt DwPopClient} object.
    //. It is possible for the constructor to fail.  To verify that the
    //. constructor succeeded, call the member function {\tt LastError()}
    //. and check that it returns zero.  (In the Win32 implementation, the
    //. constructor calls the Winsock function {\tt WSAStartup()}, which
    //. may fail.)

    virtual ~DwPopClient();

    virtual int Open(const char* aServer, DwUint16 aPort=110);
    //. Opens a TCP connection to the server {\tt aServer} at port {\tt aPort}.
    //. {\tt aServer} may be either a host name, such as "news.acme.com" or
    //. an IP number in dotted decimal format, such as "147.81.64.60".  The
    //. default value for {\tt aPort} is 110, the well-known port for POP3
    //. assigned by the Internet Assigned Numbers Authority (IANA).
    //.
    //. If the connection attempt succeeds, the server sends a response.
    //. {\tt Open()} returns the server's status code ('+' or '-'). The full
    //. response from the server can be retrieved by calling
    //. {\tt SingleLineResponse()}.
    //.
    //. If the connection attempt fails, {\tt Open()} returns 0.  To determine
    //. what error occurred when a connection attempt fails, call the inherited
    //. member function {\tt DwProtocolClient::LastError()}.  To determine if
    //. a failure also occurred, call the inherited member function
    //. {\tt DwProtocolClient::LastFailure()}.

    DwObserver* SetObserver(DwObserver* aObserver);
    //. Sets the observer object that interacts with the {\tt DwPopClient}
    //. object to retrieve a multi-line response. If an observer is set,
    //. {\tt DwPopClient} will call the observer's {\tt Notify()} method
    //. after each line of the multi-line response is received.  To remove
    //. an observer, call {\tt SetObserver()} with a NULL argument.
    //. {\tt SetObserver()} returns the previously set observer, or NULL if
    //. no observer was previously set.

    int StatusCode() const;
    //. Returns the status code received from the server in response to the
    //. last client command. The status codes in POP3 are '+', indicating
    //. success, and '-', indicating failure. If no response was received,
    //. {\tt StatusCode()} returns zero.

    const DwString& SingleLineResponse() const;
    //. Returns the single line status response last received from the server.
    //. If no response was received, perhaps because of a communications
    //. failure, {\tt SingleLineResponse()} returns an empty string.

    const DwString& MultiLineResponse() const;
    //. If no observer is set for this object, {\tt MultiLineResponse()}
    //. returns a string that comprises the entire sequence of lines
    //. received from the server.  Otherwise, if an observer {\it is} set
    //. for this object, {\tt MultiLineResponse()} returns only the most
    //. recent line received.

    int User(const char* aName);
    //. Sends the USER command and returns the status code received from
    //. the server.  If no response is received, the function returns zero.
    //. {\tt aName} is the name of the user, which is sent in the command.

    int Pass(const char* aPasswd);
    //. Sends the PASS command and returns the status code received from
    //. the server.  If no response is received, the function returns zero.
    //. {\tt aPasswd} is the password, which is sent in the command.

    int Quit();
    //. Sends the QUIT command and returns the status code received from
    //. the server.  If no response is received, the function returns zero.

    int Stat();
    //. Sends the STAT command and returns the status code received from
    //. the server.  If no response is received, the function returns zero.

    int List();
    int List(int aMsg);
    //. Sends the LIST command, with or without a message number, and
    //. returns the status code received from the server.  If no response
    //. is received, the function returns zero.

    int Retr(int aMsg);
    //. Sends the RETR command and returns the status code received from
    //. the server.  If no response is received, the function returns zero.
    //. {\tt aMsg} is the message number, which is sent in the command.

    int Dele(int aMsg);
    //. Sends the DELE command and returns the status code received from
    //. the server.  If no response is received, the function returns zero.
    //. {\tt aMsg} is the message number, which is sent in the command.

    int Noop();
    //. Sends the NOOP command and returns the status code received from
    //. the server.  If no response is received, the function returns zero.

    int Rset();
    //. Sends the RSET command and returns the status code received from
    //. the server.  If no response is received, the function returns zero.

    int Apop(const char* aName, const char* aDigest);
    //. Sends the APOP command and returns the status code received from
    //. the server.  If no response is received, the function returns zero.
    //. {\tt aName} is the name of the user, which is sent in the command.
    //. {\tt aDigest} is the digest argument for the command.

    int Top(int aMsg, int aNumLines);
    //. Sends the TOP command and returns the status code received from
    //. the server.  If no response is received, the function returns zero.
    //. {\tt aMsg} is the message number.  {\tt aNumLines} is the number
    //. of lines to send.

    int Uidl();
    int Uidl(int aMsg);
    //. Sends the TOP command, with or without a message number, and
    //. returns the status code received from the server.  If no response
    //. is received, the function returns zero.

    int Last();
    //. Sends the LAST command and returns the status code received from
    //. the server.  If no response is received, the function returns zero.

private:

    char*       mSendBuffer;
    char*       mRecvBuffer;
    int         mNumRecvBufferChars;
    int         mRecvBufferPos;
    int         mStatusCode;
    DwString    mSingleLineResponse;
    DwString    mMultiLineResponse;
    DwObserver* mObserver;

    int PGetLine(char** aPtr, int* aLen);
    // Tries to get one complete line of input from the socket.  On success,
    // the function sets {\tt *aPtr} to point to the beginning of the line in
    // the object's internal buffer, sets {\tt *aLen} to the length of the
    // line, including the CR LF, and returns 0.  On failure, the function
    // returns -1.

    void PGetSingleLineResponse();
    // Gets a single line of input, assigns that line {\tt mSingleLineResponse}, and
    // sets {\tt mStatusCode}.  On failure, clears {\tt mSingleLineResonse}
    // and sets {\tt mStatusCode} to -1.

    void PGetMultiLineResponse();
    // Gets a complete multiline response and assigns it to {\tt mMultiLineResponse},
    // or interacts with the {\tt DwObserver} object to deliver a multiline response
    // one line at a time.
    // If an error occurs, its sets {\tt mStatusCode} to -1.

};

#endif
