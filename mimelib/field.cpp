//=============================================================================
// File:       field.cpp
// Contents:   Definitions for DwField
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
#include <assert.h>
#include <ctype.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <mimelib/string.h>
#include <mimelib/field.h>
#include <mimelib/headers.h>
#include <mimelib/fieldbdy.h>
#include <mimelib/datetime.h>
#include <mimelib/mailbox.h>
#include <mimelib/mboxlist.h>
#include <mimelib/address.h>
#include <mimelib/addrlist.h>
#include <mimelib/mechansm.h>
#include <mimelib/mediatyp.h>
#include <mimelib/msgid.h>
#include <mimelib/text.h>


class DwFieldParser {
    friend class DwField;
private:
    DwFieldParser(const DwString&);
    void Parse();
    const DwString mString;
    DwString mName;
    DwString mBody;
};


DwFieldParser::DwFieldParser(const DwString& aStr)
  : mString(aStr)
{
    Parse();
}


void DwFieldParser::Parse()
{
    const char* buf = mString.data();
    size_t bufEnd   = mString.length();
    size_t pos   = 0;
    size_t start = 0;
    size_t len   = 0;
    // Get field name
    while (pos < bufEnd) {
        if (buf[pos] == ':') {
            break;
        }
        ++pos;
    }
    len = pos;
    // Remove any white space at end of field-name
    while (len > 0) {
        int ch = buf[len-1];
        if (ch != ' ' && ch != '\t') break;
        --len;
    }
    mName = mString.substr(start, len);
    if (pos < bufEnd && buf[pos] == ':') {
        ++pos;
    }
    // Skip spaces and tabs (but not newline!)
    while (pos < bufEnd) {
        if (buf[pos] != ' ' && buf[pos] != '\t') break;
        ++pos;
    }
    start = pos;
    len = 0;
    // Get field body
    while (pos < bufEnd) {
        if (buf[pos] == '\n') {
            // Are we at the end of the string?
            if (pos == bufEnd - 1) {
                ++pos;
                break;
            }
            // Is this really the end of the field body, and not just
            // the end of a wrapped line?
            else if (buf[pos+1] != ' ' && buf[pos+1] != '\t') {
                ++pos;
                break;
            }
        }
        ++pos;
    }
    // Remove white space at end of field-body
    while (pos > start) {
        if (!isspace(buf[pos-1])) break;
        --pos;
    }
    len = pos - start;
    mBody = mString.substr(start, len);
}


//===========================================================================


const char* const DwField::sClassName = "DwField";


DwField* (*DwField::sNewField)(const DwString&, DwMessageComponent*) = 0;


DwFieldBody* (*DwField::sCreateFieldBody)(const DwString&,
    const DwString&, DwMessageComponent*) = 0;


DwField* DwField::NewField(const DwString& aStr, DwMessageComponent* aParent)
{
    if (sNewField) {
        return sNewField(aStr, aParent);
    }
    else {
        return new DwField(aStr, aParent);
    }
}


DwField::DwField()
{
    mNext = 0;
    mFieldBody = 0;
    mClassId = kCidField;
    mClassName = sClassName;
}


DwField::DwField(const DwField& aField)
  : DwMessageComponent(aField),
    mFieldNameStr(aField.mFieldNameStr),
    mFieldBodyStr(aField.mFieldBodyStr)
{
    mNext = 0;
    if (aField.mFieldBody) {
        mFieldBody = (DwFieldBody*) aField.mFieldBody->Clone();
    }
    else {
        mFieldBody = 0;
    }
    mClassId = kCidField;
    mClassName = sClassName;
}


DwField::DwField(const DwString& aStr, DwMessageComponent* aParent)
  : DwMessageComponent(aStr, aParent)
{
    mNext = 0;
    mFieldBody = 0;
    mClassId = kCidField;
    mClassName = sClassName;
}


DwField::~DwField()
{
    if (mFieldBody) {
        delete mFieldBody;
    }
}


const DwField& DwField::operator = (const DwField& aField)
{
    if (this == &aField) return *this;
    DwMessageComponent::operator = (aField);
    mFieldNameStr = aField.mFieldNameStr;
    mFieldBodyStr = aField.mFieldBodyStr;
    if (mFieldBody) {
        delete mFieldBody;
        mFieldBody = (DwFieldBody*) aField.mFieldBody->Clone();
    }
    return *this;
}


const DwString& DwField::FieldNameStr() const
{
    return mFieldNameStr;
}


