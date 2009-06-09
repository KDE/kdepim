//=============================================================================
// File:       proto_un.cpp
// Contents:   Definitions for DwClientProtocol
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

// Comments:
//
// 1. The program should handle the SIGPIPE signal. Ignoring it should be okay.
//
// 2. The recv() and send() system calls are *not* restarted if they are
//    interrupted by a signal. This behavior is necessary if we want to
//    be able to timeout a blocked call.

#define DW_IMPLEMENTATION

#include <mimelib/config.h>
#include <mimelib/protocol.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/time.h>

#if defined(_AIX)
#include <sys/select.h>
#include <sys/types.h>
#include <strings.h>
#endif


#ifdef DW_WIN32
#define ETIMEDOUT WSAETIMEDOUT
#define ENOBUFS WSAENOBUFS
#define EPROTONOSUPPORT WSAEPROTONOSUPPORT
#define ENOTSOCK WSAENOTSOCK
#define EMSGSIZE WSAEMSGSIZE
#define EADDRNOTAVAIL WSAEADDRNOTAVAIL
#endif

#ifndef INADDR_NONE
#define INADDR_NONE (-1)
#endif

#if defined(DW_DEBUG_PROTO)
#  define DBG_PROTO_STMT(x) x
#else
#  define DBG_PROTO_STMT(x)
#endif

// WABA: This should be defined by netdb.h
// deller: Needed for HP/UX
#if defined(__hpux)
extern int h_errno;
#endif
static int translate_h_errno(int herrno);
static const char* get_error_text(int aErrorCode);


DwProtocolClient::DwProtocolClient()
{
    mIsDllOpen      = DwTrue;
    mIsOpen         = DwFalse;
    mSocket         = -1;
    mPort           = 0;
    mServerName     = 0;
    mReceiveTimeout = 90;
    mLastCommand    = 0;
    mFailureCode    = kFailNoFailure;
    mFailureStr     = "";
    mErrorCode      = kErrNoError;
    mErrorStr       = get_error_text(kErrNoError);
}


DwProtocolClient::~DwProtocolClient()
{
    if (mIsOpen) {
        Close();
    }
    if (mServerName) {
        delete [] mServerName;
        mServerName = 0;
    }
}


int DwProtocolClient::Open(const char* aServer, DwUint16 aPort)
{
    mFailureCode = kFailNoFailure;
    mFailureStr  = "";
    mErrorCode   = kErrNoError;
    mErrorStr    = get_error_text(mErrorCode);

    if (mIsOpen) {
        // error!
        mErrorCode = kErrBadUsage;
        mErrorStr = get_error_text(mErrorCode);
        return -1;
    }
    if (aServer == 0 || aServer[0] == 0) {
        // error!
        mErrorCode = kErrBadParameter;
        mErrorStr = get_error_text(mErrorCode);
        return -1;
    }
    if (mServerName) {
        delete [] mServerName;
        mServerName = 0;
    }
    mServerName = new char[strlen(aServer)+1];
    strcpy(mServerName, aServer);
    mPort = aPort;

    // Open the socket

    mSocket = socket(PF_INET, SOCK_STREAM, 0);
    if (mSocket == -1) {
        // error!
        int err = errno;
        HandleError(err, ksocket);
        return -1;
    }

    // If the server is specified by an IP number in dotted decimal form,
    // then try to connect to that IP number.

    int err = -1;
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(struct sockaddr_in));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(mPort);
    serverAddr.sin_addr.s_addr = inet_addr(mServerName);
    if (serverAddr.sin_addr.s_addr != INADDR_NONE) {
        DBG_PROTO_STMT(cout << "Trying connection to " << mServerName << endl;)
        err = connect(mSocket, (struct sockaddr*)&serverAddr,
            sizeof(struct sockaddr_in));
    }

    // Otherwise, do a host name lookup.

    else {
        struct hostent* hostentp = gethostbyname(mServerName);
        if (hostentp == NULL) {
            // error!
            int err = h_errno;
            close(mSocket);
            mSocket = -1;
            err = translate_h_errno(err);
            HandleError(err, kgethostbyname);
            return -1;
        }

        // Connect to the server.  Try each IP number until one succeeds.

        char** addr_list = hostentp->h_addr_list;
        while (*addr_list) {
            struct in_addr* in_addrp = (struct in_addr*)*addr_list;
            memcpy(&serverAddr.sin_addr.s_addr, in_addrp, sizeof(struct in_addr));
            DBG_PROTO_STMT(cout << "Trying connection to " << mServerName;)
            DBG_PROTO_STMT(cout << " (" << inet_ntoa(*in_addrp) << ')' << endl;)
            err = connect(mSocket, (struct sockaddr*)&serverAddr,
                sizeof(struct sockaddr_in));
            if (err != -1) {
                break;
            }
            ++addr_list;
        }
    }

    if (err == -1) {
        // error!
        mErrorCode = errno;
        close(mSocket);
        mSocket = -1;
        HandleError(err, kconnect);
        return -1;
    }
    DBG_PROTO_STMT(cout << "Connection okay" << endl;)
    mIsOpen = DwTrue;
    return 0;
}


