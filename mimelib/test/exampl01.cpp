//=============================================================================
// File:       exampl01.cpp
// Contents:   Source code for Example 1 -- Creating a simple message
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
#include <time.h>
#include <iostream>
#include <fstream>
#include "basicmsg.h"

int main()
{
    // Initialize the library

    DwInitialize();

    // Get a buffer of data from a text file

    DwString buffer = "";
    DwString line;
    std::ifstream istrm("exampl01.txt");
    while (DwTrue) {
        getline(istrm, line);
        if (istrm.eof()) {
            break;
        }
        buffer += line + DW_EOL;
    }
    istrm.close();

    // Create a message

    BasicMessage msg;

    // Create MIME-Version and Message-id header fields

    msg.SetAutomaticFields();

    // Set header fields

    msg.SetDate(time(NULL));
    msg.SetTypeStr("Text");
    msg.SetSubtypeStr("Plain");
    msg.SetCteStr("7bit");
    msg.SetFrom("Emily Postnews <emily.postnews@usenet.com>");
    msg.SetTo("verbose@noisy");
    msg.SetCc("forgetful@myvax");
    msg.SetBcc("eager@beaver.dam");
    msg.SetSubject("Re: How long should my signature be?");

    // Set body

    msg.SetBody(buffer);

    // Write it to a file
	
    std::ofstream ostrm("exampl01.out");
    ostrm << msg.AsString();

    return 0;
}

