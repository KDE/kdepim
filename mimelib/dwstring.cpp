//=============================================================================
// File:       dwstring.cpp
// Contents:   Definitions for DwString
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
#include <stdlib.h>
#include <string.h>
#include <new>
#include <mimelib/string.h>

// mmap
#include <unistd.h>
#include <sys/mman.h>


#define DW_MIN(a,b) ((a) <= (b) ? (a) : (b))
#define DW_MAX(a,b) ((a) >= (b) ? (a) : (b))

/* In some locales (such as tr_TR.UTF-8, az_AZ) using tolower() can cause
   unexpected results. Keywords must be compared in a
   locale-independent manner */
static int dw_strasciicasecmp(const char* s1, size_t len1, const char* s2,
    size_t len2)
{
    assert(s1 != 0);
    assert(s2 != 0);
    size_t len = DW_MIN(len1, len2);
    for (size_t i=0; i < len; ++i) {
        int c1 = s1[i];
        int c2 = s2[i];
        if ( c1 >= 'A' && c1 <= 'Z' )
            c1 += 'a' - 'A';
        if ( c2 >= 'A' && c2 <= 'Z' )
            c2 += 'a' - 'A';

        if ( c1 < c2 )
            return -1;
        else if ( c1 > c2 )
            return 1;
    }
    if (len1 < len2) {
        return -1;
    }
    else if (len1 > len2) {
        return 1;
    }
    return 0;
}

#if 0
static int dw_strcasecmp(const char* s1, size_t len1, const char* s2,
    size_t len2)
{
    assert(s1 != 0);
    assert(s2 != 0);
    size_t len = DW_MIN(len1, len2);
    for (size_t i=0; i < len; ++i) {
        int c1 = tolower(s1[i]);
        int c2 = tolower(s2[i]);
        if (c1 < c2) {
            return -1;
        }
        else if (c1 > c2) {
            return 1;
        }
    }
    if (len1 < len2) {
        return -1;
    }
    else if (len1 > len2) {
        return 1;
    }
    return 0;
}
#endif


static int dw_strcmp(const char* s1, size_t len1, const char* s2, size_t len2)
{
    assert(s1 != 0);
    assert(s2 != 0);
    size_t len = DW_MIN(len1, len2);
    for (size_t i=0; i < len; ++i) {
        if (s1[i] < s2[i]) {
            return -1;
        }
        else if (s1[i] > s2[i]) {
            return 1;
        }
    }
    if (len1 < len2) {
        return -1;
    }
    else if (len1 > len2) {
        return 1;
    }
    return 0;
}


// Copy

inline void mem_copy(const char* src, size_t n, char* dest)
{
    assert(src != 0);
    assert(dest != 0);
    assert(src != dest);
    if (n == 0 || src == dest || !src || !dest) return;
    memmove(dest, src, n);
}

#if !defined(DW_USE_ANSI_STRING)


// Allocate buffer whose size is a power of 2

static char* mem_alloc(size_t* aSize)
{
    assert(aSize != 0);
    // minimum size is 32
    size_t size = 32;
    while (size < *aSize) {
        size <<= 1;
    }
    *aSize = 0;
    char* buf = new char[size];
    if (buf != 0)
        *aSize = size;
    return buf;
}


// Free buffer

inline void mem_free(char* buf)
{
    assert(buf != 0);
    if (buf && buf != DwString::sEmptyBuffer)
        delete [] buf;
}


inline DwStringRep* new_rep_reference(DwStringRep* rep)
{
    assert(rep != 0);
    ++rep->mRefCount;
    return rep;
}


inline void delete_rep_safely(DwStringRep* rep)
{
    assert(rep != 0);
#if defined(DW_DEBUG_VERSION) || defined(DW_DEVELOPMENT_VERSION)
    if (rep->mRefCount <= 0) {
        std::cerr << "Error: attempt to delete a DwStringRep "
            "with ref count <= 0" << std::endl;
        std::cerr << "(Possibly 'delete' was called twice for same object)"
            << std::endl;
        abort();
    }
#endif //  defined(DW_DEBUG_VERSION) || defined(DW_DEVELOPMENT_VERSION)
    --rep->mRefCount;
    if (rep->mRefCount == 0) {
        delete rep;
    }
}


//--------------------------------------------------------------------------


//DwStringRep* DwStringRep::theirPool = NULL;
//int DwStringRep::theirPoolCount = 0;


// DwStringRep takes ownership of the buffer passed as an argument

DwStringRep::DwStringRep(char* aBuf, size_t aSize)
{
    assert(aBuf != 0);
    mSize = aSize;
    mBuffer = aBuf;
    mRefCount = 1;
    mPageMod = 0;
}

DwStringRep::DwStringRep(FILE* aFile, size_t aSize)
{
    assert(aFile != 0);
    static int pagesize = -1;
    if (pagesize < 0)
	pagesize = getpagesize();
    int tell = ftell(aFile);
    mPageMod = tell % pagesize;
    mSize = aSize;
    mRefCount = 1;

    mBuffer = (char *)mmap(0, aSize + mPageMod, PROT_READ, MAP_SHARED, fileno(aFile), tell - mPageMod) + mPageMod;
    ++mPageMod;
    if (mBuffer == MAP_FAILED) {
	mBuffer = 0;
	mSize = 0;
	mPageMod = 0;
    }
}


DwStringRep::~DwStringRep()
{
#if defined (DW_DEBUG_VERSION) || defined (DW_DEVELOPMENT_VERSION)
    if (mBuffer == 0) {
        std::cerr << "DwStringRep destructor called for bad DwStringRep object"
            << std::endl;
        std::cerr << "(Possibly 'delete' was called twice for same object)"
            << std::endl;
        abort();
    }
#endif //  defined (DW_DEBUG_VERSION) || defined (DW_DEVELOPMENT_VERSION)
    if (mPageMod) {
	--mPageMod;
	munmap(mBuffer - mPageMod, mSize + mPageMod);
    } else {
	mem_free(mBuffer);
    }
    //DEV_STMT(mBuffer = 0)
}


void DwStringRep::CheckInvariants() const
{
#if defined (DW_DEBUG_VERSION)
    assert(mBuffer != 0);
    assert(mSize > 0);
    assert(mRefCount > 0);
#endif // defined (DW_DEBUG_VERSION)
}


// Efficient memory management.  May be used at some point in the future.

#if 0
void* DwStringRep::operator new(size_t sz)
{
    void* rep;
    if (theirPoolCount > 0) {
        --theirPoolCount;
        rep = theirPool;
        theirPool = theirPool->mNext;
    }
    else {
        rep = new char[sz];
    }
    return rep;
}


void DwStringRep::operator delete(void* aRep, size_t)
{
    if (theirPoolCount < 200) {
        DwStringRep* rep = (DwStringRep*) aRep;
        ++theirPoolCount;
        rep->mNext = theirPool;
        theirPool = rep;
    }
    else {
        delete [] (char*) aRep;
    }
}
#endif


//--------------------------------------------------------------------------

const size_t DwString::kEmptyBufferSize = 4;
char DW_EXPORT DwString::sEmptyBuffer[]="    ";
DwStringRep* DW_EXPORT DwString::sEmptyRep = 0;

