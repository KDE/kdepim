//=============================================================================
// File:       headers.h
// Contents:   Declarations for DwHeaders
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

#ifndef DW_HEADERS_H
#define DW_HEADERS_H

#include <iostream>
#include <vector>

#ifndef DW_CONFIG_H
#include <mimelib/config.h>
#endif

#ifndef DW_STRING_H
#include <mimelib/string.h>
#endif

#ifndef DW_MSGCMP_H
#include <mimelib/msgcmp.h>
#endif

#ifndef DW_ENTITY_H
#include <mimelib/entity.h>
#endif

#ifndef DW_MSGID_H
#include <mimelib/msgid.h>
#endif

#ifndef DW_MAILBOX_H
#include <mimelib/mailbox.h>
#endif

#ifndef DW_MEDIATYP_H
#include <mimelib/mediatyp.h>
#endif

#ifndef DW_DATETIME_H
#include <mimelib/datetime.h>
#endif

#ifndef DW_MECHANSM_H
#include <mimelib/mechansm.h>
#endif

#ifndef DW_DISPTYPE_H
#include <mimelib/disptype.h>
#endif

class DwMessage;
class DwBodyPart;
class DwField;
class DwFieldBody;
class DwDateTime;
class DwMailboxList;
class DwAddressList;
class DwMediaType;
class DwMechanism;
class DwText;

//=============================================================================
//+ Name DwHeaders -- Class representing the collection of header fields in a message or body part
//+ Description
//. {\tt DwHeaders} represents the collection of {\it header fields} (often
//. called just {\it headers}) in an {\it entity} (either a message or body
//. part), as described in RFC-822 and RFC-2045.  A {\tt DwHeaders} object
//. manages a list of {\tt DwField} objects, which represent the individual
//. header fields.
//.
//. In the tree (broken-down) representation of a message, a {\tt DwHeaders}
//. object is an intermediate node, having both a parent node and several
//. child nodes.  The parent node is the {\tt DwEntity} object that contains
//. it.  The child nodes are the {\tt DwField} objects in the list it manages.
//. (See the man page for {\tt DwMessageComponent} for a discussion of
//. the tree representation of a message.)
//.
//. Normally, you do not create a {\tt DwHeaders} object directly, but you
//. access it through the {\tt Headers()} member function of {\tt DwEntity},
//. which creates the {\tt DwHeaders} object for you.
//.
//. While {\tt DwHeaders} has public member functions for managing the list
//. of {\tt DwField} objects it contains, you will normally use convenience
//. functions to access the field bodies of the header fields directly.
//. You can access the field body for a specific well-known header field
//. by using the member function {\tt <Field>()}, where {\tt <Field>} is
//. the field name of the header field with hyphens removed and the first
//. word following a hyphen capitalized.  For example, to access the field
//. body for the "MIME-version" header field, use {\tt MimeVersion()}.
//. The member function {\tt <Field>()} will create a header field with
//. field name {\tt <Field>} if such a header field does not already exist.
//. You can check for the existence of a particular well-known header field
//. by using the member function {\tt Has<Field>()}.  For example, to check
//. for the existence of the MIME-version header field, use
//. {\tt HasMimeVersion()}.  Well-known header fields are those documented in
//. RFC-822 (standard email), RFC-1036 (USENET messages), RFC-2045 (MIME
//. messages), and possibly other RFCs.
//.
//. In the case of an extension field or user-defined field, you can access
//. the field body of the header field by calling the member function
//. {\tt FieldBody()} with the field name as its argument.  If the extension
//. field or user-defined field does not exist, {\tt FieldBody()} will
//. create it.  You can check for the existence of an extension field or
//. user-defined field by using the member function {\tt HasField()} with
//. the field name as its argument.
//.
//. {\tt DwHeaders} has several other member functions provided for the
//. sake of completeness that are not required for most applications.
//. These functions are documented below.
//=============================================================================
// Last updated 1997-08-26
//+ Noentry ~DwHeaders sClassName CopyFields mFirstField _AddField