DwBool DwProtocolClient::IsOpen() const
{
    return mIsOpen;
}


int DwProtocolClient::Close()
{
    mFailureCode = kFailNoFailure;
    mFailureStr  = "";
    mErrorCode   = kErrNoError;
    mErrorStr    = get_error_text(mErrorCode);

    if (! mIsOpen) {
        // error!
        mErrorCode = kErrBadUsage;
        mErrorStr = get_error_text(mErrorCode);
        return -1;
    }
    int err = close(mSocket);
    if (err < 0) {
        // error!
        int err = errno;
        HandleError(err, kclose);
        return -1;
    }
    mIsOpen = DwFalse;
    return 0;
}


int DwProtocolClient::SetReceiveTimeout(int aSecs)
{
    mReceiveTimeout = aSecs;
    return 0;
}


int DwProtocolClient::LastCommand() const
{
    return mLastCommand;
}


int DwProtocolClient::LastFailure() const
{
    return mFailureCode;
}


const char* DwProtocolClient::LastFailureStr() const
{
    return mFailureStr;
}


int DwProtocolClient::LastError() const
{
    return mErrorCode;
}


const char* DwProtocolClient::LastErrorStr() const
{
    return mErrorStr;
}


int DwProtocolClient::PSend(const char* aBuf, int aBufLen)
{
    mFailureCode = kFailNoFailure;
    mFailureStr  = "";
    mErrorCode   = kErrNoError;
    mErrorStr    = get_error_text(mErrorCode);

    if (! mIsOpen) {
        // error!
        mErrorCode = kErrBadUsage;
        mErrorStr = get_error_text(mErrorCode);
        return 0;
    }
    int ret;
    int numToSend = aBufLen;
    int numSent = 0;
    while (numToSend > 0) {
        ret = send(mSocket, &aBuf[numSent], numToSend, 0);
        if (ret == -1) {
            // error!
            int err = errno;
            HandleError(err, ksend);
            break;
        }
        else {
            numSent += ret;
            numToSend -= ret;
        }
    }
    return numSent;
}


int DwProtocolClient::PReceive(char* aBuf, int aBufSize)
{
    mFailureCode = kFailNoFailure;
    mFailureStr  = "";
    mErrorCode   = kErrNoError;
    mErrorStr    = get_error_text(mErrorCode);

    if (! mIsOpen) {
        // error!
        mErrorCode = kErrBadUsage;
        mErrorStr = get_error_text(mErrorCode);
        return 0;
    }

    // Suspend until there's input to read

    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(mSocket, &readfds);
    struct timeval timeout;
    timeout.tv_sec = mReceiveTimeout;
    timeout.tv_usec = 0;
    int numFds = select(mSocket+1, &readfds, 0, 0, &timeout);
    int numReceived = 0;

    // If an error occurred, deal with it

    if (numFds == -1) {
        int err = errno;
        HandleError(err, kselect);
        numReceived = 0;
    }

    // Read the input, if available

    else if (numFds == 1) {
        int ret = recv(mSocket, aBuf, aBufSize, 0);
        if (ret == -1) {
            // error!
            int err = errno;
            HandleError(err, krecv);
            numReceived = 0;
        }
        else /* if (ret != -1) */ {
            numReceived = ret;
        }
    }

    // Otherwise, there was a timeout

    else if (numFds == 0) {
        DBG_PROTO_STMT(cout << "Receive timed out" << endl;)
        int err = ETIMEDOUT;
        HandleError(err, kselect);
        numReceived = 0;
    }

    return numReceived;
}