const size_t DwString::npos = (size_t) -1;

DwString::DwString()
{
    if (sEmptyRep == 0) {
        sEmptyBuffer[0] = 0;
        sEmptyRep = new DwStringRep(sEmptyBuffer, kEmptyBufferSize);
        assert(sEmptyRep != 0);
    }
    DBG_STMT(sEmptyRep->CheckInvariants())
    mRep = new_rep_reference(sEmptyRep);
    mStart = 0;
    mLength = 0;
}


DwString::DwString(const DwString& aStr, size_t aPos, size_t aLen)
{
    assert(aPos <= aStr.mLength);
    if (sEmptyRep == 0) {
        sEmptyBuffer[0] = 0;
        sEmptyRep = new DwStringRep(sEmptyBuffer, kEmptyBufferSize);
        assert(sEmptyRep != 0);
    }
    DBG_STMT(aStr.CheckInvariants())
    size_t pos = DW_MIN(aPos, aStr.mLength);
    size_t len = DW_MIN(aLen, aStr.mLength - pos);
    if (len > 0) {
        mRep = new_rep_reference(aStr.mRep);
        mStart = aStr.mStart + pos;
        mLength = len;
    }
    else /* if (len == 0) */ {
        mRep = new_rep_reference(sEmptyRep);
        mStart = 0;
        mLength = 0;
    }
}


DwString::DwString(const char* aBuf, size_t aLen)
{
    assert(aBuf != 0);
    assert(aLen != (size_t)-1);
    if (sEmptyRep == 0) {
        sEmptyBuffer[0] = 0;
        sEmptyRep = new DwStringRep(sEmptyBuffer, kEmptyBufferSize);
        assert(sEmptyRep != 0);
    }
    DBG_STMT(sEmptyRep->CheckInvariants())
    // Set valid values, in case an exception is thrown
    mRep = new_rep_reference(sEmptyRep);
    mStart = 0;
    mLength = 0;
    _replace(0, mLength, aBuf, aLen);
}


DwString::DwString(FILE* aFile , size_t aLen)
{
    assert(aFile != 0);
    assert(aLen != (size_t)-1);
    if (sEmptyRep == 0) {
        sEmptyBuffer[0] = 0;
        sEmptyRep = new DwStringRep(sEmptyBuffer, kEmptyBufferSize);
        assert(sEmptyRep != 0);
    }
    DBG_STMT(sEmptyRep->CheckInvariants())
    // Set valid values, in case an exception is thrown
    mRep = new DwStringRep(aFile, aLen);
    mStart = 0;
    mLength = aLen;
}


DwString::DwString(const char* aCstr)
{
    if (sEmptyRep == 0) {
        sEmptyBuffer[0] = 0;
        sEmptyRep = new DwStringRep(sEmptyBuffer, kEmptyBufferSize);
        assert(sEmptyRep != 0);
    }
    DBG_STMT(sEmptyRep->CheckInvariants())
    // Set valid values, in case an exception is thrown
    mRep = new_rep_reference(sEmptyRep);
    mStart = 0;
    mLength = 0;
    if ( aCstr ) {
        size_t len = (aCstr) ? strlen(aCstr) : 0;
        _replace(0, mLength, aCstr, len);
    }
}


DwString::DwString(size_t aLen, char aChar)
{
    assert(aLen != (size_t)-1);
    if (sEmptyRep == 0) {
        sEmptyBuffer[0] = 0;
        sEmptyRep = new DwStringRep(sEmptyBuffer, kEmptyBufferSize);
        assert(sEmptyRep != 0);
    }
    DBG_STMT(sEmptyRep->CheckInvariants())
    // Set valid values, in case an exception is thrown
    mRep = new_rep_reference(sEmptyRep);
    mStart = 0;
    mLength = 0;
    _replace(0, mLength, aLen, aChar);
}


DwString::DwString(char* aBuf, size_t aSize, size_t aStart, size_t aLen)
{
    assert(aBuf != 0);
    assert(aSize > 0);
    assert(aLen < aSize);
    assert(aStart < aSize - aLen);
    if (sEmptyRep == 0) {
        sEmptyBuffer[0] = 0;
        sEmptyRep = new DwStringRep(sEmptyBuffer, kEmptyBufferSize);
        assert(sEmptyRep != 0);
    }
    DBG_STMT(sEmptyRep->CheckInvariants())
    // Set valid values, in case an exception is thrown
    mRep = new_rep_reference(sEmptyRep);
    mStart = 0;
    mLength = 0;
    DwStringRep* rep = new DwStringRep(aBuf, aSize);
    assert(rep != 0);
    if (rep != 0) {
        mRep = rep;
        mStart = aStart;
        mLength = aLen;
    }
    else /* if (rep == 0) */ {
        delete [] aBuf;
    }
}


DwString::~DwString()
{
    assert(mRep != 0);
    delete_rep_safely(mRep);
    DEV_STMT(mRep = 0)
}


size_t DwString::max_size() const
{
    return ((size_t)-1) - 1;
}


void DwString::resize(size_t aLen, char aChar)
{
    // making string shorter?
    if (aLen < mLength) {
        mLength = aLen;
        if (mRep->mRefCount == 1) {
            mRep->mBuffer[mStart + aLen] = 0;
        }
    }
    // expanding string
    else if (aLen > mLength) {
        _replace(mLength, 0, aLen-mLength, aChar);
    }
}


void DwString::resize(size_t aLen)
{
    resize(aLen, 0);
}


void DwString::reserve(size_t aSize)
{
    if (mRep->mRefCount == 1 && aSize < mRep->mSize && mRep != sEmptyRep) {
        return;
    }
    size_t size = aSize + 1;
    char* newBuf = mem_alloc(&size);
    assert(newBuf != 0);
    if (newBuf != 0) {
        char* to = newBuf;
        const char* from = mRep->mBuffer + mStart;
        mem_copy(from, mLength, to);
        to[mLength] = 0;
        DwStringRep* rep= new DwStringRep(newBuf, size);
        assert(rep != 0);
        if (rep != 0) {
            delete_rep_safely(mRep);
            mRep = rep;
            mStart = 0;
        }
        else {
             mem_free(newBuf);
        }
    }
}


void DwString::clear()
{
    assign("");
}


DwString& DwString::append(const DwString& aStr)
{
    return append(aStr, 0, aStr.mLength);
}


DwString& DwString::append(const DwString& aStr, size_t aPos,
    size_t aLen)
{
    assert(aPos <= aStr.mLength);
    size_t pos = DW_MIN(aPos, aStr.mLength);
    size_t len = DW_MIN(aLen, aStr.mLength - pos);
    if (&aStr == this) {
        DwString temp(aStr);
        _replace(mLength, 0, &temp.mRep->mBuffer[temp.mStart+pos], len);
    }
    else {
        _replace(mLength, 0, &aStr.mRep->mBuffer[aStr.mStart+pos], len);
    }
    return *this;
}


DwString& DwString::append(const char* aBuf, size_t aLen)
{
    assert(aBuf != 0);
    if (aBuf != 0) {
        _replace(mLength, 0, aBuf, aLen);
    }
    return *this;
}


