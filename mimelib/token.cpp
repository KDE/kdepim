//=============================================================================
// File:       token.cpp
// Contents:   Definitions for DwTokenizer, DwRfc822Tokenizer
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
#include <mimelib/debug.h>
#include <assert.h>
#include <ctype.h>
#include <mimelib/string.h>
#include <mimelib/token.h>


std::ostream* DwTokenizer::mDebugOut = 0;


DwTokenizer::DwTokenizer(const DwString& aStr)
  : mString(aStr)
{
    mTokenStart  = 0;
    mTokenLength = 0;
    mNextStart   = 0;
    mTkType      = eTkError;
}


DwTokenizer::DwTokenizer(const char* aCStr)
  : mString(aCStr)
{
    mTokenStart  = 0;
    mTokenLength = 0;
    mNextStart   = 0;
    mTkType      = eTkError;
}


DwTokenizer::~DwTokenizer()
{
}


void DwTokenizer::StripDelimiters()
{
    if (mTokenLength < 2) return;
    // const ref -- avoids copy on write when using operator[]
    const DwString& token = mToken;
    switch (mTkType) {
    case eTkQuotedString:
        if (token[0] == '"') {
            mToken = mToken.substr(1);
            ++mTokenStart;
            --mTokenLength;
        }
        if (mTokenLength > 0 && token[mTokenLength-1] == '"') {
            mToken = mToken.substr(0, mTokenLength-1);
            --mTokenLength;
        }
        break;
    case eTkDomainLiteral:
        if (token[0] == '[') {
            mToken = mToken.substr(1);
            ++mTokenStart;
            --mTokenLength;
        }
        if (mTokenLength > 0 && token[mTokenLength-1] == ']') {
            mToken = mToken.substr(0, mTokenLength-1);
            --mTokenLength;
        }
        break;
    case eTkComment:
        if (token[0] == '(') {
            mToken = mToken.substr(1);
            ++mTokenStart;
            --mTokenLength;
        }
        if (mTokenLength > 0 && token[mTokenLength-1] == ')') {
            mToken = mToken.substr(0, mTokenLength-1);
            --mTokenLength;
        }
        break;
    }
}


void DwTokenizer::ParseQuotedString()
{
    size_t pos = mTokenStart;
    while (1) {
        ++pos;
        if (pos >= mString.length()) {
            // Ran out of string
            mTokenLength = 0;
            mToken = "";
            mNextStart = pos;
            mTkType = eTkError;
            break;
        }
        else if (mString[pos] == '\\') {
            // Quoted character
            ++pos;
            if (pos >= mString.length()) {
                // Ran out of string
                mTokenLength = 0;
                mToken = "";
                mNextStart = pos;
                mTkType = eTkError;
                break;
            }
        }
        else if (mString[pos] == '"') {
            // End of quoted string
            ++pos;
            mTokenLength = pos - mTokenStart;
            mToken = mString.substr(mTokenStart, mTokenLength);
            mNextStart = pos;
            break;
        }
    }
}


void DwTokenizer::ParseComment()
{
    size_t pos = mTokenStart;
    int level = 1;
    while (1) {
        ++pos;
        if (pos >= mString.length()) {
            // Ran out of string
            mTokenLength = 0;
            mToken = "";
            mNextStart = pos;
            mTkType = eTkError;
            break;
        }
        else if (mString[pos] == '\\') {
            // Quoted character
            ++pos;
            if (pos >= mString.length()) {
                // Ran out of string
                mTokenLength = 0;
                mToken = "";
                mNextStart = pos;
                mTkType = eTkError;
                break;
            }
        }
        else if (mString[pos] == ')') {
            --level;
            if (level == 0) {
                // End of comment
                ++pos;
                mTokenLength = pos - mTokenStart;
                mToken = mString.substr(mTokenStart, mTokenLength);
                mNextStart = pos;
                break;
            }
        }
        else if (mString[pos] == '(') {
            ++level;
        }
    }
}


void DwTokenizer::ParseDomainLiteral()
{
    size_t pos = mTokenStart;
    while (1) {
        ++pos;
        if (pos >= mString.length()) {
            // Ran out of string
            mTokenLength = 0;
            mToken = "";
            mNextStart = pos;
            mTkType = eTkError;
            break;
        }
        else if (mString[pos] == '\\') {
            // Quoted character
            ++pos;
            if (pos >= mString.length()) {
                // Ran out of string
                mTokenLength = 0;
                mToken = "";
                mNextStart = pos;
                mTkType = eTkError;
                break;
            }
        }
        else if (mString[pos] == ']') {
            // End of domain literal
            ++pos;
            mTokenLength = pos - mTokenStart;
            mToken = mString.substr(mTokenStart, mTokenLength);
            mNextStart = pos;
            break;
        }
    }
}