void DwField::SetFieldNameStr(const DwString& aStr)
{
    mFieldNameStr = aStr;
    SetModified();
}


const DwString& DwField::FieldBodyStr() const
{
    return mFieldBodyStr;
}


void DwField::SetFieldBodyStr(const DwString& aStr)
{
    mFieldBodyStr = aStr;
    if (mFieldBody) {
        delete mFieldBody;
        mFieldBody = 0;
    }
    SetModified();
}


DwFieldBody* DwField::FieldBody() const
{
    return mFieldBody;
}


void DwField::SetFieldBody(DwFieldBody* aFieldBody)
{
    int isModified = 0;
    if (mFieldBody != aFieldBody) {
        isModified = 1;
    }
    mFieldBody = aFieldBody;
    if (mFieldBody) {
        mFieldBody->SetParent(this);
    }
    if (isModified) {
        SetModified();
    }
}


void DwField::_SetFieldBody(DwFieldBody* aFieldBody)
{
    mFieldBody = aFieldBody;
    if (mFieldBody) {
        mFieldBody->SetParent(this);
    }
}


DwField* DwField::Next() const
{
    return (DwField*) mNext;
}


void DwField::SetNext(const DwField* aNext)
{
    mNext = aNext;
}


void DwField::Parse()
{
    mIsModified = 0;
    DwFieldParser parser(mString);
    mFieldNameStr = parser.mName;
    mFieldBodyStr = parser.mBody;
    mFieldBody = CreateFieldBody(mFieldNameStr, mFieldBodyStr, this);
    assert(mFieldBody != 0);
    mFieldBody->Parse();
}


void DwField::Assemble()
{
    if (!mIsModified) return;
    if (mFieldBody) {
        mFieldBody->Assemble();
        mFieldBodyStr = mFieldBody->AsString();
    }
    mString = "";
    mString += mFieldNameStr;
    mString += ": ";
    mString += mFieldBodyStr;
    mString += DW_EOL;
    mIsModified = 0;
}


DwMessageComponent* DwField::Clone() const
{
    return new DwField(*this);
}


DwFieldBody* DwField::CreateFieldBody(const DwString& aFieldName,
    const DwString& aFieldBody, DwMessageComponent* aParent)
{
    DwFieldBody* fieldBody;
    if (sCreateFieldBody != 0) {
        fieldBody = sCreateFieldBody(aFieldName, aFieldBody, aParent);
    }
    else {
        fieldBody = _CreateFieldBody(aFieldName, aFieldBody, aParent);
    }
    return fieldBody;
}