DwString& DwString::append(const char* aCstr)
{
    assert(aCstr != 0);
    size_t len = (aCstr) ? strlen(aCstr) : 0;
    _replace(mLength, 0, aCstr, len);
    return *this;
}


DwString& DwString::append(size_t aLen, char aChar)
{
    _replace(mLength, 0, aLen, aChar);
    return *this;
}


DwString& DwString::assign(const DwString& aStr)
{
    if (this != &aStr) {
        assign(aStr, 0, aStr.mLength);
    }
    return *this;
}


DwString& DwString::assign(const DwString& aStr, size_t aPos, size_t aLen)
{
    assert(aPos <= aStr.mLength);
    size_t pos = DW_MIN(aPos, aStr.mLength);
    size_t len = DW_MIN(aLen, aStr.mLength - pos);
    if (mRep == aStr.mRep) {
        mStart = aStr.mStart + pos;
        mLength = len;
    }
    else {
        delete_rep_safely(mRep);
        mRep = new_rep_reference(aStr.mRep);
        mStart = aStr.mStart + pos;
        mLength = len;
    }
    return *this;
}


DwString& DwString::assign(const char* aBuf, size_t aLen)
{
    assert(aBuf != 0);
    assert(aLen != (size_t)-1);
    _replace(0, mLength, aBuf, aLen);
    return *this;
}


DwString& DwString::assign(const char* aCstr)
{
    assert(aCstr != 0);
    size_t len = (aCstr) ? strlen(aCstr) : 0;
    _replace(0, mLength, aCstr, len);
    return *this;
}


DwString& DwString::assign(size_t aLen, char aChar)
{
    assert(aLen != (size_t)-1);
    _replace(0, mLength, aLen, aChar);
    return *this;
}


DwString& DwString::insert(size_t aPos, const DwString& aStr)
{
    return insert(aPos, aStr, 0, aStr.mLength);
}


DwString& DwString::insert(size_t aPos1, const DwString& aStr,
    size_t aPos2, size_t aLen2)
{
    assert(aPos1 <= mLength);
    assert(aPos2 <= aStr.mLength);
    size_t pos2 = DW_MIN(aPos2, aStr.mLength);
    size_t len2 = DW_MIN(aLen2, aStr.mLength - pos2);
    if (&aStr == this) {
        DwString temp(aStr);
        _replace(aPos1, 0, &temp.mRep->mBuffer[temp.mStart+pos2], len2);
    }
    else {
        _replace(aPos1, 0, &aStr.mRep->mBuffer[aStr.mStart+pos2], len2);
    }
    return *this;
}


DwString& DwString::insert(size_t aPos, const char* aBuf, size_t aLen)
{
    assert(aBuf != 0);
    _replace(aPos, 0, aBuf, aLen);
    return *this;
}


DwString& DwString::insert(size_t aPos, const char* aCstr)
{
    assert(aCstr != 0);
    size_t len = (aCstr) ? strlen(aCstr) : 0;
    _replace(aPos, 0, aCstr, len);
    return *this;
}


DwString& DwString::insert(size_t aPos, size_t aLen, char aChar)
{
    _replace(aPos, 0, aLen, aChar);
    return *this;
}


DwString& DwString::erase(size_t aPos, size_t aLen)
{
    assert(aPos <= mLength);
    size_t pos = DW_MIN(aPos, mLength);
    size_t len = DW_MIN(aLen, mLength - pos);
    _replace(pos, len, "", 0);
    return *this;
}


DwString& DwString::replace(size_t aPos1, size_t aLen1, const DwString& aStr)
{
    return replace(aPos1, aLen1, aStr, 0, aStr.mLength);
}


DwString& DwString::replace(size_t aPos1, size_t aLen1, const DwString& aStr,
    size_t aPos2, size_t aLen2)
{
    assert(aPos2 <= aStr.mLength);
    size_t pos2 = DW_MIN(aPos2, aStr.mLength);
    size_t len2 = DW_MIN(aLen2, aStr.mLength - pos2);
    if (&aStr == this) {
        DwString temp(aStr);
        _replace(aPos1, aLen1, &temp.mRep->mBuffer[temp.mStart+pos2], len2);
    }
    else {
        _replace(aPos1, aLen1, &aStr.mRep->mBuffer[aStr.mStart+pos2], len2);
    }
    return *this;
}


DwString& DwString::replace(size_t aPos1, size_t aLen1, const char* aBuf,
    size_t aLen2)
{
    _replace(aPos1, aLen1, aBuf, aLen2);
    return *this;
}


DwString& DwString::replace(size_t aPos1, size_t aLen1, const char* aCstr)
{
    size_t len2 = (aCstr) ? strlen(aCstr) : 0;
    _replace(aPos1, aLen1, aCstr, len2);
    return *this;
}


DwString& DwString::replace(size_t aPos1, size_t aLen1, size_t aLen2,
    char aChar)
{
    _replace(aPos1, aLen1, aLen2, aChar);
    return *this;
}


size_t DwString::copy(char* aBuf, size_t aLen, size_t aPos) const
{
    assert(aPos <= mLength);
    assert(aBuf != 0);
    size_t pos = DW_MIN(aPos, mLength);
    size_t len = DW_MIN(aLen, mLength - pos);
    char* to = aBuf;
    const char* from = mRep->mBuffer + mStart + pos;
    mem_copy(from, len, to);
    return len;
}


void DwString::swap(DwString& aStr)
{
    DwStringRep* rep = mRep;
    mRep = aStr.mRep;
    aStr.mRep = rep;
    size_t n = mStart;
    mStart = aStr.mStart;
    aStr.mStart = n;
    n = mLength;
    mLength = aStr.mLength;
    aStr.mLength = n;
}


size_t DwString::find(const DwString& aStr, size_t aPos) const
{
    return find(&aStr.mRep->mBuffer[aStr.mStart], aPos, aStr.mLength);
}


size_t DwString::find(const char* aBuf, size_t aPos, size_t aLen) const
{
    assert(aBuf != 0);
    if (aBuf == 0) return (size_t)-1;
    if (aLen > mLength) return (size_t)-1;
    if (aPos > mLength-aLen) return (size_t)-1;
    if (aLen == 0) return aPos;
    const char* buf = mRep->mBuffer + mStart;
    for (size_t i=aPos; i <= mLength-aLen; ++i) {
        size_t k = i;
        size_t j = 0;
        while (j < aLen && aBuf[j] == buf[k]) {
            ++j; ++k;
        }
        if (j == aLen) return i;
    }
    return (size_t)-1;
}


size_t DwString::find(const char* aCstr, size_t aPos) const
{
    assert(aCstr != 0);
    if (aCstr == 0) return (size_t)-1;
    size_t len = strlen(aCstr);
    return find(aCstr, aPos, len);
}


size_t DwString::find(char aChar, size_t aPos) const
{
    if (aPos >= mLength) return (size_t)-1;
    const char* buf = mRep->mBuffer + mStart;
    for (size_t i=aPos; i < mLength; ++i) {
        if (buf[i] == aChar) return i;
    }
    return (size_t)-1;
}


size_t DwString::rfind(const DwString& aStr, size_t aPos) const
{
    return rfind(&aStr.mRep->mBuffer[aStr.mStart], aPos, aStr.mLength);
}


