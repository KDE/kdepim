//=============================================================================
// File:       dw_cte.cpp
// Contents:   Function definitions for content transfer encodings
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

#define DW_IMPLEMENTATION

#include <mimelib/config.h>
#include <mimelib/debug.h>
#include <string.h>
#include <mimelib/string.h>
#include <mimelib/utility.h>

#define MAXLINE  76

static size_t calc_crlf_buff_size(const char* srcBuf, size_t srcLen);
static int to_crlf(const char* srcBuf, size_t srcLen, char* destBuf,
    size_t destSize, size_t* destLen);
static int to_lf(const char* srcBuf, size_t srcLen, char* destBuf, 
    size_t destSize, size_t* destLen);
static int to_cr(const char* srcBuf, size_t srcLen, char* destBuf,
    size_t destSize, size_t* destLen);
static int encode_base64(const char* aIn, size_t aInLen, char* aOut,
    size_t aOutSize, size_t* aOutLen);
static int decode_base64(const char* aIn, size_t aInLen, char* aOut,
    size_t aOutSize, size_t* aOutLen);
static int encode_qp(const char* aIn, size_t aInLen, char* aOut,
    size_t aOutSize, size_t* aOutLen);
static int decode_qp(const char* aIn, size_t aInLen, char* aOut,
    size_t aOutSize, size_t* aOutLen);
static size_t calc_qp_buff_size(const char* aIn, size_t aInLen);


int DwToCrLfEol(const DwString& aSrcStr, DwString& aDestStr)
{
    // Estimate required destination buffer size
    size_t srcLen = aSrcStr.length();
    const char* srcBuf = aSrcStr.data();
    size_t destSize = calc_crlf_buff_size(srcBuf, srcLen);

    // Allocate destination buffer
    DwString destStr(destSize, (char)0);
    char* destBuf = (char*) destStr.data();

    // Encode source to destination
    size_t destLen = 0;
    to_crlf(srcBuf, srcLen, destBuf, destSize, &destLen);
    aDestStr.assign(destStr, 0, destLen);
    return 0;
}


int DwToLfEol(const DwString& aSrcStr, DwString& aDestStr)
{
    size_t srcLen = aSrcStr.length();
    const char* srcBuf = aSrcStr.data();
    size_t destSize = srcLen;
    
    // Allocate destination buffer
    DwString destStr(destSize, (char)0);
    char* destBuf = (char*) destStr.data();

    // Encode source to destination
    size_t destLen = 0;
    to_lf(srcBuf, srcLen, destBuf, destSize, &destLen);
    aDestStr.assign(destStr, 0, destLen);
    return 0;
}


int DwToCrEol(const DwString& aSrcStr, DwString& aDestStr)
{
    size_t srcLen = aSrcStr.length();
    const char* srcBuf = aSrcStr.data();
    size_t destSize = srcLen;
    
    // Allocate destination buffer
    DwString destStr(destSize, (char)0);
    char* destBuf = (char*) destStr.data();

    // Encode source to destination
    size_t destLen = 0;
    to_cr(srcBuf, srcLen, destBuf, destSize, &destLen);
    aDestStr.assign(destStr, 0, destLen);
    return 0;
}


int DwToLocalEol(const DwString& aSrcStr, DwString& aDestStr)
{
#if defined(DW_EOL_CRLF)
    return DwToCrLfEol(aSrcStr, aDestStr);
#elif defined(DW_EOL_LF)
    return DwToLfEol(aSrcStr, aDestStr);
#else
#   error "Must define DW_EOL_CRLF, DW_EOL_LF"
#endif
}


int DwEncodeBase64(const DwString& aSrcStr, DwString& aDestStr)
{
    // Estimate required destination buffer size
    size_t srcLen = aSrcStr.length();
    const char* srcBuf = aSrcStr.data();
    size_t destSize = (srcLen+2)/3*4;
    destSize += strlen(DW_EOL)*destSize/72 + 2;
    destSize += 64;  // a little extra room

    // Allocate destination buffer
    DwString destStr(destSize, (char)0);
    char* destBuf = (char*) destStr.data();

    // Encode source to destination
    size_t destLen = 0;
    int result =
    encode_base64(srcBuf, srcLen, destBuf, destSize, &destLen);
    aDestStr.assign(destStr, 0, destLen);
    return result;
}


