//=============================================================================
// File:       nntp.cpp
// Contents:   Definitions for DwNntpClient
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
#include <mimelib/nntp.h>

#define NNTP_PORT 119
#define RECV_BUFFER_SIZE  8192
#define SEND_BUFFER_SIZE  1024

#if defined(DW_DEBUG_NNTP)
#  define DBG_NNTP_STMT(x) x
#else
#  define DBG_NNTP_STMT(x)
#endif


DwNntpClient::DwNntpClient()
{
    mSendBuffer = new char[SEND_BUFFER_SIZE];
    mRecvBuffer = new char[RECV_BUFFER_SIZE];
    mLastChar = -1;
    mLastLastChar = -1;
    mNumRecvBufferChars = 0;
    mRecvBufferPos = 0;
    mReplyCode = 0;
    mObserver = 0;
}


DwNntpClient::~DwNntpClient()
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


int DwNntpClient::Open(const char* aServer, DwUint16 aPort)
{
    mReplyCode = 0;
    mStatusResponse = mTextResponse = "";
    int err = DwProtocolClient::Open(aServer, aPort);
    if (! err) {
        PGetStatusResponse();
    }
    return mReplyCode;
}


DwObserver* DwNntpClient::SetObserver(DwObserver* aObserver)
{
    DwObserver* obs = mObserver;
    mObserver = aObserver;
    return obs;
}


int DwNntpClient::ReplyCode() const
{
    return mReplyCode;
}


const DwString& DwNntpClient::StatusResponse() const
{
    return mStatusResponse;
}


const DwString& DwNntpClient::TextResponse() const
{
    return mTextResponse;
}


int DwNntpClient::Article(int aArticleNum)
{
    mReplyCode = 0;
    mStatusResponse = mTextResponse = "";
    mLastCommand = kCmdArticle;
    if (aArticleNum >= 0) {
        snprintf(mSendBuffer, SEND_BUFFER_SIZE, "ARTICLE %d\r\n", aArticleNum);
    }
    else {
        strlcpy(mSendBuffer, "ARTICLE\r\n", SEND_BUFFER_SIZE);
    }
    DBG_NNTP_STMT(cout << "C: " << mSendBuffer << flush;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetStatusResponse();
        if (mReplyCode/100%10 == 2) {
            PGetTextResponse();
        }
    }
    return mReplyCode;
}


int DwNntpClient::Article(const char* aMsgId)
{
    mReplyCode = 0;
    mStatusResponse = mTextResponse = "";
    mLastCommand = kCmdArticle;
    if (!aMsgId || !*aMsgId) {
        // error!
        return mReplyCode;
    }
    strlcpy(mSendBuffer, "ARTICLE ", SEND_BUFFER_SIZE);
    strlcat(mSendBuffer, aMsgId, SEND_BUFFER_SIZE);
    strlcat(mSendBuffer, "\r\n", SEND_BUFFER_SIZE);
    DBG_NNTP_STMT(cout << "C: " << mSendBuffer << flush;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetStatusResponse();
        if (mReplyCode/100%10 == 2) {
            PGetTextResponse();
        }
    }
    return mReplyCode;
}


int DwNntpClient::Head(int aArticleNum)
{
    mReplyCode = 0;
    mStatusResponse = mTextResponse = "";
    mLastCommand = kCmdHead;
    if (aArticleNum >= 0) {
        snprintf(mSendBuffer, SEND_BUFFER_SIZE, "HEAD %d\r\n", aArticleNum);
    }
    else {
        strlcpy(mSendBuffer, "HEAD\r\n", SEND_BUFFER_SIZE);
    }
    DBG_NNTP_STMT(cout << "C: " << mSendBuffer << flush;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetStatusResponse();
        if (mReplyCode/100%10 == 2) {
            PGetTextResponse();
        }
    }
    return mReplyCode;
}


int DwNntpClient::Head(const char* aMsgId)
{
    mReplyCode = 0;
    mStatusResponse = mTextResponse = "";
    mLastCommand = kCmdHead;
    if (!aMsgId || !*aMsgId) {
        return mReplyCode;
    }
    strlcpy(mSendBuffer, "HEAD ", SEND_BUFFER_SIZE);
    strlcat(mSendBuffer, aMsgId, SEND_BUFFER_SIZE);
    strlcat(mSendBuffer, "\r\n", SEND_BUFFER_SIZE);
    DBG_NNTP_STMT(cout << "C: " << mSendBuffer << flush;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetStatusResponse();
        if (mReplyCode/100%10 == 2) {
            PGetTextResponse();
        }
    }
    return mReplyCode;
}