size_t DwString::rfind(const char* aBuf, size_t aPos, size_t aLen) const
{
    assert(aBuf != 0);
    if (aBuf == 0) return (size_t)-1;
    if (aLen > mLength) return (size_t)-1;
    size_t pos = DW_MIN(aPos, mLength - aLen);
    if (aLen == 0) return pos;
    const char* buf = mRep->mBuffer + mStart;
    for (size_t i=0; i <= pos; ++i) {
        size_t k = pos - i;
        size_t j = 0;
        while (j < aLen && aBuf[j] == buf[k]) {
            ++j; ++k;
        }
        if (j == aLen) return pos - i;
    }
    return (size_t)-1;
}


size_t DwString::rfind(const char* aCstr, size_t aPos) const
{
    assert(aCstr != 0);
    size_t len = (aCstr) ? strlen(aCstr) : 0;
    return rfind(aCstr, aPos, len);
}


size_t DwString::rfind(char aChar, size_t aPos) const
{
    size_t pos = DW_MIN(aPos, mLength - 1);
    const char* buf = mRep->mBuffer + mStart;
    for (size_t i=0; i <= pos; ++i) {
        size_t k = pos - i;
        if (buf[k] == aChar) return k;
    }
    return (size_t)-1;
}


size_t DwString::find_first_of(const DwString& aStr, size_t aPos) const
{
    return find_first_of(&aStr.mRep->mBuffer[aStr.mStart], aPos, aStr.mLength);
}


size_t DwString::find_first_of(const char* aBuf, size_t aPos, size_t aLen) const
{
    assert(aBuf != 0);
    if (aBuf == 0) return (size_t)-1;
    if (aPos >= mLength) return (size_t)-1;
    if (aLen == 0) return aPos;
    char table[256];
    memset(table, 0, sizeof(table));
    for (size_t j=0; j < aLen; ++j) {
        table[aBuf[j]&0xff] = 1;
    }
    const char* buf = mRep->mBuffer + mStart;
    for (size_t i=aPos; i < mLength; ++i) {
        if (table[buf[i]&0xff]) return i;
    }
    return (size_t)-1;
}


size_t DwString::find_first_of(const char* aCstr, size_t aPos) const
{
    assert(aCstr != 0);
    if (aCstr == 0) return (size_t)-1;
    size_t len = strlen(aCstr);
    return find_first_of(aCstr, aPos, len);
}


size_t DwString::find_last_of(const DwString& aStr, size_t aPos) const
{
    return find_last_of(&aStr.mRep->mBuffer[aStr.mStart], aPos, aStr.mLength);
}


size_t DwString::find_last_of(const char* aBuf, size_t aPos, size_t aLen) const
{
    assert(aBuf != 0);
    if (aBuf == 0) return (size_t)-1;
    if (mLength == 0) return (size_t)-1;
    size_t pos = DW_MIN(aPos, mLength - 1);
    if (aLen == 0) return pos;
    char table[256];
    memset(table, 0, sizeof(table));
    for (size_t j=0; j < aLen; ++j) {
        table[aBuf[j]&0xff] = 1;
    }
    const char* buf = mRep->mBuffer + mStart;
    for (size_t k=0; k <= pos; ++k) {
        size_t i = pos - k;
        if (table[buf[i]&0xff]) return i;
    }
    return (size_t)-1;
}


size_t DwString::find_last_of(const char* aCstr, size_t aPos) const
{
    assert(aCstr != 0);
    if (aCstr == 0) return (size_t)-1;
    size_t len = strlen(aCstr);
    return find_last_of(aCstr, aPos, len);
}


size_t DwString::find_first_not_of(const DwString& aStr, size_t aPos) const
{
    return find_first_not_of(&aStr.mRep->mBuffer[aStr.mStart], aPos, aStr.mLength);
}


size_t DwString::find_first_not_of(const char* aBuf, size_t aPos, size_t aLen) const
{
    assert(aBuf != 0);
    if (aBuf == 0) return (size_t)-1;
    if (aPos >= mLength) return (size_t)-1;
    if (aLen == 0) return (size_t)-1;
    char table[256];
    memset(table, 1, sizeof(table));
    for (size_t j=0; j < aLen; ++j) {
        table[aBuf[j]&0xff] = 0;
    }
    const char* buf = mRep->mBuffer + mStart;
    for (size_t i=aPos; i < mLength; ++i) {
        if (table[buf[i]&0xff]) return i;
    }
    return (size_t)-1;
}


size_t DwString::find_first_not_of(const char* aCstr, size_t aPos) const
{
    assert(aCstr != 0);
    if (aCstr == 0) return (size_t)-1;
    size_t len = strlen(aCstr);
    return find_first_not_of(aCstr, aPos, len);
}


size_t DwString::find_last_not_of(const DwString& aStr, size_t aPos) const
{
    return find_last_not_of(&aStr.mRep->mBuffer[aStr.mStart], aPos, aStr.mLength);
}


size_t DwString::find_last_not_of(const char* aBuf, size_t aPos, size_t aLen) const
{
    assert(aBuf != 0);
    if (aBuf == 0) return (size_t)-1;
    if (mLength == 0) return (size_t)-1;
    size_t pos = DW_MIN(aPos, mLength - 1);
    if (aLen == 0) return (size_t)-1;
    char table[256];
    memset(table, 1, sizeof(table));
    for (size_t j=0; j < aLen; ++j) {
        table[aBuf[j]&0xff] = 0;
    }
    const char* buf = mRep->mBuffer + mStart;
    for (size_t k=0; k <= pos; ++k) {
        size_t i = pos - k;
        if (table[buf[i]&0xff]) return i;
    }
    return (size_t)-1;
}


size_t DwString::find_last_not_of(const char* aCstr, size_t aPos) const
{
    assert(aCstr != 0);
    if (aCstr == 0) return (size_t)-1;
    size_t len = strlen(aCstr);
    return find_last_not_of(aCstr, aPos, len);
}


DwString DwString::substr(size_t aPos, size_t aLen) const
{
    assert(aPos <= mLength);
    size_t pos = DW_MIN(aPos, mLength);
    size_t len = DW_MIN(aLen, mLength - pos);
    return DwString(*this, pos, len);
}


int DwString::compare(const DwString& aStr) const
{
    return compare(0, mLength, aStr, 0, aStr.mLength);
}


int DwString::compare(size_t aPos1, size_t aLen1, const DwString& aStr) const
{
    return compare(aPos1, aLen1, aStr, 0, aStr.mLength);
}


int DwString::compare(size_t aPos1, size_t aLen1, const DwString& aStr,
        size_t aPos2, size_t aLen2) const
{
    assert(aPos1 <= mLength);
    assert(aPos2 <= aStr.mLength);
    size_t pos1 = DW_MIN(aPos1, mLength);
    const char* buf1 = mRep->mBuffer + mStart + pos1;
    size_t len1 = DW_MIN(aLen1, mLength - pos1);
    size_t pos2 = DW_MIN(aPos2, aStr.mLength);
    const char* buf2 = aStr.mRep->mBuffer + aStr.mStart + pos2;
    size_t len2 = DW_MIN(aLen2, aStr.mLength - pos2);
    size_t len = DW_MIN(len1, len2);
    int r = strncmp(buf1, buf2, len);
    if (r == 0) {
        if (len1 < len2)
            r = -1;
        else if (len1 > len2) {
            r = 1;
        }
    }
    return r;
}