class DW_EXPORT DwHeaders : public DwMessageComponent {

public:

    DwHeaders();
    DwHeaders(const DwHeaders& aHeaders);
    DwHeaders(const DwString& aStr, DwMessageComponent* aParent=0);
    //. The first constructor is the default constructor, which sets the
    //. {\tt DwHeaders} object's string representation to the empty string
    //. and sets its parent to {\tt NULL}.
    //.
    //. The second constructor is the copy constructor, which performs a
    //. deep copy of {\tt aHeaders}.
    //. The parent of the new {\tt DwHeaders} object is set to {\tt NULL}.
    //.
    //. The third constructor copies {\tt aStr} to the {\tt DwHeaders}
    //. object's string representation and sets {\tt aParent} as its parent.
    //. The virtual member function {\tt Parse()} should be called immediately
    //. after this constructor in order to parse the string representation.
    //. Unless it is {\tt NULL}, {\tt aParent} should point to an object of a class
    //. derived from {\tt DwEntity}.

    virtual ~DwHeaders();

    const DwHeaders& operator = (const DwHeaders& aHeaders);
    //. This is the assignment operator, which performs a deep copy of
    //. {\tt aHeaders}.  The parent node of the {\tt DwHeaders} object
    //. is not changed.

    virtual void Parse();
    //. This virtual function, inherited from {\tt DwMessageComponent},
    //. executes the parse method for {\tt DwHeaders} objects.  The parse
    //. method creates or updates the broken-down representation from the
    //. string representation.  For {\tt DwHeaders} objects,
    //. {\tt DwHeaders::Parse()} parses the string representation to create
    //. a list of {\tt DwField} objects.  This member function also calls
    //. the {\tt Parse()} member function of each {\tt DwField} object in
    //. its list.
    //.
    //. You should call this member function after you set or modify the
    //. string representation, and before you access any of the header fields.
    //.
    //. This function clears the is-modified flag.

    virtual void Assemble();
    //. This virtual function, inherited from {\tt DwMessageComponent},
    //. executes the assemble method for {\tt DwHeaders} objects.  The
    //. assemble method creates or updates the string representation from
    //. the broken-down representation.  That is, the assemble method
    //. builds the string representation from its list of {\tt DwField}
    //. objects.  Before it builds the string representation, this function
    //. first calls the {\tt Assemble()} member function of each {\tt DwField}
    //. object in its list.
    //.
    //. You should call this member function after you set or modify any
    //. of the header fields, and before you retrieve the string
    //. representation.
    //.
    //. This function clears the is-modified flag.

    virtual DwMessageComponent* Clone() const;
    //. This virtual function, inherited from {\tt DwMessageComponent},
    //. creates a new {\tt DwHeaders} on the free store that has the same
    //. value as this {\tt DwHeaders} object.  The basic idea is that of
    //. a virtual copy constructor.

    DwBool HasBcc() const;
    DwBool HasCc() const;
    DwBool HasComments() const;
    DwBool HasDate() const;
    DwBool HasEncrypted() const;
    DwBool HasFrom() const;
    DwBool HasInReplyTo() const;
    DwBool HasKeywords() const;
    DwBool HasMessageId() const;
    DwBool HasReceived() const;
    DwBool HasReferences() const;
    DwBool HasReplyTo() const;
    DwBool HasResentBcc() const;
    DwBool HasResentCc() const;
    DwBool HasResentDate() const;
    DwBool HasResentFrom() const;
    DwBool HasResentMessageId() const;
    DwBool HasResentReplyTo() const;
    DwBool HasResentSender() const;
    DwBool HasResentTo() const;
    DwBool HasReturnPath() const;
    DwBool HasSender() const;
    DwBool HasSubject() const;
    DwBool HasTo() const;
    // RFC-822 fields
    //
    DwBool HasApproved() const;
    DwBool HasControl() const;
    DwBool HasDistribution() const;
    DwBool HasExpires() const;
    DwBool HasFollowupTo() const;
    DwBool HasLines() const;
    DwBool HasNewsgroups() const;
    DwBool HasOrganization() const;
    DwBool HasPath() const;
    DwBool HasSummary() const;
    DwBool HasXref() const;
    // RFC-1036 fields
    //
    DwBool HasContentDescription() const;
    DwBool HasContentId() const;
    DwBool HasContentTransferEncoding() const;
    DwBool HasCte() const;
    DwBool HasContentType() const;
    DwBool HasMimeVersion() const;
    // RFC-2045 fields
    //
    DwBool HasContentDisposition() const;
    // RFC-1806
    //
    //. Each member function in this group returns a boolean value indicating
    //. whether a particular well-known header field is present in this
    //. object's collection of header fields.

