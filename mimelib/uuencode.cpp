//=============================================================================
// File:       uuencode.cpp
// Contents:   Definitions for DwUuencode
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
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <mimelib/uuencode.h>
#include <config.h>

#if defined(DW_TESTING_UUENCODE)
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <fstream>
#endif


DwUuencode::DwUuencode()
{
    memset(mFileName, 0, sizeof(mFileName));
    mMode = 0644;
}


DwUuencode::~DwUuencode()
{
}


void DwUuencode::SetFileName(const char* aName)
{
    size_t n = sizeof(mFileName);
    strlcpy(mFileName, aName, n);
    mFileName[n-1] = 0; // Superfluous
}


const char* DwUuencode::FileName() const
{
	return mFileName;
}


void DwUuencode::SetFileMode(DwUint16 aMode)
{
	mMode = aMode;
}


DwUint16 DwUuencode::FileMode() const
{
	return mMode;
}


void DwUuencode::SetBinaryChars(const DwString& aStr)
{
	mBinaryChars = aStr;
}


const DwString& DwUuencode::BinaryChars() const
{
	return mBinaryChars;
}


void DwUuencode::SetAsciiChars(const DwString& aStr)
{
	mAsciiChars = aStr;
}


const DwString& DwUuencode::AsciiChars() const
{
	return mAsciiChars;
}


#define ENC(c) ((char) ((c) ? ((c) & 0x3F) + ' ' : 96 ))


void DwUuencode::Encode()
{
	// Get input buffer

	size_t binLen = mBinaryChars.length();
	const char* binBuf = mBinaryChars.data();
	size_t binPos = 0;

	// Allocate buffer for binary chars

	size_t ascSize = (binLen+2)/3*4
		+ ((binLen+44)/45+1)*(strlen(DW_EOL)+1)
		+ strlen(mFileName)
		+ 13 + 2*strlen(DW_EOL)
		+ 100;
	DwString ascStr(ascSize, (char)0);
	char* ascBuf = (char*)ascStr.data();
	size_t ascPos = 0;

	// Write the "begin" line

	snprintf(ascBuf, ascSize, "begin %o %s" DW_EOL, mMode, mFileName);
	ascPos = strlen(ascBuf);

	// Encode the binary chars

	while (ascPos < ascSize) {
		int numBinChars = binLen - binPos;
		numBinChars = (numBinChars <= 45) ? numBinChars : 45;
        ascBuf[ascPos++] = ENC(numBinChars);
		if (numBinChars == 0) {
			strcpy(&ascBuf[ascPos], DW_EOL);
			ascPos += strlen(DW_EOL);
			break;
		}
		int bin, asc;
		int binCharsDone = 0;
		while (binCharsDone <= numBinChars - 3) {

			bin = binBuf[binPos++];
			asc = (bin & 0xFC) >> 2;
			ascBuf[ascPos++] = ENC(asc);

			asc = (bin & 0x03) << 4;
			bin = binBuf[binPos++];
			asc |= (bin & 0xF0) >> 4;
			ascBuf[ascPos++] = ENC(asc);

			asc = (bin & 0x0F) << 2;
			bin = binBuf[binPos++];
			asc |= (bin & 0xC0) >> 6;
			ascBuf[ascPos++] = ENC(asc);

			asc = bin & 0x3F;
			ascBuf[ascPos++] = ENC(asc);

			binCharsDone += 3;
		}

		if (binCharsDone < numBinChars) {
			int binCharsLeft = numBinChars - binCharsDone;
			switch (binCharsLeft) {

			case 1:
				bin = binBuf[binPos++];
				asc = (bin & 0xFC) >> 2;
				ascBuf[ascPos++] = ENC(asc);

				asc = (bin & 0x03) << 4;
				ascBuf[ascPos++] = ENC(asc);

				ascBuf[ascPos++] = 96;
				ascBuf[ascPos++] = 96;
				break;

			case 2:
				bin = binBuf[binPos++];
				asc = (bin & 0xFC) >> 2;
				ascBuf[ascPos++] = ENC(asc);

				asc = (bin & 0x03) << 4;
				bin = binBuf[binPos++];
				asc |= (bin & 0xF0) >> 4;
				ascBuf[ascPos++] = ENC(asc);

				asc = (bin & 0x0F) << 2;
				ascBuf[ascPos++] = ENC(asc);

				ascBuf[ascPos++] = 96;
				break;

			default:
				break;
			}
		}

		strcpy(&ascBuf[ascPos], DW_EOL);
		ascPos += strlen(DW_EOL);
	}

	// Write the "end" line

	strcpy(&ascBuf[ascPos], "end" DW_EOL);
	ascPos += 3 + strlen(DW_EOL);
	ascBuf[ascPos] = 0;

	mAsciiChars.assign(ascStr, 0, ascPos);
}


