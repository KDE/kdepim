//=============================================================================
// File:       msgid.cpp
// Contents:   Definitions for DwMsgId
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
#include <stdlib.h>
#include <string.h>
#include <time.h>

// UNIX specific includes

//#if defined(__unix__) || defined(__unix)
#if defined(DW_UNIX)
#  include <unistd.h>
// When building with SS12 but *not* using RW stdcxx, need sysent.h;
// the combination of sysent + stdcxx is fatal (and we don't really
// seem to need sysent.h anyway).
#  if defined(__SUNPRO_CC) && !defined(_RWSTD_REENTRANT)
#    include <sysent.h>
#  endif // defined(__SUNPRO_CC)
#endif // defined (DW_UNIX)

// WIN32 specific includes

#if defined(DW_WIN32)
#  include <windows.h>
#endif // defined(DW_WIN32)

#include <mimelib/string.h>
#include <mimelib/msgid.h>
#include <mimelib/token.h>

#include <k3resolver.h>

static DwUint32 GetPid();


const char* const DwMsgId::sClassName = "DwMsgId";
const char* DwMsgId::sHostName = 0;


DwMsgId* (*DwMsgId::sNewMsgId)(const DwString&, DwMessageComponent*) = 0;


DwMsgId* DwMsgId::NewMsgId(const DwString& aStr, DwMessageComponent* aParent)
{
    if (sNewMsgId) {
        return sNewMsgId(aStr, aParent);
    }
    else {
        return new DwMsgId(aStr, aParent);
    }
}


DwMsgId::DwMsgId()
{
    mClassId = kCidMsgId;
    mClassName = sClassName;
}


DwMsgId::DwMsgId(const DwMsgId& aMsgId)
  : DwFieldBody(aMsgId),
    mLocalPart(aMsgId.mLocalPart),
    mDomain(aMsgId.mDomain)
{
    mClassId = kCidMsgId;
    mClassName = sClassName;
}


DwMsgId::DwMsgId(const DwString& aStr, DwMessageComponent* aParent)
  : DwFieldBody(aStr, aParent)
{
    mClassId = kCidMsgId;
    mClassName = sClassName;
}


DwMsgId::~DwMsgId()
{
}


const DwMsgId& DwMsgId::operator = (const DwMsgId& aMsgId)
{
    if (this == &aMsgId) return *this;
    DwFieldBody::operator = (aMsgId);
    mLocalPart = aMsgId.mLocalPart;
    mDomain = aMsgId.mDomain;
    return *this;
}


const DwString& DwMsgId::LocalPart() const
{
    return mLocalPart;
}


void DwMsgId::SetLocalPart(const DwString& aLocalPart)
{
    mLocalPart = aLocalPart;
    SetModified();
}


const DwString& DwMsgId::Domain() const
{
    return mDomain;
}


void DwMsgId::SetDomain(const DwString& aDomain)
{
    mDomain = aDomain;
    SetModified();
}


void DwMsgId::Parse()
{
    mIsModified = 0;

    int ch;
    DwRfc822Tokenizer tokenizer(mString);

    // Advance to '<'
    int type = tokenizer.Type();
    int found = 0;
    while (!found && type != eTkNull) {
        if (type == eTkSpecial && tokenizer.Token()[0] == '<') {
            found = 1;
        }
        ++tokenizer;
        type = tokenizer.Type();
    }
    // Get the local part
    found = 0;
    while (type != eTkNull && !found) {
        switch (type) {
        case eTkSpecial:
            ch = tokenizer.Token()[0];
            switch (ch) {
            case '@':
                found = 1;
                break;
            case '.':
                mLocalPart += tokenizer.Token();
                break;
            }
            break;
        case eTkAtom:
        case eTkQuotedString:
            mLocalPart += tokenizer.Token();
            break;
        }
        ++tokenizer;
        type = tokenizer.Type();
    }
    // Get the domain
    found = 0;
    while (type != eTkNull && !found) {
        switch (type) {
        case eTkSpecial:
            ch = tokenizer.Token()[0];
            switch (ch) {
            case '>':
                found = 1;
                break;
            case '.':
                mDomain += tokenizer.Token();
                break;
            }
            break;
        case eTkAtom:
            mDomain += tokenizer.Token();
            break;
        case eTkDomainLiteral:
            mDomain += tokenizer.Token();
            break;
        }
        ++tokenizer;
        type = tokenizer.Type();
    }
}


