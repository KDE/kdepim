//=============================================================================
// File:       pop.cpp
// Contents:   Definitions for DwPopClient
// Maintainer: Doug Sauder <dwsauder@fwb.gulf.net>
// WWW:        http://www.fwb.gulf.net/~dwsauder/mimepp.html
// $Revision$
// $Date$
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

#define DW_IMPLEMENTATION

#include <mimelib/config.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <mimelib/pop.h>

#define POP_PORT 110
#define RECV_BUFFER_SIZE  8192
#define SEND_BUFFER_SIZE  1024

#if defined(DW_DEBUG_POP)
#  define DBG_POP_STMT(x) x
#else
#  define DBG_POP_STMT(x)
#endif


DwPopClient::DwPopClient()
{
    mSendBuffer = new char[SEND_BUFFER_SIZE];
    mRecvBuffer = new char[RECV_BUFFER_SIZE];
    mNumRecvBufferChars = 0;
    mRecvBufferPos = 0;
    mStatusCode = 0;
    mObserver = 0;
}


DwPopClient::~DwPopClient()
{
    if (mRecvBuffer) {
        delete [] mRecvBuffer;
        mRecvBuffer = 0;
    }
    if (mSendBuffer) {
        delete [] mSendBuffer;
        mSendBuffer = 0;
    }
}


int DwPopClient::Open(const char* aServer, DwUint16 aPort)
{
    mStatusCode = 0;
    mSingleLineResponse = mMultiLineResponse = "";
    int err = DwProtocolClient::Open(aServer, aPort);
    if (! err) {
        PGetSingleLineResponse();
    }
    return mStatusCode;
}


DwObserver* DwPopClient::SetObserver(DwObserver* aObserver)
{
    DwObserver* obs = mObserver;
    mObserver = aObserver;
    return obs;
}


int DwPopClient::StatusCode() const
{
    return mStatusCode;
}


const DwString& DwPopClient::SingleLineResponse() const
{
    return mSingleLineResponse;
}


const DwString& DwPopClient::MultiLineResponse() const
{
    return mMultiLineResponse;
}


int DwPopClient::User(const char* aName)
{
    mStatusCode = 0;
    mSingleLineResponse = mMultiLineResponse = "";
    mLastCommand = kCmdUser;
    strlcpy(mSendBuffer, "USER ", SEND_BUFFER_SIZE);
    strlcat(mSendBuffer, aName, SEND_BUFFER_SIZE);
    strlcat(mSendBuffer, "\r\n", SEND_BUFFER_SIZE);
    DBG_POP_STMT(cout << "C: " << mSendBuffer << flush;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetSingleLineResponse();
    }
    return mStatusCode;
}


int DwPopClient::Pass(const char* aPasswd)
{
    mStatusCode = 0;
    mSingleLineResponse = mMultiLineResponse = "";
    mLastCommand = kCmdPass;
    strlcpy(mSendBuffer, "PASS ", SEND_BUFFER_SIZE);
    strlcat(mSendBuffer, aPasswd, SEND_BUFFER_SIZE);
    strlcat(mSendBuffer, "\r\n", SEND_BUFFER_SIZE);
    DBG_POP_STMT(cout << "C: " << mSendBuffer << flush;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetSingleLineResponse();
    }
    return mStatusCode;
}


int DwPopClient::Quit()
{
    mStatusCode = 0;
    mSingleLineResponse = mMultiLineResponse = "";
    mLastCommand = kCmdQuit;
    strlcpy(mSendBuffer, "QUIT\r\n", SEND_BUFFER_SIZE);
    DBG_POP_STMT(cout << "C: " << mSendBuffer << flush;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetSingleLineResponse();
    }
    return mStatusCode;
}


int DwPopClient::Stat()
{
    mStatusCode = 0;
    mSingleLineResponse = mMultiLineResponse = "";
    mLastCommand = kCmdStat;
    strlcpy(mSendBuffer, "STAT\r\n", SEND_BUFFER_SIZE);
    DBG_POP_STMT(cout << "C: " << mSendBuffer << flush;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetSingleLineResponse();
    }
    return mStatusCode;
}


int DwPopClient::List()
{
    mStatusCode = 0;
    mSingleLineResponse = mMultiLineResponse = "";
    mLastCommand = kCmdList;
    strlcpy(mSendBuffer, "LIST\r\n", SEND_BUFFER_SIZE);
    DBG_POP_STMT(cout << "C: " << mSendBuffer << flush;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetSingleLineResponse();
        if (mStatusCode == '+') {
            PGetMultiLineResponse();
        }
    }
    return mStatusCode;
}


