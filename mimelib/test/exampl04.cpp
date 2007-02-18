//=============================================================================
// File:       exampl04.cpp
// Contents:   Source code for Example 4 -- Parsing a multipart message
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

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include "multipar.h"


int main()
{
    // Initialize the library

    DwInitialize();

    // Read message from file

    DwString messageStr = "";
    DwString line;
    std::ifstream istrm("exampl04.txt");
    while (DwTrue) {
        getline(istrm, line);
        if (istrm.eof()) {
            break;
        }
        messageStr += line + DW_EOL;
    }
    istrm.close();
    
    // Create a DwMessage and parse it.  The DwMessage should be created on
    // the free store, since it will be added to the MultipartMessage.

    DwMessage* msg = DwMessage::NewMessage(messageStr, 0);
    msg->Parse();

    // Make sure it is a multipart message
    // If is not a multipart message, we could create a BasicMessage instead,
    // but we won't do that in this example.
    
    if (msg->Headers().ContentType().Type() != DwMime::kTypeMultipart) {
        std::cerr << "Not a multipart message\n";
        return 0;
    }
    
    // Create a MultipartMessage

    MultipartMessage multipart(msg);

    // Open file stream for output

    std::ofstream ostrm("exampl04.out");

    // Print the header fields

    ostrm << "Type -> "    << multipart.TypeStr()    << std::endl;
    ostrm << "Subtype -> " << multipart.SubtypeStr() << std::endl;
    ostrm << "Date -> "    << multipart.DateStr()    << std::endl;
    ostrm << "From -> "    << multipart.From()       << std::endl;
    ostrm << "To -> "      << multipart.To()         << std::endl;
    ostrm << "Cc -> "      << multipart.Cc()         << std::endl;
    ostrm << "Bcc -> "     << multipart.Bcc()        << std::endl;
    ostrm << "Subject -> " << multipart.Subject()    << std::endl;
    
    // Read the body parts and print them

    MultipartBodyPart part;
    DwString body;
    int numParts = multipart.NumberOfParts();
    for (int idx=0; idx < numParts; ++idx) {
        multipart.BodyPart(idx, part);
        ostrm << "\nBody part number " << idx << std::endl;
        ostrm << "Type -> " << part.TypeStr() << std::endl;
        ostrm << "Subtype -> " << part.SubtypeStr() << std::endl;
        ostrm << "Content transfer encoding -> "
            << part.ContentTransferEncodingStr() << std::endl;
        ostrm << "Content description -> "
            << part.ContentDescription() << std::endl;
        int cte = part.ContentTransferEncoding();
        if (cte == DwMime::kCteBase64) {
            DwDecodeBase64(part.Body(), body);
            ostrm << "Body (decoded) ->" << std::endl << body << std::endl;
        }
        else if (cte == DwMime::kCteQuotedPrintable) {
            DwDecodeQuotedPrintable(part.Body(), body);
            ostrm << "Body (decoded) ->" << std::endl << body << std::endl;
        }
        else {
            body = part.Body();
            ostrm << "Body ->" << std::endl << body << std::endl;
        }
    }
    return 0;
}

