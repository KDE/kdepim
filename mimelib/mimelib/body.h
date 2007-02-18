//=============================================================================
// File:       body.h
// Contents:   Declarations for DwBody
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

#ifndef DW_BODY_H
#define DW_BODY_H

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
class DwBodyPart;

//=============================================================================
//+ Name DwBody -- Class representing a MIME message body
//+ Description
//. {\tt DwBody} represents a {\it body}, as described in RFC-2045.  A body
//. is always part of an {\it entity}, which could be either a {\it message}
//. or a {\it body part}.  An entity has a collection of {\it header fields}
//. and a body.  If the content type of a body is ``multipart,'' then the
//. body contains one or more body parts.  If the content type is ``message,''
//. then the body contains an encapsulated message.  In all content types,
//. the body contains a string of characters.
//.
//. In MIME++, a {\tt DwBody} object is contained in a {\tt DwEntity} object.
//. The {\tt DwBody} object may contain a discrete body consisting only of a
//. string of characters, or it may be a composite body, consisting of several
//. contained {\tt DwBodyPart} objects or a single contained {\tt DwMessage}
//. object.  The only reliable way to determine the type of {\tt DwBody} is
//. to access the Content-Type header field from the {\tt DwHeaders} object
//. of the {\tt DwEntity} that contains it.  For this reason, a {\tt DwBody}
//. should always be part of a {\tt DwEntity}.
//.
//. In the tree (broken-down) representation of a message, a {\tt DwBody}
//. object can be an intermediate node, having both a parent node and
//. one or more child nodes, or a leaf node, having a parent but no child
//. nodes.  In either case, the parent node is the {\tt DwEntity} object
//. that contains it.  If it is an intermediate node, it must be of type
//. multipart with {\tt DwBodyPart} objects as child nodes, or of type
//. message with a single {\tt DwMessage} object as its child node.
//.
//. Normally, you do not create a {\tt DwBody} object directly, but you
//. access it through the {\tt Body()} member function of {\tt DwEntity},
//. which creates the {\tt DwBody} object for you.
//.
//. To add a {\tt DwBodyPart} to a multipart {\tt DwBody}, use the member
//. function {\tt AddBodyPart()}.  To iterate over the {\tt DwBodyParts}
//. contained in multipart {\tt DwBody}, get the first {\tt DwBodyPart}
//. by calling {\tt FirstBodyPart()}.  Then get the following {\tt DwBodyParts}
//. by calling {\tt DwBodyPart::Next()} on the current {\tt DwBodyPart}.
//. To get the {\tt DwMessage} contained in a {\tt Body} with message
//. content type, call {\tt Message()}.
//=============================================================================
// Last modified 1997-08-23
//+ Noentry ~DwBody sClassName DeleteBodyParts CopyBodyParts _PrintDebugInfo


class DW_EXPORT DwBody : public DwMessageComponent {

    friend class DwHeaders;
    friend class DwEntity;
    friend class DwBodyPart;

public:

    DwBody();
    DwBody(const DwBody& aBody);
    DwBody(const DwString& aStr, DwMessageComponent* aParent=0);
    //. The first constructor is the default constructor, which sets the
    //. {\tt DwBody} object's string representation to the empty string
    //. and sets its parent to {\tt NULL}.
    //.
    //. The second constructor is the copy constructor, which performs
    //. a deep copy of {\tt aBody}.
    //. The parent of the new {\tt DwBody} object is set to {\tt NULL}.
    //.
    //. The third constructor copies {\tt aStr} to the {\tt DwBody}
    //. object's string representation and sets {\tt aParent} as its parent.
    //. The virtual member function {\tt Parse()} should be called immediately
    //. after this constructor in order to parse the string representation.
    //. Unless it is {\tt NULL}, {\tt aParent} should point to an object of
    //. a class derived from {\tt DwEntity}.

    virtual ~DwBody();

    const DwBody& operator = (const DwBody& aBody);
    //. This is the assignment operator, which performs a deep copy of
    //. {\tt aBody}.  The parent node of the {\tt DwBody} object
    //. is not changed.

    virtual void Parse();
    //. This virtual function, inherited from {\tt DwMessageComponent},
    //. executes the parse method for {\tt DwBody} objects.  The parse
    //. method creates or updates the broken-down representation from the
    //. string representation.  For a multipart {\tt DwBody} object, the
    //. parse method creates a collection of {\tt DwBodyPart} objects.
    //. For a message {\tt DwBody}, the parse method creates a single
    //. {\tt DwMessage} object.  For any other type of {\tt DwBody},
    //. the parse method does nothing.  This member function calls the
    //. {\tt Parse()} member function of any objects it creates.
    //.
    //. Note: If the {\tt DwBody} object has no parent node -- that is,
    //. it is not contained by a {\tt DwEntity} object -- then the parse
    //. method does nothing, since it is unable to determine the type of
    //. body.
    //.
    //. You should call this member function after you set or modify the
    //. string representation, and before you access a contained
    //. {\tt DwBodyPart} or {\tt DwMessage}.
    //.
    //. This function clears the is-modified flag.

