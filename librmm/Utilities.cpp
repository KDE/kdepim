/* This file is part of the KDE project

   Copyright (C) 1999, 2000 Rik Hemsley <rik@kde.org>
             (C) 1999, 2000 Wilco Greven <j.w.greven@student.utwente.nl>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <qregexp.h>
#include <qcstring.h>
#include <ctype.h>

#include <rmm/Utilities.h>
#include <rmm/Defines.h>

    QCString
RMM::toCrLfEol(const QCString & in)
{
    QCString s(in.copy());
    s.replace(QRegExp("[^\\r]\\n"), "\r\n");
    return s;
}

    QCString
RMM::toLfEol(const QCString & in)
{
    QCString s(in.copy());
    s.replace(QRegExp("\\r\\n"), "\n");
    return s;
}

    QCString
RMM::toCrEol(const QCString & in)
{
    QCString s(in.copy());
    s.replace(QRegExp("[\\r]\\n"), "\r");
    return s;
}

    QCString
RMM::encodeQuotedPrintable(const QByteArray & in)
{
    QCString s(in.copy());
    return s;
}

    QByteArray
RMM::decodeQuotedPrintable(const QCString & in)
{
    QCString s(in.copy());
    return s;
}

// The copyright notice below refers to the base64 codec functions used below,
// which are modified from the original sources.

/*
    Copyright 1999, 2000
        Rik Hemsley <rik@kde.org>
        Wilco Greven <j.w.greven@student.utwente.nl>
  
    Permission to use, copy, modify, and distribute this software and its
   documentation for any purpose and without fee is hereby granted, provided
   that the above copyright notices appear in all copies and that both the
   above copyright notices and this permission notice appear in supporting
   documentation, and that the name of the University of Washington or The
   Leland Stanford Junior University not be used in advertising or publicity
   pertaining to distribution of the software without specific, written prior
   permission.  This software is made available "as is", and
   THE UNIVERSITY OF WASHINGTON AND THE LELAND STANFORD JUNIOR UNIVERSITY
   DISCLAIM ALL WARRANTIES, EXPRESS OR IMPLIED, WITH REGARD TO THIS SOFTWARE,
   INCLUDING WITHOUT LIMITATION ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
   FITNESS FOR A PARTICULAR PURPOSE, AND IN NO EVENT SHALL THE UNIVERSITY OF
   WASHINGTON OR THE LELAND STANFORD JUNIOR UNIVERSITY BE LIABLE FOR ANY
   SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
   RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
   CONTRACT, TORT (INCLUDING NEGLIGENCE) OR STRICT LIABILITY, ARISING OUT OF
   OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
  
 */

const char * B64 =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    QCString
RMM::encodeBase64(const QByteArray & src)
{
    const unsigned char *s = (unsigned char *)(src.data());
    unsigned long srcl = src.size();
    unsigned long i = ((srcl + 2) / 3) * 4;
    unsigned long destl = i += 2 * ((i / 60) + 1);
    char * ret = new char[destl];
    unsigned char *d((unsigned char *)ret);
    
    for (i = 0; srcl != 0; s += 3) {
        
        *d++ = B64[s[0] >> 2];
        *d++ = B64[((s[0] << 4) + (--srcl ? s[1] >> 4 : 0)) & 0x3f];
        *d++ =
            srcl ? B64[((s[1] << 2) + (--srcl ? s[2] >> 6 : 0)) & 0x3f] : '=';
        *d++ =
            srcl ? B64[s[2] & 0x3f] : '=';
        
        if (srcl != 0)
            srcl--;
        
        if (++i == 15) {
            i = 0;
            *d++ = '\r';
            *d++ = '\n';
        }
    }
  
    *d++    = '\r';
    *d++    = '\n';
    *d        = '\0';
    
    QCString out(ret);
    delete [] ret;
    ret = 0;
    return out;
}

    QByteArray
RMM::decodeBase64(const QCString & s)
{
    char c;
    int e(0); 
    
    unsigned char * src = (unsigned char *)s.data();
    
    unsigned long srcl = s.length();
    unsigned long destl = srcl + (srcl / 4 + 1);
    char * ret = new char[destl];
    
    char *d((char *)ret);

    while (srcl--) {
        
        c = *src++;
        
        if         (isupper(c))    c -= 'A';
        else if    (islower(c))    c -= 'a' - 26;
        else if    (isdigit(c))    c -= '0' - 52;
        else if    (c == '+')      c = 62;
        else if    (c == '/')      c = 63;
        
        else if (c == '=') {

            switch (e++) {
                case 3:  e = 0;            break;
                case 2:  if (*src == '=')  break;
                default:                   return 0;
            }
    
            continue;
        }
        
        else continue;

        switch (e++) {
            
            case 0:    *d = c << 2;                    break;
            case 1:    *d++ |= c >> 4; *d = c << 4;    break;
            case 2:    *d++ |= c >> 2; *d = c << 6;    break;
            case 3:    *d++ |= c; e = 0;                break;
        }
    }

    QByteArray a;
    a.setRawData(ret, destl);
    return a;
}


/*
The following code does it.. but it's not entirely complete.
It's the smallest, fastest, coolest grooviest implementation.

And it's by ME! (Charles) <charles@altair.dhs.org>

#include <iostream.h>
#include <stdlib.h>
#include <string.h>
#include <iostream.h>

static const char map[] =
	{"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"};

char* _encoder_core(const unsigned char *byte, short int len)
{	
	// byte will contain 4 characters.
	char *buf = new char[4];	
	// 111111 11|1111 1111|11 111111
	buf[0] = map[byte[0] >> 2];
	buf[1] = map[(byte[1] >> 4) | ((byte[0] & 0x03) <<4)];
	buf[2] = map[((byte[1] & 0x0F) << 2) | ((byte[2] & 0xC0) >> 6)];
	buf[3] = map[byte[2] & 0x3F];

	switch (len)
	{
	case (1): buf[2]='=';
	case (2): buf[3]='=';
	}
		
	return buf;
}

void encoder(ostream &out, istream &in)
{
	unsigned char input[3];
	short int len;
	while (!in.eof())
	{
		memset(input, 0, 3);
		len=(short int)in.read((void*)input, 3);
		char *data=_encoder_core(input,len);
		out.write(data, 4);
		delete [] data;
	}
}


int main(int argc, char** argv)
{
	encoder((ofstream)cout, (ifstream)cin);
	return 0;
}








*/


// vim:ts=4:sw=4:tw=78
