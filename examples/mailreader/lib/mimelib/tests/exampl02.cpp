//=============================================================================
// File:       exampl02.cpp
// Contents:   Source code for Example 2 -- Parsing a simple message
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
#include "basicmsg.h"

#include <mimelib/token.h>


int main()
{
    // Initialize the library

    DwInitialize();

    // Read message from file

    DwString messageStr = "";
    DwString line;
    std::ifstream istrm("exampl02.txt");
    while (DwTrue) {
        getline(istrm, line);
        if (istrm.eof()) {
            break;
        }
        messageStr += line + DW_EOL;
    }
    istrm.close();
    
    // Create a DwMessage and parse it.  The DwMessage should be created on
    // the free store, since it will be added to the BasicMessage.

    DwMessage* msg = DwMessage::NewMessage(messageStr, 0);
    msg->Parse();

    // Create a Message and add the DwMessage to it

    BasicMessage message(msg);

    // Open file stream for output

    std::ofstream ostrm("exampl02.out");

    // Print the header fields

    ostrm << "Type -> " << message.TypeStr() << std::endl;
    ostrm << "Subtype -> " << message.SubtypeStr() << std::endl;
    ostrm << "Content-Transfer-Encoding -> " << message.CteStr() << std::endl;
    ostrm << "Date -> " << message.DateStr() << std::endl;
    ostrm << "From -> " << message.From() << std::endl;
    ostrm << "To -> " << message.To() << std::endl;
    ostrm << "Cc -> " << message.Cc() << std::endl;
    ostrm << "Bcc -> " << message.Bcc() << std::endl;
    ostrm << "Subject -> " << message.Subject() << std::endl;

    // Print the body

    ostrm << "\nBody ->" << std::endl;
    ostrm << message.Body() << std::endl;

    return 0;
}

