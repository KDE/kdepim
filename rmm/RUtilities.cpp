/*
    Empath - Mailer for KDE
    
    Copyright (C) 1998, 1999 Rik Hemsley rik@kde.org
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifdef __GNUG__
# pragma implementation "RMM_Utility.h"
#endif

#include <qregexp.h>
#include <qcstring.h>
#include <ctype.h>

#include <RMM_Utility.h>
#include <RMM_Defines.h>

    QCString
RMM::toCrLfEol(const QCString & in)
{
    QCString s(in);
    s.replace(QRegExp("[^\\r]\\n"), "\r\n");
    return s;
}

    QCString
RMM::toLfEol(const QCString & in)
{
    QCString s(in);
    s.replace(QRegExp("\\r\\n"), "\n");
    return s;
}

    QCString
RMM::toCrEol(const QCString & in)
{
    QCString s(in);
    s.replace(QRegExp("[\\r]\\n"), "\r");
    return s;
}

    QCString
RMM::encodeQuotedPrintable(const QCString & in)
{
    QCString s(in);
    return s;
}

    QCString
RMM::decodeQuotedPrintable(const QCString & in)
{
    QCString s(in);
    return s;
}

// The copyright notice below refers to the base64 codec functions used below,
// which are modified from the original sources.

/*
 * Original version Copyright 1988 by The Leland Stanford Junior University
 * Copyright 1998 by the University of Washington
 *
 *  Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notices appear in all copies and that both the
 * above copyright notices and this permission notice appear in supporting
 * documentation, and that the name of the University of Washington or The
 * Leland Stanford Junior University not be used in advertising or publicity
 * pertaining to distribution of the software without specific, written prior
 * permission.  This software is made available "as is", and
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

    QCString
RMM::encodeBase64(const char * src, unsigned long srcl, unsigned long & destl)
{
    const unsigned char *s = (unsigned char *)src;
    unsigned long i = ((srcl + 2) / 3) * 4;
    destl = i += 2 * ((i / 60) + 1);
    char * ret = new char[destl];
    unsigned char *d((unsigned char *)ret);
    
    for (i = 0; srcl; s += 3) {
        
        *d++ =        B64[s[0] >> 2];
        *d++ =         B64[((s[0] << 4) + (--srcl ? s[1] >> 4 : 0)) & 0x3f];
        *d++ =
            srcl ?    B64[((s[1] << 2) + (--srcl ? s[2] >> 6 : 0)) & 0x3f] : '=';
        *d++ =
            srcl ?    B64[s[2] & 0x3f] : '=';
        
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

    char *
RMM::decodeBase64(const QCString & s, unsigned long & len)
{
    char c;
    int e(0); 
    len = 0;
    
    unsigned char * src = (unsigned char *)s.data();
    
    unsigned long srcl = s.length();
    
    char * ret = new char[srcl + (srcl / 4 + 1)];
    
    char *d((char *)ret);

    while (srcl--) {
        
        c = *src++;
        
        if        (isupper(c))    c -= 'A';
        else if    (islower(c))    c -= 'a' - 26;
        else if    (isdigit(c))    c -= '0' - 52;
        else if    (c == '+')        c = 62;
        else if    (c == '/')        c = 63;
        
        else if (c == '=') {

            switch (e++) {
                case 3:        e = 0;                break;
                case 2:        if (*src == '=')    break;
                default:                        return 0;
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

    len = d - (char *)ret;
    
    return ret;
}

// vim:ts=4:sw=4:tw=78