#define DEC(c)  (((c) - ' ') & 0x3F)


int DwUuencode::Decode()
{
    int retVal = -1;

	// Get input buffer

    size_t ascLen = mAsciiChars.length();
    const char* ascBuf = mAsciiChars.data();
    size_t ascPos = 0;

    // Allocate destination buffer

    size_t binSize = (ascLen+3)/4*3;
    mBinaryChars.reserve(binSize);

	// Look for "begin " at beginning of buffer

	if (ascPos + 6 <= ascLen &&
	    strncmp(&ascBuf[ascPos], "begin ", 6) == 0) {

		ascPos += 6;
	}
	else {

		// Find "\nbegin " or "\rbegin "

		while (ascPos < ascLen) {
			int ch = ascBuf[ascPos++] & 0xff;
			switch (ch) {
			case '\n':
			case '\r':
				if (ascPos + 6 <= ascLen &&
					strncmp(&ascBuf[ascPos], "begin ", 6) == 0) {

					ascPos += 6;
					goto LOOP_EXIT_1;
				}
				break;
			default:
				break;
			}
		}
	}
LOOP_EXIT_1:

    // Get mode

    mMode = 0;
    while (ascPos < ascLen && isdigit(ascBuf[ascPos])) {
        mMode <<= 3;
        mMode += (DwUint16) (ascBuf[ascPos++] - '0');
    }

    // Get file name

    while (ascPos < ascLen &&
        (ascBuf[ascPos] == ' ' || ascBuf[ascPos] == '\t')) {

        ++ascPos;
    }
    size_t p1 = 0;
    while (ascPos < ascLen && p1 < sizeof(mFileName)-1 &&
        !isspace(ascBuf[ascPos])) {

        mFileName[p1++] = ascBuf[ascPos++];
    }
    mFileName[p1] = 0;

    // Advance to beginning of next line

    while (ascPos < ascLen) {
        int ch = ascBuf[ascPos++];
        switch (ch) {
        case '\n':
            goto LOOP_EXIT_2;
        case '\r':
            if (ascPos < ascLen && ascBuf[ascPos] == '\n') {
                ++ascPos;
            }
            goto LOOP_EXIT_2;
        default:
            break;
        }
    }
LOOP_EXIT_2:

    // Decode chars

    while (ascPos < ascLen) {
        int asc, bin;

        // Get number of binary chars in this line

        asc = ascBuf[ascPos++] & 0xff;
        size_t numBinChars = DEC(asc);
        if (numBinChars == 0) {
            break;
        }

        // Decode this line

        size_t binCharsEaten = 0;
        while (binCharsEaten <= numBinChars - 3 && ascPos <= ascLen - 4) {

            asc = ascBuf[ascPos++] & 0xff;
            bin = (DEC(asc) & 0x3F) << 2;
            asc = ascBuf[ascPos++] & 0xff;
            bin |= (DEC(asc) & 0x30) >> 4;
            mBinaryChars.append((size_t) 1, (char) bin);

            bin = (DEC(asc) & 0x0F) << 4;
            asc = ascBuf[ascPos++] & 0xff;
            bin |= (DEC(asc) & 0x3C) >> 2;
            mBinaryChars.append((size_t) 1, (char) bin);

            bin = (DEC(asc) & 0x03) << 6;
            asc = ascBuf[ascPos++] & 0xff;
            bin |= (DEC(asc) & 0x3F);
            mBinaryChars.append((size_t) 1, (char) bin);

            binCharsEaten += 3;
        }

    	// Special case if number of binary chars is not divisible by 3

    	if (binCharsEaten < numBinChars) {
    	    int binCharsLeft = numBinChars - binCharsEaten;
    	    switch (binCharsLeft) {
    	    case 2:
    	        if (ascPos >= ascLen)
    	            break;
    	        asc = ascBuf[ascPos++] & 0xff;
    	        bin = (DEC(asc) & 0x3F) << 2;
    	        if (ascPos >= ascLen)
    	            break;
    	        asc = ascBuf[ascPos++] & 0xff;
    	        bin |= (DEC(asc) & 0x30) >> 4;
                mBinaryChars.append((size_t) 1, (char) bin);

    	        bin = (DEC(asc) & 0x0F) << 4;
    	        if (ascPos >= ascLen)
    	            break;
    	        asc = ascBuf[ascPos++] & 0xff;
    	        bin |= (DEC(asc) & 0x3C) >> 2;
                mBinaryChars.append((size_t) 1, (char) bin);
    	        break;
    	    case 1:
    	        if (ascPos >= ascLen)
    	            break;
    	        asc = ascBuf[ascPos++] & 0xff;
    	        bin = (DEC(asc) & 0x3F) << 2;
    	        if (ascPos >= ascLen)
    	            break;
    	        asc = ascBuf[ascPos++] & 0xff;
    	        bin |= (DEC(asc) & 0x30) >> 4;
                mBinaryChars.append((size_t) 1, (char) bin);
    	        break;
    	    default:
    	        break;
    	    }
    	}

        // Advance to beginning of next line

        while (ascPos < ascLen) {
            int ch = ascBuf[ascPos++];
            switch (ch) {
            case '\n':
                goto LOOP_EXIT_3;
            case '\r':
                if (ascPos < ascLen &&
                    ascBuf[ascPos] == '\n') {

                    ++ascPos;
                }
                goto LOOP_EXIT_3;
            default:
                break;
            }
        }
LOOP_EXIT_3:
    	;
    }
    while (ascPos < ascLen) {
        int ch = ascBuf[ascPos++];
        switch (ch) {
        case '\n':
            goto LOOP_EXIT_4;
        case '\r':
            if (ascPos < ascLen &&
                ascBuf[ascPos] == '\n') {

                ++ascPos;
            }
            goto LOOP_EXIT_4;
        default:
            break;
        }
    }
LOOP_EXIT_4:
    if (ascPos + 3 <= ascLen &&
        strncmp(&ascBuf[ascPos], "end", 3) == 0) {

        retVal = 0;
    }
    return retVal;
}