int DwString::compare(const char* aCstr) const
{
    assert(aCstr != 0);
    size_t len = (aCstr) ? strlen(aCstr) : 0;
    return compare(0, mLength, aCstr, len);
}


int DwString::compare(size_t aPos1, size_t aLen1, const char* aBuf,
    size_t aLen2) const
{
    assert(aBuf != 0);
    assert(aPos1 <= mLength);
    if (aBuf == 0) {
        return (aLen1 > 0) ? 1 : 0;
    }
    size_t pos1 = DW_MIN(aPos1, mLength);
    const char* buf1 = mRep->mBuffer + mStart + pos1;
    size_t len1 = DW_MIN(aLen1, mLength - pos1);
    const char* buf2 = aBuf;
    size_t len2 = aLen2;
    size_t len = DW_MIN(len1, len2);
    int r = strncmp(buf1, buf2, len);
    if (r == 0) {
        if (len1 < len2)
            r = -1;
        else if (len1 > len2) {
            r = 1;
        }
    }
    return r;
}


const char* DwString::ClassName() const
{
    return "DwString";
}


int DwString::ObjectId() const
{
    return (int) (long) this;
}


void DwString::ConvertToLowerCase()
{
    if (mRep->mRefCount > 1) {
        _copy();
    }
    char* buf = mRep->mBuffer + mStart;
    for (size_t i=0; i < mLength; ++i) {
        buf[i] = (char) tolower(buf[i]);
    }
}


void DwString::ConvertToUpperCase()
{
    if (mRep->mRefCount > 1) {
        _copy();
    }
    char* buf = mRep->mBuffer + mStart;
    for (size_t i=0; i < mLength; ++i) {
        buf[i] = (char) toupper(buf[i]);
    }
}


void DwString::Trim()
{
    const char* buf = mRep->mBuffer + mStart;
    size_t i = 0;
    while (mLength > 0) {
        if (isspace(buf[i])) {
            ++mStart;
            --mLength;
            ++i;
        }
        else {
            break;
        }
    }
    buf = mRep->mBuffer + mStart;
    i = mLength - 1;
    while (mLength > 0) {
        if (isspace(buf[i])) {
            --mLength;
            --i;
        }
        else {
            break;
        }
    }
    if (mLength == 0) {
        assign("");
    }
}


void DwString::WriteTo(std::ostream& aStrm) const
{
    const char* buf = mRep->mBuffer + mStart;
    for (size_t i=0; i < mLength; ++i) {
        aStrm << buf[i];
    }
}


void DwString::TakeBuffer(char* aBuf, size_t aSize, size_t aStart, size_t aLen)
{
    assert(aBuf != 0);
    DwStringRep* rep = new DwStringRep(aBuf, aSize);
    assert(rep != 0);
    if (rep) {
        delete_rep_safely(mRep);
        mRep = rep;
        mStart = aStart;
        mLength = aLen;
    }
}


void DwString::ReleaseBuffer(char** aBuf, size_t* aSize, size_t* aStart,
    size_t* aLen)
{
    assert(aBuf != 0);
    assert(aSize != 0);
    assert(aStart != 0);
    assert(aLen != 0);
    if (mRep->mRefCount == 1) {
        *aBuf = mRep->mBuffer;
        *aSize = mRep->mSize;
    }
    else {
        size_t size = mRep->mSize;
        char* buf = new char [size];
        assert(buf != 0);
        if (buf != 0) {
            mem_copy(mRep->mBuffer, size, buf);
            *aBuf = buf;
            *aSize = size;
        }
        else {
            // If not throwing an exception, recover as best we can
            *aBuf = 0;
            *aSize = 0;
            *aStart = mStart = 0;
            *aLen = mLength = 0;
            return;
        }
    }
    *aStart = mStart;
    *aLen = mLength;
    mRep = new_rep_reference(sEmptyRep);
    mStart = 0;
    mLength = 0;
}


void DwString::CopyTo(DwString* aStr) const
{
    assert(aStr != 0);
    if (!aStr) return;
    size_t len = mLength;
    size_t size = len + 1;
    char* buf = mem_alloc(&size);
    assert(buf != 0);
    if (buf != 0) {
        mem_copy(mRep->mBuffer+mStart, len, buf);
        buf[len] = 0;
        DwStringRep* rep = new DwStringRep(buf, size);
        assert(rep != 0);
        if (rep != 0) {
            aStr->mRep = rep;
            delete_rep_safely(aStr->mRep);
            aStr->mStart = 0;
            aStr->mLength = len;
        }
    }
}


void DwString::_copy()
{
    if (mRep->mRefCount > 1) {
        size_t size = mLength + 1;
        char* newBuf = mem_alloc(&size);
        assert(newBuf != 0);
        if (newBuf != 0) {
            char* to = newBuf;
            const char* from = mRep->mBuffer + mStart;
            mem_copy(from, mLength, to);
            to[mLength] = 0;
            DwStringRep* rep = new DwStringRep(newBuf, size);
            assert(rep != 0);
            if (rep != 0) {
                delete_rep_safely(mRep);
                mRep = rep;
                mStart = 0;
            }
            else /* if (rep == 0) */ {
                mem_free(newBuf);
                mLength = 0;
            }
        }
        else /* if (newBuf == 0) */ {
            mLength = 0;
        }
    }
}


