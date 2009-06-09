//=============================================================================
// File:       field.h
// Contents:   Declarations for DwField
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

#ifndef DW_FIELD_H
#define DW_FIELD_H

#include <iostream>

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
class DwFieldBody;

//=============================================================================
//+ Name DwField -- Class representing a MIME header field
//+ Description
//. {\tt DwField} represents a header field as described in RFC-822.
//. According to RFC-822, a field contains a field name and a field body.
//. In MIME++, a {\tt DwField} contains three elements: a {\tt DwString}
//. that contains its field name, a {\tt DwString} that contains its
//. field body, and a {\tt DwFieldBody} object that contains a broken-down
//. (that is, parsed) version of its field body.
//.
//. In the tree (broken-down) representation of message, a {\tt DwField}
//. object is always an intermediate node, having a parent node and a single
//. child node.  The parent node is the {\tt DwHeaders} object that contains
//. it.  The child node is the {\tt DwFieldBody} object it contains.
//.
//. To get and set the field name, use the member functions
//. {\tt FieldNameStr()} and {\tt SetFieldNameStr()}.
//. To get and set the field body, use the member functions
//. {\tt FieldBodyStr()} and {\tt SetFieldBodyStr()}.
//. To get and set the {\tt DwFieldBody} object, use {\tt FieldBody()}
//. and {\tt SetFieldBody()}.
//.
//. A {\tt DwField} object can be included in a list of {\tt DwField}
//. objects; usually this is the list of {\tt DwField} objects maintained
//. by its parent {\tt DwHeaders} object.  To get the next {\tt DwField}
//. object in a list, use the member function {\tt Next()}.
//=============================================================================
// Last updated 1997-08-23
//+ Noentry ~DwField _CreateFieldBody mFieldNameStr mFieldBodyStr
//+ Noentry mFieldBody _PrintDebugInfo

class DW_EXPORT DwField : public DwMessageComponent {

    friend class DwHeaders;

public:

    DwField();
    DwField(const DwField& aField);
    DwField(const DwString& aStr, DwMessageComponent* aParent=0);
    //. The first constructor is the default constructor, which sets the
    //. {\tt DwField} object's field name and field body to the empty
    //. string, set its parent to {\tt NULL}, and sets its {\tt DwFieldBody}
    //. object to {\tt NULL}.
    //.
    //. The second constructor is the copy constructor, which performs
    //. a deep copy of {\tt aField}.
    //. The parent of the new {\tt DwField} object is set to {\tt NULL}.
    //.
    //. The third constructor copies {\tt aStr} to the {\tt DwField}
    //. object's string representation and sets {\tt aParent} as its parent.
    //. The virtual member function {\tt Parse()} should be called immediately
    //. after this constructor in order to parse the string representation.
    //. Unless it is {\tt NULL}, {\tt aParent} should point to an object of
    //. a class derived from {\tt DwHeaders}.

    virtual ~DwField();

    const DwField& operator = (const DwField& aField);
    //. This is the assignment operator, which performs a deep copy of
    //. {\tt aField}.  The parent node of the {\tt DwField} object
    //. is not changed.

    virtual void Parse();
    //. This virtual function, inherited from {\tt DwMessageComponent},
    //. executes the parse method for {\tt DwField} objects.  The parse
    //. method creates or updates the broken-down representation from the
    //. string representation.  For {\tt DwField} objects, the parse method
    //. parses the string representation, sets the values of the field
    //. name string and the field body string, and creates an instance
    //. of the appropriate subclass of {\tt DwFieldBody}.  This member
    //. function also calls the {\tt Parse()} member function of its
    //. contained {\tt DwFieldBody} object.
    //.
    //. You should call this member function after you set or modify the
    //. string representation, and before you access the field name, the
    //. field body, or the contained {\tt DwFieldBody} object.
    //.
    //. This function clears the is-modified flag.

    virtual void Assemble();
    //. This virtual function, inherited from {\tt DwMessageComponent},
    //. executes the assemble method for {\tt DwField} objects. The
    //. assemble method creates or updates the string representation from
    //. the broken-down representation.  In more concrete terms, the
    //. assemble method builds the string representation from the field
    //. name and the string representation of the contained {\tt DwFieldBody}
    //. object.  This member function calls the {\tt Assemble()} member
    //. function of its contained {\tt DwFieldBody} object.
    //.
    //. You should call this member function after you modify either the
    //. field name or the contained {\tt DwFieldBody} object, and before
    //. you retrieve the string representation.
    //.
    //. This function clears the is-modified flag.

