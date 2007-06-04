//=============================================================================
// File:       mailbox.h
// Contents:   Declarations for DwMailbox
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

#ifndef DW_MAILBOX_H
#define DW_MAILBOX_H

#ifndef DW_CONFIG_H
#include <mimelib/config.h>
#endif

#ifndef DW_STRING_H
#include <mimelib/string.h>
#endif

#ifndef DW_ADDRESS_H
#include <mimelib/address.h>
#endif


//=============================================================================
//+ Name DwMailbox -- Class representing an RFC-822 mailbox
//+ Description
//. RFC-822 defines a {\it mailbox} as an entity that can be the recipient
//. of a message.  A mailbox is more specific than an {\it address}, which
//. may be either a mailbox or a {\it group}.  An RFC-822 mailbox contains
//. a full name, a {\it local-part}, an optional {\it route}, and a
//. {\it domain}.  For example, in the mailbox
//.
//.   Joe Schmoe <jschmoe@aol.com>
//.
//. "Joe Schmoe" is the full name, "jschmoe" is the local-part, and
//. "aol.com" is the domain.  The optional route is rarely seen in current
//. usage, and is deprecated according to RFC-1123.
//.
//. In MIME++, an RFC-822 mailbox is represented by a {\tt DwMailbox} object.
//. {\tt DwMailbox} is a subclass of {\tt DwAddress}, which reflects the
//. fact that a mailbox is also an address.  A {\tt DwMailbox} contains
//. strings representing the full name, local-part, route, and domain
//. of a mailbox.
//.
//. In the tree (broken-down) representation of message, a {\tt DwMailbox}
//. object may be only a leaf node, having a parent but no child nodes.
//. Its parent node must be a {\tt DwField}, a {\tt DwAddressList}, or a
//. {\tt DwMailboxList} object.
//.
//. {\tt DwMailbox} has member functions for getting or setting the strings
//. it contains.
//.
//. {\tt DwMailbox} object can be included in a list of {\tt DwMailbox}
//. objects.  To get the next {\tt DwMailbox} object in a list, use the
//. inherited member function {\tt DwAddress::Next()}.
//=============================================================================
// Last updated 1997-08-30
//+ Noentry ~DwMailbox
//+ Noentry mFullName mRoute mLocalPart mDomain sClassName _PrintDebugInfo


class DW_EXPORT DwMailbox : public DwAddress {

    friend class DwMailboxList;

public:

    DwMailbox();
    DwMailbox(const DwMailbox& aMailbox);
    DwMailbox(const DwString& aStr, DwMessageComponent* aParent=0);
    //. The first constructor is the default constructor, which sets the
    //. {\tt DwMailbox} object's string representation to the empty string
    //. and sets its parent to {\tt NULL}.
    //.
    //. The second constructor is the copy constructor, which performs
    //. a deep copy of {\tt aMailbox}.
    //. The parent of the new {\tt DwMailbox} is set to {\tt NULL}.
    //.
    //. The third constructor copies {\tt aStr} to the {\tt DwMailbox}
    //. object's string representation and sets {\tt aParent} as its parent.
    //. The virtual member function {\tt Parse()} should be called immediately
    //. after this constructor in order to parse the string representation.
    //. Unless it is {\tt NULL}, {\tt aParent} should point to an object of a class
    //. derived from {\tt DwField}.

    virtual ~DwMailbox();

    const DwMailbox& operator = (const DwMailbox& aMailbox);
    //. This is the assignment operator, which performs a deep copy of
    //. {\tt aMailbox}.  The parent node of the {\tt DwMailbox} object
    //. is not changed.

    virtual void Parse();
    //. This virtual function, inherited from {\tt DwMessageComponent},
    //. executes the parse method for {\tt DwMailbox} objects.  The parse
    //. method creates or updates the broken-down representation from the
    //. string representation.  For {\tt DwMailbox} objects, the parse
    //. method parses the string representation into the substrings for
    //. the full name, local-part, route, and domain.
    //.
    //. You should call this member function after you set or modify the
    //. string representation, and before you retrieve the full name,
    //. local-part, route, or domain.
    //.
    //. This function clears the is-modified flag.

    virtual void Assemble();
    //. This virtual function, inherited from {\tt DwMessageComponent},
    //. executes the assemble method for {\tt DwMailbox} objects. The
    //. assemble method creates or updates the string representation from
    //. the broken-down representation.  For {\tt DwMailbox} objects, the
    //. assemble method builds the string representation from the full
    //. name, local-part, route, and domain strings.
    //.
    //. You should call this member function after you modify the full
    //. name, local-part, route, or domain, and before you retrieve the
    //. string representation.
    //.
    //. This function clears the is-modified flag.

    virtual DwMessageComponent* Clone() const;
    //. This virtual function, inherited from {\tt DwMessageComponent},
    //. creates a new {\tt DwMailbox} on the free store that has the same
    //. value as this {\tt DwMailbox} object.  The basic idea is that of
    //. a virtual copy constructor.

    const DwString& FullName() const;
    //. Returns the full name for this {\tt DwMailbox} object.

    void SetFullName(const DwString& aFullName);
    //. Sets the full name for this {\tt DwMailbox} object.


    const DwString& Route() const;
    //. Returns the route for this {\tt DwMailbox} object.

    void SetRoute(const DwString& aRoute);
    //. Sets the route for this {\tt DwMailbox} object.

    const DwString& LocalPart() const;
    //. Returns the local-part for this {\tt DwMailbox} object.

    void SetLocalPart(const DwString& aLocalPart);
    //. Sets the local-part for this {\tt DwMailbox} object.

    const DwString& Domain() const;
    //. Returns the domain for this {\tt DwMailbox} object.

    void SetDomain(const DwString& aDomain);
    //. Sets the domain for this {\tt DwMailbox} object.

    static DwMailbox* NewMailbox(const DwString& aStr, DwMessageComponent*
        aParent);
    //. Creates a new {\tt DwMailbox} object on the free store.
    //. If the static data member {\tt sNewMailbox} is {\tt NULL},
    //. this member function will create a new {\tt DwMailbox}
    //. and return it.  Otherwise, {\tt NewMailbox()} will call
    //. the user-supplied function pointed to by {\tt sNewMailbox},
    //. which is assumed to return an object from a class derived from
    //. {\tt DwMailbox}, and return that object.

    //+ Var sNewMailbox
    static DwMailbox* (*sNewMailbox)(const DwString&, DwMessageComponent*);
    //. If {\tt sNewMailbox} is not {\tt NULL}, it is assumed to point to a
    //. user-supplied function that returns an object from a class derived
    //. from {\tt DwMailbox}.

private:

    DwString  mFullName;
    DwString  mRoute;
    DwString  mLocalPart;
    DwString  mDomain;
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
