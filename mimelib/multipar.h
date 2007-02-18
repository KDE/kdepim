//=============================================================================
// File:       multipar.h
// Contents:   Declarations for MultiparBodyPart and MultipartMessage
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

#ifndef MULTIPAR_H
#define MULTIPAR_H

#include "basicmsg.h"


class MultipartBodyPart {

public:

    MultipartBodyPart();
    virtual ~MultipartBodyPart();

    // Get or set the 'Content-Type' header field
    // + The member functions that involve enumerated types (ints)
    //   will work only for well-known types or subtypes.  The enum
    //   values are defined in <mimepp/enum.h>.
    // Type
    const DwString& TypeStr() const;
    int Type() const;
    void SetTypeStr(const DwString& aStr);
    void SetType(int aType);
    // Subtype
    const DwString& SubtypeStr() const;
    int Subtype() const;
    void SetSubtypeStr(const DwString& aStr);
    void SetSubtype(int aSubtype);

    // Get or set the 'Content-Transfer-Encoding' header field
    // + The member functions that involve enumerated types (ints)
    //   will work only for well-known encodings.  The enum values
    //   are defined in <mimepp/enum.h>.
    const DwString& ContentTransferEncodingStr() const;
    int ContentTransferEncoding() const;
    void SetContentTransferEncodingStr(const DwString& aStr);
    void SetContentTransferEncoding(int aCte);

    // Cte is short for ContentTransferEncoding.
    // These functions are an alternative to the ones with longer names.
    const DwString& CteStr() const;
    int Cte() const;
    void SetCteStr(const DwString& aStr);
    void SetCte(int aCte);

    // Get or set the 'Content-Description' header field
    const DwString& ContentDescription() const;
    void SetContentDescription(const DwString& aStr);
    
    // Get or set the 'Content-Disposition' header field
    const DwString& ContentDisposition() const;
    void SetContentDisposition(const DwString& aStr);

    // Get or set the body of this body part
    const DwString& Body() const;
    void SetBody(const DwString& aStr);
    
protected:
    
    DwString mType;
    DwString mSubtype;
    DwString mCte;
    DwString mContentDescription;
    DwString mContentDisposition;
    DwString mBody;

};


class MultipartMessage : public BasicMessage {

public:

    // Use this constructor to create a new multipart message
    MultipartMessage();

    // Use this constructor to create a wrapper for a DwMessage that has
    // been parsed and has been verified as a multipart
    MultipartMessage(DwMessage* aMsg);

    virtual ~MultipartMessage();

    // This virtual function is overridden from BasicMessage.  In
    // MultipartMessage, we add the Content-Type header field with
    // type Multipart and subtype Mixed
    virtual void SetAutomaticFields();

    // Return the number of body parts contained
    int NumberOfParts() const;

    // Get the body part at position in aIdx.  Indexing starts at 0.
    // If there is no body part at that index, aPart will have its
    // attributes set to empty values.
    void BodyPart(int aIdx, MultipartBodyPart& aPart);
    
    // Set the body part at position in aIdx.  Indexing starts at 0.
    // If you have aIdx = 10 and there are only 2 body parts, 7 empty
    // body parts will be created to fill slots 2 through 8.  If you
    // just want to add a body part at the end, use AddBodyPart().
    void SetBodyPart(int aIdx, const MultipartBodyPart& aPart);
    
    // Append a body part to the message.
    void AddBodyPart(const MultipartBodyPart& aPart);

};

#endif

