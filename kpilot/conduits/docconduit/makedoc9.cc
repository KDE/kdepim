// based on: MakeDoc, version 2
// I only took the tBuf class from there and adapted it.
//
// Compresses text files into a format that is ready to export to a Pilot
// and work with Rick Bram's PilotDOC reader.
// Copyright (C) Reinhold Kainhofer, 2002
// Copyrigth (C) Pat Beirne, 2000
//
// Original file (makedoc9.cpp) copyright by:
// Copyright (C) Pat Beirne, 2000.
// Distributable under the GNU General Public License Version 2 or later.
//
// ver 0.6 enforce 31 char limit on database names
// ver 0.7 change header and record0 to structs
// ver 2.0 added category control on the command line
//              changed extensions from .prc to .pdb

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge,
** MA 02139, USA.
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>


#include "makedoc9.h"



//
// Issue()
//
// action: handle the details of writing a single
//              character to the compressed stream
//
unsigned
 tBuf::Issue(byte src, int &bSpace)
{
	unsigned int iDest = len;
	byte *dest = buf;

	// TODO: which of the if parts should really be included???
#if 0
	// modified version of issue
	// just issue the char
	if (src >= 0x80 || src <= 8)
		dest[iDest++] = 1;
	dest[iDest++] = src;

#else
	// if there is an outstanding space char, see if
	// we can squeeze it in with an ASCII char
	if (bSpace)
	{
		if (src >= 0x40 && src <= 0x7F)
			dest[iDest++] = src ^ 0x80;
		else
		{
			// couldn't squeeze it in, so issue the space char by itself
			// most chars go out simple, except the range 1...8,0x80...0xFF
			dest[iDest++] = ' ';
			if (src < 0x80 && (src == 0 || src > 8))
				dest[iDest++] = src;
			else
				dest[iDest++] = 1, dest[iDest++] = src;
		}
		// knock down the space flag
		bSpace = 0;
	}
	else
	{
		// check for a space char
		if (src == ' ')
			bSpace = 1;
		else
		{
			if (src < 0x80 && (src == 0 || src > 8))
				dest[iDest++] = src;
			else
				dest[iDest++] = 1, dest[iDest++] = src;

		}
	}
#endif
	len = iDest;
	return iDest;
}

//
// Compress
//
// params:      none
//
// action:      takes the given buffer,
//                                      and compresses
//                                      the original data down into a second buffer
//
// comment:     This version make heavy use of walking pointers.
//
unsigned tBuf::Compress()
{
	if (!buf)
		return 0;
	if (isCompressed) {
//		cout<<"Buffer is already compressed!"<<endl;
		return len;
//	} else {
//		cout<<" Compressing buffer!!!"<<endl;
	}

	unsigned int i;

	// run through the input buffer
	byte *pBuffer;					  // points to the input buffer
	byte *pHit;						  // points to a walking test hit; works upwards on successive matches
	byte *pPrevHit;				  // previous value of pHit; also, start of next test
	byte *pTestHead;				  // current test string
	byte *pTestTail;				  // current walking pointer; one past the current test buffer
	byte *pEnd;						  // 1 past the end of the input buffer

	pHit = pPrevHit = pTestHead = pBuffer = buf;
	pTestTail = pTestHead + 1;
	pEnd = buf + len;				  // should point to a 0!

	// make a dest buffer and reassign the local buffer
	buf = new byte[6000];
	len = 0;							  // used to walk through the output buffer

	// loop, absorbing one more char from the input buffer on each pass
	for (; pTestHead != pEnd; pTestTail++)
	{
		// if we already have 10 char match, don't bother scanning again for the 11th (wasted time)
		if (pTestTail - pTestHead != (1 << COUNT_BITS) + 3)
		{
			// scan in the previous data for a match
			// terminate the test string (and the matcher string, as well!) in a 0
			byte tmp = *pTestTail;

			*pTestTail = 0;
			pHit = (byte *) strstr((const char *) pPrevHit,
				(const char *) pTestHead);
			*pTestTail = tmp;		  // restore the char
		}

		// on a mismatch or end of buffer, issued codes
		if (pHit == pTestHead
			|| pTestTail - pTestHead > (1 << COUNT_BITS) + 2
			|| pTestTail == pEnd)
		{
			// issue the codes
			// first, check for short runs
			if (pTestTail - pTestHead < 4)
			{
				if (pTestHead[0] > 0x7F || pTestHead[0] <= 8)
					buf[len++] = 1;
				buf[len++] = pTestHead[0];
				pTestHead++;
			}
			// for longer runs, issue a run-code
			else
			{
				unsigned int dist = pTestHead - pPrevHit;
				unsigned int compound =
					(dist << COUNT_BITS) + pTestTail - pTestHead - 4;

//if (dist>=(1<<DISP_BITS)) printf("\n!! error dist overflow");
//if (pTestTail-pTestHead-4>7) printf("\n!! error len overflow");

				buf[len++] = 0x80 + (compound >> 8);
				buf[len++] = compound & 0xFF;
//printf("\nissuing code for sequence len %d <%c%c%c>",pTestTail-pTestHead-1,pTestHead[0],pTestHead[1],pTestHead[2]);
//printf("\n          <%x%x>",pOut[-2],pOut[-1]);
				// and start again
				pTestHead = pTestTail - 1;
			}
			// start the search again
			pPrevHit = pBuffer;
			// within range
			if (pTestHead - pPrevHit > ((1 << DISP_BITS) - 1))
				pPrevHit = pTestHead - ((1 << DISP_BITS) - 1);
		}
		// got a match
		else
		{
			pPrevHit = pHit;
		}
		// when we get to the end of the buffer, don't inc past the end
		// this forces the residue chars out one at a time
		if (pTestTail == pEnd)
			pTestTail--;
	}


	// final scan to merge consecutive high chars together
	// and merge space chars
	unsigned int k;

	for (i = k = 0; i < len; i++, k++)
	{
		buf[k] = buf[i];
		// skip the run-length codes
		if (buf[k] >= 0x80 && buf[k] < 0xC0)
			buf[++k] = buf[++i];
		// if we hit a high char marker, look ahead for another
		// and merge multiples together
		else if (buf[k] == 1)
		{
			buf[k + 1] = buf[i + 1];
			while (i + 2 < len && buf[i + 2] == 1 && buf[k] < 8)
			{
				buf[k]++;
				buf[k + buf[k]] = buf[i + 3];
				i += 2;
			}
			k += buf[k];
			i++;
		}
		else if (buf[k] == ' ' && i < len - 1 && buf[i + 1] <= 0x7F
			&& buf[i + 1] >= 0x40)
			buf[k] = 0x80 | buf[++i];
	}

	// delete original buffer
	delete[]pBuffer;
	len = k;

	isCompressed = true;
	return k;
}