int DwPopClient::List(int aMsg)
{
    mStatusCode = 0;
    mSingleLineResponse = mMultiLineResponse = "";
    mLastCommand = kCmdList;
    snprintf(mSendBuffer, SEND_BUFFER_SIZE, "LIST %d\r\n", aMsg);
    DBG_POP_STMT(cout << "C: " << mSendBuffer << flush;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetSingleLineResponse();
    }
    return mStatusCode;
}


int DwPopClient::Retr(int aMsg)
{
    mStatusCode = 0;
    mSingleLineResponse = mMultiLineResponse = "";
    mLastCommand = kCmdRetr;
    snprintf(mSendBuffer, SEND_BUFFER_SIZE, "RETR %d\r\n", aMsg);
    DBG_POP_STMT(cout << "C: " << mSendBuffer << flush;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetSingleLineResponse();
        if (mStatusCode == '+') {
            PGetMultiLineResponse();
        }
    }
    return mStatusCode;
}


int DwPopClient::Dele(int aMsg)
{
    mStatusCode = 0;
    mSingleLineResponse = mMultiLineResponse = "";
    mLastCommand = kCmdDele;
    snprintf(mSendBuffer, SEND_BUFFER_SIZE, "DELE %d\r\n", aMsg);
    DBG_POP_STMT(cout << "C: " << mSendBuffer << flush;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetSingleLineResponse();
    }
    return mStatusCode;
}


int DwPopClient::Noop()
{
    mStatusCode = 0;
    mSingleLineResponse = mMultiLineResponse = "";
    mLastCommand = kCmdNoop;
    strlcpy(mSendBuffer, "NOOP\r\n", SEND_BUFFER_SIZE);
    DBG_POP_STMT(cout << "C: " << mSendBuffer << flush;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetSingleLineResponse();
    }
    return mStatusCode;
}


int DwPopClient::Rset()
{
    mStatusCode = 0;
    mSingleLineResponse = mMultiLineResponse = "";
    mLastCommand = kCmdRset;
    strlcpy(mSendBuffer, "RSET\r\n", SEND_BUFFER_SIZE);
    DBG_POP_STMT(cout << "C: " << mSendBuffer << flush;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetSingleLineResponse();
    }
    return mStatusCode;
}


int DwPopClient::Last()
{
    mStatusCode = 0;
    mSingleLineResponse = mMultiLineResponse = "";
    mLastCommand = kCmdRset;
    strlcpy(mSendBuffer, "LAST\r\n", SEND_BUFFER_SIZE);
    DBG_POP_STMT(cout << "C: " << mSendBuffer << flush;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetSingleLineResponse();
    }
    return mStatusCode;
}


int DwPopClient::Apop(const char* aName, const char* aDigest)
{
    mStatusCode = 0;
    mSingleLineResponse = mMultiLineResponse = "";
    mLastCommand = kCmdApop;
    strlcpy(mSendBuffer, "APOP ", SEND_BUFFER_SIZE);
    strlcat(mSendBuffer, aName, SEND_BUFFER_SIZE);
    strlcat(mSendBuffer, " ", SEND_BUFFER_SIZE);
    strlcat(mSendBuffer, aDigest, SEND_BUFFER_SIZE);
    strlcat(mSendBuffer, "\r\n", SEND_BUFFER_SIZE);
    DBG_POP_STMT(cout << "C: " << mSendBuffer << flush;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetSingleLineResponse();
    }
    return mStatusCode;
}


int DwPopClient::Top(int aMsg, int aNumLines)
{
    mStatusCode = 0;
    mSingleLineResponse = mMultiLineResponse = "";
    mLastCommand = kCmdTop;
    snprintf(mSendBuffer, SEND_BUFFER_SIZE, "TOP %d %d\r\n", aMsg, aNumLines);
    DBG_POP_STMT(cout << "C: " << mSendBuffer << flush;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetSingleLineResponse();
        if (mStatusCode == '+') {
            PGetMultiLineResponse();
        }
    }
    return mStatusCode;
}


int DwPopClient::Uidl()
{
    mStatusCode = 0;
    mSingleLineResponse = mMultiLineResponse = "";
    mLastCommand = kCmdUidl;
    strlcpy(mSendBuffer, "UIDL\r\n", SEND_BUFFER_SIZE);
    DBG_POP_STMT(cout << "C: " << mSendBuffer << flush;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetSingleLineResponse();
        if (mStatusCode == '+') {
            PGetMultiLineResponse();
        }
    }
    return mStatusCode;
}


