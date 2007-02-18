//=============================================================================
// File:       disptype.cpp
// Contents:   Definitions for DwDispositionType
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
#include <mimelib/string.h>
#include <mimelib/param.h>
#include <mimelib/disptype.h>
#include <mimelib/token.h>
#include <mimelib/enum.h>


const char* const DwDispositionType::sClassName = "DwDispositionType";


DwDispositionType* (*DwDispositionType::sNewDispositionType)(
    const DwString&, DwMessageComponent*) = 0;


DwDispositionType* DwDispositionType::NewDispositionType(
    const DwString& aStr, DwMessageComponent* aParent)
{
    if (sNewDispositionType) {
        return sNewDispositionType(aStr, aParent);
    }
    else {
        return new DwDispositionType(aStr, aParent);
    }
}


DwDispositionType::DwDispositionType()
{
    mDispositionType = DwMime::kDispTypeNull;
    mFirstParameter = 0;
    mClassId = kCidDispositionType;
    mClassName = sClassName;
}


DwDispositionType::DwDispositionType(const DwDispositionType& aDispType)
  : DwFieldBody(aDispType),
    mDispositionTypeStr(aDispType.mDispositionTypeStr),
    mFilenameStr(aDispType.mFilenameStr)
{
    mFirstParameter = 0;
    mDispositionType = aDispType.mDispositionType;
    if (aDispType.mFirstParameter) {
        CopyParameterList(aDispType.mFirstParameter);
    }
    mClassId = kCidDispositionType;
    mClassName = sClassName;
}


DwDispositionType::DwDispositionType(const DwString& aStr,
    DwMessageComponent* aParent)
  : DwFieldBody(aStr, aParent)
{
    mDispositionType = DwMime::kDispTypeNull;
    mFirstParameter = 0;
    mClassId = kCidDispositionType;
    mClassName = sClassName;
}


DwDispositionType::~DwDispositionType()
{
    if (mFirstParameter) {
        DeleteParameterList();
    }
}


const DwDispositionType& DwDispositionType::operator = (
    const DwDispositionType& aDispType)
{
    if (this == &aDispType) return *this;
    mDispositionType    = aDispType.mDispositionType;
    mDispositionTypeStr = aDispType.mDispositionTypeStr;
    mFilenameStr        = aDispType.mFilenameStr;

    if (mFirstParameter) {
        DeleteParameterList();
    }
    if (aDispType.mFirstParameter) {
        CopyParameterList(aDispType.mFirstParameter);
    }

    if (mParent) {
        mParent->SetModified();
    }

    return *this;
}


int DwDispositionType::DispositionType() const
{
    return mDispositionType;
}


void DwDispositionType::SetDispositionType(int aType)
{
    mDispositionType = aType;
    EnumToStr();
    SetModified();
}


const DwString& DwDispositionType::DispositionTypeStr() const
{
    return mDispositionTypeStr;
}


void DwDispositionType::SetDispositionTypeStr(const DwString& aStr)
{
    mDispositionTypeStr = aStr;
    StrToEnum();
    SetModified();
}


const DwString& DwDispositionType::Filename() const
{
    DwParameter* param = mFirstParameter;
    while (param) {
        if (DwStrcasecmp(param->Attribute(), "filename") == 0) {
            // Filename parameter found. Return its value.
            // Implementation note: this member function is const, which
            // forbids us from assigning to mFilenameStr.  The following
            // trick gets around this.  (ANSI implementations could use the
            // "mutable" declaration).
            DwDispositionType* _this = (DwDispositionType*) this;
            _this->mFilenameStr = param->Value();
            break;
        }
        param = param->Next();
    }
    return mFilenameStr;
}


void DwDispositionType::SetFilename(const DwString& aStr)
{
    mFilenameStr = aStr;
    // Search for filename parameter in parameter list.  If found, set its
    // value.
    DwParameter* param = mFirstParameter;
    while (param) {
        if (DwStrcasecmp(param->Attribute(), "filename") == 0) {
            param->SetValue(mFilenameStr);
            return;
        }
        param = param->Next();
    }
    // Boundary parameter not found. Add it.
    param = DwParameter::NewParameter("", 0);
    param->SetAttribute("Filename");
    param->SetValue(aStr);
    AddParameter(param);
}


DwParameter* DwDispositionType::FirstParameter() const
{
    return mFirstParameter;
}


void DwDispositionType::AddParameter(DwParameter* aParam)
{
    _AddParameter(aParam);
    SetModified();
}


void DwDispositionType::_AddParameter(DwParameter* aParam)
{
    if (!mFirstParameter) {
        mFirstParameter = aParam;
    }
    else {
        DwParameter* cur = mFirstParameter;
        if( cur ) {
            DwParameter* next = cur->Next();
            while (next) {
                cur = next;
                next = cur->Next();
            }
            cur->SetNext(aParam);
        }
    }
    aParam->SetParent(this);
}