int DwNntpClient::Body(int articleNum)
{
    mReplyCode = 0;
    mStatusResponse = mTextResponse = "";
    mLastCommand = kCmdBody;
    if (articleNum >= 0) {
        snprintf(mSendBuffer, SEND_BUFFER_SIZE, "BODY %d\r\n", articleNum);
    }
    else {
        strlcpy(mSendBuffer, "BODY\r\n", SEND_BUFFER_SIZE);
    }
    DBG_NNTP_STMT(cout << "C: " << mSendBuffer << flush;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetStatusResponse();
        if (mReplyCode/100%10 == 2) {
            PGetTextResponse();
        }
    }
    return mReplyCode;
}


int DwNntpClient::Body(const char* aMsgId)
{
    mReplyCode = 0;
    mStatusResponse = mTextResponse = "";
    mLastCommand = kCmdBody;
    if (!aMsgId || !*aMsgId) {
        return mReplyCode;
    }
    strlcpy(mSendBuffer, "BODY ", SEND_BUFFER_SIZE);
    strlcat(mSendBuffer, aMsgId, SEND_BUFFER_SIZE);
    strlcat(mSendBuffer, "\r\n", SEND_BUFFER_SIZE);
    DBG_NNTP_STMT(cout << "C: " << mSendBuffer << flush;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetStatusResponse();
        if (mReplyCode/100%10 == 2) {
            PGetTextResponse();
        }
    }
    return mReplyCode;
}


int DwNntpClient::Stat(int articleNum)
{
    mReplyCode = 0;
    mStatusResponse = mTextResponse = "";
    mLastCommand = kCmdStat;
    if (articleNum >= 0) {
        snprintf(mSendBuffer, SEND_BUFFER_SIZE, "STAT %d\r\n", articleNum);
    }
    else {
        strlcpy(mSendBuffer, "STAT\r\n", SEND_BUFFER_SIZE);
    }
    DBG_NNTP_STMT(cout << "C: " << mSendBuffer << flush;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetStatusResponse();
    }
    return mReplyCode;
}


int DwNntpClient::Stat(const char* aMsgId)
{
    mReplyCode = 0;
    mStatusResponse = mTextResponse = "";
    mLastCommand = kCmdStat;
    if (!aMsgId || !*aMsgId) {
        return mReplyCode;
    }
    strlcpy(mSendBuffer, "STAT ", SEND_BUFFER_SIZE);
    strlcat(mSendBuffer, aMsgId, SEND_BUFFER_SIZE);
    strlcat(mSendBuffer, "\r\n", SEND_BUFFER_SIZE);
    DBG_NNTP_STMT(cout << "C: " << mSendBuffer << flush;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetStatusResponse();
    }
    return mReplyCode;
}


int DwNntpClient::Group(const char* aNewsgroupName)
{
    mReplyCode = 0;
    mStatusResponse = mTextResponse = "";
    mLastCommand = kCmdGroup;
    if (!aNewsgroupName || !*aNewsgroupName) {
        return mReplyCode;
    }
    strlcpy(mSendBuffer, "GROUP ", SEND_BUFFER_SIZE);
    strlcat(mSendBuffer, aNewsgroupName, SEND_BUFFER_SIZE);
    strlcat(mSendBuffer, "\r\n", SEND_BUFFER_SIZE);
    DBG_NNTP_STMT(cout << "C: " << mSendBuffer << flush;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetStatusResponse();
    }
    return mReplyCode;
}


int DwNntpClient::Help()
{
    mReplyCode = 0;
    mStatusResponse = mTextResponse = "";
    mLastCommand = kCmdHelp;
    strlcpy(mSendBuffer, "HELP\r\n", SEND_BUFFER_SIZE);
    DBG_NNTP_STMT(cout << "C: " << mSendBuffer << flush;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetStatusResponse();
        if (mReplyCode/100%10 == 1) {
            PGetTextResponse();
        }
    }
    return mReplyCode;
}


int DwNntpClient::Last()
{
    mReplyCode = 0;
    mStatusResponse = mTextResponse = "";
    mLastCommand = kCmdLast;
    strlcpy(mSendBuffer, "LAST\r\n", SEND_BUFFER_SIZE);
    DBG_NNTP_STMT(cout << "C: " << mSendBuffer << flush;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetStatusResponse();
    }
    return mReplyCode;
}


