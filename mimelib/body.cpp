//=============================================================================
// File:       body.cpp
// Contents:   Definitions for DwBody
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
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <iostream>
#include <mimelib/string.h>
#include <mimelib/headers.h>
#include <mimelib/bodypart.h>
#include <mimelib/body.h>
#include <mimelib/message.h>
#include <mimelib/mediatyp.h>
#include <mimelib/enum.h>

enum {
    kParseSuccess,
    kParseFail
};


struct DwBodyPartStr {
    DwBodyPartStr(const DwString& aStr) : mString(aStr), mNext(0) {}
    DwString mString;
    DwBodyPartStr* mNext;
};


class DwBodyParser {
    friend class DwBody;
public:
    ~DwBodyParser();
private:
    DwBodyParser(const DwString& aStr, const DwString& aBoundaryStr);
    const DwString& Preamble() const     { return mPreamble; }
    const DwString& Epilogue() const     { return mEpilogue; }
    DwBodyPartStr* FirstBodyPart() const { return mFirstBodyPartStr; }
    int Parse();
    int FindBoundary(size_t aStartPos, size_t* aBoundaryStart,
        size_t* aBoundaryEnd, size_t* isFinal) const;
    void AddPart(size_t start, size_t len);
    void DeleteParts();
    const DwString mString;
    const DwString mBoundary;
    DwString mPreamble;
    DwBodyPartStr* mFirstBodyPartStr;
    DwString mEpilogue;
};


DwBodyParser::DwBodyParser(const DwString& aStr, const DwString& aBoundary)
  : mString(aStr), mBoundary(aBoundary)
{
    mFirstBodyPartStr = 0;
    Parse();
}


DwBodyParser::~DwBodyParser()
{
    DeleteParts();
}


int DwBodyParser::Parse()
{
    DeleteParts();
    // Find the preamble
    size_t pos = 0;
    size_t boundaryStart, boundaryEnd, isFinal;
    int result;
    result = FindBoundary(pos, &boundaryStart, &boundaryEnd, &isFinal);
    if (result == kParseFail) {
        mPreamble = mEpilogue = "";
        mFirstBodyPartStr = 0;
        return kParseFail;
    }
    int start = pos;
    int len = boundaryStart - pos;
    mPreamble = mString.substr(start, len);
    if ( boundaryStart < mString.size() && mString[boundaryStart] != '-' )
      mPreamble += DW_EOL; // contrary to normal behaviour of
			   // DwBody::Parse(), we _do_ want a newline
			   // before the first boundary here. This is
			   // necessary since FindBoundary() can't
			   // make up it's mind on where the boundary
			   // starts - on the leading \n or the first
			   // '-'..

    // Find the body parts
    pos = boundaryEnd;
    while (1) {
        result = FindBoundary(pos, &boundaryStart, &boundaryEnd, &isFinal);
        // NOTE: For enhanced fault tolerance we *accept* a missing last
        //       boundary.
        //       If no last boundary is found (but at leat a first one was
        //       there) we just assume the end of the text ebing the end
        //       of the last part.
        //       By doing so we can safely parse some buggy MS Outlook
        //       clients' messages.                      (khz, 12.06.2002)
        start = pos;
        
        if (result == kParseFail) {
            isFinal = true;
            len = mString.length() - pos;
        } else {
            len = boundaryStart - pos;
        }
        
        AddPart(start, len);
        
        if (result == kParseFail) {
            pos = mString.length();
        } else {
            pos = boundaryEnd;
        }
        
        if (isFinal) {
            break;
        }
    }

    // Find the epilogue
    start = pos;
    len = mString.length() - pos;
    if( len )
        mEpilogue = mString.substr(start, len);

    return kParseSuccess;
}

// checks whether [cur,end[ matches -*[\r\t ]*(\n|$)
static bool isOnlyWhiteSpaceOrDashesUntilEndOfLine( const char * cur, const char * end ) {
  bool dashesStillAllowed = true;

  while ( cur < end )
    switch( *cur ) {
    case ' ':
    case '\t':
    case '\r':
      dashesStillAllowed = false;
      ++cur;
      continue;
    case '\n':
      return true;
    case '-':
      if ( !dashesStillAllowed )
	return false;
      ++cur;
      continue;
    default:
      return false;
    }
  // end of buffer is ok, too:
  return true;
}

