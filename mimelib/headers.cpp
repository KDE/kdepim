//=============================================================================
// File:       headers.cpp
// Contents:   Definitions for DwHeaders
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
#include <time.h>
#include <iostream>
#include <mimelib/string.h>
#include <mimelib/headers.h>
#include <mimelib/field.h>
#include <mimelib/body.h>
#include <mimelib/datetime.h>
#include <mimelib/mailbox.h>
#include <mimelib/address.h>
#include <mimelib/mechansm.h>
#include <mimelib/mediatyp.h>
#include <mimelib/msgid.h>
#include <mimelib/text.h>


class DwHeadersParser {
    friend class DwHeaders;
private:
    DwHeadersParser(const DwString&);
    void Rewind();
    void NextField(DwString*);
    const DwString mString;
    size_t mPos;
};


DwHeadersParser::DwHeadersParser(const DwString& aStr)
  : mString(aStr)
{
    mPos = 0;
}


void DwHeadersParser::Rewind()
{
    mPos = 0;
}


void DwHeadersParser::NextField(DwString* aStr)
{
    if (!aStr) {
        return;
    }
    const char* buf = mString.data();
    size_t bufEnd = mString.length();
    size_t pos = mPos;
    size_t start = pos;
    size_t len = 0;
    while (pos < bufEnd) {
        if (buf[pos] == '\n'
            && pos+1 < bufEnd
            && buf[pos+1] != ' '
            && buf[pos+1] != '\t') {

            ++len;
            ++pos;
            break;
        }
        ++len;
        ++pos;
    }
    *aStr = mString.substr(start, len);
    mPos = pos;
}


//============================================================================


const char* const DwHeaders::sClassName = "DwHeaders";


DwHeaders* (*DwHeaders::sNewHeaders)(const DwString&, DwMessageComponent*) = 0;


DwHeaders* DwHeaders::NewHeaders(const DwString& aStr,
    DwMessageComponent* aParent)
{
    if (sNewHeaders) {
        return sNewHeaders(aStr, aParent);
    }
    else {
        return new DwHeaders(aStr, aParent);
    }
}


DwHeaders::DwHeaders()
{
    mFirstField = 0;
    mLastField = 0;
    mClassId = kCidHeaders;
    mClassName = sClassName;
}


DwHeaders::DwHeaders(const DwHeaders& aHeader)
  : DwMessageComponent(aHeader)
{
    mFirstField = 0;
    mLastField = 0;
    if (aHeader.mFirstField) {
        CopyFields(aHeader.mFirstField);
    }
    mClassId = kCidHeaders;
    mClassName = sClassName;
}


DwHeaders::DwHeaders(const DwString& aStr, DwMessageComponent* aParent)
  : DwMessageComponent(aStr, aParent)
{
    mFirstField = 0;
    mLastField = 0;
    mClassId = kCidHeaders;
    mClassName = sClassName;
}


DwHeaders::~DwHeaders()
{
    if (mFirstField) {
        DeleteAllFields();
    }
}


const DwHeaders& DwHeaders::operator = (const DwHeaders& aHeader)
{
    if (this == &aHeader) return *this;
    DwMessageComponent::operator = (aHeader);
    if (mFirstField) {
        DeleteAllFields();
    }
    if (aHeader.mFirstField) {
        CopyFields(aHeader.mFirstField);
    }
    if (mParent) {
        mParent->SetModified();
    }
    return *this;
}


void DwHeaders::Parse()
{
    mIsModified = 0;
    DwHeadersParser parser(mString);
    DwString str;
    parser.NextField(&str);
    while (!str.empty()) {
        DwField* field = DwField::NewField(str, this);
        field->Parse();
        _AddField(field);
        parser.NextField(&str);
    }
}


void DwHeaders::Assemble()
{
    if (!mIsModified) return;
    mString = "";
    DwField* field = FirstField();
    while (field) {
        field->Assemble();
        mString += field->AsString();
        field = field->Next();
    }
    // We DwEntityParser skips the empty line separating the headers
    // from the body, so why should be add it here?
    //mString += DW_EOL;
    mIsModified = 0;
}


DwMessageComponent* DwHeaders::Clone() const
{
    return new DwHeaders(*this);
}