int DwDecodeBase64(const DwString& aSrcStr, DwString& aDestStr)
{
    // Set destination buffer size same as source buffer size
    size_t srcLen = aSrcStr.length();
    const char* srcBuf = aSrcStr.data();
    size_t destSize = srcLen;

    // Allocate destination buffer
    DwString destStr(destSize, (char)0);
    char* destBuf = (char*) destStr.data();

    // Encode source to destination
    size_t destLen = 0;
    int result =
    decode_base64(srcBuf, srcLen, destBuf, destSize, &destLen);
    aDestStr.assign(destStr, 0, destLen);
    return result;
}


int DwEncodeQuotedPrintable(const DwString& aSrcStr, DwString& aDestStr)
{
    // Estimate required destination buffer size
    size_t srcLen = aSrcStr.length();
    const char* srcBuf = aSrcStr.data();
    size_t destSize = calc_qp_buff_size(srcBuf, srcLen);
    destSize += 64;  // a little extra room

    // Allocate destination buffer
    DwString destStr(destSize, (char)0);
    char* destBuf = (char*) destStr.data();
    
    // Encode source to destination
    size_t destLen = 0;
    int result =
    encode_qp(srcBuf, srcLen, destBuf, destSize, &destLen);
    aDestStr.assign(destStr, 0, destLen);
    return result;
}


int DwDecodeQuotedPrintable(const DwString& aSrcStr, DwString& aDestStr)
{
    // Set destination buffer size same as source buffer size
    size_t srcLen = aSrcStr.length();
    const char* srcBuf = aSrcStr.data();
    size_t destSize = srcLen;
    
    // Allocate destination buffer
    DwString destStr(destSize, (char)0);
    char* destBuf = (char*) destStr.data();
    
    // Encode source to destination
    size_t destLen = 0;
    int result = 
    decode_qp(srcBuf, srcLen, destBuf, destSize, &destLen);
    aDestStr.assign(destStr, 0, destLen);
    return result;
}


//============================================================================
// Everything below this line is private to this file (static)
//============================================================================


static size_t calc_crlf_buff_size(const char* srcBuf, size_t srcLen)
{
    size_t i, extra;

    if (!srcBuf) return 0;
    extra = 0;
    for (i=0; i < srcLen; ) {
        switch (srcBuf[i]) {
        /* Bare LF (UNIX or C text) */
        case '\n':
            ++extra;
            ++i;
            break;
        case '\r':
            /* CR LF (DOS, Windows, or MIME text) */
            if (i+1 < srcLen && srcBuf[i+1] == '\n') {
                i += 2;
            }
            /* Bare CR (Macintosh text) */
            else {
                ++extra;
                ++i;
            }
            break;
        default:
            ++i;
        }
    }
    return srcLen + extra;
}


static int to_crlf(const char* srcBuf, size_t srcLen, char* destBuf,
    size_t destSize, size_t* destLen)
{
    size_t iSrc, iDest;

    if (!srcBuf || !destBuf || !destLen) return -1;
    iSrc = iDest = 0;
    while (iSrc < srcLen && iDest < destSize) {
        switch (srcBuf[iSrc]) {
        /* Bare LF (UNIX or C text) */
        case '\n':
            destBuf[iDest++] = '\r';
            if (iDest < destSize) {
                destBuf[iDest++] = srcBuf[iSrc++];
            }
            break;
        case '\r':
            /* CR LF (DOS, Windows, or MIME text) */
            if (iSrc+1 < srcLen && srcBuf[iSrc+1] == '\n') {
                destBuf[iDest++] = srcBuf[iSrc++];
                if (iDest < destSize) {
                    destBuf[iDest++] = srcBuf[iSrc++];
                }
            }
            /* Bare CR (Macintosh text) */
            else {
                destBuf[iDest++] = srcBuf[iSrc++];
                if (iDest < destSize) {
                    destBuf[iDest++] = '\n';
                }
            }
            break;
        default:
            destBuf[iDest++] = srcBuf[iSrc++];
        }
    }
    *destLen = iDest;
    if (iDest < destSize) {
        destBuf[iDest] = 0;
    }
    return 0;
}


