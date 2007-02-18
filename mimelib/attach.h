//=============================================================================
// File:       attach.h
// Contents:   Declarations for MessageWithAttachments
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

#ifndef ATTACH_H
#define ATTACH_H

#include "multipar.h"


class DwString;


class MessageWithAttachments : public MultipartMessage {

public:

    MessageWithAttachments();
    virtual ~MessageWithAttachments();
    
    void SetText(const DwString& aStr);
    int NumberOfAttachments() const;
    void Attach7bitFile(const char* aFilename, int aType=DwMime::kTypeText,
        int aSubtype=DwMime::kSubtypePlain);
    void Attach8bitFile(const char* aFilename, int aType=DwMime::kTypeText,
         int aSubtype=DwMime::kSubtypePlain);
    void AttachBinaryFile(const char* aFilename, int aType=DwMime::kTypeApplication,
        int aSubtype=DwMime::kSubtypeOctetStream);

protected:

    int PutFileInString(const char* aFilename, DwString& str);

};

#endif
