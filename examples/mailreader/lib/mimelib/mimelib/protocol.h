//=============================================================================
// File:       proto_un.h
// Contents:   Declarations for DwClientProtocol
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

#ifndef DW_PROTOCOL_H
#define DW_PROTOCOL_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#ifndef DW_CONFIG_H
#include <mimelib/config.h>
#endif

#ifndef DW_STRING_H
#include <mimelib/string.h>
#endif


class DwObserver {
public:
	virtual ~DwObserver(){}
    virtual void Notify()=0;
};


//=============================================================================
//+ Name DwProtocolClient -- Base class for all protocol clients
//+ Description
//. {\tt DwProtocolClient} is the base class for other classes that implement
//. specific protocols, such as SMTP, POP, and NNTP.  {\tt DwProtocolClient}
//. serves two purposes.  First, It combines operations common to all its
//. derived classes, such as opening a TCP connection to the server.  Second,
//. it provides a platform-independent interface to the network services
//. required by its subclasses.
//.
//. There are two separate implementations of {\tt DwProtocolClient}: one for
//. Berkeley sockets under UNIX, and one for Winsock under Win32.  The
//. interface is the same for both implementations, thus providing platform
//. independence.
//.
//. There are two platform-specific details that you should be aware of.
//. First, if you are writing a UNIX program, you should be sure to handle
//. the SIGPIPE signal.  This signal is raised when a program tries to write
//. to a TCP connection that was shutdown by the remote host.  The default
//. action for this signal is to terminate the program.  To prevent this
//. from happening in your program, you should either catch the signal or
//. tell the operating system to ignore it.  Second, if you are writing a
//. Win32 application for Windows NT or Windows95, you should be aware of
//. the fact that the constructor calls the Winsock function
//. {\tt WSAStartup()} to initialize the Winsock DLL.  (The destructor
//. calls {\tt WSACleanup()}.)  Because it is possible for {\tt WSAStartup()}
//. to fail, it is also possible that the constructor may fail.  To verify
//. that the constructor has succeeded, call the member function
//. {\tt LastError()} and check that it returns zero.
//.
//. To open a connection to a server, call {\tt Open()} with the server name
//. and TCP port number as arguments.  {\tt Open()} is declared virtual;
//. derived classes may override this member function.  {\tt Open()} may fail,
//. so you should check the return value to verify that it succeeded.  To close
//. the connection, call {\tt Close()}.  To check if a connection is open,
//. call {\tt IsOpen()}.  {\tt IsOpen()} returns a value that indicates whether
//. or not a call to {\tt Open()} was successful; it will not detect failure
//. in the network or a close operation by the remote host.
//.
//. {\tt DwProtocolClient} sets a timeout on receive operations on the TCP
//. connection.  The default value of the timeout period is 90 seconds.  To
//. change the default value, call {\tt SetReceiveTimeout()} and pass the
//. new value as an argument.
//.
//. Whenever {\tt DwProtocolClient} cannot complete an operation, it is because
//. an error has occurred.  Most member functions indicate that an error has
//. occurred via their return values.  For most member functions, a return
//. value of -1 indicates an error.  To get the specific error that has
//. occurred, call {\tt LastError()}, which returns either the system error
//. code or a MIME++ defined error code.  To get a text string that describes
//. the error, call {\tt LastErrorStr()}.
//.
//. Some errors are also considered "failures."  A failure occurs when an
//. operation cannot be completed because of conditions external to the
//. program.  For example, a failure occurs when the network is down or
//. when an application's user enters bad input.  Errors that occur because
//. of programmer error are not considered failures.  If an error occurs,
//. you should call {\tt LastError()} to determine the error, but you should
//. also call {\tt LastFailure()} to determine if a failure occurred.  In
//. interactive applications, failures should always be reported to the
//. application's user.  To get a text string that describes a failure,
//. call {\tt LastFailureStr()}.
//.
//. It is possible to translate the error and failure message strings to a
//. language other than English.  To do this, you may override the virtual
//. function {\tt HandleError()}.
//=============================================================================

//+ Noentry mFailureCode mFailureStr mErrorCode mErrorStr mLastCommand
//+ Noentry mIsDllOpen mIsOpen mSocket mPort mServerName mReceiveTimeout


class DW_EXPORT DwProtocolClient {

public:

    enum Failure {
        kFailNoFailure      = 0, // No failure
        kFailNoWinsock      = 1, // A usable Winsock DLL could not be found
        kFailNetDown        = 2, // The network is down
        kFailHostNotFound   = 3, // The server was not found
        kFailConnReset      = 4, // The connection was reset
        kFailNetUnreachable = 5, // The network is unreachable
        kFailTimedOut       = 6, // Timed out while waiting for an operation
                                 // to complete
        kFailConnDropped    = 7,
        kFailConnRefused    = 8,
        kFailNoResources    = 9
    };
    //. Enumerated values for failures.

