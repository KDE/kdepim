#include <iostream>
#include "AttrTypeAndValue.cpp"
#include "AttrValSpec.cpp"
#include "Dn.cpp"
#include "DnSpec.cpp"
#include "LDAPString.cpp"
#include "LdifAttrValRec.cpp"
#include "LdifContent.cpp"
#include "NameComponent.cpp"
#include "RToken.cpp"
#include "ValueSpec.cpp"
#include "VersionSpec.cpp"

#include <ctype.h>

// The copyright notice below refers to the base64 and quoted-printable codec
// functions used below, which are modified from the original sources.

/*
 * Original version Copyright 1988 by The Leland Stanford Junior University
 * Copyright 1998 by the University of Washington
 *
 *	Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notices appear in all copies and that both the
 * above copyright notices and this permission notice appear in supporting
 * documentation, and that the name of the University of Washington or The
 * Leland Stanford Junior University not be used in advertising or publicity
 * pertaining to distribution of the software without specific, written prior
 * permission.	This software is made available "as is", and
 * THE UNIVERSITY OF WASHINGTON AND THE LELAND STANFORD JUNIOR UNIVERSITY
 * DISCLAIM ALL WARRANTIES, EXPRESS OR IMPLIED, WITH REGARD TO THIS SOFTWARE,
 * INCLUDING WITHOUT LIMITATION ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE, AND IN NO EVENT SHALL THE UNIVERSITY OF
 * WASHINGTON OR THE LELAND STANFORD JUNIOR UNIVERSITY BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
 * CONTRACT, TORT (INCLUDING NEGLIGENCE) OR STRICT LIABILITY, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

	static const char *
B64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// the mime base64 disctionary used for decoding
	static char
b64dec[] = {
	//  -1 == bad data
	// -19 == '/'
	// -16 == '+'
	//  -4 == digit
	//   0 == '='
	//  65 == upper
	//  71 == lower
#define _ -1
	 _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,
	 _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,
	 _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,-19,  _,
	 _,  _,-16, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4,  _,  _,
	 _,	 0,  _,  _,  _, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
	65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
	65,  _,  _,  _,  _,  _,  _, 71, 71, 71, 71, 71, 71, 71, 71,
   	71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71,
	71, 71, 71,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,
   	 _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,
	 _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,
   	 _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,
	 _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,
   	 _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,
	 _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,
   	 _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,
	 _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _
#undef _
};

// the mime quoted-printable disctionary used for decoding
	static char
qpdec[] = {
	// Values here are ORed together
	// 1 == hex digit
	// 2 == upper alpha
	// 4 == lower alpha
	// 8 == digit
#define _ 0
	_,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,
	_,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,
	_,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,
	9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  _,  _,  _,  _,  _,  _,
	_,  3,  3,  3,  3,  3,  3,  2,  2,  2,  2,  2,  2,  2,  2,  2,
	2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  _,  _,  _,  _,  _,
	_,  5,  5,  5,  5,  5,  5,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  _,  _,  _,  _,  _,
	_,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,
	_,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,
	_,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,
	_,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,
	_,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,
    _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,
	_,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,
	_,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _
#undef _
};


	QByteArray
LDIF::decodeBase64(const char * s, unsigned long srcl, unsigned long & len)
{
  QByteArray a;
	register char c;
	register unsigned long e(0);
	len = 0;
	unsigned const char * src = (unsigned const char *)s;
  int retsize = srcl + (srcl / 4 + 1);
  char * ret = new char[retsize];
	register char *d = ret;
	while (srcl--) { // Critical loop
		c = *src++;
		int dec = b64dec[c];
		if (dec == -1) continue;
		if (c == '=') {
			switch (e++) {
				case 3:   e = 0;            break;
				case 2:   if (*src == '=')  break;
				default:  return a;         break;
			}
			continue;
		}
		c -= dec;
		if (e == 0) { *d = c << 2; ++e; continue; }
		switch (e) {
			case 1: *d |= c >> 4; *++d = c << 4;	break;
			case 2: *d |= c >> 2; *++d = c << 6;	break;
			case 3: *d++ |= c; e = 0; continue;		break;
		}
		++e;
	}
	len = d - (char *)ret;
  a.setRawData(ret, retsize);
	return a;
}


	QByteArray
LDIF::encodeBase64(const char * src, unsigned long srcl, unsigned long & destl)
{
  QByteArray a;
	register const unsigned char *s = (unsigned char *)src;
	register unsigned long i = ((srcl + 2) / 3) * 4;
	destl = i += 2 * ((i / 60) + 1);
	i = 0;
	char * ret = new char[destl];
	register unsigned char *d((unsigned char *)ret);
	while (srcl != 0) { // Critical loop
		*d++ = B64[s[0] >> 2];
		*d++ = B64[((s[0] << 4) + (--srcl == 0 ? 0 : s[1] >> 4)) & 0x3f];
		*d++ = srcl == 0 ? '=' :
			B64[((s[1] << 2) + (--srcl == 0 ? 0 : s[2] >> 6)) & 0x3f];
		*d++ = srcl == 0 ?	'=' : B64[s[2] & 0x3f];
		if (srcl != 0) srcl--;
		if (++i == 15) { i = 0; *d++ = '\r'; *d++ = '\n'; }
		s += 3;
	}
	*d = '\r'; *++d = '\n'; *++d = '\0';
  a.setRawData(ret, destl);
	return a;
}

	QByteArray
LDIF::decodeQP(const char * src, unsigned long srcl, unsigned long & len)
{
  QByteArray a;
	cerr << "decode \"" << src << "\"" << endl;
  int retsize = srcl + 1;
	char * ret = new char[retsize];
	char * d = ret;
	char * s = d;
	char c, e;
	
	len = 0;
	
	while ((c = *src++)) {
		
		switch (c) {
			
			case '=':
				
				if (c == '\0') { --src; break; }

				c = *src++;
				
				if (c == '\r') { s = d; if (*src == '\n') ++src; break; }
				
				if (!(qpdec[c] & 1)) return a;
				
				if (qpdec[c] & 8) e = c - '0';
			
				else e = c - (qpdec[c] & 2 ? 'A' - 10 : 'a' - 10);
				
				c = *src++;
				
				if (!(qpdec[c] & 1)) return a;
					
				if (qpdec[c] & 8) c -= '0';
			
				else c -= (qpdec[c] & 2 ? 'A' - 10 : 'a' - 10);
				
				*d++ = c + (e << 4);
				
				s = d;
				
				break;
				
			case  ' ':	*d++ = c;	break;
			case '\r':	d = s;
			default:	*d++ = c;	s = d;	break;
		}
	}

	*d = '\0';
	len = d - ret;
	a.setRawData(ret, retsize);
	return a;
}

	QByteArray
LDIF::encodeQP(const char * src, unsigned long srcl, unsigned long & len)
{
  QByteArray a;
	cerr << "encode \"" << src << "\"" << endl;
	const char * hex = "0123456789ABCDEF";
	
	unsigned long lp(0);
	int retsize = 3 * srcl + (6 * srcl) / 75 + 3;
	char * ret = new char [retsize];
	char * d = ret;
	char c;
	
	while (srcl--) {
		
		c = *src++;
		
		if (	(     c == '\r'	) &&
				(  *src	== '\n'	) &&
				(  srcl	!= 0	)	) {
			
			*d++ = '\r';
			*d++ = *src++;
			srcl--;
			lp = 0;
			
			continue;
		}
		
		if (
			iscntrl (c)			|| 
			(	c == 0x7f)		|| 
			(	c &  0x80)		|| 
			(	c == '=')		||
			((c == ' ') && (*src == '\r'))) {
			
			if ((lp += 3) > 75) {
				
				*d++ = '=';
				*d++ = '\r';
				*d++ = '\n';
				lp = 3;
			}
			
			*d++ = '=';
			*d++ = hex[c >> 4];
			*d++ = hex[c & 0xf];

			continue;
		}

		if ((++lp) > 75) {
			
			*d++ = '=';
			*d++ = '\015';
			*d++ = '\012';
			lp = 1;
		}
			
		*d++ = c;
	}
	
	*d = '\0';
	len = d - ret;
  a.setRawData(ret, retsize);
	return a;
}

