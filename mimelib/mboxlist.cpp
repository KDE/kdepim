//=============================================================================
// File:       mboxlist.cpp
// Contents:   Definitions for DwMailboxList
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
#include <iostream>
#include <mimelib/string.h>
#include <mimelib/mailbox.h>
#include <mimelib/mboxlist.h>
#include <mimelib/token.h>


const char* const DwMailboxList::sClassName = "DwMailboxList";


DwMailboxList* (*DwMailboxList::sNewMailboxList)(const DwString&,
    DwMessageComponent*) = 0;


DwMailboxList* DwMailboxList::NewMailboxList(const DwString& aStr,
    DwMessageComponent* aParent)
{
    if (sNewMailboxList) {
        return sNewMailboxList(aStr, aParent);
    }
    else {
        return new DwMailboxList(aStr, aParent);
    }
}


DwMailboxList::DwMailboxList()
{
    mFirstMailbox = 0;
    mClassId = kCidMailboxList;
    mClassName = sClassName;
}


DwMailboxList::DwMailboxList(const DwMailboxList& aList)
  : DwFieldBody(aList)
{
    mFirstMailbox = 0;
    const DwMailbox* firstMailbox = aList.mFirstMailbox;
    if (firstMailbox) {
        CopyList(firstMailbox);
    }
    mClassId = kCidMailboxList;
    mClassName = sClassName;
}


DwMailboxList::DwMailboxList(const DwString& aStr, DwMessageComponent* aParent)
  : DwFieldBody(aStr, aParent)
{
    mFirstMailbox = 0;
    mClassId = kCidMailboxList;
    mClassName = sClassName;
}


DwMailboxList::~DwMailboxList()
{
    if (mFirstMailbox) {
        _DeleteAll();
    }
}


const DwMailboxList& DwMailboxList::operator = (const DwMailboxList& aList)
{
    if (this == &aList) return *this;
    DwFieldBody::operator = (aList);
    if (mFirstMailbox) {
        _DeleteAll();
    }
    const DwMailbox* firstMailbox = aList.mFirstMailbox;
    if (firstMailbox) {
        CopyList(firstMailbox);
    }
    if (mParent && mIsModified) {
        mParent->SetModified();
    }
    return *this;
}


DwMailbox* DwMailboxList::FirstMailbox() const
{
    return mFirstMailbox;
}


void DwMailboxList::Add(DwMailbox* aMailbox)
{
    assert(aMailbox != 0);
    if (aMailbox == 0) return;
    _AddMailbox(aMailbox);
    SetModified();
}


void DwMailboxList::_AddMailbox(DwMailbox* aMailbox)
{
    assert(aMailbox != 0);
    if (aMailbox == 0) return;
    if (!mFirstMailbox) {
        mFirstMailbox = aMailbox;
    }
    else {
        DwMailbox* mb = mFirstMailbox;
        while (mb->Next()) {
            mb = (DwMailbox*) mb->Next();
        }
        mb->SetNext(aMailbox);
    }
    aMailbox->SetParent(this);
}


void DwMailboxList::Remove(DwMailbox* mailbox)
{
    DwMailbox* mb = mFirstMailbox;
    if (mb == mailbox) {
        mFirstMailbox = (DwMailbox*) mb->Next();
        return;
    }
    while (mb) {
        if (mb->Next() == mailbox) {
            mb->SetNext(mailbox->Next());
            break;
        }
    }
    SetModified();
}


void DwMailboxList::DeleteAll()
{
    _DeleteAll();
    SetModified();
}


void DwMailboxList::_DeleteAll()
{
    DwMailbox* mb = mFirstMailbox;
    while (mb) {
        DwMailbox* toDel = mb;
        mb = (DwMailbox*) mb->Next();
        delete toDel;
    }
    mFirstMailbox = 0;
}


void DwMailboxList::Parse()
{
    mIsModified = 0;
    // Mailboxes are separated by commas.  Commas may also occur in a route.
    // (See RFC822 p. 27)
    if (mFirstMailbox)
        _DeleteAll();
    DwMailboxListParser parser(mString);
    DwMailbox* mailbox;
    while (1) {
        switch (parser.MbType()) {
        case DwMailboxListParser::eMbError:
        case DwMailboxListParser::eMbEnd:
            goto LOOP_EXIT;
        case DwMailboxListParser::eMbMailbox:
            mailbox = DwMailbox::NewMailbox(parser.MbString(), this);
            mailbox->Parse();
            if (mailbox->IsValid()) {
                _AddMailbox(mailbox);
            }
            else {
                delete mailbox;
            }
            break;
        case DwMailboxListParser::eMbNull:
            break;
        }
        ++parser;
    }
LOOP_EXIT:
    return;
}