    enum Error {
        kErrNoError = 0,
        kErrUnknownError = 0x4000,
        kErrBadParameter = 0x4001,
        kErrBadUsage     = 0x4002,
        kErrNoWinsock    = 0x4003,  // Win32
        kErrHostNotFound = 0x5000,  // UNIX
        kErrTryAgain     = 0x5001,  // UNIX
        kErrNoRecovery   = 0x5002,  // UNIX
        kErrNoData       = 0x5003,  // UNIX
        kErrNoAddress    = 0x5004   // UNIX
    };
    //. MIME++-defined error codes.

protected:

    DwProtocolClient();
    //. Initializes the {\tt DwProtocolClient} object.
    //. In a Win32 environment, this constructor calls {\tt WSAStartup()}
    //. to initialize the Winsock DLL. To verify that the DLL was initialized
    //. successfully, call the member function {\tt LastError()} and verify
    //. that it returns zero.

public:

    virtual ~DwProtocolClient();
    //. Frees the resources used by this object.
    //. In a Win32 environment, the destructor calls {\tt WSACleanup()}.

    virtual int Open(const char* aServer, DwUint16 aPort);
    //. Opens a TCP connection to the server {\tt aServer} at port {\tt aPort}.
    //. {\tt aServer} may be either a host name, such as "smtp.acme.com" or an
    //. IP number in dotted decimal format, such as "147.81.64.59".  If the
    //. connection attempt succeeds, {\tt Open()} returns 0; othewise, it
    //. returns -1.  To determine what error occurred when the connection
    //. attempt fails, call the member function {\tt LastError()}. To
    //. determine if a failure also occurred, call the member function
    //. {\tt LastFailure()}.

    DwBool IsOpen() const;
    //. Returns true value if a connection to the server is open.
    //. {\tt IsOpen()} will return a true value if a call to {\tt Open()} was
    //. successful;  it will not detect failure in the network or a close
    //. operation by the remote host.

    int Close();
    //. Closes the connection to the server.  Returns 0 if successful, or
    //. returns -1 if unsuccessful.

    int SetReceiveTimeout(int aSecs);
    //. Changes the default timeout for receive operations on the socket to
    //. {\tt aSecs} seconds.
    //. The default value is 90 seconds.

    int LastCommand() const;
    //. Returns an enumerated value indicating the last command sent to
    //. the server. Enumerated values are defined in subclasses of
    //. {\tt DwProtocolClient}.

    int LastFailure() const;
    //. Returns an enumerated value indicating what failure last occurred.

    const char* LastFailureStr() const;
    //. Returns a failure message string associated with the failure code
    //. returned by {\tt LastFailure()}.

    int LastError() const;
    //. Returns an error code for the last error that occurred.  Normally, the
    //. error code returned is an error code returned by a system call;
    //. {\tt DwProtocolClient} does no translation of error codes returned
    //. by system calls.  In some cases, an error code defined by MIME++ may
    //. returned to indicate improper use of the {\tt DwProtocolClient} class.

    const char* LastErrorStr() const;
    //. Returns an error message string associated with the error code returned
    //. by {\tt LastError()}.

protected:

    enum {
        kWSAStartup=1,  // Win32
        kgethostbyname,
        ksocket,
        ksetsockopt,
        kconnect,
        ksend,
        krecv,
        kclose,         // UNIX
        kclosesocket,   // Win32
        kselect
    };
    // Enumerated values that indicate the system call that detected
    // an error

    DwBool      mIsDllOpen;
    DwBool      mIsOpen;
    int         mSocket;
    DwUint16    mPort;
    char*       mServerName;
    int         mReceiveTimeout;
    int         mLastCommand;
    int         mFailureCode;
    const char* mFailureStr;
    int         mErrorCode;
    const char* mErrorStr;

    virtual void HandleError(int aErrorCode, int aSystemCall);
    //. Interprets error codes.  {\tt aErrorCode} is an error code,
    //. which may be a system error code, or an error code defined by
    //. {\tt DwProtocolClient}.  {\tt aSystemCall} is an enumerated value
    //. defined by {\tt DwProtocolClient} that indicates the last system
    //. call made, which should be the system call that set the error code.
    //. {\tt HandleError()} sets values for {\tt mErrorStr},
    //. {\tt mFailureCode}, and {\tt mFailureStr}.

    int PSend(const char* aBuf, int aBufLen);
    //. Sends {\tt aBufLen} characters from the buffer {\tt aBuf}.  Returns
    //. the number of characters sent.  If the number of characters sent
    //. is less than the number of characters specified in {\tt aBufLen},
    //. the caller should call {\tt LastError()} to determine what, if any,
    //. error occurred.  To determine if a failure also occurred, call the
    //. member function {\tt LastFailure()}.

    int PReceive(char* aBuf, int aBufSize);
    //. Receives up to {\tt aBufSize} characters into the buffer {\tt aBuf}.
    //. Returns the number of characters received.  If zero is returned, the
    //. caller should call the member function {\tt LastError()} to determine
    //. what, if any, error occurred. To determine if a failure also occurred,
    //. call the member function {\tt LastFailure()}.

};

#endif