DwFieldBody& DwHeaders::FieldBody(const DwString& aFieldName)
{
    assert(!aFieldName.empty());
    // First, search for field
    DwField* field = FindField(aFieldName);
    // If the field is not found, create the field and its field body
    if (field == 0) {
        field = DwField::NewField("", this);
        field->SetFieldNameStr(aFieldName);
        DwFieldBody* fieldBody = DwField::CreateFieldBody(aFieldName,
            "", field);
        field->SetFieldBody(fieldBody);
        AddField(field);
    }
    // Get the field body
    DwFieldBody* fieldBody = field->FieldBody();
    // If it does not exist, create it
    if (fieldBody == 0) {
        fieldBody = DwField::CreateFieldBody(aFieldName, "", field);
        field->SetFieldBody(fieldBody);
        SetModified();
    }
    return *fieldBody;
}


std::vector<DwFieldBody*> DwHeaders::AllFieldBodies(const DwString& aFieldName)
{
    assert(!aFieldName.empty());
    // First, search for field
    DwField* field = FindField(aFieldName);
    // If the field is not found, create the field and its field body
    if (field == 0) {
        field = DwField::NewField("", this);
        field->SetFieldNameStr(aFieldName);
        DwFieldBody* fieldBody = DwField::CreateFieldBody(aFieldName,
            "", field);
        field->SetFieldBody(fieldBody);
        AddField(field);
    }
    std::vector<DwFieldBody*> v;
    for ( ; field; field = field->Next() ) {
        if (DwStrcasecmp(field->FieldNameStr(), aFieldName) == 0) {
            // Get the field body
            DwFieldBody* fieldBody = field->FieldBody();
            // If it does not exist, create it
            if (fieldBody == 0) {
                fieldBody = DwField::CreateFieldBody(aFieldName, "", field);
                field->SetFieldBody(fieldBody);
                SetModified();
            }
            v.push_back( fieldBody );
        }
    }
    return v;
}


int DwHeaders::NumFields() const
{
    int count = 0;
    DwField* field = mFirstField;
    while (field) {
        ++count;
        field = field->Next();
    }
    return count;
}


DwField* DwHeaders::FindField(const char* aFieldName) const
{
    assert(aFieldName != 0);
    if (aFieldName == 0) return 0;
    DwField* field = mFirstField;
    while (field) {
        if (DwStrcasecmp(field->FieldNameStr(), aFieldName) == 0) {
            break;
        }
        field = field->Next();
    }
    return field;
}


DwField* DwHeaders::FindField(const DwString& aFieldName) const
{
    DwField* field = mFirstField;
    while (field) {
        if (DwStrcasecmp(field->FieldNameStr(), aFieldName) == 0) {
            break;
        }
        field = field->Next();
    }
    return field;
}


void DwHeaders::AddOrReplaceField(DwField* aField)
{
    assert(aField != 0);
    if (aField == 0) return;
    SetModified();
    const DwString& fieldName = aField->FieldNameStr();
    DwField* prevField = 0;
    DwField* field = mFirstField;
    while (field) {
        if (DwStrcasecmp(field->FieldNameStr(), fieldName) == 0) {
            break;
        }
        prevField = field;
        field = field->Next();
    }
    // Field was not found, so just add it
    if (!field) {
        _AddField(aField);
    }
    // Field was found. Replace the old one with the new one.
    else {
        if (prevField) {
            prevField->SetNext(aField);
        }
        else {
            mFirstField = aField;
        }
        aField->SetNext(field->Next());
        // Check whether we've replaced the last field
        if ( !aField->Next() )
            mLastField = aField;
        delete field;
    }
}


void DwHeaders::AddField(DwField* aField)
{
    assert(aField != 0);
    if (aField == 0) return;
    _AddField(aField);
    SetModified();
}


void DwHeaders::AddFieldAt(int aPos, DwField* aField)
{
    assert(aField != 0);
    if (aField == 0) return;
    SetModified();
    // Special case: empty list
    if (mFirstField == 0) {
        aField->SetNext(0);
        mFirstField = aField;
        mLastField = aField;
        return;
    }
    // Special case: aPos == 1 --> add at beginning
    if (aPos == 1) {
        aField->SetNext(mFirstField);
        mFirstField = aField;
        return;
    }
    // aPos == 0 --> at at end
    if (aPos == 0) {
        _AddField(aField);
        return;
    }
    int count = 2;
    DwField* field = mFirstField;
    while (field->Next() && count < aPos) {
        field = field->Next();
        ++count;
    }
    aField->SetNext(field->Next());
    field->SetNext(aField);
    // Check whether we've a new last field
    if ( !aField->Next() )
        mLastField = aField;
}