int DwBodyParser::FindBoundary(size_t aStartPos, size_t* aBoundaryStart,
    size_t* aBoundaryEnd, size_t* aIsFinal) const
{
    // Assume the starting position is the beginning of a line
    const char* buf = mString.data();
    size_t pos = aStartPos;
    size_t endPos = mString.length();
    size_t blen = mBoundary.length();
    // Search for the first boundary.
    // The leading CR LF ('\n') is part of the boundary, but if there is
    // no preamble, there may be no leading CR LF ('\n').
    // The case of no leading CR LF ('\n') is a special case that will occur
    // only when '-' is the first character of the body.
    if (buf[pos] == '-'
        && pos+blen+1 < endPos
        && buf[pos+1] == '-'
        && strncmp(&buf[pos+2], mBoundary.data(), blen) == 0
	&& isOnlyWhiteSpaceOrDashesUntilEndOfLine( buf + pos + blen + 2, buf + endPos ) ) {

        *aBoundaryStart = pos;
        pos += blen + 2;
        // Check for final boundary
        if (pos+1 < endPos
            && buf[pos] == '-'
            && buf[pos+1] == '-') {

            pos += 2;
            *aIsFinal = 1;
        }
        else {
            *aIsFinal = 0;
        }
        // Advance position past end of line
        while (pos < endPos) {
            if (buf[pos] == '\n') {
                ++pos;
                break;
            }
            ++pos;
        }
        *aBoundaryEnd = pos;
        return kParseSuccess;
    }
    int isFound = 0;
    while (pos+blen+2 < endPos) {
        // Case of leading LF
        if (buf[pos] == '\n'
            && buf[pos+1] == '-'
            && buf[pos+2] == '-'
            && strncmp(&buf[pos+3], mBoundary.data(), blen) == 0
	    && isOnlyWhiteSpaceOrDashesUntilEndOfLine( buf + pos + blen + 3, buf + endPos ) ) {

            *aBoundaryStart = pos;
            pos += blen + 3;
            isFound = 1;
        }
        // Case of leading CR LF
        else if (buf[pos] == '\r'
            && buf[pos+1] == '\n'
            && buf[pos+2] == '-'
            && pos+blen+3 < endPos
            && buf[pos+3] == '-'
            && strncmp(&buf[pos+4], mBoundary.data(), blen) == 0
	    && isOnlyWhiteSpaceOrDashesUntilEndOfLine( buf + pos + blen + 4, buf + endPos ) ) {

            *aBoundaryStart = pos;
            pos += blen + 4;
            isFound = 1;
        }
        if (isFound) {
            // Check for final boundary
            if (pos < endPos
                && buf[pos] == '-') {

                // NOTE: Since we must be fault tolerant for being able to
                //       understand messaged that were damaged during
                //       transportation we now accept final boundaries
                //       ending with "-" instead of "--".
                //       (khz, 12.06.2002)
                pos += 1;
                *aIsFinal = 1;

                // if there *is* the 2nd '-' we of course process it
                if (pos+1 < endPos
                    && buf[pos+1] == '-') {
                    pos += 1;
                }
            }
            else {
                *aIsFinal = 0;
            }
            // Advance position past end of line
            while (pos < endPos) {
                if (buf[pos] == '\n') {
                    ++pos;
                    break;
                }
                ++pos;
            }
            *aBoundaryEnd = pos;
            return kParseSuccess;
        }
        ++pos;
    }
    // Exceptional case: no boundary found
    *aBoundaryStart = *aBoundaryEnd = mString.length();
    *aIsFinal = 1;
    return kParseFail;
}


void DwBodyParser::AddPart(size_t start, size_t len)
{
    DwBodyPartStr* toAdd = new DwBodyPartStr(mString.substr(start, len));
    if (toAdd != 0) {
        DwBodyPartStr* curr = mFirstBodyPartStr;
        if (curr == 0) {
            mFirstBodyPartStr = toAdd;
            return;
        }
        while (curr->mNext != 0) {
            curr = curr->mNext;
        }
        curr->mNext = toAdd;
    }
}


void DwBodyParser::DeleteParts()
{
    DwBodyPartStr* curr = mFirstBodyPartStr;
    while (curr) {
        DwBodyPartStr* next = curr->mNext;
        delete curr;
        curr = next;
    }
    mFirstBodyPartStr = 0;
}


//==========================================================================


const char* const DwBody::sClassName = "DwBody";


DwBody* (*DwBody::sNewBody)(const DwString&, DwMessageComponent*) = 0;


DwBody* DwBody::NewBody(const DwString& aStr, DwMessageComponent* aParent)
{
    if (sNewBody) {
        DwBody* newBody = sNewBody(aStr, aParent);
        //if( newBody )
        //    newBody->mFirstBodyPart = 0;
        return newBody;
    }
    else {
        return new DwBody(aStr, aParent);
    }
}


DwBody::DwBody()
{
    mFirstBodyPart = 0;
    mMessage = 0;
    mClassId = kCidBody;
    mClassName = sClassName;
}