void DwDispositionType::Parse()
{
    mIsModified = 0;
    mDispositionType = DwMime::kDispTypeNull;
    mDispositionTypeStr = "";
    if (mFirstParameter) {
        DeleteParameterList();
    }
    if (mString.length() == 0) return;
    DwRfc1521Tokenizer tokenizer(mString);
    int found = 0;
    while (!found && tokenizer.Type() != eTkNull) {
        if (tokenizer.Type() == eTkToken) {
            mDispositionTypeStr = tokenizer.Token();
            found = 1;
        }
        ++tokenizer;
    }
    // Get parameters
    DwTokenString tokenStr(mString);
    while (1) {
        // Get ';'
        found = 0;
        while (!found && tokenizer.Type() != eTkNull) {
            if (tokenizer.Type() == eTkTspecial
                && tokenizer.Token()[0] == ';') {
                found = 1;
            }
            ++tokenizer;
        }
        if (tokenizer.Type() == eTkNull) {
            // No more parameters
            break;
        }
        tokenStr.SetFirst(tokenizer);
        // Get attribute
        DwString attrib;
        int attribFound = 0;
        while (!attribFound && tokenizer.Type() != eTkNull) {
            if (tokenizer.Type() == eTkToken) {
                attrib = tokenizer.Token();
                attribFound = 1;
            }
            ++tokenizer;
        }
        // Get '='
        found = 0;
        while (!found && tokenizer.Type() != eTkNull) {
            if (tokenizer.Type() == eTkTspecial
                && tokenizer.Token()[0] == '=') {
                found = 1;
            }
            ++tokenizer;
        }
        // Get value
        int valueFound = 0;
        while (!valueFound && tokenizer.Type() != eTkNull) {
            if (tokenizer.Type() == eTkToken
                || tokenizer.Type() == eTkQuotedString) {
                valueFound = 1;
            }
            ++tokenizer;
        }
        if (attribFound && valueFound) {
            tokenStr.ExtendTo(tokenizer);
            DwParameter* param =
                DwParameter::NewParameter(tokenStr.Tokens(), this);
            param->Parse();
            _AddParameter(param);
        }
    }
    StrToEnum();
}


void DwDispositionType::Assemble()
{
    if (!mIsModified) return;
    mString = "";
    if (mDispositionTypeStr.length() == 0)
        return;
    mString += mDispositionTypeStr;
    DwParameter* param = FirstParameter();
    while (param) {
        param->Assemble();
        if (IsFolding()) {
            mString += ";" DW_EOL "  ";
        }
        else {
            mString += "; ";
        }
        mString += param->AsString();
        param = param->Next();
    }
    mIsModified = 0;
}


DwMessageComponent* DwDispositionType::Clone() const
{
    return new DwDispositionType(*this);
}


void DwDispositionType::EnumToStr()
{
    switch (mDispositionType) {
    case DwMime::kDispTypeInline:
        mDispositionTypeStr = "inline";
        break;
    case DwMime::kDispTypeAttachment:
        mDispositionTypeStr = "attachment";
        break;
    }
}


void DwDispositionType::StrToEnum()
{
    switch (mDispositionTypeStr[0]) {
    case 'i':
        if (DwStrcasecmp(mDispositionTypeStr, "inline") == 0) {
            mDispositionType = DwMime::kDispTypeInline;
        }
        else {
            mDispositionType = DwMime::kDispTypeUnknown;
        }
        break;
    case 'a':
        if (DwStrcasecmp(mDispositionTypeStr, "attachment") == 0) {
            mDispositionType = DwMime::kDispTypeAttachment;
        }
        else {
            mDispositionType = DwMime::kDispTypeUnknown;
        }
        break;
    }
}


void DwDispositionType::DeleteParameterList()
{
    DwParameter* param = mFirstParameter;
    while (param) {
        DwParameter* nextParam = param->Next();
        delete param;
        param = nextParam;
    }
    mFirstParameter = 0;
    SetModified();
}


void DwDispositionType::CopyParameterList(DwParameter* aFirst)
{
    DwParameter* param = aFirst;
    while (param) {
        DwParameter* newParam = (DwParameter*) param->Clone();
        AddParameter(newParam);
        param = param->Next();
    }
}


#if defined(DW_DEBUG_VERSION)
void DwDispositionType::PrintDebugInfo(std::ostream& aStrm, int aDepth) const
{
    aStrm <<
    "------------ Debug info for DwDispositionType class ------------\n";
    _PrintDebugInfo(aStrm);
    int depth = aDepth - 1;
    depth = (depth >= 0) ? depth : 0;
    if (aDepth == 0 || depth > 0) {
        DwParameter* param = mFirstParameter;
        while (param) {
            param->PrintDebugInfo(aStrm, depth);
            param = param->Next();
        }
    }
}
#else
void DwDispositionType::PrintDebugInfo(std::ostream&, int) const {}
#endif // defined(DW_DEBUG_VERSION)


#if defined(DW_DEBUG_VERSION)
void DwDispositionType::_PrintDebugInfo(std::ostream& aStrm) const
{
    DwFieldBody::_PrintDebugInfo(aStrm);
    aStrm << "Disposition Type: " << mDispositionTypeStr
        << " (" << mDispositionType    << ")\n";
    aStrm << "Filename:         " << mFilenameStr << "\n";
    aStrm << "Parameters:       ";
    DwParameter* param = mFirstParameter;
    if (param) {
        int count = 0;
        while (param) {
            if (count) aStrm << ' ';
            aStrm << param->ObjectId();
            param = param->Next();
            ++count;
        }
        aStrm << '\n';
    }
    else {
        aStrm << "(none)\n";
    }
}
#else
void DwDispositionType::_PrintDebugInfo(std::ostream& ) const {}
#endif // defined(DW_DEBUG_VERSION)


void DwDispositionType::CheckInvariants() const
{
#if defined(DW_DEBUG_VERSION)
    mDispositionTypeStr.CheckInvariants();
    mFilenameStr.CheckInvariants();
    DwParameter* param = mFirstParameter;
    while (param) {
        param->CheckInvariants();
        assert((DwMessageComponent*) this == param->Parent());
        param = param->Next();
    }
#endif // defined(DW_DEBUG_VERSION)
}