void DwMailboxList::Assemble()
{
    if (!mIsModified) return;
    mString = "";
    int count = 0;
    DwMailbox* mb = mFirstMailbox;
    while (mb) {
        mb->Assemble();
        if (mb->IsValid()) {
            if (count > 0){
                if (IsFolding()) {
                    mString += "," DW_EOL "  ";
                }
                else {
                    mString += ", ";
                }
            }
            mString += mb->AsString();
            ++count;
        }
        mb = (DwMailbox*) mb->Next();
    }
    mIsModified = 0;
}


DwMessageComponent* DwMailboxList::Clone() const
{
    return new DwMailboxList(*this);
}


void DwMailboxList::CopyList(const DwMailbox* aFirst)
{
    const DwMailbox* mailbox = aFirst;
    while (mailbox) {
        DwMailbox* newMailbox = (DwMailbox*) mailbox->Clone();
        Add(newMailbox);
        mailbox = (DwMailbox*) mailbox->Next();
    }
}


#if defined (DW_DEBUG_VERSION)
void DwMailboxList::PrintDebugInfo(std::ostream& aStrm, int aDepth) const
{
    aStrm <<
    "-------------- Debug info for DwMailboxList class --------------\n";
    _PrintDebugInfo(aStrm);
    int depth = aDepth - 1;
    depth = (depth >= 0) ? depth : 0;
    if (aDepth == 0 || depth > 0) {
        DwMailbox* mbox = mFirstMailbox;
        while (mbox) {
            mbox->PrintDebugInfo(aStrm, depth);
            mbox = (DwMailbox*) mbox->Next();
        }
    }
}
#else
void DwMailboxList::PrintDebugInfo(std::ostream& , int ) const {}
#endif // defined (DW_DEBUG_VERSION)


#if defined (DW_DEBUG_VERSION)
void DwMailboxList::_PrintDebugInfo(std::ostream& aStrm) const
{
    DwFieldBody::_PrintDebugInfo(aStrm);
    aStrm << "Mailbox objects:  ";
    DwMailbox* mbox = mFirstMailbox;
    if (mbox) {
        int count = 0;
        while (mbox) {
            if (count) aStrm << ' ';
            aStrm << mbox->ObjectId();
            mbox = (DwMailbox*) mbox->Next();
            ++count;
        }
        aStrm << '\n';
    }
    else {
        aStrm << "(none)\n";
    }
}
#else
void DwMailboxList::_PrintDebugInfo(std::ostream& ) const {}
#endif // defined (DW_DEBUG_VERSION)


void DwMailboxList::CheckInvariants() const
{
#if defined (DW_DEBUG_VERSION)
    DwMailbox* mbox = mFirstMailbox;
    while (mbox) {
        mbox->CheckInvariants();
        assert((DwMessageComponent*) this == mbox->Parent());
        mbox = (DwMailbox*) mbox->Next();
    }
#endif // defined (DW_DEBUG_VERSION)
}


//-------------------------------------------------------------------------


DwMailboxListParser::DwMailboxListParser(const DwString& aStr)
  : mTokenizer(aStr),
    mMbString(aStr)
{
    mMbType = eMbError;
    ParseNextMailbox();
}


DwMailboxListParser::~DwMailboxListParser()
{
}


int DwMailboxListParser::Restart()
{
    mTokenizer.Restart();
    ParseNextMailbox();
    return mMbType;
}


int DwMailboxListParser::operator ++ ()
{
    ParseNextMailbox();
    return mMbType;
}


void DwMailboxListParser::ParseNextMailbox()
{
    mMbString.SetFirst(mTokenizer);
    mMbType = eMbEnd;
    int type = mTokenizer.Type();
    if (type == eTkNull) {
        return;
    }
    enum {
        eTopLevel,
        eInRouteAddr
    } state;
    state = eTopLevel;
    mMbType = eMbMailbox;
    int done = 0;
    while (!done) {
        if (type == eTkNull) {
            mMbString.ExtendTo(mTokenizer);
            break;
        }
        if (type == eTkSpecial) {
            int ch = mTokenizer.Token()[0];
            switch (state) {
            case eTopLevel:
                switch (ch) {
                case ',':
                    mMbString.ExtendTo(mTokenizer);
                    done = 1;
                    break;
                case '<':
                    state = eInRouteAddr;
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
            }
        }
        ++mTokenizer;
        type = mTokenizer.Type();
    }
    if (mMbString.Tokens().length() == 0) {
        mMbType = eMbNull;
    }
}

