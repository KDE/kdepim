//=============================================================================
// File:       message.h
// Contents:   Declarations for DwMessage
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

#ifndef DW_MESSAGE_H
#define DW_MESSAGE_H

#ifndef DW_CONFIG_H
#include <mimelib/config.h>
#endif

#ifndef DW_ENTITY_H
#include <mimelib/entity.h>
#endif

//=============================================================================
//+ Name DwMessage -- Class representing an RFC-822/MIME message
//+ Description
//. {\tt DwMessage} represents an RFC-822/MIME {\it message}.
//.
//. A {\it message} contains both a collection of {\it header fields} and
//. a {\it body}. In the terminology of RFC-2045, the general term for the
//. headers-body combination is {\it entity}. In MIME++, {\tt DwMessage}
//. is a direct subclass of {\tt DwEntity}, and therefore contains both
//. a {\tt DwHeaders} object and a {\tt DwBody} object.
//.
//. In the tree (broken-down) representation of message, a {\tt DwMessage}
//. object is almost always a root node, having child nodes but no parent node.
//. The child nodes are the {\tt DwHeaders} object and the {\tt DwBody} object
//. it contains.  A {\tt DwMessage} may sometimes be an intermediate node.  In
//. this special case, the parent node is a {\tt DwBody} object of type
//. "message/*" and the {\tt DwMessage} object represents an encapsulated
//. message.
//.
//. To access the contained {\tt DwHeaders} object, use the inherited member
//. function {\tt DwEntity::Headers()}.  To access the contained {\tt DwBody}
//. object, use the inherited member function {\tt DwEntity::Body()}.
//=============================================================================
// Last modified 1997-08-30
//+ Noentry ~DwMessage _PrintDebugInfo

class DW_EXPORT DwMessage : public DwEntity {

public:

    DwMessage();
    DwMessage(const DwMessage& aMessage);
    DwMessage(const DwString& aStr, DwMessageComponent* aParent=0);
    //. The first constructor is the default constructor, which sets the
    //. {\tt DwMessage} object's string representation to the empty string
    //. and sets its parent to {\tt NULL}.
    //.
    //. The second constructor is the copy constructor, which performs
    //. a deep copy of {\tt aMessage}.
    //. The parent of the new {\tt DwMessage} object is set to {\tt NULL}.
    //.
    //. The third constructor copies {\tt aStr} to the {\tt DwMessage}
    //. object's string representation and sets {\tt aParent} as its parent.
    //. The virtual member function {\tt Parse()} should be called immediately
    //. after this constructor in order to parse the string representation.

    virtual ~DwMessage();

    const DwMessage& operator = (const DwMessage& aMessage);
    //. This is the assignment operator, which performs a deep copy of
    //. {\tt aMessage}.  The parent node of the {\tt DwMessage} object
    //. is not changed.

    virtual DwMessageComponent* Clone() const;
    //. This virtual function, inherited from {\tt DwMessageComponent},
    //. creates a new {\tt DwMessage} on the free store that has the same
    //. value as this {\tt DwMessage} object.  The basic idea is that of
    //. a ``virtual copy constructor.''

    static DwMessage* NewMessage(const DwString& aStr,
        DwMessageComponent* aParent);
    //. Creates a new {\tt DwMessage} object on the free store.
    //. If the static data member {\tt sNewMessage} is {\tt NULL},
    //. this member function will create a new {\tt DwMessage}
    //. and return it.  Otherwise, {\tt NewMessage()} will call
    //. the user-supplied function pointed to by {\tt sNewMessage},
    //. which is assumed to return an object from a class derived from
    //. {\tt DwMessage}, and return that object.

    //+ Var sNewMessage
    static DwMessage* (*sNewMessage)(const DwString&, DwMessageComponent*);
    //. If {\tt sNewMessage} is not {\tt NULL}, it is assumed to point
    //. to a user supplied function that returns an object from a class
    //. derived from  {\tt DwMessage}.

private:

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

protected:

    void _PrintDebugInfo(std::ostream& aStrm) const;

};

#endif