void DwString::_replace(size_t aPos1, size_t aLen1, const char* aBuf, size_t aLen2)
{
    assert(aPos1 <= mLength);
    assert(aBuf != 0);
    size_t pos1 = DW_MIN(aPos1, mLength);
    size_t len1 = DW_MIN(aLen1, mLength - pos1);
    assert(mStart + mLength - len1 < ((size_t)-1) - aLen2);
    size_t len2 = DW_MIN(aLen2, ((size_t)-1) - (mStart + mLength - len1));
    size_t i;
    char* to;
    const char* from;
    size_t newLen = (mLength - len1) + len2;
    // Is new string empty?
    if (newLen == 0 || aBuf == 0) {
        if (mRep != sEmptyRep) {
            delete_rep_safely(mRep);
            mRep = new_rep_reference(sEmptyRep);
            mStart = 0;
            mLength = 0;
        }
    }
    // Is buffer shared?  Is buffer too small?
    else if (mRep->mRefCount > 1 || newLen >= mRep->mSize) {
        size_t size = newLen + 1;
        char* newBuf = mem_alloc(&size);
        assert(newBuf != 0);
        if (newBuf != 0) {
            to = newBuf;
            from = mRep->mBuffer + mStart;
            for (i=0; i < pos1; ++i) *to++ = *from++;
            from = aBuf;
            for (i=0; i < len2; ++i) *to++ = *from++;
            from = mRep->mBuffer + mStart + pos1 + len1;
            for (i=0; i < mLength - pos1 - len1; ++i) *to++ = *from++;
            *to = 0;
            DwStringRep* rep = new DwStringRep(newBuf, size);
            assert(rep != 0);
            if (rep != 0) {
                delete_rep_safely(mRep);
                mRep = rep;
                mStart = 0;
                mLength = newLen;
            }
        }
    }
    // Is the replacement smaller than the replaced?
    else if (len2 < len1) {
        to = mRep->mBuffer + mStart + pos1;
        from = aBuf;
        for (i=0; i < len2; ++i) *to++ = *from++;
        from = mRep->mBuffer + mStart + pos1 + len1;
        for (i=0; i < mLength - pos1 - len1; ++i) *to++ = *from++;
        *to = 0;
        mLength = newLen;
    }
    // Is there enough room at end of buffer?
    else if (mStart + newLen < mRep->mSize) {
        to = mRep->mBuffer + mStart + newLen;
        from = mRep->mBuffer + mStart + mLength - 1;
        *to-- = 0;
        for (i=0; i < mLength-pos1-len1; ++i) *to-- = *from--;
        from = aBuf + (len2 - 1);
        for (i=0; i < len2; ++i) *to-- = *from--;
        mLength = newLen;
    }
    // Is there enough room at beginning of buffer?
    else if (len2 - len1 <= mStart) {
        to = mRep->mBuffer + mStart - (len2 - len1);
        from = mRep->mBuffer + mStart;
        for (i=0; i < pos1; ++i) *to++ = *from++;
        from = aBuf;
        for (i=0; i < len2; ++i) *to++ = *from++;
        mStart -= len2 - len1;
        mLength = newLen;
    }
    // There's enough room, but we must move characters.
    else {
        to = mRep->mBuffer + newLen;
        from = mRep->mBuffer + mStart + mLength - 1;
        *to-- = 0;
        for (i=0; i < mLength-pos1-len1; ++i) *to-- = *from--;
        to = mRep->mBuffer;
        from = mRep->mBuffer + mStart;
        for (i=0; i < pos1; ++i) *to++ = *from++;
        from = aBuf;
        for (i=0; i < len2; ++i) *to++ = *from++;
        mStart = 0;
        mLength = newLen;
    }
}


void DwString::_replace(size_t aPos1, size_t aLen1, size_t aLen2, char aChar)
{
    assert(aPos1 <= mLength);
    size_t pos1 = DW_MIN(aPos1, mLength);
    size_t len1 = DW_MIN(aLen1, mLength - pos1);
    assert(mStart + mLength - len1 < ((size_t)-1) - aLen2);
    size_t len2 = DW_MIN(aLen2, ((size_t)-1) - (mStart + mLength - len1));
    size_t i;
    char* to;
    const char* from;
    size_t newLen = mLength - len1 + len2;
    // Is new string empty?
    if (newLen == 0) {
        if (mRep != sEmptyRep) {
            delete_rep_safely(mRep);
            mRep = new_rep_reference(sEmptyRep);
            mStart = 0;
            mLength = 0;
        }
    }
    // Is buffer shared?  Is buffer too small?
    else if (mRep->mRefCount > 1 || newLen >= mRep->mSize) {
        size_t size = newLen + 1;
        char* newBuf = mem_alloc(&size);
        assert(newBuf != 0);
        if (newBuf != 0) {
            to = newBuf;
            from = mRep->mBuffer + mStart;
            for (i=0; i < pos1; ++i) *to++ = *from++;
            for (i=0; i < len2; ++i) *to++ = aChar;
            from = mRep->mBuffer + mStart + pos1 + len1;
            for (i=0; i < mLength - pos1 - len1; ++i) *to++ = *from++;
            *to = 0;
            DwStringRep* rep = new DwStringRep(newBuf, size);
            assert(rep != 0);
            if (rep != 0) {
                delete_rep_safely(mRep);
                mRep = rep;
                mStart = 0;
                mLength = newLen;
            }
        }
    }
    // Is the replacement smaller than the replaced?
    else if (len2 < len1) {
        to = mRep->mBuffer + mStart + pos1;
        for (i=0; i < len2; ++i) *to++ = aChar;
        from = mRep->mBuffer + mStart + pos1 + len1;
        for (i=0; i < mLength - pos1 - len1; ++i) *to++ = *from++;
        *to = 0;
        mLength = newLen;
    }
    // Is there enough room at end of buffer?
    else if (mStart + newLen < mRep->mSize) {
        to = mRep->mBuffer + mStart + newLen;
        from = mRep->mBuffer + mStart + mLength - 1;
        *to-- = 0;
        for (i=0; i < mLength-pos1-len1; ++i) *to-- = *from--;
        for (i=0; i < len2; ++i) *to-- = aChar;
        mLength = newLen;
    }
    // Is there enough room at beginning of buffer?
    else if (len2 - len1 <= mStart) {
        to = mRep->mBuffer + mStart - (len2 - len1);
        from = mRep->mBuffer + mStart;
        for (i=0; i < pos1; ++i) *to++ = *from++;
        for (i=0; i < len2; ++i) *to++ = aChar;
        mStart -= len2 - len1;
        mLength = newLen;
    }
    // There's enough room, but we must move characters.
    else {
        to = mRep->mBuffer + newLen;
        from = mRep->mBuffer + mStart + mLength - 1;
        *to-- = 0;
        for (i=0; i < mLength-pos1-len1; ++i) *to-- = *from--;
        to = mRep->mBuffer;
        from = mRep->mBuffer + mStart;
        for (i=0; i < pos1; ++i) *to++ = *from++;
        for (i=0; i < len2; ++i) *to++ = aChar;
        mStart = 0;
        mLength = newLen;
    }
}


#if defined (DW_DEBUG_VERSION)
void DwString::PrintDebugInfo(std::ostream& aStrm) const
{
    aStrm <<
    "----------------- Debug info for DwString class ----------------\n";
    aStrm << "Id:               " << ClassName() << ", " << ObjectId() << "\n";
    aStrm << "Rep:              " << (void*) mRep << "\n";
    aStrm << "Buffer:           " << (void*) mRep->mBuffer << "\n";
    aStrm << "Buffer size:      " << mRep->mSize << "\n";
    aStrm << "Start:            " << mStart << "\n";
    aStrm << "Length:           " << mLength << "\n";
    aStrm << "Contents:         ";
    for (size_t i=0; i < mLength && i < 64; ++i) {
        aStrm << mRep->mBuffer[mStart+i];
    }
    aStrm << std::endl;
}
#else
void DwString::PrintDebugInfo(std::ostream& ) const {}
#endif // defined (DW_DEBUG_VERSION)


void DwString::CheckInvariants() const
{
#if defined (DW_DEBUG_VERSION)
    assert(mRep != 0);
    mRep->CheckInvariants();
#endif // defined (DW_DEBUG_VERSION)
}


DwString operator + (const DwString& aStr1, const DwString& aStr2)
{
    DwString str(aStr1);
    str.append(aStr2);
    return str;
}


DwString operator + (const char* aCstr, const DwString& aStr2)
{
    DwString str(aCstr);
    str.append(aStr2);
    return str;
}


DwString operator + (char aChar, const DwString& aStr2)
{
    DwString str(1, aChar);
    str.append(aStr2);
    return str;
}


DwString operator + (const DwString& aStr1, const char* aCstr)
{
    DwString str(aStr1);
    str.append(aCstr);
    return str;
}