DwBody::DwBody(const DwBody& aBody)
  : DwMessageComponent(aBody),
    mBoundaryStr(aBody.mBoundaryStr),
    mPreamble(aBody.mPreamble),
    mEpilogue(aBody.mEpilogue)
{
    mFirstBodyPart = 0;
    const DwBodyPart* firstPart = aBody.mFirstBodyPart;
    if (firstPart) {
        CopyBodyParts(firstPart);
    }
    mMessage = 0;
    const DwMessage* message = aBody.mMessage;
    if (message) {
        DwMessage* msg = (DwMessage*) message->Clone();
        _SetMessage(msg);
    }
    mClassId = kCidBody;
    mClassName = sClassName;
}


DwBody::DwBody(const DwString& aStr, DwMessageComponent* aParent)
  : DwMessageComponent(aStr, aParent)
{
    mFirstBodyPart = 0;
    mMessage = 0;
    mClassId = kCidBody;
    mClassName = sClassName;
}


DwBody::~DwBody()
{
    if (mFirstBodyPart) {
        DeleteBodyParts();
    }
    if (mMessage) {
        delete mMessage;
    }
}


const DwBody& DwBody::operator = (const DwBody& aBody)
{
    if (this == &aBody) return *this;
    mBoundaryStr = aBody.mBoundaryStr;
    mPreamble    = aBody.mPreamble;
    mEpilogue    = aBody.mEpilogue;
    if (mFirstBodyPart) {
        DeleteBodyParts();
    }
    const DwBodyPart* firstPart = aBody.FirstBodyPart();
    if (firstPart) {
        CopyBodyParts(firstPart);
    }
    if (mMessage) {
        delete mMessage;
    }
    const DwMessage* message = aBody.Message();
    if (message) {
        DwMessage* msg = (DwMessage*) message->Clone();
        _SetMessage(msg);
    }
    if (mParent) {
        mParent->SetModified();
    }
    return *this;
}


void DwBody::Parse()
{
    mIsModified = 0;
    // Only types "multipart" and "message" need to be parsed, and
    // we cannot determine the type if there is no header.
    if (!mParent) {
        return;
    }
    // Get the content type from the headers
    DwEntity* entity = (DwEntity*) mParent;
    if (entity->Headers().HasContentType()) {
        const DwMediaType& contentType = entity->Headers().ContentType();
        int type = contentType.Type();
        int subtype = contentType.Subtype();
        if (type == DwMime::kTypeMultipart) {
            mBoundaryStr = contentType.Boundary();
            // Now parse body into body parts
            DwBodyParser parser(mString, mBoundaryStr);
            mPreamble = parser.Preamble();
            mEpilogue = parser.Epilogue();
            DwBodyPartStr* partStr = parser.FirstBodyPart();
            while (partStr) {
                DwBodyPart* part =
                    DwBodyPart::NewBodyPart(partStr->mString, this);
                part->Parse();
                _AddBodyPart(part);
                partStr = partStr->mNext;
            }
        }
        else if (type == DwMime::kTypeMessage &&
                 subtype == DwMime::kSubtypeRfc822) {
            if (mMessage)
              mMessage->FromString(mString);
            else
              mMessage = DwMessage::NewMessage(mString, this);
            mMessage->Parse();
        }
    }
}


void DwBody::Assemble()
{
    if (!mIsModified) return;
    if (!mFirstBodyPart && !mMessage) return;
    if (!mParent) return;

    DwEntity* entity = (DwEntity*) mParent;
    /*
    DwString partStr;
    */
    const DwMediaType& contentType = entity->Headers().ContentType();
    int type = contentType.Type();
    int subtype = contentType.Subtype();
    if (type == DwMime::kTypeMultipart) {
        /*
        int len;
        */
        mBoundaryStr = contentType.Boundary();
        mString = "";
        mString += mPreamble;
        DwBodyPart* part = mFirstBodyPart;
        while (part) {
            part->Assemble();
            /*
            partStr = part->AsString();
            len = mString.length();
            if( ! (    (    (1 < len)
                         && ('\n' == mString.at(len-1) )
                         && ('\n' == mString.at(len-2) ) )
                    || (    (2 < len)
                         && ('\n' == mString.at(len-1) )
                         && ('\r' == mString.at(len-2) )
                         && ('\n' == mString.at(len-3) ) ) ) )
            */
	    if ( part != mFirstBodyPart )
                mString += DW_EOL;
            mString += "--";
            mString += mBoundaryStr;
            /*
            len = partStr.length();
            if( ! (    (0 < len)
                    && (    ('\n' == partStr.at(0) )
                         || ('\r' == partStr.at(0) ) ) ) )
            */
                mString += DW_EOL;
            /*
            mString += partStr;
            */
            mString += part->AsString();
            part = part->Next();
        }
        /*
        if( ! (    (    (1 < len)
                    && ('\n' == mString.at(len-1) )
                    && ('\n' == mString.at(len-2) ) )
                || (    (2 < len)
                    && ('\n' == mString.at(len-1) )
                    && ('\r' == mString.at(len-2) )
                    && ('\n' == mString.at(len-3) ) ) ) )
        */
            mString += DW_EOL;
        mString += "--";
        mString += mBoundaryStr;
        mString += "--";
        mString += DW_EOL;
        mString += mEpilogue;
        mIsModified = 0;
    }
    else if (type == DwMime::kTypeMessage &&
             subtype == DwMime::kSubtypeRfc822 &&
             mMessage) {
        mMessage->Assemble();
        mString = mMessage->AsString();
    }
    else {
        // Empty block
    }
}


