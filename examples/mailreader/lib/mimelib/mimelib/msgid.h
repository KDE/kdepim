//=============================================================================
// File:       msgid.h
// Contents:   Declarations for DwMsgId
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

#ifndef DW_MSGID_H
#define DW_MSGID_H

#ifndef DW_CONFIG_H
#include <mimelib/config.h>
#endif

#ifndef DW_FIELDBDY_H
#include <mimelib/fieldbdy.h>
#endif

//=============================================================================
//+ Name DwMsgId -- Class representing an RFC-822 msg-id
//+ Description
//. {\tt DwMsgId} represents a {\it msg-id} as described in RFC-822.  In
//. the BNF grammar in RFC-822, a msg-id has a {\it local-part} and a
//. {\it domain}.  In MIME++, a {\tt DwMsgId} contains strings that
//. contain the local-part and the domain.
//.
//. In the tree (broken-down) representation of message, a {\tt DwMsgId}
//. object may only be a leaf node, having a parent but no child nodes.
//. Its parent node must be a {\tt DwField} object.
//.
//. {\tt DwMsgId} has member functions for getting or setting its local-part
//. and its domain.  You can have the library to create the contents of a
//. {\tt DwMsgId} object for you by calling the member function
//. {\tt CreateDefault()}.
//=============================================================================
// Last modified 1997-07-28
//+ Noentry ~DwMsgId mLocalPart mDomain sClassName _PrintDebugInfo


class DW_EXPORT DwMsgId : public DwFieldBody {

public:

    DwMsgId();
    DwMsgId(const DwMsgId& aMsgId);
    DwMsgId(const DwString& aStr, DwMessageComponent* aParent=0);
    //. The first constructor is the default constructor, which sets the
    //. {\tt DwMsgId} object's string representation to the empty string
    //. and sets its parent to NULL.
    //.
    //. The second constructor is the copy constructor, which performs
    //. a deep copy of {\tt aMsgId}.
    //. The parent of the new {\tt DwMsgId} object is set to NULL.
    //.
    //. The third constructor copies {\tt aStr} to the {\tt DwMsgId}
    //. object's string representation and sets {\tt aParent} as its parent.
    //. The virtual member function {\tt Parse()} should be called immediately
    //. after this constructor in order to parse the string representation.
    //. Unless it is NULL, {\tt aParent} should point to an object of a class
    //. derived from {\tt DwField}.

    virtual ~DwMsgId();

    const DwMsgId& operator = (const DwMsgId& aMsgId);
    //. This is the assignment operator, which performs a deep copy of
    //. {\tt aMsgId}.  The parent node of the {\tt DwMsgId} object
    //. is not changed.

    virtual void Parse();
    //. This virtual function, inherited from {\tt DwMessageComponent},
    //. executes the parse method for {\tt DwMsgId} objects.  The parse
    //. method parses the local-part and the domain from the string
    //. representation.
    //.
    //. You should call this member function after you set or modify the
    //. string representation, and before you retrieve local-part or
    //. domain.
    //.
    //. This function clears the is-modified flag.

    virtual void Assemble();
    //. This virtual function, inherited from {\tt DwMessageComponent},
    //. executes the assemble method for {\tt DwMsgId} objects.  The
    //. assemble method creates or updates the string representation
    //. from the local-part and the domain.
    //.
    //. You should call this member function after you modify the
    //. local-part or the domain, and before you retrieve the string
    //. representation.
    //.
    //. This function clears the is-modified flag.

    virtual DwMessageComponent* Clone() const;
    //. This virtual function, inherited from {\tt DwMessageComponent},
    //. creates a new {\tt DwMsgId} on the free store that has the same
    //. value as this {\tt DwMsgId} object.  The basic idea is that of
    //. a ``virtual copy constructor.''

    virtual void CreateDefault();
    //. Creates a value for the msg-id.  Uses the current time,
    //. process id, and fully qualified domain name for the host.

    const DwString& LocalPart() const;
    //. Returns the local-part of the msg-id.

    void SetLocalPart(const DwString& aLocalPart);
    //. Sets the local-part of the msg-id.

    const DwString& Domain() const;
    //. Returns the domain of the msg-id.

    void SetDomain(const DwString& aDomain);
    //. Sets the domain of the msg-id.

    static DwMsgId* NewMsgId(const DwString& aStr,
        DwMessageComponent* aParent);
    //. Creates a new {\tt DwMsgId} object on the free store.
    //. If the static data member {\tt sNewMsgId} is NULL,
    //. this member function will create a new {\tt DwMsgId}
    //. and return it.  Otherwise, {\tt NewMsgId()} will call
    //. the user-supplied function pointed to by {\tt sNewMsgId},
    //. which is assumed to return an object from a class derived from
    //. {\tt DwMsgId}, and return that object.

    //+ Var sNewMsgId
    static DwMsgId* (*sNewMsgId)(const DwString&, DwMessageComponent*);
    //. If {\tt sNewMsgId} is not NULL, it is assumed to point to a
    //. user-supplied function that returns an object from a class derived from
    //. {\tt DwMsgId}.

    static const char* sHostName;
    //. Host name of machine, used to create msg-id string.  This data member
    //. is ignored if the platform supports a gethostname() function call.

private:

    DwString mLocalPart;
    DwString mDomain;
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
