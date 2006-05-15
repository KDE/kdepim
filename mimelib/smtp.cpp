//=============================================================================
// File:       smtp.cpp
// Contents:   Definitions for DwSmtpClient
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

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#include <mimelib/config.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <mimelib/smtp.h>

#if defined(DW_UNIX)
#include <unistd.h>
#endif

#define SMTP_PORT 25
#define RECV_BUFFER_SIZE  8192
#define SEND_BUFFER_SIZE  1024

#if defined(DW_DEBUG_SMTP)
#  define DBG_SMTP_STMT(x) x
#else
#  define DBG_SMTP_STMT(x)
#endif


DwSmtpClient::DwSmtpClient()
{
    mRecvBuffer = new char[RECV_BUFFER_SIZE];
    mSendBuffer = new char[SEND_BUFFER_SIZE];
    mNumRecvBufferChars = 0;
    mRecvBufferPos = 0;
    mReplyCode = 0;
}


DwSmtpClient::~DwSmtpClient()
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


int DwSmtpClient::Open(const char* aServer, DwUint16 aPort)
{
    mReplyCode = 0;
    mResponse = "";
    int err = DwProtocolClient::Open(aServer, aPort);
    if (! err) {
        PGetResponse();
    }
    return mReplyCode;
}


int DwSmtpClient::ReplyCode() const
{
    return mReplyCode;
}


const DwString& DwSmtpClient::Response() const
{
    return mResponse;
}


int DwSmtpClient::Helo()
{
    mReplyCode = 0;
    mResponse = "";
    mLastCommand = kCmdHelo;
    strcpy(mSendBuffer, "HELO ");
    gethostname(&mSendBuffer[5], SEND_BUFFER_SIZE-32);
    mSendBuffer[5+SEND_BUFFER_SIZE-32 -1] = '\0';
    strcat(mSendBuffer, "\r\n");
    DBG_SMTP_STMT(cout << "C: " << mSendBuffer << flush;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetResponse();
    }
    return mReplyCode;
}


int DwSmtpClient::Mail(const char* aFrom)
{
    mReplyCode = 0;
    mResponse = "";
    mLastCommand = kCmdMail;
    strcpy(mSendBuffer, "MAIL FROM:<");
    strncat(mSendBuffer, aFrom, SEND_BUFFER_SIZE-32);
    strcat(mSendBuffer, ">\r\n");
    DBG_SMTP_STMT(cout << "C: " << mSendBuffer << flush;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetResponse();
    }
    return mReplyCode;
}


int DwSmtpClient::Rcpt(const char* aTo)
{
    mReplyCode = 0;
    mResponse = "";
    mLastCommand = kCmdRcpt;
    strcpy(mSendBuffer, "RCPT TO:<");
    strncat(mSendBuffer, aTo, SEND_BUFFER_SIZE-32);
    strcat(mSendBuffer, ">\r\n");
    DBG_SMTP_STMT(cout << "C: " << mSendBuffer << flush;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetResponse();
    }
    return mReplyCode;
}


int DwSmtpClient::Data()
{
    mReplyCode = 0;
    mResponse = "";
    mLastCommand = kCmdData;
    strcpy(mSendBuffer, "DATA\r\n");
    DBG_SMTP_STMT(cout << "C: " << mSendBuffer << flush;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetResponse();
    }
    return mReplyCode;
}


int DwSmtpClient::Rset()
{
    mReplyCode = 0;
    mResponse = "";
    mLastCommand = kCmdRset;
    strcpy(mSendBuffer, "RSET\r\n");
    DBG_SMTP_STMT(cout << "C: " << mSendBuffer << flush;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetResponse();
    }
    return mReplyCode;
}


int DwSmtpClient::Send(const char* aFrom)
{
    mReplyCode = 0;
    mResponse = "";
    mLastCommand = kCmdSend;
    strcpy(mSendBuffer, "SEND FROM:<");
    strncat(mSendBuffer, aFrom, SEND_BUFFER_SIZE-32);
    strcat(mSendBuffer, ">\r\n");
    DBG_SMTP_STMT(cout << "C: " << mSendBuffer << flush;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetResponse();
    }
    return mReplyCode;
}