static int to_lf(const char* srcBuf, size_t srcLen, char* destBuf,
    size_t destSize, size_t* destLen)
{
    size_t iSrc, iDest;

    if (!srcBuf || !destBuf || !destLen) return -1;
    iSrc = iDest = 0;
    while (iSrc < srcLen && iDest < destSize) {
        switch (srcBuf[iSrc]) {
        /* Bare LF (UNIX or C text) */
        case '\n':
            destBuf[iDest++] = srcBuf[iSrc++];
            break;
        case '\r':
            /* CR LF (DOS, Windows, or MIME text) */
            if (iSrc+1 < srcLen && srcBuf[iSrc+1] == '\n') {
                ++iSrc;
                destBuf[iDest++] = srcBuf[iSrc++];
            }
            /* Bare CR (Macintosh text) */
            else {
                destBuf[iDest++] = '\n';
                ++iSrc;
            }
            break;
        default:
            destBuf[iDest++] = srcBuf[iSrc++];
        }
    }
    *destLen = iDest;
    if (iDest < destSize) {
        destBuf[iDest] = 0;
    }
    return 0;
}


static int to_cr(const char* srcBuf, size_t srcLen, char* destBuf,
    size_t destSize, size_t* destLen)
{
    size_t iSrc, iDest;

    if (!srcBuf || !destBuf || !destLen) return -1;
    iSrc = iDest = 0;
    while (iSrc < srcLen && iDest < destSize) {
        switch (srcBuf[iSrc]) {
        /* Bare LF (UNIX or C text) */
        case '\n':
            destBuf[iDest++] = '\r';
            ++iSrc;
            break;
        case '\r':
            /* CR LF (DOS, Windows, or MIME text) */
            if (iSrc+1 < srcLen && srcBuf[iSrc+1] == '\n') {
                destBuf[iDest++] = srcBuf[iSrc++];
                ++iSrc;
            }
            /* Bare CR (Macintosh text) */
            else {
                destBuf[iDest++] = srcBuf[iSrc++];
            }
            break;
        default:
            destBuf[iDest++] = srcBuf[iSrc++];
        }
    }
    *destLen = iDest;
    if (iDest < destSize) {
        destBuf[iDest] = 0;
    }
    return 0;
}


static char base64tab[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz0123456789+/";

static char base64idx[128] = {
    '\377','\377','\377','\377','\377','\377','\377','\377',
    '\377','\377','\377','\377','\377','\377','\377','\377',
    '\377','\377','\377','\377','\377','\377','\377','\377',
    '\377','\377','\377','\377','\377','\377','\377','\377',
    '\377','\377','\377','\377','\377','\377','\377','\377',
    '\377','\377','\377',    62,'\377','\377','\377',    63,
        52,    53,    54,    55,    56,    57,    58,    59,
        60,    61,'\377','\377','\377','\377','\377','\377',
    '\377',     0,     1,     2,     3,     4,     5,     6,
         7,     8,     9,    10,    11,    12,    13,    14,
        15,    16,    17,    18,    19,    20,    21,    22,
        23,    24,    25,'\377','\377','\377','\377','\377',
    '\377',    26,    27,    28,    29,    30,    31,    32,
        33,    34,    35,    36,    37,    38,    39,    40,
        41,    42,    43,    44,    45,    46,    47,    48,
        49,    50,    51,'\377','\377','\377','\377','\377'
};

static char hextab[] = "0123456789ABCDEF";

#ifdef __cplusplus
inline int isbase64(int a) {
    return ('A' <= a && a <= 'Z')
        || ('a' <= a && a <= 'z')
        || ('0' <= a && a <= '9')
        || a == '+' || a == '/';
}
#else
#define isbase64(a) (  ('A' <= (a) && (a) <= 'Z') \
                    || ('a' <= (a) && (a) <= 'z') \
                    || ('0' <= (a) && (a) <= '9') \
                    ||  (a) == '+' || (a) == '/'  )
#endif