void DwHeaders::RemoveField(DwField* aField)
{
    DwField* prevField = 0;
    DwField* field = mFirstField;
    while (field) {
        if (field == aField) {
            break;
        }
        prevField = field;
        field = field->Next();
    }
    // If we found the field...
    if (field) {
        if (prevField == 0) {
            mFirstField = field->Next();
        }
        else {
            prevField->SetNext(field->Next());
        }
        // Check whether we've removed the last field
        if ( field == mLastField )
            mLastField = prevField;
        field->SetNext(0);
        SetModified();
    }
}


void DwHeaders::DeleteAllFields()
{
    DwField* field = mFirstField;
    while (field) {
        DwField* nextField = field->Next();
        delete field;
        field = nextField;
    }
    mFirstField = 0;
    mLastField = 0;
}


void DwHeaders::_AddField(DwField* aField)
{
    if (aField == 0) return;
    // Add field with setting is-modified flag for header
    aField->SetParent(this);
    // Special case: empty list
    if (mFirstField == 0) {
        mFirstField = aField;
        mLastField = aField;
        return;
    }
    mLastField->SetNext(aField);
    mLastField = aField;
}


void DwHeaders::CopyFields(DwField* aFirst)
{
    DwField* field = aFirst;
    DwField* newField;
    while (field) {
        newField = (DwField*) field->Clone();
        _AddField(newField);
        field = field->Next();
    }
}


DwBool DwHeaders::HasBcc() const
{
    return FindField("bcc") ? 1 : 0;
}


DwBool DwHeaders::HasCc() const
{
    return FindField("cc") ? 1 : 0;
}


DwBool DwHeaders::HasComments() const
{
    return FindField("comments") ? 1 : 0;
}


DwBool DwHeaders::HasDate() const
{
    return FindField("date") ? 1 : 0;
}


DwBool DwHeaders::HasEncrypted() const
{
    return FindField("encrypted") ? 1 : 0;
}


DwBool DwHeaders::HasFrom() const
{
    return FindField("from") ? 1 : 0;
}


DwBool DwHeaders::HasInReplyTo() const
{
    return FindField("in-reply-to") ? 1 : 0;
}


DwBool DwHeaders::HasKeywords() const
{
    return FindField("keywords") ? 1 : 0;
}


DwBool DwHeaders::HasMessageId() const
{
    return FindField("message-id") ? 1 : 0;
}


DwBool DwHeaders::HasReceived() const
{
    return FindField("received") ? 1 : 0;
}


DwBool DwHeaders::HasReferences() const
{
    return FindField("references") ? 1 : 0;
}


DwBool DwHeaders::HasReplyTo() const
{
    return FindField("reply-to") ? 1 : 0;
}


DwBool DwHeaders::HasResentBcc() const
{
    return FindField("resent-bcc") ? 1 : 0;
}


DwBool DwHeaders::HasResentCc() const
{
    return FindField("resent-cc") ? 1 : 0;
}


DwBool DwHeaders::HasResentDate() const
{
    return FindField("resent-date") ? 1 : 0;
}


DwBool DwHeaders::HasResentFrom() const
{
    return FindField("resent-from") ? 1 : 0;
}


DwBool DwHeaders::HasResentMessageId() const
{
    return FindField("resent-message-id") ? 1 : 0;
}


DwBool DwHeaders::HasResentReplyTo() const
{
    return FindField("resent-reply-to") ? 1 : 0;
}


DwBool DwHeaders::HasResentSender() const
{
    return FindField("resent-sender") ? 1 : 0;
}


DwBool DwHeaders::HasResentTo() const
{
    return FindField("resent-to") ? 1 : 0;
}


DwBool DwHeaders::HasReturnPath() const
{
    return FindField("return-path") ? 1 : 0;
}


DwBool DwHeaders::HasSender() const
{
    return FindField("sender") ? 1 : 0;
}


DwBool DwHeaders::HasSubject() const
{
    return FindField("subject") ? 1 : 0;
}


DwBool DwHeaders::HasTo() const
{
    return FindField("to") ? 1 : 0;
}


DwBool DwHeaders::HasApproved() const
{
    return FindField("approved") ? 1 : 0;
}