int DwSmtpClient::Soml(const char* aFrom)
{
    mReplyCode = 0;
    mResponse = "";
    mLastCommand = kCmdSoml;
    strcpy(mSendBuffer, "SOML FROM:<");
    strncat(mSendBuffer, aFrom, SEND_BUFFER_SIZE-32);
    strcat(mSendBuffer, ">\r\n");
    DBG_SMTP_STMT(cout << "C: " << mSendBuffer << flush;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetResponse();
    }
    return mReplyCode;
}


int DwSmtpClient::Saml(const char* aFrom)
{
    mReplyCode = 0;
    mResponse = "";
    mLastCommand = kCmdSaml;
    strcpy(mSendBuffer, "SAML FROM:<");
    strncat(mSendBuffer, aFrom, SEND_BUFFER_SIZE-32);
    strcat(mSendBuffer, ">\r\n");
    DBG_SMTP_STMT(cout << "C: " << mSendBuffer << flush;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetResponse();
    }
    return mReplyCode;
}


int DwSmtpClient::Vrfy(const char* aName)
{
    mReplyCode = 0;
    mResponse = "";
    mLastCommand = kCmdVrfy;
    strcpy(mSendBuffer, "VRFY ");
    strncat(mSendBuffer, aName, SEND_BUFFER_SIZE-32);
    strcat(mSendBuffer, "\r\n");
    DBG_SMTP_STMT(cout << "C: " << mSendBuffer << flush;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetResponse();
    }
    return mReplyCode;
}


int DwSmtpClient::Expn(const char* aName)
{
    mReplyCode = 0;
    mResponse = "";
    mLastCommand = kCmdExpn;
    strcpy(mSendBuffer, "EXPN ");
    strncat(mSendBuffer, aName, SEND_BUFFER_SIZE-32);
    strcat(mSendBuffer, "\r\n");
    DBG_SMTP_STMT(cout << "C: " << mSendBuffer << flush;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetResponse();
    }
    return mReplyCode;
}


int DwSmtpClient::Help(const char* aArg)
{
    mReplyCode = 0;
    mResponse = "";
    mLastCommand = kCmdHelp;
    strcpy(mSendBuffer, "HELP");
    if (aArg) {
        strcat(mSendBuffer, " ");
        strncat(mSendBuffer, aArg, SEND_BUFFER_SIZE-32);
    }
    strcat(mSendBuffer, "\r\n");
    DBG_SMTP_STMT(cout << "C: " << mSendBuffer << flush;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetResponse();
    }
    return mReplyCode;
}


int DwSmtpClient::Noop()
{
    mReplyCode = 0;
    mResponse = "";
    mLastCommand = kCmdNoop;
    strcpy(mSendBuffer, "NOOP\r\n");
    DBG_SMTP_STMT(cout << "C: " << mSendBuffer << flush;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetResponse();
    }
    return mReplyCode;
}


int DwSmtpClient::Quit()
{
    mReplyCode = 0;
    mResponse = "";
    mLastCommand = kCmdQuit;
    strcpy(mSendBuffer, "QUIT\r\n");
    DBG_SMTP_STMT(cout << "C: " << mSendBuffer << flush;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetResponse();
    }
    return mReplyCode;
}


int DwSmtpClient::Turn()
{
    mReplyCode = 0;
    mResponse = "";
    mLastCommand = kCmdTurn;
    strcpy(mSendBuffer, "TURN\r\n");
    DBG_SMTP_STMT(cout << "C: " << mSendBuffer << flush;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetResponse();
    }
    return mReplyCode;
}


int DwSmtpClient::SendData(const DwString& aStr)
{
    return SendData(aStr.data(), aStr.length());
}


int DwSmtpClient::SendData(const char* aBuf, int aBufLen)
{
    mReplyCode = 0;
    mResponse = "";

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

    PGetResponse();
    return mReplyCode;
}


void DwSmtpClient::PGetResponse()
{
    mReplyCode = 0;
    char* ptr = 0;
    int len = 0;
    int err = 0;
    int done = 0;
    while (! done) {
        err = PGetLine(&ptr, &len);
        if (! err) {
            mResponse.append(ptr, len);
            if (len <= 3 || ptr[3] != '-') {
                done = 1;
            }
        }
        else {
            done = 1;
        }
    }
    if (! err) {
        mReplyCode = strtol(ptr, NULL, 10);
    }
    DBG_SMTP_STMT(cout << "S: " << mResponse << flush;)
}


int DwSmtpClient::PGetLine(char** aPtr, int* aLen)
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
