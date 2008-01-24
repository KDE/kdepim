//=============================================================================
// File:       text.h
// Contents:   Declarations for DwText
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

#ifndef DW_TEXT_H
#define DW_TEXT_H

#ifndef DW_CONFIG_H
#include <mimelib/config.h>
#endif

#ifndef DW_STRING_H
#include <mimelib/string.h>
#endif

#ifndef DW_FIELDBDY_H
#include <mimelib/fieldbdy.h>
#endif

//=============================================================================
//+ Name DwText -- Class representing text in a RFC-822 header field-body
//+ Description
//. {\tt DwText} represents an unstructured field body in a header field.
//. It roughly corresponds to the {\it text} element of the BNF grammar
//. defined in RFC-822.
//=============================================================================
// Last modified 1997-07-30
//+ Noentry ~DwText sClassName _PrintDebugInfo


class DW_EXPORT DwText : public DwFieldBody {

public:

    DwText();
    DwText(const DwText& aText);
    DwText(const DwString& aStr, DwMessageComponent* aParent=0);
    //. The first constructor is the default constructor, which sets the
    //. {\tt DwText} object's string representation to the empty string
    //. and sets its parent to NULL.
    //.
    //. The second constructor is the copy constructor, which copies the
    //. string representation from {\tt aText}.
    //. The parent of the new {\tt DwText} object is set to NULL.
    //.
    //. The third constructor copies {\tt aStr} to the {\tt DwText}
    //. object's string representation and sets {\tt aParent} as its parent.
    //. The virtual member function {\tt Parse()} should be called immediately
    //. after this constructor in order to parse the string representation.
    //. Unless it is NULL, {\tt aParent} should point to an object of a class
    //. derived from {\tt DwField}.

    virtual ~DwText();

    const DwText& operator = (const DwText& aText);
    //. This is the assignment operator.

    virtual void Parse();
    //. This virtual member function is inherited from
    //. {\tt DwMessageComponent}, where it is declared a pure virtual
    //. function.  For a {\tt DwText} object, this member function does
    //. nothing, since {\tt DwText} represents an unstructured field body
    //. (like the Subject header field) that does not have a broken-down
    //. form.
    //.
    //. Note, however, that this function should still be called consistently,
    //. since a subclass of {\tt DwText} may implement a parse method.
    //.
    //. This function clears the is-modified flag.

    virtual void Assemble();
    //. This virtual member function is inherited from
    //. {\tt DwMessageComponent}, where it is declared a pure virtual
    //. function.  For a {\tt DwText} object, this member function does
    //. nothing, since {\tt DwText} represents an unstructured field body
    //. (like the Subject header field) that does not have a broken-down
    //. form.
    //.
    //. Note, however, that this function should still be called consistently,
    //. since a subclass of {\tt DwText} may implement an assemble method.
    //.
    //. This function clears the is-modified flag.

    virtual DwMessageComponent* Clone() const;
    //. This virtual function, inherited from {\tt DwMessageComponent},
    //. creates a new {\tt DwText} on the free store that has the same
    //. value as this {\tt DwText} object.  The basic idea is that of
    //. a ``virtual copy constructor.''

    static DwText* NewText(const DwString& aStr, DwMessageComponent* aParent);
    //. Creates a new {\tt DwText} object on the free store.
    //. If the static data member {\tt sNewText} is NULL,
    //. this member function will create a new {\tt DwText}
    //. and return it.  Otherwise, {\tt NewText()} will call
    //. the user-supplied function pointed to by {\tt sNewText},
    //. which is assumed to return an object from a class derived from
    //. {\tt DwText}, and return that object.

    //+ Var sNewText
    static DwText* (*sNewText)(const DwString&, DwMessageComponent*);
    //. If {\tt sNewText} is not NULL, it is assumed to point to a
    //. user-supplied function that returns an object from a class derived from
    //. {\tt DwText}.

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