    virtual DwMessageComponent* Clone() const;
    //. This virtual function, inherited from {\tt DwMessageComponent},
    //. creates a new {\tt DwField} on the free store that has the same
    //. value as this {\tt DwField} object.  The basic idea is that of
    //. a virtual copy constructor.

    DwFieldBody* FieldBody() const;
    //. Returns the {\tt DwFieldBody} object contained by this {\tt DwField}
    //. object.  If there is no field body, {\tt NULL} will be returned.

    const DwString& FieldNameStr() const;
    //. Returns the field name of this header field as a string.

    const DwString& FieldBodyStr() const;
    //. Returns the field body of this header field as a string.

    DwField* Next() const;
    //. Returns the next {\tt DwField} object following this
    //. {\tt DwField} object in the list contained in a {\tt DwHeaders}.
    //. Returns {\tt NULL} if this object is last in the list.

    void SetFieldBody(DwFieldBody* aFieldBody);
    //. Sets the {\tt DwFieldBody} object contained by this object.

    void SetFieldNameStr(const DwString& aStr);
    //. Sets the field name of this header field.

    void SetFieldBodyStr(const DwString& aStr);
    //. Sets the field body of this header field.

    void SetNext(const DwField* aField);
    //. This {\it advanced} function sets {\tt aField} as the next field
    //. following this field in the list of fields contained in the headers.
    //. Since {\tt DwHeaders} contains member functions for adding
    //. {\tt DwField} objects to its list, this function should be
    //. avoided for most applications.

    static DwField* NewField(const DwString& aStr,
        DwMessageComponent* aParent);
    //. Creates a new {\tt DwField} object on the free store.
    //. If the static data member {\tt sNewField} is {\tt NULL},
    //. this member function will create a new {\tt DwField}
    //. and return it.  Otherwise, {\tt NewField()} will call
    //. the user-supplied function pointed to by {\tt sNewField},
    //. which is assumed to return an object from a class derived from
    //. {\tt DwField}, and return that object.

    static DwFieldBody* CreateFieldBody(const DwString& aFieldName,
        const DwString& aFieldBody, DwMessageComponent* aParent);
    //. The static member function {\tt CreateFieldBody()} is called from
    //. the {\tt Parse()} member function and is responsible for creating a
    //. {\tt DwFieldBody} object for this particular field.  A typical
    //. scenario might go as follows:
    //. This member function examines the field name for this field,
    //. finds that it contains "To", creates a {\tt DwAddressList} object
    //. to contain the field body, calls the {\tt Parse()} member
    //. function for the {\tt DwAddressList}, and sets the {\tt DwAddressList}
    //. object as this {\tt DwField} object's {\tt DwFieldBody}.
    //.
    //. If you want to override the behavior of {\tt CreateFieldBody()},
    //. you can do so by setting the public data member
    //. {\tt sCreateFieldBody} to point to your own function.
    //. {\tt CreateFieldBody()} first checks to see if
    //. {\tt sCreateFieldBody} is {\tt NULL}.  If it is not,
    //. {\tt CreateFieldBody()} will assume that it points to a user-supplied
    //. function and will call that function.  If it is {\tt NULL},
    //. {\tt CreateFieldBody()} will call {\tt _CreateFieldBody()}, which
    //. actually creates the {\tt DwFieldBody} object.  You may call
    //. {\tt _CreateFieldBody()} from your own function for fields you
    //. do not wish to handle.

    static DwFieldBody* _CreateFieldBody(const DwString& aFieldName,
        const DwString& aFieldBody, DwMessageComponent* aParent);

    //+ Var sNewField
    static DwField* (*sNewField)(const DwString&, DwMessageComponent*);
    //. If {\tt sNewField} is not {\tt NULL}, it is assumed to point
    //. to a user-supplied function that returns an object from a class
    //. derived from {\tt DwField}.

    //+ Var sCreateFieldBody
    static DwFieldBody* (*sCreateFieldBody)(const DwString& aFieldName,
        const DwString& aFieldBody, DwMessageComponent* aParent);
    //. See {\tt CreateFieldBody()}.

protected:

    DwString mFieldNameStr;
    // the {\it field-name}

    DwString mFieldBodyStr;
    // the {\it field-body}

    DwFieldBody* mFieldBody;
    // pointer to the {\tt DwFieldBody} object

    void _SetFieldBody(DwFieldBody* aFieldBody);
    //. Sets the {\tt DwFieldBody} object contained by this object.  This
    //. function differs from {\tt SetFieldBody()} in that it does not
    //. set the is-modified flag.

private:

    const DwField* mNext;
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
