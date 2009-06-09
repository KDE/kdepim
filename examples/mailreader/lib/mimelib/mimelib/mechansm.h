//=============================================================================
// File:       mechansm.h
// Contents:   Declarations for DwMechanism
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

#ifndef DW_MECHANSM_H
#define DW_MECHANSM_H

#ifndef DW_CONFIG_H
#include <mimelib/config.h>
#endif

#ifndef DW_STRING_H
#include <mimelib/string.h>
#endif

#ifndef DW_FIELDBDY_H
#include <mimelib/fieldbdy.h>
#endif


//=============================================================================
//+ Name DwMechanism -- Class representing a MIME content-transfer-encoding field-body
//+ Description
//. {\tt DwMechanism} represents a field body for the
//. Content-Transfer-Encoding header field as described in RFC-2045.
//. {\tt DwMechanism} provides convenience functions that allow you to
//. set or get the content-transfer-encoding attribute as an enumerated
//. value.
//=============================================================================
// Last updated 1997-08-30
//+ Noentry ~DwMechanism _PrintDebugInfo


class DW_EXPORT DwMechanism : public DwFieldBody {

public:

    DwMechanism();
    DwMechanism(const DwMechanism& aCte);
    DwMechanism(const DwString& aStr, DwMessageComponent* aParent=0);
    //. The first constructor is the default constructor, which sets the
    //. {\tt DwMechanism} object's string representation to the empty
    //. string and sets its parent to {\tt NULL}.
    //.
    //. The second constructor is the copy constructor, which copies the
    //. string representation from {\tt aCte}.
    //. The parent of the new {\tt DwMechanism} object is set to {\tt NULL}.
    //.
    //. The third constructor copies {\tt aStr} to the {\tt DwMechanism}
    //. object's string representation and sets {\tt aParent} as its parent.
    //. The virtual member function {\tt Parse()} should be called immediately
    //. after this constructor in order to parse the string representation.
    //. Unless it is {\tt NULL}, {\tt aParent} should point to an object of
    //. a class derived from {\tt DwField}.

    virtual ~DwMechanism();

    const DwMechanism& operator = (const DwMechanism& aCte);
    //. This is the assignment operator, which performs a deep copy of
    //. {\tt aCte}.  The parent node of the {\tt DwMechanism} object
    //. is not changed.

    virtual void Parse();
    //. This virtual function, inherited from {\tt DwMessageComponent},
    //. executes the parse method for {\tt DwMechanism} objects.
    //. It should be called immediately after the string representation
    //. is modified and before any of the object's attributes are retrieved.
    //.
    //. This function clears the is-modified flag.

    virtual void Assemble();
    //. This virtual function, inherited from {\tt DwMessageComponent},
    //. executes the assemble method for {\tt DwMechanism} objects.
    //. It should be called whenever one of the object's attributes
    //. is changed in order to assemble the string representation.
    //. It will be called automatically for this object by the parent
    //. object's {\tt Assemble()} member function if the is-modified
    //. flag is set.
    //.
    //. This function clears the is-modified flag.

    virtual DwMessageComponent* Clone() const;
    //. This virtual function, inherited from {\tt DwMessageComponent},
    //. creates a new {\tt DwMechanism} object on the free store that has
    //. the same value as this {\tt DwMechanism} object.  The basic idea
    //. is that of a virtual copy constructor.

    int AsEnum() const;
    //. Returns the content transfer encoding as an enumerated value.
    //. Enumerated values are defined for all standard content transfer
    //. encodings in the file enum.h.  If the content transfer encoding
    //. is non-standard {\tt DwMime::kCteUnknown} is returned.  The
    //. inherited member function {\tt DwMessageComponent::AsString()}
    //. may be used to get the content transfer encoding, standard or
    //. non-standard, as a string.

    void FromEnum(int aCte);
    //. Sets the content transfer encoding from an enumerated value.
    //. Enumerated values are defined for all standard content transfer
    //. encodings in the file enum.h.  You may set the content transfer
    //. encoding to any string value, standard or non-standard, by using the
    //. inherited member function {\tt DwMessageComponent::FromString()}.

    static DwMechanism*
        NewMechanism(const DwString& aStr, DwMessageComponent* aParent);
    //. Creates a new {\tt DwMechanism} object on the free store.
    //. If the static data member {\tt sNewMechanism} is {\tt NULL},
    //. this member function will create a new {\tt DwMechanism}
    //. and return it.  Otherwise, {\tt NewMechanism()} will call
    //. the user-supplied function pointed to by
    //. {\tt sNewMechanism}, which is assumed to return an
    //. object from a class derived from {\tt DwMechanism}, and
    //. return that object.

    //+ Var sNewMechanism
    static DwMechanism*
        (*sNewMechanism)(const DwString&, DwMessageComponent*);
    //. If {\tt sNewMechanism} is not {\tt NULL}, it is assumed
    //. to point to a user-supplied function that returns an object from
    //. a class derived from {\tt DwMechanism}.

private:

    int mCteEnum;
    static const char* const sClassName;

    void EnumToString();
    void StringToEnum();

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

#endif
