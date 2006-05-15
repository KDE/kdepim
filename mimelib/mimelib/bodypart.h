//=============================================================================
// File:       bodypart.h
// Contents:   Declarations for DwBodyPart
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

#ifndef DW_BODYPART_H
#define DW_BODYPART_H

#ifndef DW_CONFIG_H
#include <mimelib/config.h>
#endif

#ifndef DW_STRING_H
#include <mimelib/string.h>
#endif

#ifndef DW_ENTITY_H
#include <mimelib/entity.h>
#endif

class DwMessage;
class DwEntity;
class DwBody;


//=============================================================================
//+ Name DwBodyPart -- Class representing a MIME body-part
//+ Description
//. {\tt DwBodyPart} represents a {\it body part}, as described in RFC-2045
//. and RFC-2046.  A body part is an {\it entity}, so it has a collection
//. of headers and a {\it body}.  A body part is different from a {\it message}
//. in that a body part is part of a multipart body.
//.
//. In MIME++, a {\tt DwBodyPart} is a subclass of {\tt DwEntity}; therefore,
//. it contains both a {\tt DwHeaders} object and a {\tt DwBody} object,
//. and it is contained in a multipart {\tt DwBody} object.
//.
//. As with {\tt DwMessage}, most of the functionality of {\tt DwBodyPart} is
//. implemented by the abstract class {\tt DwEntity}.
//=============================================================================
// Last modified 1997-08-23
//+ Noentry ~DwBodyPart _PrintDebugInfo mNext sClassName


class DW_EXPORT DwBodyPart : public DwEntity {

public:

    DwBodyPart();
    DwBodyPart(const DwBodyPart& aPart);
    DwBodyPart(const DwEntity& aPart);
    DwBodyPart(const DwString& aStr, DwMessageComponent* aParent=0);
    //. The first constructor is the default constructor, which sets the
    //. {\tt DwBodyPart} object's string representation to the empty string
    //. and sets its parent to {\tt NULL}.
    //.
    //. The second constructor is the copy constructor, which performs
    //. a deep copy of {\tt aPart}.
    //. The parent of the new {\tt DwBodyPart} object is set to {\tt NULL}.
    //.
    //. The third constructor copies {\tt aStr} to the {\tt DwBodyPart}
    //. object's string representation and sets {\tt aParent} as its parent.
    //. The virtual member function {\tt Parse()} should be called immediately
    //. after this constructor in order to parse the string representation.
    //. Unless it is {\tt NULL}, {\tt aParent} should point to an object of
    //. a class derived from {\tt DwBody}.

    virtual ~DwBodyPart();

    const DwBodyPart& operator = (const DwBodyPart& aPart);
    //. This is the assignment operator, which performs a deep copy of
    //. {\tt aPart}.  The parent node of the {\tt DwBodyPart} object
    //. is not changed.

    virtual DwMessageComponent* Clone() const;
    //. This virtual function, inherited from {\tt DwMessageComponent},
    //. creates a new {\tt DwBodyPart} on the free store that has the same
    //. value as this {\tt DwBodyPart} object.  The basic idea is that of
    //. a virtual copy constructor.

    static DwBodyPart* NewBodyPart(const DwString& aStr,
        DwMessageComponent* aParent);
    //. Creates a new {\tt DwBodyPart} on the free store.
    //. If the static data member {\tt sNewBodyPart} is {\tt NULL},
    //. this member function will create a new {\tt DwBodyPart}
    //. and return it.  Otherwise, {\tt NewBodyPart()} will call
    //. the user-supplied function pointed to by {\tt sNewBodyPart},
    //. which is assumed to return an object from a class derived from
    //. {\tt DwBodyPart}, and return that object.

    DwBodyPart* Next() const;
    //. This member function returns the next {\tt DwBodyPart} object
    //. following this {\tt DwBodyPart} in the list of {\tt DwBodyPart}
    //. objects contained in a multipart {\tt DwBody}.

    void SetNext(const DwBodyPart* aPart);
    //. This advanced function sets {\tt aPart} as the next {\tt DwBodyPart}
    //. object following this {\tt DwBodyPart} in the list of {\tt DwBodyPart}
    //. objects contained in a multipart {\tt DwBody}.  Since {\tt DwBody}
    //. contains a member function for adding a {\tt DwBodyPart} object to
    //. its list, this function should be avoided for most applications.

    //+ Var sNewBodyPart
    static DwBodyPart* (*sNewBodyPart)(const DwString&, DwMessageComponent*);
    //. If {\tt sNewBodyPart} is not {\tt NULL}, it is assumed to point to a
    //. user-supplied function that returns an object from a class
    //. derived from {\tt DwBodyPart}.

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

private:

    const DwBodyPart* mNext;
    static const char* const sClassName;

};

#endif

