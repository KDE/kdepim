//=============================================================================
// File:       mailbox.cpp
// Contents:   Definitions for DwMailbox
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
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <mimelib/string.h>
#include <mimelib/mailbox.h>
#include <mimelib/token.h>

void RemoveCrAndLf(DwString& aStr);

const char* const DwMailbox::sClassName = "DwMailbox";


DwMailbox* (*DwMailbox::sNewMailbox)(const DwString&, DwMessageComponent*) = 0;


DwMailbox* DwMailbox::NewMailbox(const DwString& aStr,
    DwMessageComponent* aParent)
{
    if (sNewMailbox) {
        return sNewMailbox(aStr, aParent);
    }
    else {
        return new DwMailbox(aStr, aParent);
    }
}


DwMailbox::DwMailbox()
{
    mClassId = kCidMailbox;
    mClassName = sClassName;
}


DwMailbox::DwMailbox(const DwMailbox& aMailbox)
  : DwAddress(aMailbox),
    mFullName(aMailbox.mFullName),
    mRoute(aMailbox.mRoute),
    mLocalPart(aMailbox.mLocalPart),
    mDomain(aMailbox.mDomain)
{
    mClassId = kCidMailbox;
    mClassName = sClassName;
}


DwMailbox::DwMailbox(const DwString& aStr, DwMessageComponent* aParent)
  : DwAddress(aStr, aParent)
{
    mClassId = kCidMailbox;
    mClassName = sClassName;
}


DwMailbox::~DwMailbox()
{
}


const DwMailbox& DwMailbox::operator = (const DwMailbox& aMailbox)
{
    if (this == &aMailbox) return *this;
    DwAddress::operator = (aMailbox);
    mFullName  = aMailbox.mFullName;
    mRoute     = aMailbox.mRoute;
    mLocalPart = aMailbox.mLocalPart;
    mDomain    = aMailbox.mDomain;
    return *this;
}


const DwString& DwMailbox::FullName() const
{
    return mFullName;
}


void DwMailbox::SetFullName(const DwString& aFullName)
{
    mFullName = aFullName;
    SetModified();
}


const DwString& DwMailbox::Route() const
{
    return mRoute;
}


void DwMailbox::SetRoute(const DwString& aRoute)
{
    mRoute = aRoute;
    SetModified();
}


const DwString& DwMailbox::LocalPart() const
{
    return mLocalPart;
}


void DwMailbox::SetLocalPart(const DwString& aLocalPart)
{
    mLocalPart = aLocalPart;
    SetModified();
}


const DwString& DwMailbox::Domain() const
{
    return mDomain;
}


void DwMailbox::SetDomain(const DwString& aDomain)
{
    mDomain = aDomain;
    SetModified();
}