DwString operator + (const DwString& aStr1, char aChar)
{
    DwString str(aStr1);
    str.append(1, aChar);
    return str;
}


DwBool operator == (const DwString& aStr1, const DwString& aStr2)
{
    const char* s1 = aStr1.data();
    size_t len1 = aStr1.length();
    const char* s2 = aStr2.data();
    size_t len2 = aStr2.length();
    int r = dw_strcmp(s1, len1, s2, len2);
    r = (r == 0) ? 1 : 0;
    return r;
}


DwBool operator == (const DwString& aStr1, const char* aCstr)
{
    assert(aCstr != 0);
    const char* s1 = aStr1.data();
    size_t len1 = aStr1.length();
    const char* s2 = aCstr;
    size_t len2 = (aCstr) ? strlen(aCstr) : 0;
    int r = dw_strcmp(s1, len1, s2, len2);
    r = (r == 0) ? 1 : 0;
    return r;
}


DwBool operator == (const char* aCstr, const DwString& aStr2)
{
    assert(aCstr != 0);
    const char* s1 = aCstr;
    size_t len1 = (aCstr) ? strlen(aCstr) : 0;
    const char* s2 = aStr2.data();
    size_t len2 = aStr2.length();
    int r = dw_strcmp(s1, len1, s2, len2);
    r = (r == 0) ? 1 : 0;
    return r;
}


DwBool operator != (const DwString& aStr1, const DwString& aStr2)
{
    const char* s1 = aStr1.data();
    size_t len1 = aStr1.length();
    const char* s2 = aStr2.data();
    size_t len2 = aStr2.length();
    int r = dw_strcmp(s1, len1, s2, len2);
    r = (r == 0) ? 0 : 1;
    return r;
}


DwBool operator != (const DwString& aStr1, const char* aCstr)
{
    assert(aCstr != 0);
    const char* s1 = aStr1.data();
    size_t len1 = aStr1.length();
    const char* s2 = aCstr;
    size_t len2 = (aCstr) ? strlen(aCstr) : 0;
    int r = dw_strcmp(s1, len1, s2, len2);
    r = (r == 0) ? 0 : 1;
    return r;
}


DwBool operator != (const char* aCstr, const DwString& aStr2)
{
    assert(aCstr != 0);
    const char* s1 = aCstr;
    size_t len1 = (aCstr) ? strlen(aCstr) : 0;
    const char* s2 = aStr2.data();
    size_t len2 = aStr2.length();
    int r = dw_strcmp(s1, len1, s2, len2);
    r = (r == 0) ? 0 : 1;
    return r;
}


DwBool operator < (const DwString& aStr1, const DwString& aStr2)
{
    const char* s1 = aStr1.data();
    size_t len1 = aStr1.length();
    const char* s2 = aStr2.data();
    size_t len2 = aStr2.length();
    int r = dw_strcmp(s1, len1, s2, len2);
    r = (r < 0) ? 1 : 0;
    return r;
}


DwBool operator < (const DwString& aStr1, const char* aCstr)
{
    assert(aCstr != 0);
    const char* s1 = aStr1.data();
    size_t len1 = aStr1.length();
    const char* s2 = aCstr;
    size_t len2 = (aCstr) ? strlen(aCstr) : 0;
    int r = dw_strcmp(s1, len1, s2, len2);
    r = (r < 0) ? 1 : 0;
    return r;
}


DwBool operator < (const char* aCstr, const DwString& aStr2)
{
    assert(aCstr != 0);
    const char* s1 = aCstr;
    size_t len1 = (aCstr) ? strlen(aCstr) : 0;
    const char* s2 = aStr2.data();
    size_t len2 = aStr2.length();
    int r = dw_strcmp(s1, len1, s2, len2);
    r = (r < 0) ? 1 : 0;
    return r;
}


DwBool operator > (const DwString& aStr1, const DwString& aStr2)
{
    const char* s1 = aStr1.data();
    size_t len1 = aStr1.length();
    const char* s2 = aStr2.data();
    size_t len2 = aStr2.length();
    int r = dw_strcmp(s1, len1, s2, len2);
    r = (r > 0) ? 1 : 0;
    return r;
}


DwBool operator > (const DwString& aStr1, const char* aCstr)
{
    assert(aCstr != 0);
    const char* s1 = aStr1.data();
    size_t len1 = aStr1.length();
    const char* s2 = aCstr;
    size_t len2 = (aCstr) ? strlen(aCstr) : 0;
    int r = dw_strcmp(s1, len1, s2, len2);
    r = (r > 0) ? 1 : 0;
    return r;
}


DwBool operator > (const char* aCstr, const DwString& aStr2)
{
    assert(aCstr != 0);
    const char* s1 = aCstr;
    size_t len1 = (aCstr) ? strlen(aCstr) : 0;
    const char* s2 = aStr2.data();
    size_t len2 = aStr2.length();
    int r = dw_strcmp(s1, len1, s2, len2);
    r = (r > 0) ? 1 : 0;
    return r;
}


DwBool operator <= (const DwString& aStr1, const DwString& aStr2)
{
    const char* s1 = aStr1.data();
    size_t len1 = aStr1.length();
    const char* s2 = aStr2.data();
    size_t len2 = aStr2.length();
    int r = dw_strcmp(s1, len1, s2, len2);
    r = (r <= 0) ? 1 : 0;
    return r;
}


DwBool operator <= (const DwString& aStr1, const char* aCstr)
{
    assert(aCstr != 0);
    const char* s1 = aStr1.data();
    size_t len1 = aStr1.length();
    const char* s2 = aCstr;
    size_t len2 = (aCstr) ? strlen(aCstr) : 0;
    int r = dw_strcmp(s1, len1, s2, len2);
    r = (r <= 0) ? 1 : 0;
    return r;
}


DwBool operator <= (const char* aCstr, const DwString& aStr2)
{
    assert(aCstr != 0);
    const char* s1 = aCstr;
    size_t len1 = (aCstr) ? strlen(aCstr) : 0;
    const char* s2 = aStr2.data();
    size_t len2 = aStr2.length();
    int r = dw_strcmp(s1, len1, s2, len2);
    r = (r <= 0) ? 1 : 0;
    return r;
}


DwBool operator >= (const DwString& aStr1, const DwString& aStr2)
{
    const char* s1 = aStr1.data();
    size_t len1 = aStr1.length();
    const char* s2 = aStr2.data();
    size_t len2 = aStr2.length();
    int r = dw_strcmp(s1, len1, s2, len2);
    r = (r >= 0) ? 1 : 0;
    return r;
}


DwBool operator >= (const DwString& aStr1, const char* aCstr)
{
    assert(aCstr != 0);
    const char* s1 = aStr1.data();
    size_t len1 = aStr1.length();
    const char* s2 = aCstr;
    size_t len2 = (aCstr) ? strlen(aCstr) : 0;
    int r = dw_strcmp(s1, len1, s2, len2);
    r = (r >= 0) ? 1 : 0;
    return r;
}


DwBool operator >= (const char* aCstr, const DwString& aStr2)
{
    assert(aCstr != 0);
    const char* s1 = aCstr;
    size_t len1 = (aCstr) ? strlen(aCstr) : 0;
    const char* s2 = aStr2.data();
    size_t len2 = aStr2.length();
    int r = dw_strcmp(s1, len1, s2, len2);
    r = (r >= 0) ? 1 : 0;
    return r;
}


