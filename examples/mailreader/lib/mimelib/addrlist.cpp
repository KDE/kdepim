//=============================================================================
// File:       addrlist.cpp
// Contents:   Definitions for DwAddressList
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
#include <mimelib/addrlist.h>
#include <mimelib/token.h>
#include <mimelib/group.h>
#include <mimelib/mailbox.h>

const char* const DwAddressList::sClassName = "DwAddressList";


DwAddressList* (*DwAddressList::sNewAddressList)(const DwString&,
    DwMessageComponent*) = 0;


DwAddressList* DwAddressList::NewAddressList(const DwString& aStr,
    DwMessageComponent* aParent)
{
    if (sNewAddressList) {
        return sNewAddressList(aStr, aParent);
    }
    else {
        return new DwAddressList(aStr, aParent);
    }
}


DwAddressList::DwAddressList()
{
    mFirstAddress = 0;
    mClassId = kCidAddressList;
    mClassName = sClassName;
}


DwAddressList::DwAddressList(const DwAddressList& aList)
  : DwFieldBody(aList)
{
    mFirstAddress = 0;
    const DwAddress* addr = aList.mFirstAddress;
    if (addr) {
        CopyList(addr);
    }
    mClassId = kCidAddressList;
    mClassName = sClassName;
}


DwAddressList::DwAddressList(const DwString& aStr, DwMessageComponent* aParent)
  : DwFieldBody(aStr, aParent)
{
    mFirstAddress = 0;
    mClassId = kCidAddressList;
    mClassName = sClassName;
}


DwAddressList::~DwAddressList()
{
    if (mFirstAddress) {
        DeleteAll();
    }
}


const DwAddressList& DwAddressList::operator = (const DwAddressList& aList)
{
    if (this == &aList) return *this;
    DwFieldBody::operator = (aList);
    if (mFirstAddress) {
        DeleteAll();
    }
    const DwAddress* addr = aList.mFirstAddress;
    if (addr) {
        CopyList(addr);
    }
    return *this;
}


DwMessageComponent* DwAddressList::Clone() const
{
    return new DwAddressList(*this);
}


DwAddress* DwAddressList::FirstAddress() const
{
    return mFirstAddress;
}


void DwAddressList::Add(DwAddress* aAddr)
{
    aAddr->SetNext(0);
    aAddr->SetParent(this);
    if (!mFirstAddress) {
        mFirstAddress = aAddr;
    }
    else {
        DwAddress* addr = mFirstAddress;
        while (addr->Next()) {
            addr = (DwAddress*) addr->Next();
        }
        addr->SetNext(aAddr);
    }
    SetModified();
}


void DwAddressList::Remove(DwAddress* aAddr)
{
    DwAddress* addr = mFirstAddress;
    if (addr == aAddr) {
        mFirstAddress = (DwAddress*) addr->Next();
        aAddr->SetNext(0);
        return;
    }
    while (addr) {
        if (addr->Next() == aAddr) {
            addr->SetNext(aAddr->Next());
            aAddr->SetNext(0);
            break;
        }
    }
    SetModified();
}


void DwAddressList::DeleteAll()
{
    DwAddress* addr = mFirstAddress;
    while (addr) {
        DwAddress* nextAddr = (DwAddress*) addr->Next();
        delete addr;
        addr = nextAddr;
    }
    mFirstAddress = 0;
}


void DwAddressList::Parse()
{
    mIsModified = 0;
    if (mFirstAddress) {
        DeleteAll();
    }
    DwAddressListParser parser(mString);
    DwAddress* address;
    while (1) {
        switch (parser.AddrType()) {
        case DwAddressListParser::eAddrError:
        case DwAddressListParser::eAddrEnd:
            goto LOOP_EXIT;
        case DwAddressListParser::eAddrMailbox:
            address = DwMailbox::NewMailbox(parser.AddrString(), this);
            address->Parse();
            if (address->IsValid()) {
                Add(address);
            }
            else {
                delete address;
            }
            break;
        case DwAddressListParser::eAddrGroup:
            address = DwGroup::NewGroup(parser.AddrString(), this);
            address->Parse();
            if (address->IsValid()) {
                Add(address);
            }
            else {
                delete address;
            }
            break;
        case DwAddressListParser::eAddrNull:
            break;
        }
        ++parser;
    }
LOOP_EXIT:
    return;
}