    DwBool HasField(const char* aFieldName) const;
    DwBool HasField(const DwString& aFieldName) const;
    //. Returns true if the header field specified by {\tt aFieldName} is
    //. present in this object's collection of header fields.  These member
    //. functions are used for extension fields or user-defined fields.

    DwAddressList&  Bcc();
    DwAddressList&  Cc();
    DwText&         Comments();
    DwDateTime&     Date();
    DwText&         Encrypted();
    DwMailboxList&  From();
    DwText&         InReplyTo();
    DwText&         Keywords();
    DwMsgId&        MessageId();
    DwText&         Received();
    DwText&         References();
    DwAddressList&  ReplyTo();
    DwAddressList&  ResentBcc();
    DwAddressList&  ResentCc();
    DwDateTime&     ResentDate();
    DwMailboxList&  ResentFrom();
    DwMsgId&        ResentMessageId();
    DwAddressList&  ResentReplyTo();
    DwMailbox&      ResentSender();
    DwAddressList&  ResentTo();
    DwAddress&      ReturnPath();
    DwMailbox&      Sender();
    DwText&         Subject();
    DwAddressList&  To();
    // RFC-822 fields
    //
    DwText& Approved();
    DwText& Control();
    DwText& Distribution();
    DwText& Expires();
    DwText& FollowupTo();
    DwText& Lines();
    DwText& Newsgroups();
    DwText& Organization();
    DwText& Path();
    DwText& Summary();
    DwText& Xref();
    // RFC-1036 fields (USENET messages)
    //
    DwText&         ContentDescription();
    DwMsgId&        ContentId();
    DwMechanism&    ContentTransferEncoding();
    DwMechanism&    Cte();
    DwMediaType&    ContentType();
    DwText&         MimeVersion();
    // RFC-2045 fields
    //
    DwDispositionType& ContentDisposition();
    // RFC-1806 Content-Disposition field
    //
    //. Each member function in this group returns a reference to a
    //. {\tt DwFieldBody} object for a particular header field.  If the
    //. header field does not already exist, it is created.  Use the
    //. corresponding {\tt Has<Field>()} function to test if the header
    //. field already exists without creating it.

    DwFieldBody& FieldBody(const DwString& aFieldName);
    //. Returns a reference to the {\tt DwFieldBody} object for a particular
    //. header field with field name {\tt aFieldName}.  If the header field
    //. does not already exist, it is created.  Use {\tt HasField()}
    //. to test if the header field already exists without creating it.
    //. This member function allows access to extension fields or
    //. user-defined fields.

    std::vector<DwFieldBody*> AllFieldBodies(const DwString& aFieldName);
    //. Returns a vector of pointers to the {\tt DwFieldBody} objects for
    //. all header fields with field name {\tt aFieldName}.  If the header
    //. field does not already exist, it is created.  Use {\tt HasField()}
    //. to test if the header field already exists without creating it.
    //. This member function allows access to extension fields or
    //. user-defined fields.

    int NumFields() const;
    //. Returns the number of {\tt DwField} objects contained by this
    //. {\tt DwHeaders} object.