static int encode_base64(const char* aIn, size_t aInLen, char* aOut,
    size_t aOutSize, size_t* aOutLen)
{
    if (!aIn || !aOut || !aOutLen)
        return -1;
    size_t inLen = aInLen;
    char* out = aOut;
    size_t outSize = (inLen+2)/3*4;     /* 3:4 conversion ratio */
    outSize += strlen(DW_EOL)*outSize/MAXLINE + 2;  /* Space for newlines and NUL */
    if (aOutSize < outSize)
        return -1;
    size_t inPos  = 0;
    size_t outPos = 0;
    int c1, c2, c3;
    int lineLen = 0;
    /* Get three characters at a time and encode them. */
    for (size_t i=0; i < inLen/3; ++i) {
        c1 = aIn[inPos++] & 0xFF;
        c2 = aIn[inPos++] & 0xFF;
        c3 = aIn[inPos++] & 0xFF;
        out[outPos++] = base64tab[(c1 & 0xFC) >> 2];
        out[outPos++] = base64tab[((c1 & 0x03) << 4) | ((c2 & 0xF0) >> 4)];
        out[outPos++] = base64tab[((c2 & 0x0F) << 2) | ((c3 & 0xC0) >> 6)];
        out[outPos++] = base64tab[c3 & 0x3F];
		lineLen += 4;
        if (lineLen >= MAXLINE-3) {
			const char* cp = DW_EOL;
            out[outPos++] = *cp++;
			if (*cp) {
				out[outPos++] = *cp;
			}
			lineLen = 0;
        }
    }
    /* Encode the remaining one or two characters. */
	const char* cp;
    switch (inLen % 3) {
    case 0:
		cp = DW_EOL;
        out[outPos++] = *cp++;
		if (*cp) {
			out[outPos++] = *cp;
		}
        break;
    case 1:
        c1 = aIn[inPos] & 0xFF;
        out[outPos++] = base64tab[(c1 & 0xFC) >> 2];
        out[outPos++] = base64tab[((c1 & 0x03) << 4)];
        out[outPos++] = '=';
        out[outPos++] = '=';
		cp = DW_EOL;
        out[outPos++] = *cp++;
		if (*cp) {
			out[outPos++] = *cp;
		}
        break;
    case 2:
        c1 = aIn[inPos++] & 0xFF;
        c2 = aIn[inPos] & 0xFF;
        out[outPos++] = base64tab[(c1 & 0xFC) >> 2];
        out[outPos++] = base64tab[((c1 & 0x03) << 4) | ((c2 & 0xF0) >> 4)];
        out[outPos++] = base64tab[((c2 & 0x0F) << 2)];
        out[outPos++] = '=';
		cp = DW_EOL;
        out[outPos++] = *cp++;
		if (*cp) {
			out[outPos++] = *cp;
		}
        break;
    }
    out[outPos] = 0;
    *aOutLen = outPos;
    return 0;
}