DwFieldBody* DwField::_CreateFieldBody(const DwString& aFieldName,
    const DwString& aFieldBody, DwMessageComponent* aParent)
{
    enum {
        kAddressList,
        kDispositionType,
        kDateTime,
        kMailbox,
        kMailboxList,
        kMechanism,
        kMediaType,
        kMsgId,
        kText
    } fieldBodyType;
    // Default field type is 'text'
    fieldBodyType = kText;
    int ch = aFieldName[0];
    ch = tolower(ch);
    switch (ch) {
    case 'b':
        if (DwStrcasecmp(aFieldName, "bcc") == 0) {
            fieldBodyType = kAddressList;
        }
        break;
    case 'c':
        if (DwStrcasecmp(aFieldName, "cc") == 0) {
            fieldBodyType = kAddressList;
        }
        else if (DwStrcasecmp(aFieldName, "content-id") == 0) {
            fieldBodyType = kMsgId;
        }
        else if (DwStrcasecmp(aFieldName, "content-transfer-encoding") == 0) {
            fieldBodyType = kMechanism;
        }
        else if (DwStrcasecmp(aFieldName, "content-type") == 0) {
            fieldBodyType = kMediaType;
        }
        else if (DwStrcasecmp(aFieldName, "content-disposition") == 0) {
            fieldBodyType = kDispositionType;
        }
        break;
    case 'd':
        if (DwStrcasecmp(aFieldName, "date") == 0) {
            fieldBodyType = kDateTime;
        }
        break;
    case 'f':
        if (DwStrcasecmp(aFieldName, "from") == 0) {
            fieldBodyType = kMailboxList;
        }
        break;
    case 'm':
        if (DwStrcasecmp(aFieldName, "message-id") == 0) {
            fieldBodyType = kMsgId;
        }
        break;
    case 'r':
        if (DwStrcasecmp(aFieldName, "reply-to") == 0) {
            fieldBodyType = kAddressList;
        }
        else if (DwStrcasecmp(aFieldName, "resent-bcc") == 0) {
            fieldBodyType = kAddressList;
        }
        else if (DwStrcasecmp(aFieldName, "resent-cc") == 0) {
            fieldBodyType = kAddressList;
        }
        else if (DwStrcasecmp(aFieldName, "resent-date") == 0) {
            fieldBodyType = kDateTime;
        }
        else if (DwStrcasecmp(aFieldName, "resent-from") == 0) {
            fieldBodyType = kMailboxList;
        }
        else if (DwStrcasecmp(aFieldName, "resent-message-id") == 0) {
            fieldBodyType = kMsgId;
        }
        else if (DwStrcasecmp(aFieldName, "resent-reply-to") == 0) {
            fieldBodyType = kAddressList;
        }
        else if (DwStrcasecmp(aFieldName, "resent-sender") == 0) {
            fieldBodyType = kMailbox;
        }
        else if (DwStrcasecmp(aFieldName, "return-path") == 0) {
            fieldBodyType = kMailbox;
        }
        break;
    case 's':
        if (DwStrcasecmp(aFieldName, "sender") == 0) {
            fieldBodyType = kMailbox;
        }
        break;
    case 't':
        if (DwStrcasecmp(aFieldName, "to") == 0) {
            fieldBodyType = kAddressList;
        }
        break;
    }
    DwFieldBody* fieldBody;
    switch (fieldBodyType) {
    case kAddressList:
        fieldBody = DwAddressList::NewAddressList(aFieldBody, aParent);
        break;
    case kDispositionType:
        fieldBody = DwDispositionType::NewDispositionType(aFieldBody, aParent);
        break;
    case kMediaType:
        fieldBody = DwMediaType::NewMediaType(aFieldBody, aParent);
        break;
    case kMechanism:
        fieldBody = DwMechanism::NewMechanism(aFieldBody, aParent);
        break;
    case kDateTime:
        fieldBody = DwDateTime::NewDateTime(aFieldBody, aParent);
        break;
    case kMailbox:
        fieldBody = DwMailbox::NewMailbox(aFieldBody, aParent);
        break;
    case kMailboxList:
        fieldBody = DwMailboxList::NewMailboxList(aFieldBody, aParent);
        break;
    case kMsgId:
        fieldBody = DwMsgId::NewMsgId(aFieldBody, aParent);
        break;
    case kText:
    default:
        fieldBody = DwText::NewText(aFieldBody, aParent);
        break;
    }
    return fieldBody;
}


#if defined (DW_DEBUG_VERSION)
void DwField::PrintDebugInfo(std::ostream& aStrm, int aDepth) const
{
    aStrm <<
    "----------------- Debug info for DwField class -----------------\n";
    _PrintDebugInfo(aStrm);
    int depth = aDepth - 1;
    depth = (depth >= 0) ? depth : 0;
    if (mFieldBody && (aDepth == 0 || depth > 0)) {
        mFieldBody->PrintDebugInfo(aStrm, depth);
    }
}
#else
void DwField::PrintDebugInfo(std::ostream& , int ) const {}
#endif // defined (DW_DEBUG_VERSION)


#if defined (DW_DEBUG_VERSION)
void DwField::_PrintDebugInfo(std::ostream& aStrm) const
{
    DwMessageComponent::_PrintDebugInfo(aStrm);
    aStrm << "Field name:       " << mFieldNameStr << '\n';
    aStrm << "Field body:       " << mFieldBodyStr << '\n';
    aStrm << "Field body object:";
    if (mFieldBody) {
    aStrm << mFieldBody->ObjectId() << '\n';
    }
    else {
    aStrm << "(none)\n";
    }
    aStrm << "Next field:       ";
    if (mNext) {
        aStrm << mNext->ObjectId() << '\n';
    }
    else {
    aStrm << "(none)\n";
    }
}
#else
void DwField::_PrintDebugInfo(std::ostream& ) const {}
#endif // defined (DW_DEBUG_VERSION)


void DwField::CheckInvariants() const
{
#if defined (DW_DEBUG_VERSION)
    DwMessageComponent::CheckInvariants();
    mFieldNameStr.CheckInvariants();
    mFieldBodyStr.CheckInvariants();
    if (mFieldBody) {
        mFieldBody->CheckInvariants();
    }
    if (mFieldBody) {
        assert((DwMessageComponent*) this == mFieldBody->Parent());
    }
#endif // defined (DW_DEBUG_VERSION)
}