#if defined(DW_TESTING_UUENCODE)

// Test harness for DwUudecode

int main(int argc, char** argv)
{
	srand(time(0));
	DwString binStr;
	binStr.reserve(5000);
	char ch;
	int i;
	for (i=0; i < 4000; ++i) {
		ch = rand()/(double)RAND_MAX*256;
		binStr += (char) ch;
	}
	for ( ; i < 4100; ++i) {
		binStr += (char) 0;
	}
	DwUuencode uu;
	uu.SetFileName("Testfile.dat");
	uu.SetMode(0600);
	uu.SetBinaryChars(binStr);
	uu.Encode();
	DwString asciiStr = uu.AsciiChars();
	// std::ofstream out("test.out", ios::out|ios::binary);
	std::ofstream out("test.out", ios::out);
	out << asciiStr;

	DwUuencode uu1;
	uu1.SetAsciiChars(uu.AsciiChars());
	uu1.Decode();

	size_t n = uu1.BinaryChars().length();
	const char* b1 = binStr.data();
	const char* b2 = uu1.BinaryChars().data();
	int bad = 0;
	for (i=0; i < n; ++i) {
		if (b1[i] != b2[i]) {
			cout << "Binary chars not equal at position " << i << "\n";
			bad = 1;
			break;
		}
	}
	if (! bad) {
		cout << "A-okay\n";
	}
	return 0;
}

#endif