int DwPopClient::Uidl(int aMsg)
{
    mStatusCode = 0;
    mSingleLineResponse = mMultiLineResponse = "";
    mLastCommand = kCmdUidl;
    snprintf(mSendBuffer, SEND_BUFFER_SIZE, "UIDL %d\r\n", aMsg);
    DBG_POP_STMT(cout << "C: " << mSendBuffer << flush);
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetSingleLineResponse();
        if (mStatusCode == '+') {
            PGetMultiLineResponse();
        }
    }
    return mStatusCode;
}


void DwPopClient::PGetSingleLineResponse()
{
    mStatusCode = 0;
    mSingleLineResponse = "";
    char* ptr;
    int len;
    int err = PGetLine(&ptr, &len);
    if (! err) {
        mStatusCode = ptr[0];
        mSingleLineResponse.assign(ptr, len);
        DBG_POP_STMT(char buffer[256];)
        DBG_POP_STMT(strncpy(buffer, ptr, len);)
        DBG_POP_STMT(buffer[len] = 0;)
        DBG_POP_STMT(cout << "S: " << buffer;)
    }
}


void DwPopClient::PGetMultiLineResponse()
{
    mMultiLineResponse = "";

    // Get a line at a time until we get CR LF . CR LF

    while (1) {
        char* ptr;
        int len;
        int err = PGetLine(&ptr, &len);

        // Check for an error

        if (err) {
            mStatusCode = 0;
            return;
        }

        // Check for '.' on a line by itself, which indicates end of multiline
        // response

        if (len >= 3 && ptr[0] == '.' && ptr[1] == '\r' && ptr[2] == '\n') {
            break;
        }

        // Remove '.' at beginning of line

        if (*ptr == '.') ++ptr;

        // If an observer is assigned, notify it.
        // Implementation note: An observer is assumed to fetch the multiline
        // response one line at a time, therefore we assign to the string,
        // rather than append to it.

        if (mObserver) {
            mMultiLineResponse.assign(ptr, len);
            mObserver->Notify();
        }
        else {
            mMultiLineResponse.append(ptr, len);
        }
    }
}


int DwPopClient::PGetLine(char** aPtr, int* aLen)
{
    // Restore the saved state

    int startPos = mRecvBufferPos;
    int pos = mRecvBufferPos;
    int lastChar = -1;

    // Keep trying until we get a complete line, detect an error, or
    // determine that the connection has been closed

    int isEndOfLineFound = 0;
    while (1) {

        // Search buffer for end of line chars. Stop when we find them or when
        // we exhaust the buffer.

        while (pos < mNumRecvBufferChars) {
            if (lastChar == '\r' && mRecvBuffer[pos] == '\n') {
                isEndOfLineFound = 1;
                ++pos;
                break;
            }
            lastChar = mRecvBuffer[pos];
            ++pos;
        }
        if (isEndOfLineFound) {
            *aPtr = &mRecvBuffer[startPos];
            *aLen = pos - startPos;
            mRecvBufferPos = pos;
            return 0;
        }

        // If the buffer has no room, return without an error; otherwise,
        // replenish the buffer.

        // Implementation note: The standard does not allow long lines,
        // however, that does not mean that they won't occur.  The way
        // this function deals with long lines is to return a full buffer's
        // worth of characters as a line.  The next call to this function
        // will start where this call left off.  In essence, we have
        // *forced* a line break, but without putting in CR LF characters.

        if (startPos == 0 && pos == RECV_BUFFER_SIZE) {
            *aPtr = mRecvBuffer;
            *aLen = RECV_BUFFER_SIZE;
            mRecvBufferPos = pos;
            return 0;
        }
        memmove(mRecvBuffer, &mRecvBuffer[startPos],
            mNumRecvBufferChars-startPos);
        mNumRecvBufferChars -= startPos;
        mRecvBufferPos = mNumRecvBufferChars;
        int bufFreeSpace = RECV_BUFFER_SIZE - mRecvBufferPos;
        int n = PReceive(&mRecvBuffer[mRecvBufferPos], bufFreeSpace);
        if (n == 0) {
            // The connection has been closed or an error occurred
            return -1;
        }
        mNumRecvBufferChars += n;
        startPos = 0;
        pos = mRecvBufferPos;
    }
}
