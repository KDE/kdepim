//=============================================================================
// File:       fieldbdy.cpp
// Contents:   Definitions for DwFieldBody
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
#include <mimelib/fieldbdy.h>
#include <mimelib/field.h>


const char* const DwFieldBody::sClassName = "DwFieldBody";


DwFieldBody::DwFieldBody()
{
    mLineOffset = 0;
    mDoFolding = DwTrue;
    mClassId = kCidFieldBody;
    mClassName = sClassName;
}


DwFieldBody::DwFieldBody(const DwFieldBody& aFieldBody)
  : DwMessageComponent(aFieldBody)
{
    mLineOffset = aFieldBody.mLineOffset;
    mDoFolding = aFieldBody.mDoFolding;
    mClassId = kCidFieldBody;
    mClassName = sClassName;
}


DwFieldBody::DwFieldBody(const DwString& aStr, DwMessageComponent* aParent)
  : DwMessageComponent(aStr, aParent)
{
    mLineOffset = 0;
    mDoFolding = DwTrue;
    mClassId = kCidFieldBody;
    mClassName = sClassName;
}


DwFieldBody::~DwFieldBody()
{
}


const DwFieldBody& DwFieldBody::operator = (const DwFieldBody& aFieldBody)
{
    if (this == &aFieldBody) return *this;
    DwMessageComponent::operator = (aFieldBody);
    mLineOffset = aFieldBody.mLineOffset;
    return *this;
}


#if defined (DW_DEBUG_VERSION)
void DwFieldBody::PrintDebugInfo(std::ostream& aStrm, int /*aDepth*/) const
{
    aStrm <<
    "--------------- Debug info for DwFieldBody class ---------------\n";
    _PrintDebugInfo(aStrm);
}
#else
void DwFieldBody::PrintDebugInfo(std::ostream& , int ) const {}
#endif // defined (DW_DEBUG_VERSION)


#if defined (DW_DEBUG_VERSION)
void DwFieldBody::_PrintDebugInfo(std::ostream& aStrm) const
{
    DwMessageComponent::_PrintDebugInfo(aStrm);
    aStrm << "LineOffset:       " << mLineOffset << '\n';
    aStrm << "IsFolding:        " << (IsFolding() ? "True" : "False") << '\n';
}
#else
void DwFieldBody::_PrintDebugInfo(std::ostream& ) const {}
#endif // defined (DW_DEBUG_VERSION)


void DwFieldBody::CheckInvariants() const
{
#if defined (DW_DEBUG_VERSION)
    DwMessageComponent::CheckInvariants();
#endif // defined (DW_DEBUG_VERSION)
}

