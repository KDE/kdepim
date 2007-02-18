//=============================================================================
// File:       disptype.h
// Contents:   Declarations for DwDispositionType
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

#ifndef DW_DISPTYPE_H
#define DW_DISPTYPE_H

#ifndef DW_CONFIG_H
#include <mimelib/config.h>
#endif

#ifndef DW_STRING_H
#include <mimelib/string.h>
#endif

#ifndef DW_FIELDBDY_H
#include <mimelib/fieldbdy.h>
#endif

class DwParameter;

//=============================================================================
//+ Name DwDispositionType -- Class representing a MIME content-disposition field body
//+ Description
//. {\tt DwDispositionType} represents a field body for the
//. Content-Disposition header field as described in RFC-1806.  This header
//. field specifies whether the content of a message or body part should
//. be displayed automatically to a user.  A disposition-type of inline
//. indicates that the content should be displayed; a disposition-type
//. of attachment indicates that it should not be.  RFC-1806 specifies
//. that a filename parameter may be optionally included in the field
//. body; the filename parameter suggests a file name for saving the
//. message or body part's content.
//.
//. {\tt DwDispositionType} provides convenience functions that allow you
//. to set or get the disposition-type as an enumerated value, to set or
//. get the filename parameter, or to manage a list of parameters.
//.
//. RFC-1806 specifically states that the Content-Disposition header field
//. is experimental and not a proposed standard.
//=============================================================================
// Last modified 1997-08-23
//+ Noentry ~DwDispositionType _AddParameter EnumToStr StrToEnum
//+ Noentry DeleteParameterList CopyParameterList mDispositionType
//+ Noentry mDispositionTypeStr mFilenameStr mFirstParameter
//+ Noentry PrintDebugInfo _PrintDebugInfo CheckInvariants


class DW_EXPORT DwDispositionType : public DwFieldBody {

public:

    DwDispositionType();
    DwDispositionType(const DwDispositionType& aDispType);
    DwDispositionType(const DwString& aStr, DwMessageComponent* aParent=0);
    //. The first constructor is the default constructor, which sets the
    //. {\tt DwDispositionType} object's string representation to the empty
    //. string and sets its parent to {\tt NULL}.
    //.
    //. The second constructor is the copy constructor, which performs
    //. deep copy of {\tt aDispType}.
    //. The parent of the new {\tt DwDispositionType} object is set to
    //. {\tt NULL}.
    //.
    //. The third constructor copies {\tt aStr} to the {\tt DwDispositionType}
    //. object's string representation and sets {\tt aParent} as its parent.
    //. The virtual member function {\tt Parse()} should be called immediately
    //. after this constructor in order to parse the string representation.
    //. Unless it is {\tt NULL}, {\tt aParent} should point to an object of
    //. a class derived from {\tt DwField}.

    virtual ~DwDispositionType();

    const DwDispositionType& operator = (const DwDispositionType& aDispType);
    //. This is the assignment operator, which performs a deep copy of
    //. {\tt aDispType}.  The parent node of the {\tt DwDipositionType}
    //. object is not changed.

    virtual void Parse();
    //. This virtual function, inherited from {\tt DwMessageComponent},
    //. executes the parse method for {\tt DwDispositionType} objects.
    //. It should be called immediately after the string representation
    //. is modified and before the parts of the broken-down
    //. representation are accessed.
    //.
    //. This function clears the is-modified flag.

    virtual void Assemble();
    //. This virtual function, inherited from {\tt DwMessageComponent},
    //. executes the assemble method for {\tt DwDispositionType} objects.
    //. It should be called whenever one of the object's attributes
    //. is changed in order to assemble the string representation from
    //. its broken-down representation.  It will be called
    //. automatically for this object by the parent object's
    //. {\tt Assemble()} member function if the is-modified flag is set.
    //.
    //. This function clears the is-modified flag.

    virtual DwMessageComponent* Clone() const;
    //. This virtual function, inherited from {\tt DwMessageComponent},
    //. creates a new {\tt DwDispositionType} object on the free store that
    //. has the same value as this {\tt DwDispositionType} object.  The basic
    //. idea is that of a virtual copy constructor.

    int DispositionType() const;
    //. Returns the disposition-type as an enumerated value. Valid
    //. enumerated types, which are defined in enum.h, include
    //. {\tt DwMime::kDispTypeNull}, {\tt DwMime::kDispTypeUnknown},
    //. {\tt DwMime::kDispTypeInline}, and {\tt DwMime::kDispTypeAttachment}.

    void SetDispositionType(int aType);
    //. Sets the disposition-type from the enumerated value {\tt aType}.
    //. Valid enumerated types, which are defined in enum.h, include
    //. {\tt DwMime::kDispTypeNull}, {\tt DwMime::kDispTypeUnknown},
    //. {\tt DwMime::kDispTypeInline}, and {\tt DwMime::kDispTypeAttachment}.

    const DwString& DispositionTypeStr() const;
    //. Returns the disposition-type as a string.

    void SetDispositionTypeStr(const DwString& aStr);
    //. Sets the disposition-type from a string.

    const DwString& Filename() const;
    //. This convenience function returns the value from the filename
    //. parameter, if present.  If no filename parameter is present,
    //. an empty string is returned.

    void SetFilename(const DwString& aStr);
    //. This convenience function sets the value of the filename parameter
    //. to {\tt aStr}.

    DwParameter* FirstParameter() const;
    //. Returns the first {\tt DwParameter} object in the list managed by
    //. this {\tt DwDispositionType} object, or {\tt NULL} if no parameters are
    //. present.  Use {\tt DwParameter::Next()} to iterate through the list.

    void AddParameter(DwParameter* aParam);
    //. Adds a {\tt DwParameter} object to the list managed by this
    //. {\tt DwDispositionType} object.

    static DwDispositionType* NewDispositionType(const DwString& aStr,
        DwMessageComponent* aParent);
    //. Creates a new {\tt DwDispositionType} object on the free store.
    //. If the static data member {\tt sNewDispositionType} is {\tt NULL},
    //. this member function will create a new {\tt DwDispositionType}
    //. and return it.  Otherwise, {\tt NewDispositionType()} will call
    //. the user-supplied function pointed to by {\tt sNewDispositionType},
    //. which is assumed to return an object from a class derived from
    //. {\tt DwDispositionType}, and return that object.

    //+ Var sNewDispositionType
    static DwDispositionType* (*sNewDispositionType)(const DwString&,
        DwMessageComponent*);
    //. If {\tt sNewDispositionType} is not {\tt NULL}, it is assumed to
    //. point to a  user-supplied function that returns an object from a
    //. class derived from {\tt DwDispositionType}.

protected:

    void _AddParameter(DwParameter* aParam);
    //. Adds a parameter to the list without setting the is-modified flag.

    virtual void EnumToStr();
    virtual void StrToEnum();
    void DeleteParameterList();
    void CopyParameterList(DwParameter* aFirst);

    int mDispositionType;
    DwString mDispositionTypeStr;
    DwString mFilenameStr;
    DwParameter* mFirstParameter;

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