void DwTokenizer::PrintToken(std::ostream* aOut)
{
    if (!aOut) return;
    const char* type = 0;
    switch (mTkType) {
    case eTkError:
        type = "error          ";
        break;
    case eTkNull:
        type = "null           ";
        break;
    case eTkSpecial:
        type = "special        ";
        break;
    case eTkAtom:
        type = "atom           ";
        break;
    case eTkComment:
        type = "comment        ";
        break;
    case eTkQuotedString:
        type = "quoted string  ";
        break;
    case eTkDomainLiteral:
        type = "domain literal ";
        break;
    case eTkTspecial:
        type = "tspecial       ";
        break;
    case eTkToken:
        type = "token          ";
        break;
    default:
        type = "unknown        ";
        break;
    }
    *aOut << type << mToken << '\n';
}


static inline bool isspecialorspaceorcntrl( int c ) 
{
  switch ( c ) {
     case '(':
     case ')':
     case '<':
     case '>':
     case '@':
     case ',':
     case ';':
     case ':':
     case '\\':
     case '"':
     case '.':
     case '[':
     case ']':
     // isspace()
     case ' ':
         return true;
     //case '\r': included in iscntrl()
     //case '\f': included in iscntrl()
     //case '\t': included in iscntrl()
     //case '\n': included in iscntrl()
     //case '\v': included in iscntrl()
     // iscntrl()
     default:
         return ( (c >= 0 && c <= 15) || (c >= 17 && c <= 31) );
  }
}

static inline bool isnotspaceorcntrl( int c ) 
{
  switch ( c ) {
     // isspace()
     case ' ':
     //case '\r': included in iscntrl()
     //case '\f': included in iscntrl()
     //case '\t': included in iscntrl()
     //case '\n': included in iscntrl()
     //case '\v': included in iscntrl()
     // iscntrl()
         return false;
     default:
         return !( (c >= 0 && c <= 15) || (c >= 17 && c <= 31) );
  }
}

DwRfc822Tokenizer::DwRfc822Tokenizer(const DwString& aStr)
  : DwTokenizer(aStr)
{
    ParseToken();
}


DwRfc822Tokenizer::DwRfc822Tokenizer(const char* aCStr)
  : DwTokenizer(aCStr)
{
    ParseToken();
}


DwRfc822Tokenizer::~DwRfc822Tokenizer()
{
}


int DwRfc822Tokenizer::Restart()
{
    mNextStart = 0;
    ParseToken();
    return mTkType;
}


int DwRfc822Tokenizer::operator ++ ()
{
    ParseToken();
    return mTkType;
}


void DwRfc822Tokenizer::ParseToken()
{
    // Assume the field body has already been extracted.  That is, we don't
    // have to watch for the end of the field body or folding.  We just
    // treat any CRs or LFs as white space.
    mTokenStart = mNextStart;
    mTokenLength = 0;
    mTkType = eTkNull;
    // Skip leading space.  Also, since control chars are not permitted
    // in atoms, skip these, too.
    while (1) {
        if (mTokenStart >= mString.length()) {
            return;
        }
        if (isnotspaceorcntrl(mString[mTokenStart]))
            break;
        ++mTokenStart;
    }
    char ch = mString[mTokenStart];
    switch (ch) {
        // Quoted string
        case '"':
            mTkType = eTkQuotedString;
            ParseQuotedString();
        break;
        // Comment
        case '(':
            mTkType = eTkComment;
            ParseComment();
        break;
        // Domain literal
        case '[':
            mTkType = eTkDomainLiteral;
            ParseDomainLiteral();
        break;
        // Special
        case ')':
        case '<':
        case '>':
        case '@':
        case ',':
        case ';':
        case ':':
        case '\\':
        case '.':
        case ']':
            mTkType = eTkSpecial;
            mTokenLength = 1;
            mToken = mString.substr(mTokenStart, 1);
            mNextStart = mTokenStart + 1;
        break;
        default:
            mTkType = eTkAtom;
            ParseAtom();
        break;
    }
    if (mDebugOut) PrintToken(mDebugOut);
}


void DwRfc822Tokenizer::ParseAtom()
{
    size_t pos = mTokenStart;
    while (1) {
        ++pos;
        char ch = (pos < mString.length()) ? mString[pos] : (char) 0;
        if (pos >= mString.length()
            || isspecialorspaceorcntrl(ch)) {

            mTokenLength = pos - mTokenStart;
            mToken = mString.substr(mTokenStart, mTokenLength);
            mNextStart = pos;
            break;
        }
    }
}

