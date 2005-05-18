//=============================================================================
// File:       boyermor.h
// Contents:   Declarations for DwBoyerMoore
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

#ifndef DW_BOYERMOR_H
#define DW_BOYERMOR_H

#include <stddef.h>

#ifndef DW_CONFIG_H
#include <mimelib/config.h>
#endif

#ifndef DW_STRING_H
#include <mimelib/string.h>
#endif


//=============================================================================
//+ Name DwBoyerMoore -- Class for executing Boyer-Moore string search algorithm
//+ Description
//. {\tt DwBoyerMoore} implements the Boyer-Moore algorithm for searching
//. for a string.  The Boyer-Moore algorithm is fast, but requires a bit
//. of start-up overhead compared to a brute force algorithm.
//=============================================================================
// Last modified 1997-08-23
//+ Noentry ~DwBoyerMoore


class DW_EXPORT DwBoyerMoore {

public:
    
    DwBoyerMoore(const char* aCstr);
    DwBoyerMoore(const DwString& aStr);
    DwBoyerMoore(const DwBoyerMoore& other);
    //. Constructs a {\tt DwBoyerMoore} object for searching for a particular
    //. string.

    virtual ~DwBoyerMoore();

    const DwBoyerMoore & operator=( const DwBoyerMoore & other );

    void Assign(const char* aCstr);
    void Assign(const DwString& aStr);
    //. Sets the string to search for.

    size_t FindIn(const DwString& aStr, size_t aPos, bool aCs = true) const;
    //. Searches for the search string in {\tt aStr} starting at position
    //. {\tt aPos}.  If found, the function returns the first position in
    //. {\tt aStr} where the search string was found.  If not found, the
    //. function returns {\tt DwString::npos}. Search is case sensitive iff
    //. {\tt aCs} is true.

private:

    size_t mPatLen;
    char* mPat;
    char* mCiPat;
    unsigned char mSkipAmt[256];
    unsigned char mCiSkipAmt[256]; // case insensitive skip table

    void _Assign(const char* aPat, size_t aPatLen);
};

#endif