void DwMsgId::Assemble()
{
    if (!mIsModified) return;
    mString = "<";
    mString += mLocalPart;
    mString += "@";
    mString += mDomain;
    mString += ">";
    mIsModified = 0;
}


DwMessageComponent* DwMsgId::Clone() const
{
    return new DwMsgId(*this);
}


static char base35chars[] = "0123456789ABCDEFGHIJKLMNPQRSTUVWXYZ";

void DwMsgId::CreateDefault()
{
    char scratch[80];
    time_t tt = time(NULL);
    struct tm tms = *localtime(&tt);
    int pos = 0;
    scratch[pos++] = '<';
    int n = tms.tm_year;
    scratch[pos++] = char(n / 10   % 10 + '0');
    scratch[pos++] = char(n        % 10 + '0');
    n = tms.tm_mon + 1;
    scratch[pos++] = char(n / 10 % 10 + '0');
    scratch[pos++] = char(n      % 10 + '0');
    n = tms.tm_mday;
    scratch[pos++] = char(n / 10 % 10 + '0');
    scratch[pos++] = char(n      % 10 + '0');
    n = tms.tm_hour;
    scratch[pos++] = char(n / 10 % 10 + '0');
    scratch[pos++] = char(n      % 10 + '0');
    n = tms.tm_min;
    scratch[pos++] = char(n / 10 % 10 + '0');
    scratch[pos++] = char(n      % 10 + '0');
    n = tms.tm_sec;
    scratch[pos++] = char(n / 10 % 10 + '0');
    scratch[pos++] = char(n      % 10 + '0');
    static int counter = 0;
    scratch[pos++] = base35chars[counter/35%35];
    scratch[pos++] = base35chars[counter   %35];
    ++counter;
    scratch[pos++] = '.';
    DwUint32 pid = GetPid();
    scratch[pos++] = char(pid / 10000 % 10 + '0');
    scratch[pos++] = char(pid / 1000  % 10 + '0');
    scratch[pos++] = char(pid / 100   % 10 + '0');
    scratch[pos++] = char(pid / 10    % 10 + '0');
    scratch[pos++] = char(pid         % 10 + '0');
    scratch[pos++] = '@';
    QByteArray hostname = KNetwork::KResolver::localHostName().toLocal8Bit();
    const char* cp = hostname.constData();
    while (*cp && pos < 79) {
        scratch[pos++] = *cp++;
    }
    scratch[pos++] = '>';
    scratch[pos] = 0;
    mString = scratch;
    mIsModified = 0;
    Parse();
}


#if defined (DW_DEBUG_VERSION)
void DwMsgId::PrintDebugInfo(std::ostream& aStrm, int /*aDepth*/) const
{
    aStrm <<
    "----------------- Debug info for DwMsgId class -----------------\n";
    _PrintDebugInfo(aStrm);
}
#else
void DwMsgId::PrintDebugInfo(std::ostream& , int ) const {}
#endif // defined (DW_DEBUG_VERSION)


#if defined (DW_DEBUG_VERSION)
void DwMsgId::_PrintDebugInfo(std::ostream& aStrm) const
{
    DwFieldBody::_PrintDebugInfo(aStrm);
    aStrm << "Local part:       " << mLocalPart << '\n';
    aStrm << "Domain:           " << mDomain    << '\n';
}
#else
void DwMsgId::_PrintDebugInfo(std::ostream& ) const {}
#endif // defined (DW_DEBUG_VERSION)


void DwMsgId::CheckInvariants() const
{
#if defined (DW_DEBUG_VERSION)
    DwFieldBody::CheckInvariants();
    mLocalPart.CheckInvariants();
    mDomain.CheckInvariants();
#endif // defined (DW_DEBUG_VERSION)
}

//============================================================================
// Platform dependent code follows
//============================================================================

//----------------------------------------------------------------------------
// WIN32
//----------------------------------------------------------------------------

#if defined(DW_WIN32)

#ifndef _PID_T_
typedef unsigned pid_t;
#endif

static DwUint32 GetPid()
{
    return GetCurrentProcessId();
}

#endif // defined(DW_WIN32)

//----------------------------------------------------------------------------
// UNIX
//----------------------------------------------------------------------------

#if defined(DW_UNIX)

static DwUint32 GetPid()
{
    return getpid();
}

#endif // defined(DW_UNIX)