static inline bool istspecialorspaceorcntrl( int c ) 
{
  switch ( c ) {
     case '(':
     case ')':
     case '<':
     case '>':
     case '@':
     case ',':
     case ';':
     case ':':
     case '\\':
     case '"':
     case '/':
     case '[':
     case ']':
     case '?':
     case '=':
     // isspace()
     case ' ':
         return true;
     //case '\r': included in iscntrl()
     //case '\f': included in iscntrl()
     //case '\t': included in iscntrl()
     //case '\n': included in iscntrl()
     //case '\v': included in iscntrl()
     // iscntrl()
     default:
        return ( ( c >= 0 && c <= 15) || (c >= 17 && c <= 31) );
  }
 }

DwRfc1521Tokenizer::DwRfc1521Tokenizer(const DwString& aStr)
  : DwTokenizer(aStr)
{
    ParseToken();
}


DwRfc1521Tokenizer::DwRfc1521Tokenizer(const char* aCStr)
  : DwTokenizer(aCStr)
{
    ParseToken();
}


DwRfc1521Tokenizer::~DwRfc1521Tokenizer()
{
}


int DwRfc1521Tokenizer::Restart()
{
    mNextStart = 0;
    ParseToken();
    return mTkType;
}


int DwRfc1521Tokenizer::operator ++ ()
{
    ParseToken();
    return mTkType;
}


void DwRfc1521Tokenizer::ParseToken()
{
    // Assume the field body has already been extracted.  That is, we don't
    // have to watch for the end of the field body or folding.  We just
    // treat any CRs or LFs as white space.
    mTokenStart = mNextStart;
    mTokenLength = 0;
    mTkType = eTkNull;
    // Skip leading space.  Also, since control chars are not permitted
    // in atoms, skip these, too.
    while (1) {
        if (mTokenStart >= mString.length()) {
            return;
        }
        if (isnotspaceorcntrl(mString[mTokenStart]))
            break;
        ++mTokenStart;
    }
    char ch = mString[mTokenStart];
    switch (ch) {
        // Quoted string
        case '"':
            mTkType = eTkQuotedString;
            ParseQuotedString();
        break;
        // Comment
        case '(':
            mTkType = eTkComment;
            ParseComment();
        break;
        // Domain literal
        case '[':
            mTkType = eTkDomainLiteral;
            ParseDomainLiteral();
        break;
        // Special
        case ')':
        case '<':
        case '>':
        case '@':
        case ',':
        case ';':
        case ':':
        case '\\':
        case '/':
        case ']':
        case '?':
        case '=':
            mTkType = eTkTspecial;
            mTokenLength = 1;
            mToken = mString.substr(mTokenStart, 1);
            mNextStart = mTokenStart + 1;
        break;
        default:
            mTkType = eTkToken;
            ParseAtom();
        break;
    }
    if (mDebugOut) PrintToken(mDebugOut);
}


void DwRfc1521Tokenizer::ParseAtom()
{
    size_t pos = mTokenStart;
    while (1) {
        ++pos;
        char ch = (pos < mString.length()) ? mString[pos] : (char) 0;
        if (pos >= mString.length()
            || istspecialorspaceorcntrl(ch)) {

            mTokenLength = pos - mTokenStart;
            mToken = mString.substr(mTokenStart, mTokenLength);
            mNextStart = pos;
            break;
        }
    }
}


DwTokenString::DwTokenString(const DwString& aStr)
  : mString(aStr)
{
    mTokensStart  = 0;
    mTokensLength = 0;
}


DwTokenString::~DwTokenString()
{
}


void DwTokenString::SetFirst(const DwTokenizer& aTkzr)
{
    switch (aTkzr.Type()) {
    case eTkError:
    case eTkNull:
        mTokensStart  = aTkzr.mTokenStart;
        mTokensLength = 0;
        break;
    case eTkComment:
    case eTkDomainLiteral:
    case eTkQuotedString:
    case eTkSpecial:
    case eTkAtom:
    case eTkTspecial:
    case eTkToken:
        mTokensStart  = aTkzr.mTokenStart;
        mTokensLength = aTkzr.mTokenLength;
        break;
    }
    mTokens = mString.substr(mTokensStart, mTokensLength);
}


void DwTokenString::SetLast(const DwTokenizer& aTkzr)
{
    assert(aTkzr.mTokenStart >= mTokensStart);
    if (aTkzr.mTokenStart < mTokensStart) return;
    mTokensLength = aTkzr.mTokenStart + aTkzr.mTokenLength - mTokensStart;
    mTokens = mString.substr(mTokensStart, mTokensLength);
}


void DwTokenString::ExtendTo(const DwTokenizer& aTkzr)
{
    assert(aTkzr.mTokenStart >= mTokensStart);
    if (aTkzr.mTokenStart < mTokensStart) return;
    mTokensLength = aTkzr.mTokenStart - mTokensStart;
    mTokens = mString.substr(mTokensStart, mTokensLength);
}