int DwNntpClient::List()
{
    mReplyCode = 0;
    mStatusResponse = mTextResponse = "";
    mLastCommand = kCmdList;
    strlcpy(mSendBuffer, "LIST\r\n", SEND_BUFFER_SIZE);
    DBG_NNTP_STMT(cout << "C: " << mSendBuffer << flush;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetStatusResponse();
        if (mReplyCode/100%10 == 2) {
            PGetTextResponse();
        }
    }
    return mReplyCode;
}


int DwNntpClient::Newgroups(const char* aDate, const char* aTime,
    DwBool aIsGmt, const char* aDistribution)
{
    mReplyCode = 0;
    mStatusResponse = mTextResponse = "";
    mLastCommand = kCmdNewgroups;
    strlcpy(mSendBuffer, "NEWGROUPS ", SEND_BUFFER_SIZE);
    strlcat(mSendBuffer, aDate, SEND_BUFFER_SIZE);
    strlcat(mSendBuffer, " ", SEND_BUFFER_SIZE);
    strlcat(mSendBuffer, aTime, SEND_BUFFER_SIZE);
    if (aIsGmt) {
        strlcat(mSendBuffer, " GMT", SEND_BUFFER_SIZE);
    }
    if (aDistribution) {
        strlcat(mSendBuffer, " ", SEND_BUFFER_SIZE);
        strlcat(mSendBuffer, aDistribution, SEND_BUFFER_SIZE);
    }
    strlcat(mSendBuffer, "\r\n", SEND_BUFFER_SIZE);
    DBG_NNTP_STMT(cout << "C: " << mSendBuffer << flush;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetStatusResponse();
        if (mReplyCode/100%10 == 2) {
            PGetTextResponse();
        }
    }
    return mReplyCode;
}


int DwNntpClient::Newnews(const char* aNewsgroups, const char* aDate,
    const char* aTime, DwBool aIsGmt, const char* aDistribution)
{
    mReplyCode = 0;
    mStatusResponse = mTextResponse = "";
    mLastCommand = kCmdNewnews;
    strlcpy(mSendBuffer, "NEWNEWS ", SEND_BUFFER_SIZE);
    strlcat(mSendBuffer, aNewsgroups, SEND_BUFFER_SIZE);
    strlcat(mSendBuffer, " ", SEND_BUFFER_SIZE);
    strlcat(mSendBuffer, aDate, SEND_BUFFER_SIZE);
    strlcat(mSendBuffer, " ", SEND_BUFFER_SIZE);
    strlcat(mSendBuffer, aTime, SEND_BUFFER_SIZE);
    if (aIsGmt) {
        strlcat(mSendBuffer, " GMT", SEND_BUFFER_SIZE);
    }
    if (aDistribution) {
        strlcat(mSendBuffer, " ", SEND_BUFFER_SIZE);
        strlcat(mSendBuffer, aDistribution, SEND_BUFFER_SIZE);
    }
    strlcat(mSendBuffer, "\r\n", SEND_BUFFER_SIZE);
    DBG_NNTP_STMT(cout << "C: " << mSendBuffer << flush;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetStatusResponse();
        if (mReplyCode/100%10 == 2) {
            PGetTextResponse();
        }
    }
    return mReplyCode;
}


int DwNntpClient::Next()
{
    mReplyCode = 0;
    mStatusResponse = mTextResponse = "";
    mLastCommand = kCmdNext;
    strlcpy(mSendBuffer, "NEXT\r\n", SEND_BUFFER_SIZE);
    DBG_NNTP_STMT(cout << "C: " << mSendBuffer << flush;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetStatusResponse();
    }
    return mReplyCode;
}


int DwNntpClient::Post()
{
    mReplyCode = 0;
    mStatusResponse = mTextResponse = "";
    mLastCommand = kCmdPost;
    strlcpy(mSendBuffer, "POST\r\n", SEND_BUFFER_SIZE);
    DBG_NNTP_STMT(cout << "C: " << mSendBuffer << flush;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetStatusResponse();
    }
    return mReplyCode;
}


int DwNntpClient::Quit()
{
    mReplyCode = 0;
    mStatusResponse = mTextResponse = "";
    mLastCommand = kCmdQuit;
    strlcpy(mSendBuffer, "QUIT\r\n", SEND_BUFFER_SIZE);
    DBG_NNTP_STMT(cout << "C: " << mSendBuffer << flush;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetStatusResponse();
    }
    return mReplyCode;
}