static int decode_base64(const char* aIn, size_t aInLen, char* aOut,
    size_t aOutSize, size_t* aOutLen)
{
    if (!aIn || !aOut || !aOutLen)
        return -1;
    size_t inLen = aInLen;
    char* out = aOut;
    size_t outSize = ( ( inLen + 3 ) / 4 ) * 3;
    if (aOutSize < outSize)
        return -1;
    /* Get four input chars at a time and decode them. Ignore white space
     * chars (CR, LF, SP, HT). If '=' is encountered, terminate input. If
     * a char other than white space, base64 char, or '=' is encountered,
     * flag an input error, but otherwise ignore the char.
     */
    int isErr = 0;
    int isEndSeen = 0;
    int b1, b2, b3;
    int a1, a2, a3, a4;
    size_t inPos = 0;
    size_t outPos = 0;
    while (inPos < inLen) {
        a1 = a2 = a3 = a4 = 0;
        while (inPos < inLen) {
            a1 = aIn[inPos++] & 0xFF;
            if (isbase64(a1)) {
                break;
            }
            else if (a1 == '=') {
                isEndSeen = 1;
                break;
            }
            else if (a1 != '\r' && a1 != '\n' && a1 != ' ' && a1 != '\t') {
                isErr = 1;
            }
        }
        while (inPos < inLen) {
            a2 = aIn[inPos++] & 0xFF;
            if (isbase64(a2)) {
                break;
            }
            else if (a2 == '=') {
                isEndSeen = 1;
                break;
            }
            else if (a2 != '\r' && a2 != '\n' && a2 != ' ' && a2 != '\t') {
                isErr = 1;
            }
        }
        while (inPos < inLen) {
            a3 = aIn[inPos++] & 0xFF;
            if (isbase64(a3)) {
                break;
            }
            else if (a3 == '=') {
                isEndSeen = 1;
                break;
            }
            else if (a3 != '\r' && a3 != '\n' && a3 != ' ' && a3 != '\t') {
                isErr = 1;
            }
        }
        while (inPos < inLen) {
            a4 = aIn[inPos++] & 0xFF;
            if (isbase64(a4)) {
                break;
            }
            else if (a4 == '=') {
                isEndSeen = 1;
                break;
            }
            else if (a4 != '\r' && a4 != '\n' && a4 != ' ' && a4 != '\t') {
                isErr = 1;
            }
        }
        if (isbase64(a1) && isbase64(a2) && isbase64(a3) && isbase64(a4)) {
            a1 = base64idx[a1] & 0xFF;
            a2 = base64idx[a2] & 0xFF;
            a3 = base64idx[a3] & 0xFF;
            a4 = base64idx[a4] & 0xFF;
            b1 = ((a1 << 2) & 0xFC) | ((a2 >> 4) & 0x03);
            b2 = ((a2 << 4) & 0xF0) | ((a3 >> 2) & 0x0F);
            b3 = ((a3 << 6) & 0xC0) | ( a4       & 0x3F);
            out[outPos++] = char(b1);
            out[outPos++] = char(b2);
            out[outPos++] = char(b3);
        }
        else if (isbase64(a1) && isbase64(a2) && isbase64(a3) && a4 == '=') {
            a1 = base64idx[a1] & 0xFF;
            a2 = base64idx[a2] & 0xFF;
            a3 = base64idx[a3] & 0xFF;
            b1 = ((a1 << 2) & 0xFC) | ((a2 >> 4) & 0x03);
            b2 = ((a2 << 4) & 0xF0) | ((a3 >> 2) & 0x0F);
            out[outPos++] = char(b1);
            out[outPos++] = char(b2);
            break;
        }
        else if (isbase64(a1) && isbase64(a2) && a3 == '=' && a4 == '=') {
            a1 = base64idx[a1] & 0xFF;
            a2 = base64idx[a2] & 0xFF;
            b1 = ((a1 << 2) & 0xFC) | ((a2 >> 4) & 0x03);
            out[outPos++] = char(b1);
            break;
        }
        else {
            break;
        }
        if (isEndSeen) {
            break;
        }
    } /* end while loop */
    *aOutLen = outPos;
    return (isErr) ? -1 : 0;
}


/***** Warning: calc_qp_buff_size must stay in sync with encode_qp ******/

