//=============================================================================
// File:       attach.cpp
// Contents:   Definitions for MessageWithAttachments
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

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <mimelib/string.h>
#include <mimelib/utility.h>
#include "attach.h"


MessageWithAttachments::MessageWithAttachments()
{
}


MessageWithAttachments::~MessageWithAttachments()
{   
}


void MessageWithAttachments::SetText(const DwString& aStr)
{
    // Create a body part and set the necessary fields

    MultipartBodyPart part;
    part.SetType(DwMime::kTypeText);
    part.SetSubtype(DwMime::kSubtypePlain);
    part.SetCte(DwMime::kCte7bit);

    // Set the string as the body of the body part

    part.SetBody(aStr);

    // Set this body part as the first one

    SetBodyPart(0, part);
}


int MessageWithAttachments::NumberOfAttachments() const
{
    int n = NumberOfParts() - 1;
    return (n >= 0) ? n : 0;
}


void MessageWithAttachments::Attach7bitFile(const char* aFilename,
    int aType, int aSubtype)
{
    // Get the file contents

    DwString str;
    PutFileInString(aFilename, str);
    
    // Create a body part and set the necessary fields

    MultipartBodyPart part;
    part.SetType(aType);
    part.SetSubtype(aSubtype);
    part.SetCte(DwMime::kCte7bit);

    // Set content-disposition to attachment, with filename parameter
    // (see RFC-1806 for information on this *experimental* header field)
    
    DwString contDisp = "attachment; filename=";
    contDisp += '\"';
    contDisp += aFilename;
    contDisp += '\"';
    part.SetContentDisposition(contDisp);
    
    // Set the file contents as the body of the body part

    part.SetBody(str);

    // Make sure this is not the first part, since that is reserved for
    // the text
    
    if (NumberOfParts() == 0) {
        SetBodyPart(1, part);
    }
    else {
        AddBodyPart(part);
    }
}


void MessageWithAttachments::Attach8bitFile(const char* aFilename,
    int aType, int aSubtype)
{
    // Get the file contents

    DwString str;
    PutFileInString(aFilename, str);
    
    // Encode using quoted-printable encoding

    DwString encStr;
    DwEncodeQuotedPrintable(str, encStr);
    
    // Create a body part and set the necessary fields

    MultipartBodyPart part;
    part.SetType(aType);
    part.SetSubtype(aSubtype);
    part.SetCte(DwMime::kCteQuotedPrintable);

    // Set content-disposition to attachment, with filename parameter
    // (see RFC-1806 for information on this *experimental* header field)
    
    DwString contDisp = "attachment; filename=";
    contDisp += '\"';
    contDisp += aFilename;
    contDisp += '\"';
    part.SetContentDisposition(contDisp);

    // Set the encoded file contents as the body of the body part

    part.SetBody(encStr);

    // Make sure this is not the first part, since that is reserved for
    // the text
    
    if (NumberOfParts() == 0) {
        SetBodyPart(1, part);
    }
    else {
        AddBodyPart(part);
    }
}


void MessageWithAttachments::AttachBinaryFile(const char* aFilename,
    int aType, int aSubtype)
{
    // Get the file contents

    DwString str;
    PutFileInString(aFilename, str);
    
    // Encode using base64 encoding

    DwString encStr;
    DwEncodeBase64(str, encStr);
    
    // Create a body part and set the necessary fields

    MultipartBodyPart part;
    part.SetType(aType);
    part.SetSubtype(aSubtype);
    part.SetCte(DwMime::kCteBase64);

    // Set content-disposition to attachment, with filename parameter
    // (see RFC-1806 for information on this *experimental* header field)
    
    DwString contDisp = "attachment; filename=";
    contDisp += '\"';
    contDisp += aFilename;
    contDisp += '\"';
    part.SetContentDisposition(contDisp);

    // Set the encoded file contents as the body of the body part

    part.SetBody(encStr);

    // Make sure this is not the first part, since that is reserved for
    // the text
    
    if (NumberOfParts() == 0) {
        SetBodyPart(1, part);
    }
    else {
        AddBodyPart(part);
    }
}


int MessageWithAttachments::PutFileInString(const char* aFilename, 
    DwString& str)
{
    // Get the file size
    struct stat statBuf;
    int k = stat(aFilename, &statBuf);
    if (k < 0) {
        str = "";
        return -1;
    }
    int fileSize = (int) statBuf.st_size;

    // Allocate a buffer

    int bufSize = fileSize + 8; // a little elbow room added
    char* buf = new char[bufSize];

    // Read the file into the buffer

    FILE* fp = fopen(aFilename, "rb");
    if (fp == 0) {
        delete[] buf;
        str = "";
        return -1;
    }
    int len = 0;
    while (1) {
        int ch = getc(fp);
        if (feof(fp) || len == fileSize) {
            break;
        }
        buf[len++] = ch;
    }
    buf[len] = 0;
    fclose(fp);

    // Place the buffer in the string

    str.TakeBuffer(buf, bufSize, 0, len);
    return 0;
}