    virtual void Assemble();
    //. This virtual function, inherited from {\tt DwMessageComponent},
    //. executes the assemble method for {\tt DwBody} objects.  The
    //. assemble method creates or updates the string representation
    //. from the broken-down representation.  Only {\tt DwBody} objects
    //. with content type of multipart or message require assembling.
    //. In either case, the {\tt DwBody} object must be able to find the
    //. headers of the message or body part that contains it.  Therefore,
    //. if the {\tt DwBody} object is not the child of a {\tt DwEntity}
    //. ({\it i.e.}, {\tt DwMessage} or {\tt DwBodyPart}) object, the
    //. {\tt DwBody} cannot be assembled because the content type cannot
    //. be determined.
    //.
    //. This function calls the {\tt Parse()} member function of any
    //. {\tt DwBodyPart} or {\tt DwMessage} object it contains.
    //.
    //. You should call this member function after you add a {\tt DwBodyPart}
    //. object to a multipart body, or add a {\tt DwMessage} object to a
    //. message body, and before you access the object's string representation.
    //.
    //. This function clears the is-modified flag.

    virtual DwMessageComponent* Clone() const;
    //. This virtual function, inherited from {\tt DwMessageComponent},
    //. creates a new {\tt DwBody} on the free store that has the same
    //. value as this {\tt DwBody} object.  The basic idea is that of
    //. a virtual copy constructor.

    DwBodyPart* FirstBodyPart() const;
    //. For a multipart {\tt DwBody}, this member function returns the
    //. first contained {\tt DwBodyPart} object.
    //. Use {\tt DwBodyPart::Next()} to iterate through the list of
    //. {\tt DwBodyPart}s.

    void AddBodyPart(DwBodyPart* aPart);
    //. For a multipart {\tt DwBody}, this member function appends a
    //. {\tt DwBodyPart} object to the list.  Any {\tt DwBodyPart} objects
    //. added to a {\tt DwBody} object's list will be deleted by the
    //. {\tt DwBody} object's destructor.

    void RemoveBodyPart(DwBodyPart* aPart);
    //. For a multipart {\tt DwBody}, this member function removes a
    //. {\tt DwBodyPart} object from the list. The caller is responsible
    //. for deleting the bodypart afterwards!

    DwMessage* Message() const;
    //. For a {\tt DwBody} with content type of message, this member function
    //. returns the {\tt DwMessage} encapsulated in it.

    void SetMessage(DwMessage* aMessage);
    //. For a {\tt DwBody} with content type of message, this member function
    //. sets the {\tt DwMessage} object it contains.

    static DwBody* NewBody(const DwString& aStr, DwMessageComponent* aParent);
    //. Creates a new {\tt DwBody} object on the free store.
    //. If the static data member {\tt sNewBody} is {\tt NULL},
    //. this member function will create a new {\tt DwBody}
    //. and return it.  Otherwise, {\tt NewBody()} will call
    //. the user-supplied function pointed to by {\tt sNewBody},
    //. which is assumed to return an object from a class derived from
    //. {\tt DwBody}, and return that object.

    //+ Var sNewBody
    static DwBody* (*sNewBody)(const DwString&, DwMessageComponent*);
    //. If {\tt sNewBody} is not {\tt NULL}, it is assumed to point to a
    //. user-supplied function that returns an object from a class
    //. derived from {\tt DwBody}.

protected:

    DwString    mBoundaryStr;
    //. A cache for the boundary string, which is obtained from the
    //. headers associated with this body.

    DwString    mPreamble;
    //. Contains the preamble -- the text preceding the first boundary --
    //. in a ``multipart/*'' media type.

    DwString    mEpilogue;
    //. Contains the epilogue -- the text following the last boundary --
    //. in a ``multipart/*'' media type.

    DwBodyPart* mFirstBodyPart;
    //. Points to the first body part in a ``multipart/*'' media type.
    //. Is {\tt NULL} if there are no body parts.

    DwMessage*  mMessage;
    //. Points to the contained message, in a ``message/*'' media type.

    static const char* const sClassName;

    void _AddBodyPart(DwBodyPart*);
    //. Adds a body part to a multipart body.  This function differs
    //. from {\tt AddBodyPart} in that it does not set the is-modified
    //. flag.

    void _RemoveBodyPart(DwBodyPart*);
    //. Removes a body part from a multipart body.  This function differs
    //. from {\tt RemoveBodyPart} in that it does not set the is-modified
    //. flag.

    void _SetMessage(DwMessage*);
    //. Sets a message to a body.  This function differs from
    //. {\tt SetMessage()} in that it does not set the is-modified
    //. flag.

    void CopyBodyParts(const DwBodyPart* aFirst);

public:
    void DeleteBodyParts();

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