DwBool DwHeaders::HasControl() const
{
    return FindField("control") ? 1 : 0;
}


DwBool DwHeaders::HasDistribution() const
{
    return FindField("distribution") ? 1 : 0;
}


DwBool DwHeaders::HasExpires() const
{
    return FindField("expires") ? 1 : 0;
}


DwBool DwHeaders::HasFollowupTo() const
{
    return FindField("followup-to") ? 1 : 0;
}


DwBool DwHeaders::HasLines() const
{
    return FindField("lines") ? 1 : 0;
}


DwBool DwHeaders::HasNewsgroups() const
{
    return FindField("newsgroups") ? 1 : 0;
}


DwBool DwHeaders::HasOrganization() const
{
    return FindField("organization") ? 1 : 0;
}


DwBool DwHeaders::HasPath() const
{
    return FindField("path") ? 1 : 0;
}


DwBool DwHeaders::HasSummary() const
{
    return FindField("summary") ? 1 : 0;
}


DwBool DwHeaders::HasXref() const
{
    return FindField("xref") ? 1 : 0;
}


DwBool DwHeaders::HasContentDescription() const
{
    return FindField("content-description") ? 1 : 0;
}


DwBool DwHeaders::HasContentId() const
{
    return FindField("content-id") ? 1 : 0;
}


DwBool DwHeaders::HasContentTransferEncoding() const
{
    return FindField("content-transfer-encoding") ? 1 : 0;
}


DwBool DwHeaders::HasCte() const
{
    return FindField("content-transfer-encoding") ? 1 : 0;
}


DwBool DwHeaders::HasContentType() const
{
    return FindField("content-type") ? 1 : 0;
}


DwBool DwHeaders::HasMimeVersion() const
{
    return FindField("mime-version") ? 1 : 0;
}


DwBool DwHeaders::HasContentDisposition() const
{
    return FindField("content-disposition") ? 1 : 0;
}


DwBool DwHeaders::HasField(const char* aFieldName) const
{
    return FindField(aFieldName) ? 1 : 0;
}


DwBool DwHeaders::HasField(const DwString& aFieldName) const
{
    return FindField(aFieldName) ? 1 : 0;
}


DwAddressList& DwHeaders::Bcc()
{
    return (DwAddressList&) FieldBody("Bcc");
}


DwAddressList& DwHeaders::Cc()
{
    return (DwAddressList&) FieldBody("Cc");
}


DwText& DwHeaders::Comments()
{
    return (DwText&) FieldBody("Comments");
}


DwDateTime& DwHeaders::Date()
{
    return (DwDateTime&) FieldBody("Date");
}


DwText& DwHeaders::Encrypted()
{
    return (DwText&) FieldBody("Encrypted");
}


DwMailboxList& DwHeaders::From()
{
    return (DwMailboxList&) FieldBody("From");
}


DwText& DwHeaders::InReplyTo()
{
    return (DwText&) FieldBody("In-Reply-To");
}


DwText& DwHeaders::Keywords()
{
    return (DwText&) FieldBody("Keywords");
}


DwMsgId& DwHeaders::MessageId()
{
    return (DwMsgId&) FieldBody("Message-Id");
}


DwText& DwHeaders::Received()
{
    return (DwText&) FieldBody("Received");
}


DwText& DwHeaders::References()
{
    return (DwText&) FieldBody("References");
}


DwAddressList& DwHeaders::ReplyTo()
{
    return (DwAddressList&) FieldBody("Reply-To");
}


DwAddressList& DwHeaders::ResentBcc()
{
    return (DwAddressList&) FieldBody("Resent-Bcc");
}


DwAddressList& DwHeaders::ResentCc()
{
    return (DwAddressList&) FieldBody("Resent-Cc");
}


DwDateTime& DwHeaders::ResentDate()
{
    return (DwDateTime&) FieldBody("Resent-Date");
}


DwMailboxList& DwHeaders::ResentFrom()
{
    return (DwMailboxList&) FieldBody("Resent-From");
}


DwMsgId& DwHeaders::ResentMessageId()
{
    return (DwMsgId&) FieldBody("Resent-Message-Id");
}


DwAddressList& DwHeaders::ResentReplyTo()
{
    return (DwAddressList&) FieldBody("Resent-Reply-To");
}


DwMailbox& DwHeaders::ResentSender()
{
    return (DwMailbox&) FieldBody("Resent-Sender");
}


