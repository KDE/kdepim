//=============================================================================
// File:       basicmsg.h
// Contents:   Declarations for BasicMessage
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

// BasicMessage is a wrapper class that serves two purposes.  First, it
// hides many of the underlying details of the class library, making the
// library easier to use.  Second, it provides good example code to show
// you how to create your own customized wrapper classes.

// BasicMessage contains a DwMessage by reference.  The reason BasicMessage
// "has-a" DwMessage and not "is-a" DwMessage is because we can assign
// the DwMessage to an appropriately specialized subclass of BasicMessage
// *after* the DwMessage is parsed.  For example, after we parse a DwMessage,
// we can determine that it is a multipart and assign it to a
// MultipartMessage instead of a BasicMessage.

#ifndef BASICMSG_H
#define BASICMSG_H

#ifndef MIMEPP_H
#include <mimelib/mimepp.h>
#endif


class BasicMessage {

public:

    // Use this constructor to create a new message
    BasicMessage();

    // Use this constructor to create a wrapper for a DwMessage that has
    // been parsed.  BasicMessage takes responsibility for deleting the
    // DwMessage object passed to the constructor, therefore, make sure
    // it is allocated on the free store.
    BasicMessage(DwMessage* aMsg);

    virtual ~BasicMessage();

    // Replace the contained DwMessage with a new DwMessage. Note:
    // + The previous DwMessage will be deleted.
    // + The BasicMessage destructor will delete the DwMessage passed as an
    //   argument.
    // Use this function to set a parsed DwMessage for a BasicMessage that
    // was created using the default constructor.
    void TakeMessage(DwMessage* aMsg);

    // Return the BasicMessage contents as a string
    const DwString& AsString();

    // Set fields that are either automatically set (Message-id)
    // or that do not change from one message to another (MIME-Version).
    // We make it a virtual function so it can be easily overridden in
    // a subclass.  In your own subclass, or your customized version of
    // this class, you may want to set the date field automatically to
    // the current date and time in this member function.
    virtual void SetAutomaticFields();
    
    // Get or set the 'Date' header field
    const DwString& DateStr() const;
    DwUint32 Date() const;
    void SetDate(DwUint32 aUnixTime);

    // Get or set the 'To' header field
    const DwString& To() const;
    void SetTo(const DwString& aStr);

    // Get or set the 'Cc' header field
    const DwString& Cc() const;
    void SetCc(const DwString& aStr);

    // Get or set the 'Bcc' header field
    const DwString& Bcc() const;
    void SetBcc(const DwString& aStr);

    // Get or set the 'From' header field
    const DwString& From() const;
    void SetFrom(const DwString& aStr);

    // Get or set the 'Subject' header field
    const DwString& Subject() const;
    void SetSubject(const DwString& aStr);

    // Get or set the 'Content-Type' header field
    // + The member functions that involve enumerated types (ints)
    //   will work only for well-known types or subtypes.
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
    //   will work only for well-known encodings
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

    // Get or set the message body
    const DwString& Body() const;
    void SetBody(const DwString& aStr);

protected:

    DwMessage* mMessage;
    DwString   mEmptyString;

};

#endif

