//=============================================================================
// File:       group.h
// Contents:   Declarations for DwGroup
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

#ifndef DW_GROUP_H
#define DW_GROUP_H

#ifndef DW_CONFIG_H
#include <mimelib/config.h>
#endif

#ifndef DW_STRING_H
#include <mimelib/string.h>
#endif

#ifndef DW_MAILBOX_H
#include <mimelib/mailbox.h>
#endif

#ifndef DW_MBOXLIST_H
#include <mimelib/mboxlist.h>
#endif

#ifndef DW_ADDRESS_H
#include <mimelib/address.h>
#endif

//=============================================================================
//+ Name DwGroup -- Class representing an RFC-822 address group
//+ Description
//. {\tt DwGroup} represents a {\it group} as described in RFC-822.  A group
//. contains a group name and a (possibly empty) list of {\it mailboxes}.
//. In MIME++, a {\tt DwGroup} object contains a string for the group name
//. and a {\tt DwMailboxList} object for the list of mailboxes.
//.
//. In the tree (broken-down) representation of message, a {\tt DwGroup}
//. object may be only an intermediate node, having both a parent and a single
//. child node.  Its parent node must be a {\tt DwField} or a
//. {\tt DwAddressList}.  Its child is a {\tt DwMailboxList}.
//.
//. A {\tt DwGroup} is a {\tt DwAddress}, and therefore it can be included
//. in a list of {\tt DwAddress} objects.  To get the next {\tt DwAddress}
//. object in a list, use the inherited member function
//. {\tt DwAddress::Next()}.
//=============================================================================
// Last updated 1997-08-24
//+ Noentry ~DwGroup _PrintDebugInfo


class DW_EXPORT DwGroup : public DwAddress {

public:

    DwGroup();
    DwGroup(const DwGroup& aGroup);
    DwGroup(const DwString& aStr, DwMessageComponent* aParent=0);
    //. The first constructor is the default constructor, which sets the
    //. {\tt DwGroup} object's string representation to the empty string
    //. and sets its parent to {\tt NULL}.
    //.
    //. The second constructor is the copy constructor, which performs
    //. a deep copy of {\tt aGroup}.
    //. The parent of the new {\tt DwGroup} object is set to {\tt NULL}.
    //.
    //. The third constructor copies {\tt aStr} to the {\tt DwGroup}
    //. object's string representation and sets {\tt aParent} as its parent.
    //. The virtual member function {\tt Parse()} should be called immediately
    //. after this constructor in order to parse the string representation.
    //. Unless it is {\tt NULL}, {\tt aParent} should point to an object of
    //. a class derived from {\tt DwField} or {\tt DwAddressList}.

    virtual ~DwGroup();

    const DwGroup& operator = (const DwGroup& aGroup);
    //. This is the assignment operator, which performs a deep copy of
    //. {\tt aGroup}.  The parent node of the {\tt DwGroup} object
    //. is not changed.

    virtual void Parse();
    //. This virtual function, inherited from {\tt DwMessageComponent},
    //. executes the parse method for {\tt DwGroup} objects. The parse
    //. method creates or updates the broken-down representation from the
    //. string representation.  For {\tt DwGroup} objects, the parse method
    //. parses the string representation to extract the group name and to
    //. create a {\tt DwMailboxList} object from the list of mailboxes. This
    //. member function also calls the {\tt Parse()} member function of
    //. the {\tt DwMailboxList} object it creates.
    //.
    //. You should call this member function after you set or modify the
    //. string representation, and before you access the group name or the
    //. mailbox list.
    //.
    //. This function clears the is-modified flag.

    virtual void Assemble();
    //. This virtual function, inherited from {\tt DwMessageComponent},
    //. executes the assemble method for {\tt DwGroup} objects. The
    //. assemble method creates or updates the string representation from
    //. the broken-down representation.  That is, the assemble method
    //. builds the string representation from its group name and mailbox
    //. list.  Before it builds the string representation, this function
    //. calls the {\tt Assemble()} member function of its contained
    //. {\tt DwMailboxList} object.
    //.
    //. You should call this member function after you set or modify either
    //. the group name or the contained {\tt DwMailboxList} object, and
    //. before you retrieve the string representation.
    //.
    //. This function clears the is-modified flag.

    virtual DwMessageComponent* Clone() const;
    //. This virtual function, inherited from {\tt DwMessageComponent},
    //. creates a new {\tt DwGroup} on the free store that has the same
    //. value as this {\tt DwGroup} object.  The basic idea is that of
    //. a virtual copy constructor.

    const DwString& GroupName() const;
    //. Returns the name of the group.

    const DwString& Phrase() const;
    //. Returns the name of the phrase part of a group as described in
    //. RFC-822.  The phrase is the same as the group name.

    void SetGroupName(const DwString& aName);
    //. Sets the name of the group.

    void SetPhrase(const DwString& aPhrase);
    //. Sets the name of the phrase part of a group as described in RFC-822.
    //. The phrase is the same as the group name.

    DwMailboxList& MailboxList() const;
    //. Provides access to the list of mailboxes that is part of a group as
    //. described in RFC-822.

    static DwGroup* NewGroup(const DwString& aStr,
        DwMessageComponent* aParent);
    //. Creates a new {\tt DwGroup} object on the free store.
    //. If the static data member {\tt sNewGroup} is {\tt NULL},
    //. this member function will create a new {\tt DwGroup}
    //. and return it.  Otherwise, {\tt NewGroup()} will call
    //. the user-supplied function pointed to by {\tt sNewGroup},
    //. which is assumed to return an object from a class derived from
    //. {\tt DwGroup}, and return that object.

    //+ Var sNewGroup
    static DwGroup* (*sNewGroup)(const DwString&, DwMessageComponent*);
    //. If {\tt sNewGroup} is not {\tt NULL}, it is assumed to point to a
    //. user-supplied function that returns an object from a class derived from
    //. {\tt DwGroup}.

protected:

    DwMailboxList* mMailboxList;
    //. Points to the {\tt DwMailboxList} object.


private:

    DwString mGroupName;
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
