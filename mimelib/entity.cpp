//=============================================================================
// File:       entity.cpp
// Contents:   Definitions for DwEntity
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
#include <mimelib/string.h>
#include <mimelib/enum.h>
#include <mimelib/entity.h>
#include <mimelib/headers.h>
#include <mimelib/body.h>
#include <mimelib/mediatyp.h>


class DwEntityParser {
    friend class DwEntity;
private:
    DwEntityParser(const DwString&);
    void Parse();
    const DwString mString;
    DwString mHeaders;
    DwString mBody;
};


DwEntityParser::DwEntityParser(const DwString& aStr)
  : mString(aStr)
{
    Parse();
}


void DwEntityParser::Parse()
{
    const char* buf = mString.data();
    size_t bufEnd = mString.length();
    size_t pos = 0;
    size_t headersStart = 0;
    size_t headersLength = 0;
    size_t lineStart = pos;
    DwBool isHeaderLine = DwFalse;
    // If first character is a LF (ANSI C or UNIX)
    // or if first two characters are CR LF (MIME or DOS),
    // there are no headers.
    if (pos < bufEnd && buf[pos] != '\n'
        && ! (buf[pos] == '\r' && pos+1 < bufEnd && buf[pos+1] == '\n')) {

        while (pos < bufEnd) {
            // End of line marked by LF
            if (buf[pos] == '\n') {
                ++pos;
                if (!isHeaderLine) {
                    pos = lineStart;
                    break;
                }
                // Check for LF LF
                else if (pos+1 < bufEnd && buf[pos+1] == '\n') {
                    break;
                }
                lineStart = pos;
                isHeaderLine = DwFalse;
            }
            // End of line marked by CRLF
            else if (buf[pos] == '\r' && pos+1 < bufEnd
                && buf[pos+1] == '\n') {
                pos += 2;
                if (!isHeaderLine) {
                    pos = lineStart;
                    break;
                }
                // Check for CR LF CR LF
                else if (pos+1 < bufEnd && buf[pos] == '\r'
                    && buf[pos+1] == '\n') {
                    break;
                }
                lineStart = pos;
                isHeaderLine = DwFalse;
            }
            // End of line marked by CRLF
            else if (buf[pos] == '\r' && pos+1 < bufEnd
                && buf[pos+1] == '\n') {
                pos += 2;
                if (!isHeaderLine) {
                    pos = lineStart;
                    break;
                }
                // Check for CR LF CR LF
                else if (pos+1 < bufEnd && buf[pos] == '\r'
                    && buf[pos+1] == '\n') {
                    break;
                }
                lineStart = pos;
                isHeaderLine = DwFalse;
            }
            else if (buf[pos] == ':') {
                isHeaderLine = DwTrue;
                ++pos;
            }
            else if (pos == lineStart &&
                (buf[pos] == ' ' || buf[pos] == '\t')) {
                isHeaderLine = DwTrue;
                ++pos;
                            }
            else {
                ++pos;
            }
        }
    }
    headersLength = pos;
    mHeaders = mString.substr(headersStart, headersLength);
    // Skip blank line
    // LF (ANSI C or UNIX)
    if (pos < bufEnd && buf[pos] == '\n') {
        ++pos;
    }
    // CR LF (MIME or DOS)
    else if (pos < bufEnd && buf[pos] == '\r'
        && pos+1 < bufEnd && buf[pos+1] == '\n') {

        pos += 2;
    }
    size_t bodyStart = pos;
    size_t bodyLength = mString.length() - bodyStart;
    mBody = mString.substr(bodyStart, bodyLength);
}


//==========================================================================


const char* const DwEntity::sClassName = "DwEntity";


DwEntity::DwEntity()
{
    mHeaders = DwHeaders::NewHeaders("", this);
    ASSERT(mHeaders != 0);
    mBody = DwBody::NewBody("", this);
    ASSERT(mBody != 0);
    mClassId = kCidEntity;
    mClassName = sClassName;
    mBodySize = -1;
}


DwEntity::DwEntity(const DwEntity& aEntity)
  : DwMessageComponent(aEntity)
{
    mHeaders = (DwHeaders*) aEntity.mHeaders->Clone();
    ASSERT(mHeaders != 0);
    mHeaders->SetParent(this);
    mBody = (DwBody*) aEntity.mBody->Clone();
    ASSERT(mBody != 0);
    mBody->SetParent(this);
    mClassId = kCidEntity;
    mClassName = sClassName;
    mBodySize = aEntity.mBodySize;
}