static int encode_qp(const char* aIn, size_t aInLen, char* aOut,
    size_t /*aOutSize */, size_t* aOutLen)
{
    size_t inPos, outPos, lineLen;
    int ch;

    if (!aIn || !aOut || !aOutLen) {
        return -1;
    }
    inPos  = 0;
    outPos = 0;
    lineLen = 0;
    while (inPos < aInLen) {
        ch = aIn[inPos++] & 0xFF;
        /* '.' at beginning of line (confuses some SMTPs) */
        if (lineLen == 0 && ch == '.') {
            aOut[outPos++] = '=';
            aOut[outPos++] = hextab[(ch >> 4) & 0x0F];
            aOut[outPos++] = hextab[ch & 0x0F];
            lineLen += 3;
        }
        /* "From " at beginning of line (gets mangled in mbox folders) */
        else if (lineLen == 0 && inPos+3 < aInLen && ch == 'F'
                 && aIn[inPos  ] == 'r' && aIn[inPos+1] == 'o'
                 && aIn[inPos+2] == 'm' && aIn[inPos+3] == ' ') {
            aOut[outPos++] = '=';
            aOut[outPos++] = hextab[(ch >> 4) & 0x0F];
            aOut[outPos++] = hextab[ch & 0x0F];
            lineLen += 3;
        }
        /* Normal printable char */
        else if ((62 <= ch && ch <= 126) || (33 <= ch && ch <= 60)) {
            aOut[outPos++] = (char) ch;
            ++lineLen;
        }
        /* Space */
        else if (ch == ' ') {
            /* Space at end of line or end of input must be encoded */
#if defined(DW_EOL_LF)
            if (inPos >= aInLen           /* End of input? */
                || aIn[inPos] == '\n') {  /* End of line? */

                aOut[outPos++] = '=';
                aOut[outPos++] = '2';
                aOut[outPos++] = '0';
                lineLen += 3;
            }
#elif defined(DW_EOL_CRLF)
			if (inPos >= aInLen           /* End of input? */
				|| (inPos < aInLen-1      /* End of line? */
				    && aIn[inPos  ] == '\r' 
					&& aIn[inPos+1] == '\n') ) {

                aOut[outPos++] = '=';
                aOut[outPos++] = '2';
                aOut[outPos++] = '0';
                lineLen += 3;
			}
#else
# error Must define DW_EOL_LF or DW_EOL_CRLF
#endif
            else {
                aOut[outPos++] = ' ';
                ++lineLen;
            }
        }
        /* Hard line break */
#if defined(DW_EOL_LF)
        else if (ch == '\n') {
            aOut[outPos++] = '\n';
            lineLen = 0;
        }
#elif defined(DW_EOL_CRLF)
        else if (inPos < aInLen && ch == '\r' && aIn[inPos] == '\n') {
            ++inPos;
            aOut[outPos++] = '\r';
            aOut[outPos++] = '\n';
            lineLen = 0;
        }
#endif
        /* Non-printable char */
        else if (ch & 0x80        /* 8-bit char */
                 || !(ch & 0xE0)  /* control char */
                 || ch == 0x7F    /* DEL */
                 || ch == '=') {  /* special case */
            aOut[outPos++] = '=';
            aOut[outPos++] = hextab[(ch >> 4) & 0x0F];
            aOut[outPos++] = hextab[ch & 0x0F];
            lineLen += 3;
        }
        /* Soft line break */
#if defined(DW_EOL_LF)
        if (lineLen >= MAXLINE-3 && inPos < aInLen && aIn[inPos] != '\n') {
            aOut[outPos++] = '=';
            aOut[outPos++] = '\n';
            lineLen = 0;
        }
#elif defined(DW_EOL_CRLF)
        if (lineLen >= MAXLINE-3 && !(inPos < aInLen-1 && 
			aIn[inPos] == '\r' && aIn[inPos+1] == '\n')) {

            aOut[outPos++] = '=';
            aOut[outPos++] = '\r';
            aOut[outPos++] = '\n';
            lineLen = 0;
        }
#endif
    }
    aOut[outPos] = 0;
    *aOutLen = outPos;
    return 0;
}


static int decode_qp(const char* aIn, size_t aInLen, char* aOut,
    size_t /* aOutSize */, size_t* aOutLen)
{
    size_t i, inPos, outPos, lineLen, nextLineStart, numChars, charsEnd;
    int isEolFound, softLineBrk, isError;
    int ch, c1, c2;

    if (!aIn || !aOut || !aOutLen)
        return -1;
    isError = 0;
    inPos = 0;
    outPos = 0;
    for (i=0; i < aInLen; ++i) {
        if (aIn[i] == 0) {
            aInLen = i;
            break;
        }
    }
    if (aInLen == 0) {
        aOut[0] = 0;
        *aOutLen = 0;
        return 0;
    }
    while (inPos < aInLen) {
        /* Get line */
        lineLen = 0;
        isEolFound = 0;
        while (!isEolFound && lineLen < aInLen - inPos) {
            ch = aIn[inPos+lineLen];
            ++lineLen;
            if (ch == '\n') {
                isEolFound = 1;
            }
        }
        nextLineStart = inPos + lineLen;
        numChars = lineLen;
        /* Remove white space from end of line */
        while (numChars > 0) {
            ch = aIn[inPos+numChars-1] & 0x7F;
            if (ch != '\n' && ch != '\r' && ch != ' ' && ch != '\t') {
                break;
            }
            --numChars;
        }
        charsEnd = inPos + numChars;
        /* Decode line */
        softLineBrk = 0;
        while (inPos < charsEnd) {
            ch = aIn[inPos++] & 0x7F;
            if (ch != '=') {
                /* Normal printable char */
                aOut[outPos++] = (char) ch;
            }
            else /* if (ch == '=') */ {
                /* Soft line break */
                if (inPos >= charsEnd) {
                    softLineBrk = 1;
                    break;
                }
                /* Non-printable char */
                else if (inPos < charsEnd-1) {
                    c1 = aIn[inPos++] & 0x7F;
                    if ('0' <= c1 && c1 <= '9')
                        c1 -= '0';
                    else if ('A' <= c1 && c1 <= 'F')
                        c1 = c1 - 'A' + 10;
                    else if ('a' <= c1 && c1 <= 'f')
                        c1 = c1 - 'a' + 10;
                    else
                        isError = 1;
                    c2 = aIn[inPos++] & 0x7F;
                    if ('0' <= c2 && c2 <= '9')
                        c2 -= '0';
                    else if ('A' <= c2 && c2 <= 'F')
                        c2 = c2 - 'A' + 10;
                    else if ('a' <= c2 && c2 <= 'f')
                        c2 = c2 - 'a' + 10;
                    else
                        isError = 1;
                    aOut[outPos++] = (char) ((c1 << 4) + c2);
                }
                else /* if (inPos == charsEnd-1) */ {
                    isError = 1;
                }
            }
        }
        if (isEolFound && !softLineBrk) {
            const char* cp = DW_EOL;
            aOut[outPos++] = *cp++;
            if (*cp) {
                aOut[outPos++] = *cp;
            }
        }
        inPos = nextLineStart;
    }
    aOut[outPos] = 0;
    *aOutLen = outPos;
    return (isError) ? -1 : 0;
}