void DwProtocolClient::HandleError(int aErrorCode, int aSystemCall)
{
    mErrorCode = aErrorCode;
    mErrorStr = get_error_text(mErrorCode);
    switch (aSystemCall) {
    case ksocket:
        switch (mErrorCode) {
        case EMFILE:
        case ENFILE:
        case ENOBUFS:
            mFailureCode = kFailNoResources;
            mFailureStr = "Cannot get required system resources";
            break;
        case EPROTONOSUPPORT:
        case EACCES:
            break;
        }
        break;
    case kgethostbyname:
        switch (mErrorCode) {
        case kErrHostNotFound:
        case kErrTryAgain:
        case kErrNoRecovery:
        case kErrNoData:
            mFailureCode = kFailHostNotFound;
            mFailureStr = "The server was not found";
            break;
        default:
            break;
        }
        break;
    case ksetsockopt:
        break;
    case kconnect:
        switch (aErrorCode) {
        case ETIMEDOUT:
            mFailureCode = kFailTimedOut;
            mFailureStr = "The connection attempt to the server timed out";
            break;
        case ECONNREFUSED:
            mFailureCode = kFailConnRefused;
            mFailureStr = "The connection was refused by the server";
            break;
        case ENETUNREACH:
            mFailureCode = kFailNetUnreachable;
            mFailureStr = "The network is unreachable";
            break;
        case EBADF:
        case ENOTSOCK:
        case EADDRNOTAVAIL:
        case EAFNOSUPPORT:
        case EISCONN:
        case EADDRINUSE:
        case EFAULT:
        case EINPROGRESS:
        case EALREADY:
            break;
        }
        break;
    case ksend:
        switch(aErrorCode) {
        case ENOBUFS:
            mFailureCode = kFailNoResources;
            mFailureStr = "Cannot get required system resources";
            break;
        case EBADF:
        case ENOTSOCK:
        case EFAULT:
        case EMSGSIZE:
        case EWOULDBLOCK:
        case ECONNREFUSED:
        case EISCONN:
        case EACCES:
            break;
        }
        break;
    case krecv:
        switch(aErrorCode) {
        case EBADF:
        case ENOTSOCK:
        case EWOULDBLOCK:
        case EINTR:
        case EFAULT:
            break;
        }
        break;
    case kclose:
        switch (aErrorCode) {
        case EBADF:
        case EINTR:
        case ETIMEDOUT:
            break;
        }
        break;
    case kselect:
        switch (aErrorCode) {
        case ETIMEDOUT:
            mFailureCode = kFailTimedOut;
            mFailureStr = "Timed out while waiting for the server";
            break;
        case EBADF:
        case EINTR:
        case EINVAL:
            break;
        }
        break;
    default:
        break;
    }
}


static int translate_h_errno(int herrno)
{
    int err = 0;
    switch (herrno) {
    case HOST_NOT_FOUND:
        err = DwProtocolClient::kErrHostNotFound;
        break;
    case TRY_AGAIN:
        err = DwProtocolClient::kErrTryAgain;
        break;
    case NO_RECOVERY:
        err = DwProtocolClient::kErrNoRecovery;
        break;
    case NO_DATA:
        err = DwProtocolClient::kErrNoData;
        break;
    default:
        err = DwProtocolClient::kErrUnknownError;
        break;
    }
    return err;
}


static const char* get_error_text(int aErrorCode)
{
    const char* msg = "";
    switch (aErrorCode) {
    case DwProtocolClient::kErrNoError:
        msg = "No error";
        break;
    case DwProtocolClient::kErrUnknownError:
        msg = "Unknown error";
        break;
    case DwProtocolClient::kErrBadParameter:
        msg = "(MIME++) bad parameter passed to function";
        break;
    case DwProtocolClient::kErrBadUsage:
        msg = "(MIME++) bad library usage";
        break;
    case DwProtocolClient::kErrNoWinsock:
        msg = "(MIME++) incompatible Winsock version";
        break;
    case DwProtocolClient::kErrHostNotFound:
        msg = "Host not found";
        break;
    case DwProtocolClient::kErrTryAgain:
        msg = "Nonauthoritative host not found";
        break;
    case DwProtocolClient::kErrNoRecovery:
        msg = "Nonrecoverable errors: FORMERR, REFUSED, NOTIMP";
        break;
    case DwProtocolClient::kErrNoData:
        msg = "Valid name, no data record of requested type";
        break;
    case DwProtocolClient::kErrNoAddress:
        msg = "No address, look for MX record";
        break;
	default:
        msg = strerror(aErrorCode);
        break;
    }
    return msg;
}
