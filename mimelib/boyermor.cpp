//=============================================================================
// File:       boyermor.cpp
// Contents:   Definitions for DwBoyerMoore
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

#define DW_IMPLEMENTATION

#include <mimelib/config.h>
#include <mimelib/debug.h>
#include <ctype.h>
#include <string.h>
#include <mimelib/boyermor.h>


DwBoyerMoore::DwBoyerMoore(const char* aCstr, bool cs)
  : mPat( 0 )
{
    size_t len = strlen(aCstr);
    _Assign(aCstr, len, cs);
}


DwBoyerMoore::DwBoyerMoore(const DwString& aStr, bool cs)
  : mPat( 0 )
{
    _Assign(aStr.data(), aStr.length(), cs);
}

DwBoyerMoore::DwBoyerMoore(const DwBoyerMoore & other)
  : mPat( 0 )
{
    _Assign(other.mPat, other.mPatLen, other.mCS);
}


DwBoyerMoore::~DwBoyerMoore()
{
    delete[] mPat; mPat = 0;
}

const DwBoyerMoore & DwBoyerMoore::operator=( const DwBoyerMoore & other )
{
    if (this != &other)
        _Assign(other.mPat, other.mPatLen, other.mCS);
    return *this;
}


void DwBoyerMoore::Assign(const char* aCstr, bool cs)
{
    size_t len = strlen(aCstr);
    _Assign(aCstr, len, cs);
}


void DwBoyerMoore::Assign(const DwString& aStr, bool cs)
{
    _Assign(aStr.data(), aStr.length(), cs);
}


void DwBoyerMoore::_Assign(const char* aPat, size_t aPatLen, bool cs)
{
    mCS = cs;
    mPatLen = 0;
    delete[] mPat; mPat = 0;
    mPat = new char[aPatLen+1];
    if (mPat != 0) {
        mPatLen = aPatLen;
	// for case-insensitive search, make a lower-case copy of the pattern:
	for (size_t i=0; i < mPatLen; ++i) {
	    mPat[i] = cs ? aPat[i] : tolower(aPat[i]);
	}
        mPat[mPatLen] = 0;
        // Initialize the jump table for Boyer-Moore-Horspool algorithm
        for (size_t i=0; i < 256; ++i) {
            mSkipAmt[i] = (unsigned char) mPatLen;
        }
        for (size_t i=0; i < mPatLen-1; ++i) {
	    unsigned char skip = mPatLen - i - 1;
	    mSkipAmt[(unsigned)mPat[i]] = skip;
	    if ( !cs ) {
		mSkipAmt[(unsigned)toupper(mPat[i])] = skip;
	    }
        }
    }
}


size_t DwBoyerMoore::FindIn(const DwString& aStr, size_t aPos) const
{
    if (aStr.length() <= aPos) {
        return (size_t) -1;
    }
    if (mPat == 0 || mPatLen == 0) {
        return 0;
    }
    size_t bufLen = aStr.length() - aPos;
    const char* buf = aStr.data() + aPos;
    size_t i;
    for (i=mPatLen-1; i < bufLen; i += mSkipAmt[(unsigned char)buf[i]]) {
        int iBuf = i;
        int iPat = mPatLen - 1;
        while (iPat >= 0 && (mCS ? buf[iBuf] : tolower(buf[iBuf])) == mPat[iPat]) {
            --iBuf;
            --iPat;
        }
        if (iPat == -1)
            return aPos + iBuf + 1;
    }
    return (size_t)-1;
}
