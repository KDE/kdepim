//=============================================================================
// File:       param.cpp
// Contents:   Definitions for DwParameter
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
#include <mimelib/string.h>
#include <mimelib/param.h>
#include <mimelib/token.h>


const char* const DwParameter::sClassName = "DwParameter";


DwParameter* (*DwParameter::sNewParameter)(const DwString&,
    DwMessageComponent*) = 0;


DwParameter* DwParameter::NewParameter(const DwString& aStr,
    DwMessageComponent* aParent)
{
    if (sNewParameter) {
        return sNewParameter(aStr, aParent);
    }
    else {
        return new DwParameter(aStr, aParent);
    }
}


DwParameter::DwParameter()
{
    mNext = 0;
    mClassId = kCidParameter;
    mClassName = sClassName;
}


DwParameter::DwParameter(const DwParameter& aParam)
  : DwMessageComponent(aParam),
    mAttribute(aParam.mAttribute),
    mValue(aParam.mValue),
    mForceNoQuotes(aParam.mForceNoQuotes)
{
    mNext = 0;
    mClassId = kCidParameter;
    mClassName = sClassName;
}


DwParameter::DwParameter(const DwString& aStr, DwMessageComponent* aParent)
    : DwMessageComponent(aStr, aParent)
{
    mNext = 0;
    mClassId = kCidParameter;
    mClassName = sClassName;
    mForceNoQuotes = false;
}


DwParameter::~DwParameter()
{
}


const DwParameter& DwParameter::operator = (const DwParameter& aParam)
{
    if (this == &aParam) return *this;
    DwMessageComponent::operator = (aParam);
    mAttribute = aParam.mAttribute;
    mValue     = aParam.mValue;
    mForceNoQuotes = aParam.mForceNoQuotes;
    mNext      = 0;
    return *this;
}


const DwString& DwParameter::Attribute() const
{
    return mAttribute;
}


void DwParameter::SetAttribute(const DwString& aAttribute)
{
    mAttribute = aAttribute;
    SetModified();
}


const DwString& DwParameter::Value() const
{
    return mValue;
}


void DwParameter::SetValue(const DwString& aValue, bool forceNoQuote)
{
    mValue = aValue;
    mForceNoQuotes = forceNoQuote;
    SetModified();
}


DwParameter* DwParameter::Next() const
{
    return mNext;
}


void DwParameter::SetNext(DwParameter* aParam)
{
    mNext = aParam;
}


void DwParameter::Parse()
{
    mIsModified = 0;
    mAttribute = mValue = "";
    if (mString.length() == 0) return;
    DwRfc1521Tokenizer tokenizer(mString);
    // Get attribute
    int found = 0;
    while (!found && tokenizer.Type() != eTkNull) {
        if (tokenizer.Type() == eTkToken) {
            mAttribute = tokenizer.Token();
            found = 1;
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
    found = 0;
    while (!found && tokenizer.Type() != eTkNull) {
        if (tokenizer.Type() == eTkToken) {
            mValue = tokenizer.Token();
            found = 1;
        }
        else if (tokenizer.Type() == eTkQuotedString) {
            tokenizer.StripDelimiters();
            mValue = tokenizer.Token();
            found = 1;
        }
        ++tokenizer;
    }
    // Some nonstandard MIME implementations use single quotes to quote
    // the boundary string.  This is incorrect, but we will try to detect
    // it and work with it.
    //
	// If the first character and last character of the boundary string
    // are single quote, strip them off.
    if (DwStrcasecmp(mAttribute, "boundary") == 0) {
        size_t len = mValue.length();
        if (len > 2 && mValue[0] == '\'' && mValue[len-1] == '\'') {
            mValue = mValue.substr(1, len-2);
        }
    }
}


void DwParameter::Assemble()
{
    if (mIsModified == 0) return;
    mString = "";
    mString += mAttribute;
    bool noQuotes = mForceNoQuotes || (DwStrcasecmp(mAttribute, "micalg") == 0);
    if( noQuotes )
      mString += "=";
    else
      mString += "=\"";
    mString += mValue;
    if( !noQuotes )
      mString += "\"";
    mIsModified = 0;
}


DwMessageComponent* DwParameter::Clone() const
{
    return new DwParameter(*this);
}


#if defined (DW_DEBUG_VERSION)
void DwParameter::PrintDebugInfo(std::ostream& aStrm, int /*aDepth*/) const
{
    aStrm <<
    "--------------- Debug info for DwParameter class ---------------\n";
    _PrintDebugInfo(aStrm);
}
#else
void DwParameter::PrintDebugInfo(std::ostream& , int ) const {}
#endif // defined (DW_DEBUG_VERSION)


#if defined (DW_DEBUG_VERSION)
void DwParameter::_PrintDebugInfo(std::ostream& aStrm) const
{
    DwMessageComponent::_PrintDebugInfo(aStrm);
    aStrm << "Attribute:        " << mAttribute << '\n';
    aStrm << "Value:            " << mValue << '\n';
    if (mNext) {
        aStrm << "Next parameter:   " << mNext->ObjectId() << '\n';
    }
    else {
        aStrm << "Next parameter:   " << "(none)\n";
    }
}
#else
void DwParameter::_PrintDebugInfo(std::ostream& ) const {}
#endif // defined (DW_DEBUG_VERSION)


void DwParameter::CheckInvariants() const
{
#if defined (DW_DEBUG_VERSION)
    DwMessageComponent::CheckInvariants();
    mAttribute.CheckInvariants();
    mValue.CheckInvariants();
#endif // defined (DW_DEBUG_VERSION)
}