/*
	Decompress

	params:	none

	action: make a new buffer
					run through the source data
					check the 4 cases:
						0,9...7F represent self
						1...8		escape n chars
						80...bf reference earlier run
						c0...ff	space+ASCII

*/
unsigned tBuf::Decompress()
{
	if (!buf)
		return 0;
	if (!isCompressed) {
//		cout<<"Buffer already uncompressed. Doing nothing"<<endl;
		return len;
//	} else {
//		cout<<"Decompressing buffer"<<endl;
	}

	// we "know" that all decompresses fit within 4096, right?
	byte *pOut = new byte[6000];
	byte *in_buf = buf;
	byte *out_buf = pOut;

	unsigned int i, j;

	for (j = i = 0; j < len;)
	{
		unsigned int c;

		// take a char from the input buffer
		c = in_buf[j++];

		// separate the char into zones: 0, 1...8, 9...0x7F, 0x80...0xBF, 0xC0...0xFF

		// codes 1...8 mean copy that many bytes; for accented chars & binary
		if (c > 0 && c < 9)
			while (c--)
				out_buf[i++] = in_buf[j++];

		// codes 0, 9...0x7F represent themselves
		else if (c < 0x80)
			out_buf[i++] = c;

		// codes 0xC0...0xFF represent "space + ascii char"
		else if (c >= 0xC0)
			out_buf[i++] = ' ', out_buf[i++] = c ^ 0x80;

		// codes 0x80...0xBf represent sequences
		else
		{
			int m, n;

			c <<= 8;
			c += in_buf[j++];
			m = (c & 0x3FFF) >> COUNT_BITS;
			n = c & ((1 << COUNT_BITS) - 1);
			n += 3;
			while (n--)
			{
				out_buf[i] = out_buf[i - m];
				i++;
			}
		}
	}
	out_buf[i++]='\0';
	out_buf[i++]='\0';
	delete[]buf;
	buf = pOut;
	len = i;

	isCompressed = false;
	return i;
}

unsigned tBuf::DuplicateCR()
{
	if (!buf)
		return 0;
	byte *pBuf = new byte[2 * len];

	unsigned int k, j;

	for (j = k = 0; j < len; j++, k++)
	{
		pBuf[k] = buf[j];
		if (pBuf[k] == 0x0A)
			pBuf[k++] = 0x0D, pBuf[k] = 0x0A;
	}
	delete[]buf;
	buf = pBuf;
	len = k;
	return k;
}



// this nasty little beast removes really low ASCII and 0's
// and handles the CR problem
//
// if a cr appears before a lf, then remove the cr
// if a cr appears in isolation, change to a lf
unsigned tBuf::RemoveBinary()
{
	if (!buf)
		return 0;
	byte *in_buf = buf;
	byte *out_buf = new byte[len];

	unsigned int k, j;

	for (j = k = 0; j < len; j++, k++)
	{
		// copy each byte
		out_buf[k] = in_buf[j];

		// throw away really low ASCII
		if (( /*out_buf[k]>=0 && */ out_buf[k] < 9))
			k--;

		// for CR
		if (out_buf[k] == 0x0D)
		{
			// if next is LF, then drop it
			if (j < len - 1 && in_buf[j + 1] == 0x0A)
				k--;
			else						  // turn it into a LF
				out_buf[k] = 0x0A;
		}
	}
	delete[]buf;
	buf = out_buf;
	len = k;
	return k;
}

void tBuf::setText(const byte * text, unsigned txtlen, bool txtcomp)
{
	if (buf)
		delete[]buf;
	buf = 0L;

	if (txtlen <= 0)
		txtlen = strlen((const char *) text);
	len = txtlen;
	buf = new byte[len];

	memcpy(buf, text, len*sizeof(char));
//	strncpy((char *) buf, (const char *) text, len);
	isCompressed = txtcomp;
//	cout<<"Setting text, compressed="<<txtcomp<<endl;
}