void DwAddressList::Assemble()
{
    if (!mIsModified) return;
    mString = "";
    int count = 0;
    DwAddress* addr = mFirstAddress;
    while (addr) {
        addr->Assemble();
        if (addr->IsValid()) {
            if (count > 0){
                if (IsFolding()) {
                    mString += "," DW_EOL " ";
                }
                else {
                    mString += ", ";
                }
            }
            mString += addr->AsString();
            ++count;
        }
        addr = (DwAddress*) addr->Next();
    }
    mIsModified = 0;
}


void DwAddressList::CopyList(const DwAddress* aFirstAddr)
{
    const DwAddress* addr = aFirstAddr;
    while (addr) {
        DwAddress* newAddr = (DwAddress*) addr->Clone();
        Add(newAddr);
        addr = (const DwAddress*) addr->Next();
    }
}


#if defined (DW_DEBUG_VERSION)
void DwAddressList::PrintDebugInfo(std::ostream& aStrm, int aDepth/*=0*/) const
{
    aStrm <<
    "-------------- Debug info for DwAddressList class --------------\n";
    _PrintDebugInfo(aStrm);
    int depth = aDepth - 1;
    depth = (depth >= 0) ? depth : 0;
    if (aDepth == 0 || depth > 0) {
        DwAddress* addr = mFirstAddress;
        while (addr) {
            addr->PrintDebugInfo(aStrm, depth);
            addr = addr->Next();
        }
    }
}
#else
void DwAddressList::PrintDebugInfo(std::ostream&, int) const {}
#endif // defined (DW_DEBUG_VERSION)


#if defined (DW_DEBUG_VERSION)
void DwAddressList::_PrintDebugInfo(std::ostream& aStrm) const
{
    DwFieldBody::_PrintDebugInfo(aStrm);
    aStrm << "Address objects:  ";
    DwAddress* addr = mFirstAddress;
    if (addr) {
        int count = 0;
        while (addr) {
            if (count > 0) aStrm << ' ';
            aStrm << addr->ObjectId();
            addr = addr->Next();
            ++count;
        }
        aStrm << '\n';
    }
    else {
        aStrm << "(none)\n";
    }
}
#else
void DwAddressList::_PrintDebugInfo(std::ostream&) const {}
#endif // defined (DW_DEBUG_VERSION)


void DwAddressList::CheckInvariants() const
{
#if defined (DW_DEBUG_VERSION)
    DwAddress* addr = mFirstAddress;
    while (addr) {
        addr->CheckInvariants();
        assert((DwMessageComponent*) this == addr->Parent());
        addr = addr->Next();
    }
#endif // defined (DW_DEBUG_VERSION)
}


//-------------------------------------------------------------------------


DwAddressListParser::DwAddressListParser(const DwString& aStr)
  : mTokenizer(aStr),
    mAddrString(aStr)
{
    mAddrType = eAddrError;
    ParseNextAddress();
}


DwAddressListParser::~DwAddressListParser()
{
}


int DwAddressListParser::Restart()
{
    mTokenizer.Restart();
    ParseNextAddress();
    return mAddrType;
}


int DwAddressListParser::operator ++ ()
{
    ParseNextAddress();
    return mAddrType;
}


void DwAddressListParser::ParseNextAddress()
{
    mAddrString.SetFirst(mTokenizer);
    mAddrType = eAddrEnd;
    int type = mTokenizer.Type();
    if (type == eTkNull) {
        return;
    }
    enum {
        eTopLevel,
        eInGroup,
        eInRouteAddr
    } state;
    state = eTopLevel;
    // The type will be a mailbox, unless we discover otherwise
    mAddrType = eAddrMailbox;
    int done = 0;
    while (!done) {
        if (type == eTkNull) {
            mAddrString.ExtendTo(mTokenizer);
            break;
        }
        else if (type == eTkSpecial) {
            int ch = mTokenizer.Token()[0];
            switch (state) {
            case eTopLevel:
                switch (ch) {
                case ',':
                    mAddrString.ExtendTo(mTokenizer);
                    done = 1;
                    break;
                case '<':
                    state = eInRouteAddr;
                    break;
                case ':':
                    mAddrType = eAddrGroup;
                    state = eInGroup;
                    break;
                }
                break;
            case eInRouteAddr:
                switch (ch) {
                case '>':
                    state = eTopLevel;
                    break;
                }
                break;
            case eInGroup:
                switch (ch) {
                case ';':
                    state = eTopLevel;
                    break;
                }
                break;
            }
        }
        ++mTokenizer;
        type = mTokenizer.Type();
    }
    if (mAddrString.Tokens().length() == 0) {
        mAddrType = eAddrNull;
    }
}
