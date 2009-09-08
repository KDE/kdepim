//=============================================================================
// File:       mboxlist.h
// Contents:   Declarations for DwMailboxList
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

#ifndef DW_MBOXLIST_H
#define DW_MBOXLIST_H

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
//+ Name DwMailboxList -- Class representing a list of RFC-822 mailboxes
//+ Description
//. {\tt DwMailboxList} represents a list of {\it mailboxes} as described
//. in RFC-822.  In MIME++, {\tt DwMailboxList} is a container for objects
//. of type {\tt DwMailbox}, and it contains various member functions to
//. manage its contained objects.  {\tt DwAddressList} is also a
//. {\tt DwFieldBody}.  This reflects the fact that certain RFC-822 header
//. fields, such as the "From" header field, have a list of mailboxes as
//. their field bodies.
//=============================================================================
// Last modified 1997-08-30
//+ Noentry ~DwMailboxList _PrintDebugInfo

class DW_EXPORT DwMailboxList : public DwFieldBody {

public:

    DwMailboxList();
    DwMailboxList(const DwMailboxList& aList);
    DwMailboxList(const DwString& aStr, DwMessageComponent* aParent=0);
    //. The first constructor is the default constructor, which sets the
    //. {\tt DwMailboxList} object's string representation to the empty string
    //. and sets its parent to {\tt NULL}.
    //.
    //. The second constructor is the copy constructor, which copies the
    //. string representation and all {\tt DwMailbox} objects from {\tt aList}.
    //. The parent of the new {\tt DwMailboxList} object is set to {\tt NULL}.
    //.
    //. The third constructor copies {\tt aStr} to the {\tt DwMailboxList}
    //. object's string representation and sets {\tt aParent} as its parent.
    //. The virtual member function {\tt Parse()} should be called immediately
    //. after this constructor in order to parse the string representation.
    //. Unless it is {\tt NULL}, {\tt aParent} should point to an object of
    //. a class derived from {\tt DwField}.

    virtual ~DwMailboxList();

    const DwMailboxList& operator = (const DwMailboxList& aList);
    //. This is the assignment operator, which performs a deep copy of
    //. {\tt aList}.  The parent node of the {\tt DwMailboxList} object
    //. is not changed.

    virtual void Parse();
    //. This virtual function, inherited from {\tt DwMessageComponent},
    //. executes the parse method for {\tt DwMailboxList} objects. The parse
    //. method creates or updates the broken-down representation from the
    //. string representation.  For {\tt DwMailboxList} objects, the parse
    //. method parses the string representation to create a list of
    //. {\tt DwMailbox} objects.  This member function also calls the
    //. {\tt Parse()} member function of each {\tt DwMailbox} object in
    //. its list.
    //.
    //. You should call this member function after you set or modify the
    //. string representation, and before you access any of the contained
    //. {\tt DwMailbox} objects.
    //.
    //. This function clears the is-modified flag.

    virtual void Assemble();
    //. This virtual function, inherited from {\tt DwMessageComponent},
    //. executes the assemble method for {\tt DwMailboxList} objects. The
    //. assemble method creates or updates the string representation from
    //. the broken-down representation.  For {\tt DwMailboxList} objects,
    //. the assemble method builds the string representation from its list
    //. of {\tt DwMailbox} objects. Before it builds the string representation
    //. for the {\tt DwMailboxList} object, this function first calls the
    //. {\tt Assemble()} member function of each {\tt DwMailbox} object
    //. in its list.
    //.
    //. You should call this member function after you set or modify any
    //. of the contained {\tt DwMailbox} objects, and before you retrieve
    //. the string representation.
    //.
    //. This function clears the is-modified flag.

    virtual DwMessageComponent* Clone() const;
    //. This virtual function, inherited from {\tt DwMessageComponent},
    //. creates a new {\tt DwMailboxList} on the free store that has the same
    //. value as this {\tt DwMailboxList} object.  The basic idea is that of
    //. a virtual copy constructor.

    DwMailbox* FirstMailbox() const;
    //. Gets the first {\tt DwMailbox} object in the list.
    //. Use the member function {\tt DwMailbox::Next()} to iterate.
    //. Returns {\tt NULL} if the list is empty.

    void Add(DwMailbox* aMailbox);
    //. Adds {\tt aMailbox} to the end of the list of {\tt DwMailbox} objects
    //. maintained by this {\tt DwMailboxList} object.

    void Remove(DwMailbox* aMailbox);
    //. Removes {\tt aMailbox} from the list of {\tt DwMailbox} objects
    //. maintained by this {\tt DwMailboxList} object.  The {\tt DwMailbox}
    //. object is not deleted by this member function.

    void DeleteAll();
    //. Removes and deletes all {\tt DwMailbox} objects from the list
    //. maintained by this {\tt DwMailboxList} object.

    static DwMailboxList* NewMailboxList(const DwString& aStr,
        DwMessageComponent* aParent);
    //. Creates a new {\tt DwMailboxList} object on the free store.
    //. If the static data member {\tt sNewMailboxList} is {\tt NULL},
    //. this member function will create a new {\tt DwMailboxList}
    //. and return it.  Otherwise, {\tt NewMailboxList()} will call
    //. the user-supplied function pointed to by {\tt sNewMailboxList},
    //. which is assumed to return an object from a class derived from
    //. {\tt DwMailboxList}, and return that object.

    //+ Var sNewMailboxList
    static DwMailboxList* (*sNewMailboxList)(const DwString&,
        DwMessageComponent*);
    //. If {\tt sNewMailboxList} is not {\tt NULL}, it is assumed to point
    //. to a user-supplied function that returns an object from a class
    //. derived from {\tt DwMailboxList}.

protected:

    DwMailbox* mFirstMailbox;
    //. Points to first {\tt DwMailbox} object in list.

    void _AddMailbox(DwMailbox* aMailbox);
    //. Adds a mailbox, but does not set the is-modified flag.

    void _DeleteAll();
    //. Removes and deletes all {\tt DwMailbox} objects from the list
    //. maintained by this {\tt DwMailboxList} object.  Doesn't set the
    //. is-modified flag.

private:

    static const char* const sClassName;

    void CopyList(const DwMailbox* aFirst);

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


class DW_EXPORT DwMailboxListParser {
public:
    enum {
        eMbError,
        eMbGroup,
        eMbMailbox,
        eMbNull,
        eMbEnd
    };
    DwMailboxListParser(const DwString& aStr);
    virtual ~DwMailboxListParser();
    const DwString& MbString() { return mMbString.Tokens(); }
    int MbType() { return mMbType; }
    int IsNull() { return (mMbType == eMbNull) ? 1 : 0; }
    int IsEnd()  { return (mMbType == eMbEnd) ? 1 : 0; }
    int Restart();
    int operator ++ (); // prefix increment operator
protected:
    void ParseNextMailbox();
    DwRfc822Tokenizer mTokenizer;
    DwTokenString mMbString;
    int mMbType;
};

#endif
