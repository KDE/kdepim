//=============================================================================
// File:       entity.h
// Contents:   Declarations for DwEntity
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

#ifndef DW_ENTITY_H
#define DW_ENTITY_H

#ifndef DW_CONFIG_H
#include <mimelib/config.h>
#endif

#ifndef DW_STRING_H
#include <mimelib/string.h>
#endif

#ifndef DW_MSGCMP_H
#include <mimelib/msgcmp.h>
#endif

class DwHeaders;
class DwBody;

//=============================================================================
//+ Name DwEntity -- Abstract class representing a MIME entity
//+ Description
//. RFC-2045 defines an {\it entity} as either a {\it message} or a
//. {\it body part}, both of which have a collection of headers and
//. a {\it body}.  In MIME++, an entity is represented by the class
//. {\tt DwEntity}, which contains both a {\tt DwHeaders} object and
//. a {\tt DwBody} object.
//.
//. In the tree (broken-down) representation of message, a {\tt DwEntity}
//. object may be either a root node, having child nodes but no parent
//. node, or an intermediate node, having both a parent node and child nodes.
//. A {\tt DwEntity} object that is a root node must also be a {\tt DwMessage}
//. object.  If a {\tt DwEntity} object is an intermediate node, its parent
//. must be a {\tt DwBody} object.  The child nodes of a {\tt DwEntity}
//. object are the {\tt DwHeaders} and {\tt DwBody} objects it contains.
//.
//. Since {\tt DwEntity} is an abstract base class, you cannot create
//. instances of it directly.  {\tt DwEntity} has two derived classes,
//. {\tt DwMessage} and {\tt DwBodyPart}, which are concrete classes.
//.
//. To access the contained {\tt DwHeaders} object, use the member function
//. {\tt Headers()}.  To access the contained {\tt DwBody} object, use the
//. member function {\tt Body()}.
//=============================================================================
// Last updated 1997-08-23
//+ Noentry ~DwEntity mHeaders mBody _PrintDebugInfo

class DW_EXPORT DwEntity : public DwMessageComponent {

public:

    DwEntity();
    DwEntity(const DwEntity& aEntity);
    DwEntity(const DwString& aStr, DwMessageComponent* aParent=0);
    //. The first constructor is the default constructor, which sets the
    //. {\tt DwEntity} object's string representation to the empty string
    //. and sets its parent to {\tt NULL}.
    //.
    //. The second constructor is the copy constructor, which performs
    //. a deep copy of {\tt aEntity}.
    //. The parent of the new {\tt DwEntity} object is set to {\tt NULL}.
    //.
    //. The third constructor copies {\tt aStr} to the {\tt DwEntity}
    //. object's string representation and sets {\tt aParent} as its parent.
    //. The virtual member function {\tt Parse()} should be called immediately
    //. after this constructor in order to parse the string representation.
    //. Unless it is {\tt NULL}, {\tt aParent} should point to an object of
    //. a class derived from {\tt DwBody}.

    virtual ~DwEntity();

    const DwEntity& operator = (const DwEntity& aEntity);
    //. This is the assignment operator, which performs a deep copy of
    //. {\tt aEntity}.  The parent node of the {\tt DwEntity} object
    //. is not changed.

    virtual void Parse();
    //. This virtual function, inherited from {\tt DwMessageComponent},
    //. executes the parse method for {\tt DwEntity} objects.  The parse
    //. method creates or updates the broken-down representation from the
    //. string representation.  For {\tt DwEntity} objects, the parse
    //. method parses the string representation and sets the values of
    //. the {\tt DwHeaders} and {\tt DwBody} objects it contains. This
    //. member function also calls the {\tt Parse()} member functions
    //. of the contained {\tt DwHeaders} and {\tt DwBody} objects.
    //.
    //. You should call this member function after you set or modify the
    //. string representation, and before you access either the contained
    //. headers or body.
    //.
    //. This function clears the is-modified flag.

    virtual void Assemble(DwHeaders& aHeaders, DwBody& aBody);
    // Assemble *without* first assembling the Header and the Body.
    
    virtual void Assemble();
    //. This virtual function, inherited from {\tt DwMessageComponent},
    //. executes the assemble method for {\tt DwEntity} objects.  The
    //. assemble method creates or updates the string representation from
    //. the broken-down representation.  In more concrete terms, the
    //. assemble method builds the string representation from the string
    //. representations of the contained {\tt DwHeaders} and {\tt DwBody}
    //. objects.  This member function calls the {\tt Assemble()} member
    //. functions of its {\tt DwHeaders} and {\tt DwBody} objects.
    //.
    //. You should call this member function after you modify either the
    //. contained headers or body, and before you retrieve the string
    //. representation.
    //.
    //. This function clears the is-modified flag.

    bool hasHeaders() const { return 0 != mHeaders; }

    DwHeaders& Headers() const;
    //. This function returns the {\tt DwHeaders} object contained by
    //. this object.

    DwBody& Body() const;
    //. This function returns the {\tt DwBody} object contained by this object.

    int BodySize() const;
    //. Get the size of the Body

    void SetBodySize( int size ) { mBodySize = size; }
    //. Explicitly set the size of the Body
    //. This is needed if the body is empty but you know the size and others 
    //. should be able to access it

protected:

    DwHeaders* mHeaders;
    DwBody*    mBody;

private:

    static const char* const sClassName;
    int mBodySize; // normally mBody->AsString().length()

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