/***** Warning: calc_qp_buff_size must stay in sync with encode_qp ******/

static size_t calc_qp_buff_size(const char* aIn, size_t aInLen)
{
    size_t inPos, outLen, lineLen;
    int ch;

    if (!aIn || aInLen == 0) {
        return 0;
    }
    inPos  = 0;
    outLen = 0;
    lineLen = 0;
    while (inPos < aInLen) {
        ch = aIn[inPos++] & 0xFF;
        /* '.' at beginning of line (confuses some SMTPs) */
        if (lineLen == 0 && ch == '.') {
            outLen += 3;
            lineLen += 3;
        }
        /* "From " at beginning of line (gets mangled in mbox folders) */
        else if (lineLen == 0 && inPos+3 < aInLen && ch == 'F'
                 && aIn[inPos  ] == 'r' && aIn[inPos+1] == 'o'
                 && aIn[inPos+2] == 'm' && aIn[inPos+3] == ' ') {
            outLen += 3;
            lineLen += 3;
        }
        /* Normal printable char */
        else if ((62 <= ch && ch <= 126) || (33 <= ch && ch <= 60)) {
            ++outLen;
            ++lineLen;
        }
        /* Space */
        else if (ch == ' ') {
            /* Space at end of line or end of input must be encoded */
#if defined(DW_EOL_LF)
            if (inPos >= aInLen           /* End of input? */
                || aIn[inPos] == '\n') {  /* End of line? */

                outLen += 3;
                lineLen += 3;
            }
#elif defined(DW_EOL_CRLF)
			if (inPos >= aInLen           /* End of input? */
				|| (inPos < aInLen-1      /* End of line? */
				    && aIn[inPos  ] == '\r' 
					&& aIn[inPos+1] == '\n') ) {

                outLen += 3;
                lineLen += 3;
			}
#else
# error Must define DW_EOL_LF or DW_EOL_CRLF
#endif
            else {
                ++outLen;
                ++lineLen;
            }
        }
        /* Hard line break */
#if defined(DW_EOL_LF)
        else if (ch == '\n') {
            ++outLen;
            lineLen = 0;
        }
#elif defined(DW_EOL_CRLF)
        else if (inPos < aInLen && ch == '\r' && aIn[inPos] == '\n') {
            ++inPos;
            outLen += 2;
            lineLen = 0;
        }
#endif
        /* Non-printable char */
        else if (ch & 0x80        /* 8-bit char */
                 || !(ch & 0xE0)  /* control char */
                 || ch == 0x7F    /* DEL */
                 || ch == '=') {  /* special case */
            outLen += 3;
            lineLen += 3;
        }
        /* Soft line break */
#if defined(DW_EOL_LF)
        if (lineLen >= MAXLINE-3 && inPos < aInLen && aIn[inPos] != '\n') {
            outLen += 2;
            lineLen = 0;
        }
#elif defined(DW_EOL_CRLF)
        if (lineLen >= MAXLINE-3 && !(inPos < aInLen-1 && 
			aIn[inPos] == '\r' && aIn[inPos+1] == '\n')) {

            outLen += 3;
            lineLen = 0;
        }
#endif
    }
    return outLen;
}

