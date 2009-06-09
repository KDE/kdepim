//=============================================================================
// File:       group.cpp
// Contents:   Definitions for DwGroup
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
#include <mimelib/string.h>
#include <mimelib/group.h>
#include <mimelib/token.h>

const char* const DwGroup::sClassName = "DwGroup";


DwGroup* (*DwGroup::sNewGroup)(const DwString&, DwMessageComponent*) = 0;


DwGroup* DwGroup::NewGroup(const DwString& aStr, DwMessageComponent* aParent)
{
    if (sNewGroup) {
        return sNewGroup(aStr, aParent);
    }
    else {
        return new DwGroup(aStr, aParent);
    }
}


DwGroup::DwGroup()
{
    mMailboxList =
        DwMailboxList::NewMailboxList("", this);
    mClassId = kCidGroup;
    mClassName = sClassName;
}


DwGroup::DwGroup(const DwGroup& aGroup)
   : DwAddress(aGroup),
     mGroupName(aGroup.mGroupName)
{
    mMailboxList = (DwMailboxList*) aGroup.mMailboxList->Clone();
    mMailboxList->SetParent(this);
    mClassId = kCidGroup;
    mClassName = sClassName;
}


DwGroup::DwGroup(const DwString& aStr, DwMessageComponent* aParent)
   : DwAddress(aStr, aParent)
{
    mMailboxList =
        DwMailboxList::NewMailboxList("", this);
    mClassId = kCidGroup;
    mClassName = sClassName;
}


DwGroup::~DwGroup()
{
    delete mMailboxList;
}


const DwGroup& DwGroup::operator = (const DwGroup& aGroup)
{
    if (this == &aGroup) return *this;
    DwAddress::operator = (aGroup);
    mGroupName    =  aGroup.mGroupName;
    delete mMailboxList;
    mMailboxList = (DwMailboxList*) aGroup.mMailboxList->Clone();
    // *mMailboxList = *aGroup.mMailboxList;
    return *this;
}


const DwString& DwGroup::GroupName() const
{
    return mGroupName;
}


const DwString& DwGroup::Phrase() const
{
    return mGroupName;
}


void DwGroup::SetGroupName(const DwString& aName)
{
    mGroupName = aName;
}


void DwGroup::SetPhrase(const DwString& aPhrase)
{
    mGroupName = aPhrase;
}


DwMailboxList& DwGroup::MailboxList() const
{
    return *mMailboxList;
}


void DwGroup::Parse()
{
    mIsModified = 0;
    mGroupName = "";
    int isGroupNameNull = 1;
    if (mMailboxList) {
        delete mMailboxList;
    }
    mMailboxList = DwMailboxList::NewMailboxList("", this);
    mIsValid = 0;
    DwRfc822Tokenizer tokenizer(mString);
    int type = tokenizer.Type();
    int ch;

    // Everything up to the first ':' is the group name
    int done = 0;
    while (!done && type != eTkNull) {
        switch (type) {
        case eTkSpecial:
            ch = tokenizer.Token()[0];
            switch (ch) {
            case ':':
                done = 1;
            }
            break;
        case eTkQuotedString:
        case eTkAtom:
            if (isGroupNameNull) {
                isGroupNameNull = 0;
            }
            else {
                mGroupName += " ";
            }
            mGroupName += tokenizer.Token();
            break;
        }
        ++tokenizer;
        type = tokenizer.Type();
    }

    // Find mailbox list, which ends with ';'
    DwTokenString tokenString(mString);
    tokenString.SetFirst(tokenizer);
    done = 0;
    while (!done && type != eTkNull) {
        if (type == eTkSpecial && tokenizer.Token()[0] == ';') {
            tokenString.ExtendTo(tokenizer);
            break;
        }
        ++tokenizer;
        type = tokenizer.Type();
    }
    if (mMailboxList) {
        delete mMailboxList;
    }
    mMailboxList = DwMailboxList::NewMailboxList(tokenString.Tokens(), this);
    mMailboxList->Parse();
    if (mGroupName.length() > 0) {
        mIsValid = 1;
    }
    else {
        mIsValid = 0;
    }
}


void DwGroup::Assemble()
{
    if (!mIsModified) return;
    if (mGroupName.length() == 0) {
        mIsValid = 0;
        mString = "";
        return;
    }
    mMailboxList->Assemble();
    mString = "";
    mString += mGroupName;
    mString += ":";
    mString += mMailboxList->AsString();
    mString += ";";
    mIsModified = 0;
}


DwMessageComponent* DwGroup::Clone() const
{
    return new DwGroup(*this);
}


#if defined (DW_DEBUG_VERSION)
void DwGroup::PrintDebugInfo(std::ostream& aStrm, int aDepth) const
{
    aStrm << "------------ Debug info for DwGroup class ------------\n";
    _PrintDebugInfo(aStrm);
    int depth = aDepth - 1;
    depth = (depth >= 0) ? depth : 0;
    if (aDepth == 0 || depth > 0) {
        mMailboxList->PrintDebugInfo(aStrm, depth);
    }
}
#else
void DwGroup::PrintDebugInfo(std::ostream&, int) const {}
#endif // defined (DW_DEBUG_VERSION)


#if defined (DW_DEBUG_VERSION)
void DwGroup::_PrintDebugInfo(std::ostream& aStrm) const
{
    DwAddress::_PrintDebugInfo(aStrm);
    aStrm << "Group name:       " << mGroupName << '\n';
    aStrm << "Mailbox list:     " << mMailboxList->ObjectId() << '\n';
}
#else
void DwGroup::_PrintDebugInfo(std::ostream& ) const {}
#endif // defined (DW_DEBUG_VERSION)


void DwGroup::CheckInvariants() const
{
#if defined (DW_DEBUG_VERSION)
    DwAddress::CheckInvariants();
    mGroupName.CheckInvariants();
    mMailboxList->CheckInvariants();
    assert((DwMessageComponent*) this == mMailboxList->Parent());
#endif // defined (DW_DEBUG_VERSION)
}
