//=============================================================================
// File:       mediatyp.cpp
// Contents:   Definitions for DwMediaType
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
#include <time.h>
#include <mimelib/string.h>
#include <mimelib/param.h>
#include <mimelib/mediatyp.h>
#include <mimelib/token.h>
#include <mimelib/utility.h>
#include <mimelib/enum.h>


const char* const DwMediaType::sClassName = "DwMediaType";


DwMediaType* (*DwMediaType::sNewMediaType)(const DwString&,
    DwMessageComponent*) = 0;


DwMediaType* DwMediaType::NewMediaType(const DwString& aStr,
    DwMessageComponent* aParent)
{
    if (sNewMediaType) {
        return sNewMediaType(aStr, aParent);
    }
    else {
        return new DwMediaType(aStr, aParent);
    }
}


DwMediaType::DwMediaType()
{
    mType = DwMime::kTypeNull;
    mSubtype = DwMime::kSubtypeNull;
    mFirstParameter = 0;
    mClassId = kCidMediaType;
    mClassName = sClassName;
}


DwMediaType::DwMediaType(const DwMediaType& aCntType)
  : DwFieldBody(aCntType),
    mTypeStr(aCntType.mTypeStr),
    mSubtypeStr(aCntType.mSubtypeStr),
    mBoundaryStr(aCntType.mBoundaryStr)
{
    mType = aCntType.mType;
    mSubtype = aCntType.mSubtype;
    mFirstParameter = 0;

    if (aCntType.mFirstParameter) {
        CopyParameterList(aCntType.mFirstParameter);
    }

    mClassId = kCidMediaType;
    mClassName = sClassName;
}


DwMediaType::DwMediaType(const DwString& aStr, DwMessageComponent* aParent)
  : DwFieldBody(aStr, aParent)
{
    mType = DwMime::kTypeNull;
    mSubtype = DwMime::kSubtypeNull;
    mFirstParameter = 0;
    mClassId = kCidMediaType;
    mClassName = sClassName;
}


DwMediaType::~DwMediaType()
{
    if (mFirstParameter) {
        DeleteParameterList();
    }
}


const DwMediaType& DwMediaType::operator = (const DwMediaType& aCntType)
{
    if (this == &aCntType) return *this;
    DwFieldBody::operator = (aCntType);

    mType        = aCntType.mType;
    mSubtype     = aCntType.mSubtype;
    mTypeStr     = aCntType.mTypeStr;
    mSubtypeStr  = aCntType.mSubtypeStr;
    mBoundaryStr = aCntType.mBoundaryStr;

    if (mFirstParameter) {
        DeleteParameterList();
    }
    if (aCntType.mFirstParameter) {
        CopyParameterList(aCntType.mFirstParameter);
    }

    if (mParent) {
        mParent->SetModified();
    }

    return *this;
}


int DwMediaType::Type() const
{
    return mType;
}


void DwMediaType::SetType(int aType)
{
    mType = aType;
    TypeEnumToStr();
    SetModified();
}


const DwString& DwMediaType::TypeStr() const
{
    return mTypeStr;
}


void DwMediaType::SetTypeStr(const DwString& aStr)
{
    mTypeStr = aStr;
    TypeStrToEnum();
    SetModified();
}


int DwMediaType::Subtype() const
{
    return mSubtype;
}


void DwMediaType::SetSubtype(int aSubtype)
{
    mSubtype = aSubtype;
    SubtypeEnumToStr();
    SetModified();
}


const DwString& DwMediaType::SubtypeStr() const
{
    return mSubtypeStr;
}


void DwMediaType::SetSubtypeStr(const DwString& aStr)
{
    mSubtypeStr = aStr;
    SubtypeStrToEnum();
    SetModified();
}


const DwString& DwMediaType::Boundary() const
{
    // Implementation note: this member function is const, which
    // forbids us from assigning to mBoundaryStr.  The following
    // trick gets around this.  (ANSI implementations could use the
    // "mutable" declaration).
    DwMediaType* _this = (DwMediaType*) this;
    _this->mBoundaryStr = "";
    DwParameter* param = mFirstParameter;
    while (param) {
        if (DwStrcasecmp(param->Attribute(), "boundary") == 0) {
            // Boundary parameter found. Return its value.
            _this->mBoundaryStr = param->Value();
            break;
        }
        param = param->Next();
    }
    return mBoundaryStr;
}