DwMessageComponent* DwBody::Clone() const
{
    return new DwBody(*this);
}


DwBodyPart* DwBody::FirstBodyPart() const
{
    return mFirstBodyPart;
}


void DwBody::AddBodyPart(DwBodyPart* aPart)
{
    _AddBodyPart(aPart);
    SetModified();
}


DwMessage* DwBody::Message() const
{
    return mMessage;
}


void DwBody::SetMessage(DwMessage* aMessage)
{
    _SetMessage(aMessage);
    SetModified();
}


void DwBody::_AddBodyPart(DwBodyPart* aPart)
{
    aPart->SetParent(this);
    if (!mFirstBodyPart) {
        mFirstBodyPart = aPart;
        return;
    }
    DwBodyPart* part = mFirstBodyPart;
    while (part->Next()) {
        part = part->Next();
    }
    part->SetNext(aPart);
}


void DwBody::_SetMessage(DwMessage* aMessage)
{
    aMessage->SetParent(this);
    if (mMessage && mMessage != aMessage) {
        delete mMessage;
    }
    mMessage = aMessage;
}


void DwBody::DeleteBodyParts()
{
    DwBodyPart* part = mFirstBodyPart;
    while (part) {
        DwBodyPart* nextPart = part->Next();
        delete part;
        part = nextPart;
    }
    mFirstBodyPart = 0;
}


void DwBody::CopyBodyParts(const DwBodyPart* aFirst)
{
    const DwBodyPart* part = aFirst;
    while (part) {
        DwBodyPart* newPart = (DwBodyPart*) part->Clone();
        AddBodyPart(newPart);
        part = part->Next();
    }
}


#if defined(DW_DEBUG_VERSION)
void DwBody::PrintDebugInfo(std::ostream& aStrm, int /*aDepth*/) const
{
    aStrm <<
    "------------------ Debug info for DwBody class -----------------\n";
    _PrintDebugInfo(aStrm);
}
#else
void DwBody::PrintDebugInfo(std::ostream&, int) const {}
#endif // defined(DW_DEBUG_VERSION)


#if defined(DW_DEBUG_VERSION)
void DwBody::_PrintDebugInfo(std::ostream& aStrm) const
{
    DwMessageComponent::_PrintDebugInfo(aStrm);
    aStrm << "Boundary:         " << mBoundaryStr << '\n';
    aStrm << "Preamble:         " << mPreamble << '\n';
    aStrm << "Epilogue:         " << mEpilogue << '\n';
    aStrm << "Body Parts:       ";
    int count = 0;
    DwBodyPart* bodyPart = mFirstBodyPart;
    if (bodyPart) {
        while (bodyPart) {
            if (count > 0) aStrm << ' ';
            aStrm << bodyPart->ObjectId();
            bodyPart = (DwBodyPart*) bodyPart->Next();
            ++count;
        }
        aStrm << '\n';
    }
    else {
        aStrm << "(none)\n";
    }
    aStrm << "Message:       ";
    if (mMessage) {
        aStrm << mMessage->ObjectId() << '\n';
    }
    else {
        aStrm << "(none)\n";
    }
}
#else
void DwBody::_PrintDebugInfo(std::ostream& ) const {}
#endif // defined(DW_DEBUG_VERSION)


void DwBody::CheckInvariants() const
{
#if defined(DW_DEBUG_VERSION)
    DwMessageComponent::CheckInvariants();
    mBoundaryStr.CheckInvariants();
    mPreamble.CheckInvariants();
    mEpilogue.CheckInvariants();
    DwBodyPart* bodyPart = mFirstBodyPart;
    while (bodyPart) {
        bodyPart->CheckInvariants();
        bodyPart = (DwBodyPart*) bodyPart->Next();
    }
    if (mMessage) {
        mMessage->CheckInvariants();
    }
#endif // defined(DW_DEBUG_VERSION)
}
