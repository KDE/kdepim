//=============================================================================
// File:       address.cpp
// Contents:   Definitions for DwAddress
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
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <mimelib/address.h>
#include <mimelib/token.h>
#include <mimelib/group.h>
#include <mimelib/mailbox.h>

const char* const DwAddress::sClassName = "DwAddress";


DwAddress::DwAddress()
{
    mIsValid = 0;
    mNext = 0;
    mClassId = kCidAddress;
    mClassName = sClassName;
}


DwAddress::DwAddress(const DwAddress& aAddr)
  : DwFieldBody(aAddr)
{
    mIsValid = aAddr.mIsValid;
    mNext = 0;
    mClassId = kCidAddress;
    mClassName = sClassName;
}


DwAddress::DwAddress(const DwString& aStr, DwMessageComponent* aParent)
  : DwFieldBody(aStr, aParent)
{
    mIsValid = 0;
    mNext = 0;
    mClassId = kCidAddress;
    mClassName = sClassName;
}


DwAddress::~DwAddress()
{
}


const DwAddress& DwAddress::operator = (const DwAddress& aAddr)
{
    if (this == &aAddr) return *this;
    DwFieldBody::operator = (aAddr);
    mIsValid = aAddr.mIsValid;
    return *this;
}


DwBool DwAddress::IsMailbox() const
{
    DwBool r = (mClassId == kCidMailbox) ? 1 : 0;
    return r;
}


DwBool DwAddress::IsGroup() const
{
    DwBool r = (mClassId == kCidGroup) ? 1 : 0;
    return r;
}


DwAddress* DwAddress::Next() const
{
    return mNext;
}


void DwAddress::SetNext(DwAddress* aAddress)
{
    mNext = aAddress;
}


#if defined (DW_DEBUG_VERSION)
void DwAddress::PrintDebugInfo(std::ostream& aStrm, int /*aDepth*/) const
{
    aStrm <<
    "---------------- Debug info for DwAddress class ----------------\n";
    _PrintDebugInfo(aStrm);
}
#else
void DwAddress::PrintDebugInfo(std::ostream&, int ) const {}
#endif // defined (DW_DEBUG_VERSION)


#if defined (DW_DEBUG_VERSION)
void DwAddress::_PrintDebugInfo(std::ostream& aStrm) const
{
    DwFieldBody::_PrintDebugInfo(aStrm);
    aStrm << "IsValid:          ";
    if (mIsValid) {
        aStrm << "True\n";
    }
    else {
        aStrm << "False\n";
    }
    aStrm << "Next address:     ";
    if (mNext) {
        aStrm << mNext->ObjectId() << '\n';
    }
    else {
        aStrm << "(none)\n";
    }
}
#else
void DwAddress::_PrintDebugInfo(std::ostream& ) const {}
#endif // defined (DW_DEBUG_VERSION)


void DwAddress::CheckInvariants() const
{
#if defined (DW_DEBUG_VERSION)
    DwFieldBody::CheckInvariants();
#endif // defined (DW_DEBUG_VERSION)
}