// Some mailboxes to test
//
//  John Doe <john.doe@acme.com>
//  John@acme.com (John Doe)
//  John.Doe@acme.com (John Doe)
//  John.Doe (Jr) @acme.com (John Doe)
//  John <@domain1.com,@domain2.com:jdoe@acme.com>
//  <jdoe@acme>
//  <@node1.[128.129.130.131],@node2.uu.edu:
//      jdoe(John Doe)@node3.[131.130.129.128]> (John Doe)
//
void DwMailbox::Parse()
{
    mIsModified = 0;
    DwString emptyString("");
    DwString space(" ");
    int isFirstPhraseNull = 1;
    int isSimpleAddress = 1;
    DwString firstPhrase(emptyString);
    DwString lastComment(emptyString);
    mRoute     = emptyString;
    mLocalPart = emptyString;
    mDomain    = emptyString;
    mFullName  = emptyString;
    DwRfc822Tokenizer tokenizer(mString);
    int ch;

    enum {
        eStart,       // start
        eLtSeen,      // less-than-seen
        eInRoute,     // in-route
        eInAddrSpec,  // in-addr-spec
        eAtSeen,      // at-seen
        eGtSeen       // greater-than-seen
    };

    // Start state -- terminated by '<' or '@'

    int type = tokenizer.Type();
    int state = eStart;
    while (state == eStart && type != eTkNull) {
        switch (type) {
        case eTkSpecial:
            ch = tokenizer.Token()[0];
            switch (ch) {
            case '@':
                state = eAtSeen;
                break;
            case '<':
                isSimpleAddress = 0;
                mLocalPart = emptyString;
                state = eLtSeen;
                break;
            case '.':
                mLocalPart += tokenizer.Token();
                break;
            }
            break;
        case eTkAtom:
        case eTkQuotedString:
            if (isFirstPhraseNull) {
                firstPhrase = tokenizer.Token();
                isFirstPhraseNull = 0;
            }
            else {
                firstPhrase += space;
                firstPhrase += tokenizer.Token();
            }
            mLocalPart += tokenizer.Token();
            break;
        case eTkComment:
            tokenizer.StripDelimiters();
            lastComment = tokenizer.Token();
            break;
        }
        ++tokenizer;
        type = tokenizer.Type();
    }

    // Less-than-seen state -- process only one valid token and transit to
    // in-route state or in-addr-spec state

    while (state == eLtSeen && type != eTkNull) {
        switch (type) {
        case eTkSpecial:
            ch = tokenizer.Token()[0];
            switch (ch) {
            case '@':
                // '@' immediately following '<' indicates a route
                mRoute = tokenizer.Token();
                state = eInRoute;
                break;
            }
            break;
        case eTkAtom:
        case eTkQuotedString:
            mLocalPart = tokenizer.Token();
            state = eInAddrSpec;
            break;
        }
        ++tokenizer;
        type = tokenizer.Type();
    }

    // In-route state -- terminated by ':'

    while (state == eInRoute && type != eTkNull) {
        switch (type) {
        case eTkSpecial:
            ch = tokenizer.Token()[0];
            switch (ch) {
            case ':':
                state = eInAddrSpec;
                break;
            case '@':
            case ',':
            case '.':
                mRoute += tokenizer.Token();
                break;
            }
            break;
        case eTkAtom:
            mRoute += tokenizer.Token();
            break;
        case eTkDomainLiteral:
            mRoute += tokenizer.Token();
            break;
        }
        ++tokenizer;
        type = tokenizer.Type();
    }

    // in-addr-spec state -- terminated by '@'

    while (state == eInAddrSpec && type != eTkNull) {
        switch (type) {
        case eTkSpecial:
            ch = tokenizer.Token()[0];
            switch (ch) {
            case '@':
                state = eAtSeen;
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

    // at-seen state -- terminated by '>' or end of string

    while (state == eAtSeen && type != eTkNull) {
        switch (type) {
        case eTkSpecial:
            ch = tokenizer.Token()[0];
            switch (ch) {
            case '>':
                state = eGtSeen;
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
        case eTkComment:
            tokenizer.StripDelimiters();
            lastComment = tokenizer.Token();
            break;
        }
        ++tokenizer;
        type = tokenizer.Type();
    }

    // greater-than-seen state -- terminated by end of string

    while (state == eGtSeen && type != eTkNull) {
        switch (type) {
        case eTkComment:
            tokenizer.StripDelimiters();
            lastComment = tokenizer.Token();
            break;
        }
        ++tokenizer;
        type = tokenizer.Type();
    }

    // Get full name, if possible

    if (isSimpleAddress) {
        mFullName = lastComment;
    }
    else if (firstPhrase != emptyString) {
        mFullName = firstPhrase;
    }
    else if (lastComment != emptyString) {
        mFullName = lastComment;
    }

    // Check validity

    if (mLocalPart.length() > 0) {
        mIsValid = 1;
    }
    else {
        mIsValid = 0;
    }

    // Remove CR or LF from local-part or full name

    RemoveCrAndLf(mFullName);
    RemoveCrAndLf(mLocalPart);
}


void DwMailbox::Assemble()
{
    if (!mIsModified) return;
    mIsValid = 1;
    if (mLocalPart.length() == 0 || mDomain.length() == 0) {
        mIsValid = 0;
        mString = "";
        return;
    }
    mString = "";
    if (mFullName.length() > 0) {
        mString += mFullName;
        mString += " ";
    }
    mString += "<";
    if (mRoute.length() > 0) {
        mString += mRoute;
        mString += ":";
    }
    mString += mLocalPart;
    mString += "@";
    mString += mDomain;
    mString += ">";
    mIsModified = 0;
}

DwMessageComponent* DwMailbox::Clone() const
{
    return new DwMailbox(*this);
}


#if defined(DW_DEBUG_VERSION)
void DwMailbox::PrintDebugInfo(std::ostream& aStrm, int /*aDepth*/) const
{
    aStrm <<
    "---------------- Debug info for DwMailbox class ----------------\n";
    _PrintDebugInfo(aStrm);
}
#else
void DwMailbox::PrintDebugInfo(std::ostream& , int) const {}
#endif // defined(DW_DEBUG_VERSION)


#if defined(DW_DEBUG_VERSION)
void DwMailbox::_PrintDebugInfo(std::ostream& aStrm) const
{
    DwAddress::_PrintDebugInfo(aStrm);
    aStrm << "Full Name:        " << mFullName << '\n';
    aStrm << "Route:            " << mRoute << '\n';
    aStrm << "Local Part:       " << mLocalPart << '\n';
    aStrm << "Domain:           " << mDomain << '\n';
}
#else
void DwMailbox::_PrintDebugInfo(std::ostream& ) const {}
#endif // defined(DW_DEBUG_VERSION)


void DwMailbox::CheckInvariants() const
{
#if defined(DW_DEBUG_VERSION)
    DwAddress::CheckInvariants();
    mFullName.CheckInvariants();
    mRoute.CheckInvariants();
    mLocalPart.CheckInvariants();
    mDomain.CheckInvariants();
#endif // defined(DW_DEBUG_VERSION)
}


void RemoveCrAndLf(DwString& aStr)
{
    // Do a quick check to see if at least one CR or LF is present

    size_t n = aStr.find_first_of("\r\n");
    if (n == DwString::npos)
        return;

    // At least one CR or LF is present, so copy the string

    const DwString& in = aStr;
    size_t inLen = in.length();
    DwString out;
    out.reserve(inLen);
    int lastChar = 0;
    size_t i = 0;
    while (i < inLen) {
        int ch = in[i];
        if (ch == (int) '\r') {
            out += ' ';
        }
        else if (ch == (int) '\n') {
            if (lastChar != (int) '\r') {
                out += ' ';
            }
        }
        else {
            out += (char) ch;
        }
        lastChar = ch;
        ++i;
    }
    aStr = out;
}