    DwField* FirstField() const;
    //. Returns a pointer to the first {\tt DwField} object contained by
    //. this {\tt DwHeaders} object.  Use this member function to begin an
    //. iteration over the entire list of {\tt DwField} objects.
    //. Continue the iteration by calling {\tt DwField::Next()} on each
    //. {\tt DwField} object.

    DwField* FindField(const char* aFieldName) const;
    DwField* FindField(const DwString& aFieldName) const;
    //. Searches for a header field by its field name.  Returns {\tt NULL}
    //. if the field is not found.  This is an {\it advanced} function:
    //. most applications should use the {\tt <Field>()} or
    //. {\tt Has<Field>()} family of functions.

    void AddOrReplaceField(DwField* aField);
    //. Adds a {\tt DwField} object to the list.  If a header field with
    //. the same field name already exists, it is replaced by the new
    //. header field.
    //.
    //. {\tt DwHeaders} takes responsibility for deleting the added
    //. {\tt DwField} object.
    //.
    //. This is an advanced function.  Consider using the member functions
    //. {\tt <Field>()} (e.g. {\tt To()}, {\tt ContentType()}, and so on)
    //. and {\tt FieldBody()} to add header fields.

    void AddField(DwField* aField);
    //. Adds a {\tt DwField} object to the list. If a header field with
    //. the same field name already exists, it is {\it not} replaced;
    //. thus, duplicate header fields may occur when using this member
    //. function.  (This is what you want for some header fields, such as
    //. the "Received" header field).
    //.
    //. {\tt DwHeaders} takes responsibility for deleting the added
    //. {\tt DwField} object.
    //.
    //. This is an advanced function.  Consider using the member functions
    //. {\tt <Field>()} (e.g. {\tt To()}, {\tt ContentType()}, and so on)
    //. and {\tt FieldBody()} for adding header fields.

    void AddFieldAt(int aPos, DwField* aField);
    //. This member functions follows the semantics of {\tt AddField()}
    //. except that {\tt aPos} specifies a position for adding the field.
    //. A position of 1 indicates the beginning of the list.  A position of
    //. 0 indicates the end of the list.
    //.
    //. This is an advanced function.  Consider using the member functions
    //. {\tt <Field>()} (e.g. {\tt To()}, {\tt ContentType()}, and so on)
    //. and {\tt FieldBody()} for adding header fields.

    void RemoveField(DwField* aField);
    //. Removes the {\tt DwField} object from the list.  The {\tt DwField}
    //. object is not deleted.

    void DeleteAllFields();
    //. Removes all {\tt DwField} objects from the list and deletes them.

    static DwHeaders* NewHeaders(const DwString& aStr,
        DwMessageComponent* aParent);
    //. Creates a new {\tt DwHeaders} object on the free store.
    //. If the static data member {\tt sNewHeaders} is {\tt NULL},
    //. this member function will create a new {\tt DwHeaders}
    //. and return it.  Otherwise, {\tt NewHeaders()} will call
    //. the user-supplied function pointed to by {\tt sNewHeaders},
    //. which is assumed to return an object from a class derived from
    //. {\tt DwHeaders}, and return that object.

    //+ Var sNewHeaders
    static DwHeaders* (*sNewHeaders)(const DwString&, DwMessageComponent*);
    //. If {\tt sNewHeaders} is not {\tt NULL}, it is assumed to point to a
    //. user-supplied function that returns an object from a class derived from
    //. {\tt DwHeaders}.

protected:

    void _AddField(DwField* aField);
    //. Add field but don't set the is-modified flag

    DwField* mFirstField;
    DwField* mLastField;

protected:

    static const char* const sClassName;

    void CopyFields(DwField* aFirst);

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

private:

    void _PrintDebugInfo(std::ostream& aStrm) const;

};


inline DwField* DwHeaders::FirstField() const
{
    return mFirstField;
}

#endif

