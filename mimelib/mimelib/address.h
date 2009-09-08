//=============================================================================
// File:       address.h
// Contents:   Declarations for DwAddress
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

#ifndef DW_ADDRESS_H
#define DW_ADDRESS_H

#ifndef DW_CONFIG_H
#include <mimelib/config.h>
#endif

#ifndef DW_FIELDBDY_H
#include <mimelib/fieldbdy.h>
#endif

#ifndef DW_TOKEN_H
#include <mimelib/token.h>
#endif

class DwAddressList;

//=============================================================================
//+ Name DwAddress -- Abstract class representing an RFC-822 address
//+ Description
//. {\tt DwAddress} represents an {\it address} as described in RFC-822.
//. You may not instantiate objects of type {\tt DwAddress}, since
//. {\tt DwAddress} is an abstract base class.  Instead, you must instantiate
//. objects of type {\tt DwMailbox} or {\tt DwGroup}, which are subclasses
//. of {\tt DwAddress}.
//.
//. To determine the actual type of a {\tt DwAddress} object, you can use
//. the member functions {\tt IsMailbox()} and {\tt IsGroup()}.
//.
//. If the string representation assigned to a {\tt DwAddress} is improperly
//. formed, the parse method will fail.  To determine if the parse method
//. failed, call the member function {\tt IsValid()}.
//.
//. A {\tt DwAddress} object can be contained in list.  To get the next
//. {\tt DwAddress} object in the list, call the member function {\tt Next()}.
//=============================================================================
// Last modified 1997-08-23
//+ Noentry ~DwAddress mNext mIsValid sClassName _PrintDebugInfo


class DW_EXPORT DwAddress : public DwFieldBody {

    friend class DwAddressList;

public:

    virtual ~DwAddress();

    DwBool IsMailbox() const;
    //. Returns true value if this object is a {\tt DwMailbox}.

    DwBool IsGroup() const;
    //. Returns true value if this object is a {\tt DwGroup}.

    inline DwBool IsValid() const;
    //. Returns true value if the last parse was successful.
    //. Returns false if the last parse failed (bad address) or
    //. the {\tt Parse()} member function was never called.

    DwAddress* Next() const;
    //. Returns the next {\tt DwAddress} object in the list when the object
    //. is included in a list of addresses.  The function is used when
    //. iterating a list.

    void SetNext(DwAddress* aAddress);
    //. Sets the next {\tt DwAddress} object in the list.  This member function
    //. generally should not be used, since {\tt DwAddressList} has member
    //. functions to manage its list of {\tt DwAddress} objects.

protected:

    DwAddress();
    DwAddress(const DwAddress& aAddr);
    DwAddress(const DwString& aStr, DwMessageComponent* aParent=0);
    //. The first constructor is the default constructor, which sets the
    //. {\tt DwAddress} object's string representation to the empty string
    //. and sets its parent to {\tt NULL}.
    //.
    //. The second constructor is the copy constructor, which copies the
    //. string representation and all attributes from {\tt aAddress}.
    //. The parent of the new {\tt DwAddress} object is set to {\tt NULL}.
    //.
    //. The third constructor copies {\tt aStr} to the {\tt DwAddress}
    //. object's string representation and sets {\tt aParent} as its parent.
    //. The virtual member function {\tt Parse()} should be called immediately
    //. after this constructor in order to parse the string representation.
    //. Unless it is {\tt NULL}, {\tt aParent} should point to an object of
    //. a class derived from {\tt DwField}.

    const DwAddress& operator = (const DwAddress& aAddr);
    //. This is the assignment operator, which performs a deep copy of
    //. {\tt aAddr}.  The parent node of the {\tt DwAddress} object
    //. is not changed.

    int mIsValid;
    //. This data member is set to true if the parse method was successful.

private:

    DwAddress* mNext;
    static const char* const sClassName;

public:

    virtual void PrintDebugInfo(std::ostream& aStrm, int aDepth=0) const;
    //. This virtual function, inherited from {\tt DwMessageComponent},
    //. prints debugging information about this object to {\tt aStrm}.
    //. It will also call {\tt PrintDebugInfo()} for any of its child
    //. components down to a level of {\tt aDepth}.
    //.
    //. This member function is available only in the debug version of
    //. the library.

    virtual void CheckInvariants() const;
    //. Aborts if one of the invariants of the object fails.  Use this
    //. member function to track down bugs.
    //.
    //. This member function is available only in the debug version of
    //. the library.

protected:

    void _PrintDebugInfo(std::ostream& aStrm) const;

};

inline DwBool DwAddress::IsValid() const
{
    return mIsValid != 0;
}

#endif