DwAddressList& DwHeaders::ResentTo()
{
    return (DwAddressList&) FieldBody("Resent-To");
}


DwAddress& DwHeaders::ReturnPath()
{
    return (DwAddress&) FieldBody("Return-Path");
}


DwMailbox& DwHeaders::Sender()
{
    return (DwMailbox&) FieldBody("Sender");
}


DwText& DwHeaders::Subject()
{
    return (DwText&) FieldBody("Subject");
}


DwAddressList& DwHeaders::To()
{
    return (DwAddressList&) FieldBody("To");
}


DwText& DwHeaders::Approved()
{
    return (DwText&) FieldBody("Approved");
}


DwText& DwHeaders::Control()
{
    return (DwText&) FieldBody("Control");
}


DwText& DwHeaders::Distribution()
{
    return (DwText&) FieldBody("Distribution");
}


DwText& DwHeaders::Expires()
{
    return (DwText&) FieldBody("Expires");
}


DwText& DwHeaders::FollowupTo()
{
    return (DwText&) FieldBody("Followup-To");
}


DwText& DwHeaders::Lines()
{
    return (DwText&) FieldBody("Lines");
}


DwText& DwHeaders::Newsgroups()
{
    return (DwText&) FieldBody("Newsgroups");
}


DwText& DwHeaders::Organization()
{
    return (DwText&) FieldBody("Organization");
}


DwText& DwHeaders::Path()
{
    return (DwText&) FieldBody("Path");
}


DwText& DwHeaders::Summary()
{
    return (DwText&) FieldBody("Summary");
}


DwText& DwHeaders::Xref()
{
    return (DwText&) FieldBody("Xref");
}



DwText& DwHeaders::ContentDescription()
{
    return (DwText&) FieldBody("Content-Description");
}


DwMsgId& DwHeaders::ContentId()
{
    return (DwMsgId&) FieldBody("Content-Id");
}


DwMechanism& DwHeaders::ContentTransferEncoding()
{
    return (DwMechanism&)
        FieldBody("Content-Transfer-Encoding");
}


DwMechanism& DwHeaders::Cte()
{
    return (DwMechanism&)
        FieldBody("Content-Transfer-Encoding");
}


DwMediaType& DwHeaders::ContentType()
{
    return (DwMediaType&) FieldBody("Content-Type");
}


DwText& DwHeaders::MimeVersion()
{
    return (DwText&) FieldBody("MIME-Version");
}


DwDispositionType& DwHeaders::ContentDisposition()
{
    return (DwDispositionType&) FieldBody("Content-Disposition");
}


#if defined (DW_DEBUG_VERSION)
void DwHeaders::PrintDebugInfo(std::ostream& aStrm, int aDepth) const
{
    aStrm <<
    "---------------- Debug info for DwHeaders class ----------------\n";
    _PrintDebugInfo(aStrm);
    int depth = aDepth - 1;
    depth = (depth >= 0) ? depth : 0;
    if (aDepth == 0 || depth > 0) {
        DwField* field = mFirstField;
        while (field) {
            field->PrintDebugInfo(aStrm, depth);
            field = (DwField*) field->Next();
        }
    }
}
#else
void DwHeaders::PrintDebugInfo(std::ostream& , int ) const {}
#endif // defined (DW_DEBUG_VERSION)


#if defined (DW_DEBUG_VERSION)
void DwHeaders::_PrintDebugInfo(std::ostream& aStrm) const
{
    DwMessageComponent::_PrintDebugInfo(aStrm);
    aStrm << "Fields:           ";
    int count = 0;
    DwField* field = mFirstField;
    while (field) {
        if (count > 0) aStrm << ' ';
        aStrm << field->ObjectId();
        field = (DwField*) field->Next();
        ++count;
    }
    aStrm << '\n';
}
#else
void DwHeaders::_PrintDebugInfo(std::ostream& ) const {}
#endif // defined (DW_DEBUG_VERSION)


void DwHeaders::CheckInvariants() const
{
#if defined (DW_DEBUG_VERSION)
    DwMessageComponent::CheckInvariants();
    DwField* field = mFirstField;
    while (field) {
        field->CheckInvariants();
        assert((DwMessageComponent*) this == field->Parent());
        field = (DwField*) field->Next();
    }
#endif // defined (DW_DEBUG_VERSION)
}