void DwMediaType::SetBoundary(const DwString& aStr)
{
    mBoundaryStr = aStr;
    // Search for boundary parameter in parameter list.  If found, set its
    // value.
    DwParameter* param = mFirstParameter;
    while (param) {
        if (DwStrcasecmp(param->Attribute(), "boundary") == 0) {
            param->SetValue(mBoundaryStr);
            return;
        }
        param = param->Next();
    }
    // Boundary parameter not found. Add it.
    param = DwParameter::NewParameter("", 0);
    param->SetAttribute("boundary");
    param->SetValue(aStr);
    AddParameter(param);
}


void DwMediaType::CreateBoundary(unsigned aLevel)
{
    // Create a random printable string and set it as the boundary parameter
    static const char c[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    const int cLen = 64;
    char buf[80];
    strcpy(buf, "Boundary-");
    int pos = strlen(buf);
    int n = aLevel / 10;
    buf[pos++] = (n % 10) + '0';
    n = aLevel;
    buf[pos++] = (n % 10) + '0';
    buf[pos++] = '=';
    buf[pos++] = '_';
    DwUint32 r = (DwUint32) time(0);
    buf[pos++] = c[r % cLen];
    r /= cLen;
    buf[pos++] = c[r % cLen];
    r /= cLen;
    buf[pos++] = c[r % cLen];
    r /= cLen;
    buf[pos++] = c[r % cLen];
    r /= cLen;
    buf[pos++] = c[r % cLen];
    for (int i=0; i < 2; ++i) {
        r = rand();
        buf[pos++] = c[r % cLen];
        r >>= 6;
        buf[pos++] = c[r % cLen];
        r >>= 6;
        buf[pos++] = c[r % cLen];
        r >>= 6;
        buf[pos++] = c[r % cLen];
        r >>= 6;
        buf[pos++] = c[r % cLen];
    }
    buf[pos] = 0;
    SetBoundary(buf);
}


const DwString& DwMediaType::Name() const
{
    // Implementation note: this member function is const, which
    // forbids us from assigning to mNameStr.  The following
    // trick gets around this.  (ANSI implementations could use the
    // "mutable" declaration).
    DwMediaType* _this = (DwMediaType*) this;
    _this->mNameStr = "";
    DwParameter* param = mFirstParameter;
    while (param) {
        if (DwStrcasecmp(param->Attribute(), "name") == 0) {
            // Name parameter found. Return its value.
            _this->mNameStr = param->Value();
            break;
        }
        param = param->Next();
    }
    return mNameStr;
}


void DwMediaType::SetName(const DwString& aStr)
{
    mNameStr = aStr;
    // Search for name parameter in parameter list.  If found, set its
    // value.
    DwParameter* param = mFirstParameter;
    while (param) {
        if (DwStrcasecmp(param->Attribute(), "name") == 0) {
            param->SetValue(mNameStr);
            return;
        }
        param = param->Next();
    }
    // Name parameter not found. Add it.
    param = DwParameter::NewParameter("", 0);
    param->SetAttribute("name");
    param->SetValue(aStr);
    AddParameter(param);
}


DwParameter* DwMediaType::FirstParameter() const
{
    return mFirstParameter;
}


void DwMediaType::AddParameter(DwParameter* aParam)
{
    _AddParameter(aParam);
    SetModified();
}


void DwMediaType::_AddParameter(DwParameter* aParam)
{
    if (!mFirstParameter) {
        mFirstParameter = aParam;
    }
    else {
        DwParameter* cur = mFirstParameter;
        DwParameter* next = cur->Next();
        while (next) {
            cur = next;
            next = cur->Next();
        }
        cur->SetNext(aParam);
    }
    aParam->SetParent(this);
}


void DwMediaType::Parse()
{
    mIsModified = 0;
    mTypeStr = "";
    mSubtypeStr = "";
    mType = DwMime::kTypeNull;
    mSubtype = DwMime::kSubtypeNull;
    if (mFirstParameter) {
        DeleteParameterList();
    }
    if (mString.length() == 0) return;
    DwRfc1521Tokenizer tokenizer(mString);

    // Get type.
    int found = 0;
    while (!found && tokenizer.Type() != eTkNull) {
        if (tokenizer.Type() == eTkToken) {
            mTypeStr = tokenizer.Token();
            found = 1;
        }
        ++tokenizer;
    }
    // Get '/'
    found = 0;
    while (!found && tokenizer.Type() != eTkNull) {
        if (tokenizer.Type() == eTkTspecial
            && tokenizer.Token()[0] == '/') {
            found = 1;
        }
        ++tokenizer;
    }
    // Get subtype
    found = 0;
    while (!found && tokenizer.Type() != eTkNull) {
        if (tokenizer.Type() == eTkToken) {
            mSubtypeStr = tokenizer.Token();
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
        // Get value but do _not_ stop when finding a '/' in it
        int valueFound = 0;
        while (!valueFound && tokenizer.Type() != eTkNull) {
            if (tokenizer.Type() == eTkToken
                || tokenizer.Type() == eTkQuotedString) {
                ++tokenizer;
                if (tokenizer.Type() != eTkTspecial
                    || tokenizer.Token()[0] != '/')
                    valueFound = 1;
            }
            else
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
    TypeStrToEnum();
    SubtypeStrToEnum();
}


void DwMediaType::Assemble()
{
    if (!mIsModified) return;
    mString = "";
    if (mTypeStr.length() == 0 || mSubtypeStr.length() == 0)
        return;
    mString += mTypeStr;
    mString += '/';
    mString += mSubtypeStr;
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


DwMessageComponent* DwMediaType::Clone() const
{
    return new DwMediaType(*this);
}


void DwMediaType::TypeEnumToStr()
{
    DwTypeEnumToStr(mType, mTypeStr);
}


void DwMediaType::TypeStrToEnum()
{
    mType = DwTypeStrToEnum(mTypeStr);

}


void DwMediaType::SubtypeEnumToStr()
{
    DwSubtypeEnumToStr(mSubtype, mSubtypeStr);
}


void DwMediaType::SubtypeStrToEnum()
{
    mSubtype = DwSubtypeStrToEnum(mSubtypeStr);

}


void DwMediaType::DeleteParameterList()
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


void DwMediaType::CopyParameterList(DwParameter* aFirst)
{
    DwParameter* param = aFirst;
    while (param) {
        DwParameter* newParam = (DwParameter*) param->Clone();
        AddParameter(newParam);
        param = param->Next();
    }
}


#if defined(DW_DEBUG_VERSION)
void DwMediaType::PrintDebugInfo(std::ostream& aStrm, int aDepth) const
{
    aStrm <<
    "--------------- Debug info for DwMediaType class ---------------\n";
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
void DwMediaType::PrintDebugInfo(std::ostream& , int ) const {}
#endif // defined(DW_DEBUG_VERSION)


#if defined(DW_DEBUG_VERSION)
void DwMediaType::_PrintDebugInfo(std::ostream& aStrm) const
{
    DwFieldBody::_PrintDebugInfo(aStrm);
    aStrm << "Type:             " << mTypeStr    << " (" << mType    << ")\n";
    aStrm << "Subtype:          " << mSubtypeStr << " (" << mSubtype << ")\n";
    aStrm << "Boundary:         " << mBoundaryStr << '\n';
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
void DwMediaType::_PrintDebugInfo(std::ostream& ) const {}
#endif // defined(DW_DEBUG_VERSION)


void DwMediaType::CheckInvariants() const
{
#if defined(DW_DEBUG_VERSION)
    mTypeStr.CheckInvariants();
    mSubtypeStr.CheckInvariants();
    mBoundaryStr.CheckInvariants();
    DwParameter* param = mFirstParameter;
    while (param) {
        param->CheckInvariants();
        assert((DwMessageComponent*) this == param->Parent());
        param = param->Next();
    }
#endif // defined(DW_DEBUG_VERSION)
}