std::ostream& operator << (std::ostream& aOstrm, const DwString& aStr)
{
    const char* buf = aStr.data();
    for (size_t i=0; i < aStr.length(); ++i) {
        aOstrm << buf[i];
    }
    return aOstrm;
}


std::istream& getline(std::istream& aIstrm, DwString& aStr, char aDelim)
{
    aStr.clear();
    char ch;
    while (aIstrm.get(ch)) {
        if (ch == aDelim) break;
        if (aStr.length() < aStr.max_size()) {
            aStr.append(1, ch);
        }
    }
    return aIstrm;
}


std::istream& getline(std::istream& aIstrm, DwString& aStr)
{
    return getline(aIstrm, aStr, '\n');
}

#endif // !defined(DW_USE_ANSI_STRING)


int DwStrcasecmp(const DwString& aStr1, const DwString& aStr2)
{
    const char* s1 = aStr1.data();
    size_t len1 = aStr1.length();
    const char* s2 = aStr2.data();
    size_t len2 = aStr2.length();
    return dw_strasciicasecmp(s1, len1, s2, len2);
}


int DwStrcasecmp(const DwString& aStr, const char* aCstr)
{
    assert(aCstr != 0);
    const char* s1 = aStr.data();
    size_t len1 = aStr.length();
    const char* s2 = aCstr;
    size_t len2 = (aCstr) ? strlen(aCstr) : 0;
    return dw_strasciicasecmp(s1, len1, s2, len2);
}


int DwStrcasecmp(const char* aCstr, const DwString& aStr)
{
    assert(aCstr != 0);
    const char* s1 = aCstr;
    size_t len1 =  (aCstr) ? strlen(aCstr) : 0;
    const char* s2 = aStr.data();
    size_t len2 = aStr.length();
    return dw_strasciicasecmp(s1, len1, s2, len2);
}


int DwStrncasecmp(const DwString& aStr1, const DwString& aStr2, size_t n)
{
    const char* s1 = aStr1.data();
    size_t len1 = aStr1.length();
    len1 = DW_MIN(len1, n);
    const char* s2 = aStr2.data();
    size_t len2 = aStr2.length();
    len2 = DW_MIN(len2, n);
    return dw_strasciicasecmp(s1, len1, s2, len2);
}


int DwStrncasecmp(const DwString& aStr, const char* aCstr, size_t n)
{
    assert(aCstr != 0);
    const char* s1 = aStr.data();
    size_t len1 = aStr.length();
    len1 = DW_MIN(len1, n);
    const char* s2 = aCstr;
    size_t len2 = (aCstr) ? strlen(aCstr) : 0;
    len2 = DW_MIN(len2, n);
    return dw_strasciicasecmp(s1, len1, s2, len2);
}


int DwStrncasecmp(const char* aCstr, const DwString& aStr, size_t n)
{
    assert(aCstr != 0);
    const char* s1 = aCstr;
    size_t len1 = (aCstr) ? strlen(aCstr) : 0;
    len1 = DW_MIN(len1, n);
    const char* s2 = aStr.data();
    size_t len2 = aStr.length();
    len2 = DW_MIN(len2, n);
    return dw_strasciicasecmp(s1, len1, s2, len2);
}


int DwStrcmp(const DwString& aStr1, const DwString& aStr2)
{
    const char* s1 = aStr1.data();
    size_t len1 = aStr1.length();
    const char* s2 = aStr2.data();
    size_t len2 = aStr2.length();
    return dw_strcmp(s1, len1, s2, len2);
}


int DwStrcmp(const DwString& aStr, const char* aCstr)
{
    assert(aCstr != 0);
    const char* s1 = aStr.data();
    size_t len1 = aStr.length();
    const char* s2 = aCstr;
    size_t len2 = (aCstr) ? strlen(aCstr) : 0;
    return dw_strcmp(s1, len1, s2, len2);
}


int DwStrcmp(const char* aCstr, const DwString& aStr)
{
    assert(aCstr != 0);
    const char* s1 = aCstr;
    size_t len1 = (aCstr) ? strlen(aCstr) : 0;
    const char* s2 = aStr.data();
    size_t len2 = aStr.length();
    return dw_strcmp(s1, len1, s2, len2);
}


int DwStrncmp(const DwString& aStr1, const DwString& aStr2, size_t n)
{
    const char* s1 = aStr1.data();
    size_t len1 = aStr1.length();
    len1 = DW_MIN(len1, n);
    const char* s2 = aStr2.data();
    size_t len2 = aStr2.length();
    len2 = DW_MIN(len2, n);
    return dw_strcmp(s1, len1, s2, len2);
}


int DwStrncmp(const DwString& aStr, const char* aCstr, size_t n)
{
    assert(aCstr != 0);
    const char* s1 = aStr.data();
    size_t len1 = aStr.length();
    len1 = DW_MIN(len1, n);
    const char* s2 = aCstr;
    size_t len2 = (aCstr) ? strlen(aCstr) : 0;
    len2 = DW_MIN(len2, n);
    return dw_strcmp(s1, len1, s2, len2);
}


int DwStrncmp(const char* aCstr, const DwString& aStr, size_t n)
{
    assert(aCstr != 0);
    const char* s1 = aCstr;
    size_t len1 = (aCstr) ? strlen(aCstr) : 0;
    len1 = DW_MIN(len1, n);
    const char* s2 = aStr.data();
    size_t len2 = aStr.length();
    len2 = DW_MIN(len2, n);
    return dw_strcmp(s1, len1, s2, len2);
}


void DwStrcpy(DwString& aStrDest, const DwString& aStrSrc)
{
    aStrDest.assign(aStrSrc);
}


void DwStrcpy(DwString& aStrDest, const char* aCstrSrc)
{
    aStrDest.assign(aCstrSrc);
}


void DwStrcpy(char* aCstrDest, const DwString& aStrSrc)
{
    assert(aCstrDest != 0);
    const char* buf = aStrSrc.data();
    size_t len = aStrSrc.length();
    mem_copy(buf, len, aCstrDest);
    aCstrDest[len] = 0;
}


void DwStrncpy(DwString& aStrDest, const DwString& aStrSrc, size_t n)
{
    aStrDest.assign(aStrSrc, 0, n);
}


void DwStrncpy(DwString& aStrDest, const char* aCstrSrc, size_t n)
{
    aStrDest.assign(aCstrSrc, 0, n);
}


void DwStrncpy(char* aCstrDest, const DwString& aStrSrc, size_t n)
{
    assert(aCstrDest != 0);
    const char* buf = aStrSrc.data();
    size_t len = aStrSrc.length();
    len = DW_MIN(len, n);
    mem_copy(buf, len, aCstrDest);
    for (size_t i=len; i < n; ++i) {
        aCstrDest[i] = 0;
    }
}


char* DwStrdup(const DwString& aStr)
{
    size_t len = aStr.length();
    char* buf = new char[len+1];
    assert(buf != 0);
    if (buf != 0) {
        DwStrncpy(buf, aStr, len);
        buf[len] = 0;
    }
    return buf;
}