int DwNntpClient::Slave()
{
    mReplyCode = 0;
    mStatusResponse = mTextResponse = "";
    mLastCommand = kCmdSlave;
    strlcpy(mSendBuffer, "SLAVE\r\n", SEND_BUFFER_SIZE);
    DBG_NNTP_STMT(cout << "C: " << mSendBuffer << flush;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetStatusResponse();
    }
    return mReplyCode;
}


int DwNntpClient::SendData(const DwString& aStr)
{
    return SendData(aStr.data(), aStr.length());
}


int DwNntpClient::SendData(const char* aBuf, int aBufLen)
{
    mReplyCode = 0;
    mStatusResponse = mTextResponse = "";

    int pos = 0;
    int len = 0;
    const char* buf = 0;

    int lastLastChar = '\r';
    int lastChar = '\n';

    while (1) {

        len = SEND_BUFFER_SIZE;
        len = (len < aBufLen - pos) ? len : aBufLen - pos;
        if (len == 0) break;

        // Look for CR LF '.'.  If it is found, then we have to copy the buffer
        // and stuff an extra '.'.

        int hasCrLfDot = 0;
        int tLastChar = lastChar;
        int tLastLastChar = lastLastChar;
        for (int i=0; i < len; ++i) {
            int ch = aBuf[pos+i];
            if (tLastLastChar == '\r' && tLastChar == '\n' && ch == '.') {
                hasCrLfDot = 1;
                break;
            }
            tLastLastChar = tLastChar;
            tLastChar = ch;
        }
        if (! hasCrLfDot) {
            lastChar = tLastChar;
            lastLastChar = tLastLastChar;
            buf = &aBuf[pos];
            pos += len;
        }

        // If CR LF '.' was found, copy the chars to a different buffer and stuff
        // the extra '.'.

        else /* (hasCrLfDot) */ {
            tLastChar = lastChar;
            tLastLastChar = lastLastChar;
            int iDst = 0;
            int iSrc = 0;
            // Implementation note: be careful to avoid overrunning the
            // destination buffer when CR LF '.' are the last three characters
            // of the source buffer.
            while (iDst < SEND_BUFFER_SIZE && iSrc < len) {
                int ch = aBuf[pos+iSrc];
                if (tLastLastChar == '\r' && tLastChar == '\n' && ch == '.') {
                    if (iDst == SEND_BUFFER_SIZE-1) {
                        break;
                    }
                    mSendBuffer[iDst++] = '.';
                }
                mSendBuffer[iDst++] = (char) ch;
                ++iSrc;
                tLastLastChar = tLastChar;
                tLastChar = ch;
            }
            lastChar = tLastChar;
            lastLastChar = tLastLastChar;
            len = iDst;
            buf = mSendBuffer;
            pos += iSrc;
        }

        // Send the buffer

        int numSent = PSend(buf, len);
        if (numSent != len) {
            mReplyCode = 0;
            return mReplyCode;
        }
    }

    // Send final '.' CR LF.  If CR LF are not at the end of the buffer, then
    // send a CR LF '.' CR LF.

    if (lastLastChar == '\r' && lastChar == '\n') {
        PSend(".\r\n", 3);
    }
    else {
        PSend("\r\n.\r\n", 5);
    }

    // Get the server's response

    PGetStatusResponse();
    return mReplyCode;
}


void DwNntpClient::PGetStatusResponse()
{
    mReplyCode = 0;
    mStatusResponse = "";
    char* ptr;
    int len;
    int err = PGetLine(&ptr, &len);
    if (! err) {
        mReplyCode = strtol(ptr, NULL, 10);
        mStatusResponse.assign(ptr, len);
        DBG_NNTP_STMT(char buffer[256];)
        DBG_NNTP_STMT(strncpy(buffer, ptr, len);)
        DBG_NNTP_STMT(buffer[len] = 0;)
        DBG_NNTP_STMT(cout << "S: " << buffer;)
    }
}


void DwNntpClient::PGetTextResponse()
{
    mTextResponse = "";

    // Get a line at a time until we get CR LF . CR LF

    while (1) {
        char* ptr;
        int len;
        int err = PGetLine(&ptr, &len);

        // Check for an error

        if (err) {
            mReplyCode = 0;
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
            mTextResponse.assign(ptr, len);
            mObserver->Notify();
        }
        else {
            mTextResponse.append(ptr, len);
        }
    }
}


int DwNntpClient::PGetLine(char** aPtr, int* aLen)
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
