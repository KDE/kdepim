//=============================================================================
// File:       text.cpp
// Contents:   Definitions for DwText
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
#include <mimelib/text.h>


const char* const DwText::sClassName = "DwText";


DwText* (*DwText::sNewText)(const DwString&, DwMessageComponent*) = 0;


DwText* DwText::NewText(const DwString& aStr, DwMessageComponent* aParent)
{
    DwText* text;
    if (sNewText) {
        text = sNewText(aStr, aParent);
    }
    else {
        text = new DwText(aStr, aParent);
    }
    return text;
}


DwText::DwText()
{
    mClassId = kCidText;
    mClassName = sClassName;
}


DwText::DwText(const DwText& aText)
  : DwFieldBody(aText)
{
    mClassId = kCidText;
    mClassName = sClassName;
}


DwText::DwText(const DwString& aStr, DwMessageComponent* aParent)
  : DwFieldBody(aStr, aParent)
{
    mClassId = kCidText;
    mClassName = sClassName;
}


DwText::~DwText()
{
}


const DwText& DwText::operator = (const DwText& aText)
{
    if (this == &aText) return *this;
    DwFieldBody::operator = (aText);
    return *this;
}


void DwText::Parse()
{
    mIsModified = 0;
}


void DwText::Assemble()
{
    mIsModified = 0;
}


DwMessageComponent* DwText::Clone() const
{
    return new DwText(*this);
}


#if defined (DW_DEBUG_VERSION)
void DwText::PrintDebugInfo(std::ostream& aStrm, int /*aDepth*/) const
{
    aStrm <<
    "------------------ Debug info for DwText class -----------------\n";
    _PrintDebugInfo(aStrm);
}
#else
void DwText::PrintDebugInfo(std::ostream& , int ) const {}
#endif // defined (DW_DEBUG_VERSION)


#if defined (DW_DEBUG_VERSION)
void DwText::_PrintDebugInfo(std::ostream& aStrm) const
{
    DwFieldBody::_PrintDebugInfo(aStrm);
}
#else
void DwText::_PrintDebugInfo(std::ostream& ) const {}
#endif // defined (DW_DEBUG_VERSION)


void DwText::CheckInvariants() const
{
#if defined (DW_DEBUG_VERSION)
    DwFieldBody::CheckInvariants();
#endif // defined (DW_DEBUG_VERSION)
}