DwEntity::DwEntity(const DwString& aStr, DwMessageComponent* aParent)
  : DwMessageComponent(aStr, aParent)
{
    mHeaders = DwHeaders::NewHeaders("", this);
    ASSERT(mHeaders != 0);
    mBody = DwBody::NewBody("", this);
    ASSERT(mBody != 0);
    mClassId = kCidEntity;
    mClassName = sClassName;
    mBodySize = -1;
}


DwEntity::~DwEntity()
{
    delete mHeaders;
    delete mBody;
}


const DwEntity& DwEntity::operator = (const DwEntity& aEntity)
{
    if (this == &aEntity) return *this;
    DwMessageComponent::operator = (aEntity);
    // Note: Because of the derived assignment problem, we cannot use the
    // assignment operator for DwHeaders and DwBody in the following.
    delete mHeaders;
    mHeaders = (DwHeaders*) aEntity.mHeaders->Clone();
    ASSERT(mHeaders != 0);
    mHeaders->SetParent(this);
    delete mBody;
    mBody = (DwBody*) aEntity.mBody->Clone();
    ASSERT(mBody != 0);
    mBody->SetParent(this);
    if (mParent) {
        mParent->SetModified();
    }
    return *this;
}


void DwEntity::Parse()
{
    mIsModified = 0;
    DwEntityParser parser(mString);
    mHeaders->FromString(parser.mHeaders);
    mHeaders->Parse();
    mBody->FromString(parser.mBody);
    mBody->Parse();
}


void DwEntity::Assemble(DwHeaders& aHeaders, DwBody& aBody)
{
    mString = "";
    mString += aHeaders.AsString();

    int len = mString.length();
#if defined(DW_EOL_CRLF)
    if (len>=3 && (mString[len-1]!='\n' || mString[len-3]!='\n'))
#else
    if (len>=2 && (mString[len-1]!='\n' || mString[len-2]!='\n'))
#endif
    {
        if ( (mString[len-1] == '\n') &&
             mHeaders->HasContentType() &&
             (mHeaders->ContentType().Type()    == DwMime::kTypeMultipart) &&
	     aBody.FirstBodyPart() )
        {
            /* this is the case when we should not add an DW_EOL since
               another newline is already added by DwBody::Assemble()
               right before a boundary string.
               Why is this bad for multipart-mixed? Well, multipart-
               mixed can be part of a multipart-signed and an additional
               empty line makes the mail invalid for a verification
               (the signature was computed based on the same content,
               but with a single newline).

	       We check for the existence of the first body part since
	       we treat the body as opaque in case a failure to parse
	       multipart-subtypes occurs.
            */
        }
        else
            mString += DW_EOL;
    }
    mString += aBody.AsString();
    mIsModified = 0;
}


void DwEntity::Assemble()
{
    if (!mIsModified) return;
    mBody->Assemble();
    mHeaders->Assemble();
    Assemble( *mHeaders, *mBody );
}


DwHeaders& DwEntity::Headers() const
{
    ASSERT(mHeaders != 0);
    return *mHeaders;
}


DwBody& DwEntity::Body() const
{
    return *mBody;
}

int DwEntity::BodySize() const
{
  if ( mBody->AsString().length() > 0 )
    return mBody->AsString().length();
  else if ( mBodySize > 0 )
    return mBodySize;
  else
    return 0;
}


#if defined(DW_DEBUG_VERSION)
void DwEntity::PrintDebugInfo(std::ostream& aStrm, int aDepth) const
{
    aStrm << "------------ Debug info for DwEntity class ------------\n";
    _PrintDebugInfo(aStrm);
    int depth = aDepth - 1;
    depth = (depth >= 0) ? depth : 0;
    if (aDepth == 0 || depth > 0) {
        mHeaders->PrintDebugInfo(aStrm, depth);
        mBody->PrintDebugInfo(aStrm, depth);
    }
}
#else
void DwEntity::PrintDebugInfo(std::ostream& , int ) const {}
#endif // defined(DW_DEBUG_VERSION)


#if defined(DW_DEBUG_VERSION)
void DwEntity::_PrintDebugInfo(std::ostream& aStrm) const
{
    DwMessageComponent::_PrintDebugInfo(aStrm);
    aStrm << "Headers:          " << mHeaders->ObjectId() << '\n';
    aStrm << "Body:             " << mBody->ObjectId() << '\n';
}
#else
void DwEntity::_PrintDebugInfo(std::ostream& ) const {}
#endif // defined(DW_DEBUG_VERSION)


void DwEntity::CheckInvariants() const
{
#if defined(DW_DEBUG_VERSION)
    DwMessageComponent::CheckInvariants();
    mHeaders->CheckInvariants();
    assert((DwMessageComponent*) this == mHeaders->Parent());
    mBody->CheckInvariants();
    assert((DwMessageComponent*) this == mBody->Parent());
#endif // defined(DW_DEBUG_VERSION)
}
