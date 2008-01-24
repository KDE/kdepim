//=============================================================================
// File:       param.h
// Contents:   Declarations for DwParameter
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

#ifndef DW_PARAM_H
#define DW_PARAM_H

#ifndef DW_CONFIG_H
#include <mimelib/config.h>
#endif

#ifndef DW_STRING_H
#include <mimelib/string.h>
#endif

#ifndef DW_MSGCMP_H
#include <mimelib/msgcmp.h>
#endif


//=============================================================================
//+ Name DwParameter -- Class representing a MIME field body parameter
//+ Description
//. {\tt DwParameter} represents the {\it parameter} component of the
//. Content-Type header field as described in RFC-2045.  A parameter
//. consists of an attribute/value pair.  {\tt DwParameter} has member
//. functions for getting or setting a parameter's attribute and value.
//.
//. A {\tt DwParameter} object may be included in a list of {\tt DwParameter}
//. objects.  You can get the next {\tt DwParameter} object in the list by
//. calling the member function {\tt Next()}.
//=============================================================================
// Last modified 1997-08-13
//+ Noentry ~DwParameter _PrintDebugInfo

class DW_EXPORT DwParameter : public DwMessageComponent {

    friend class DwMediaType;

public:

    DwParameter();
    DwParameter(const DwParameter& aParam);
    DwParameter(const DwString& aStr, DwMessageComponent* aParent=0);
    //. The first constructor is the default constructor, which sets the
    //. {\tt DwParameter} object's string representation to the empty string
    //. and sets its parent to NULL.
    //.
    //. The second constructor is the copy constructor, which copies the
    //. string representation, attribute, and value from {\tt aParam}.
    //. The parent of the new {\tt DwParameter} object is set to NULL.
    //.
    //. The third constructor copies {\tt aStr} to the {\tt DwParameter}
    //. object's string representation and sets {\tt aParent} as its parent.
    //. The virtual member function {\tt Parse()} should be called immediately
    //. after this constructor in order to parse the string representation.
    //. Unless it is NULL, {\tt aParent} should point to an object of a class
    //. derived from {\tt DwMediaType}.

    virtual ~DwParameter();

    const DwParameter& operator = (const DwParameter& aParam);
    //. This is the assignment operator.

    virtual void Parse();
    //. This virtual function, inherited from {\tt DwMessageComponent},
    //. executes the parse method for {\tt DwParameter} objects.
    //. It should be called immediately after the string representation
    //. is modified and before the parts of the broken-down
    //. representation are accessed.

    virtual void Assemble();
    //. This virtual function, inherited from {\tt DwMessageComponent},
    //. executes the assemble method for {\tt DwParameter} objects.
    //. It should be called whenever one of the object's attributes
    //. is changed in order to assemble the string representation from
    //. its broken-down representation.  It will be called
    //. automatically for this object by the parent object's
    //. {\tt Assemble()} member function if the is-modified flag is set.

    virtual DwMessageComponent* Clone() const;
    //. This virtual function, inherited from {\tt DwMessageComponent},
    //. creates a new {\tt DwParameter} on the free store that has the same
    //. value as this {\tt DwParameter} object.  The basic idea is that of
    //. a ``virtual copy constructor.''

    const DwString& Attribute() const;
    //. Returns the attribute contained by this parameter.

    void SetAttribute(const DwString& aAttribute);
    //. Sets the attribute contained by this parameter.

    const DwString& Value() const;
    //. Returns the value contained by this parameter.

    void SetValue(const DwString& aValue, bool forceNoQuotes=false);
    //. Sets the value contained by this parameter.

    DwParameter* Next() const ;
    //. Returns the next {\tt DwParameter} object in the list.

    void SetNext(DwParameter* aParam);
    //. Returns the next {\tt DwParameter} object in the list.  Since
    //. {\tt DwMediaType} has member functions for adding {\tt DwParameter}
    //. objects to its list, you should avoid using this function.

    static DwParameter* NewParameter(const DwString& aStr,
        DwMessageComponent* aParent);
    //. Creates a new {\tt DwParameter} object on the free store.
    //. If the static data member {\tt sNewParameter} is NULL,
    //. this member function will create a new {\tt DwParameter}
    //. and return it.  Otherwise, {\tt NewParameter()} will call
    //. the user-supplied function pointed to by {\tt sNewParameter},
    //. which is assumed to return an object from a class derived from
    //. {\tt DwParameter}, and return that object.

    //+ Var sNewParameter
    static DwParameter* (*sNewParameter)(const DwString&, DwMessageComponent*);
    //. If {\tt sNewParameter} is not NULL, it is assumed to point to a
    //. user-supplied function that returns an object from a class derived from
    //. {\tt DwParameter}.

private:

    DwString mAttribute;
    DwString mValue;
    bool     mForceNoQuotes;
    DwParameter* mNext;
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

#endif
