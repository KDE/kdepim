/*
	Empath - Mailer for KDE
	
	Copyright (C) 1998 Rik Hemsley rik@kde.org
	
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

#include <RMM_Utility.h>
#include <RMM_Defines.h>

static const char * B64Chars =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	QCString
RToCrLfEol(const QCString & in)
{
	QCString s;
	return in;
}

	QCString
RToLfEol(const QCString & in)
{
	QCString s;
	return in;
}

	QCString
RToCrEol(const QCString & in)
{
	QCString s;
	return in;
}

	QCString
RToLocalEol(const QCString & in)
{
	QCString s;
	return in;
}

	QCString
REncodeBase64(const QCString & input)
{
	QCString out;

	int c1, c2, c3, ch;
	int linelen = 0;
	register int i = 0;
	
	const char * c = input.data();
	int length = input.length();
	
	while (i < length) {
		
		c1 = c[i++];
		c2 = c[i++];
		c3 = c[i++];
		
		if (linelen + 4 >= 76) {
			out += '\n';
			linelen = 0;
		}
		
		ch = c1 >> 2;
		out += B64Chars[ch];
		
		if (c2 != '\0') {
			ch = ((c1 & 0x3) << 4) | (c2 >> 4);
			out += B64Chars[ch];
		} else {
			ch = (c1 & 0x3) << 4;
			out += B64Chars[ch];
			out += "==";
			break;
		}
		
		if (c3 != '\0') {
			ch = ((c2 & 0xf) << 2) | (c3 >> 6);
			out += B64Chars[ch];
		} else {
			ch = (c2 & 0xf) << 2;
			out += B64Chars[ch];
			out += '=';
			break;
		}
		
		ch = c3 & 0x3f;
		out += ch;
		linelen += 4;
	}
	
	out += '\n';

	return out;
}

	QCString
RDecodeBase64(const QCString & in)
{
	QCString out;

	const char * input = in.data();
	register int cursor = 0;

    static char inalphabet[256], decoder[256];
	
    int i, bits, c, char_count, errors = 0;

    for (i = (sizeof B64Chars) - 1; i >= 0 ; i--) {
	
		inalphabet	[B64Chars[i]] = 1;
		decoder		[B64Chars[i]] = i;
    }

    char_count = bits = 0;
    
	c = input[cursor];
	
	while (c != '\0') {
	
		if (c == '=')
			break;
		
		if (c > 255 || ! inalphabet[c])
	  		continue;
		
		bits += decoder[c];
		
		++char_count;
		
		if (char_count == 4) {
	    
			out += bits >> 16;
			out += (bits >> 8) & 0xff;
		   	out += bits & 0xff;
			bits = char_count = 0;
		
		} else {
			
			bits <<= 6;
		}
    
	}
	
    if (c == '\0') {
	
		if (char_count) {
	    
//			rmmDebug("base64 encoding incomplete: at least " +
//				QCString().setNum((4 - char_count) * 6) + " bits truncated");
			++errors;
		}
		
    } else { // c == "="
	
		switch (char_count) {
	
		  	case 1:
	  
//			  	rmmDebug("base64 encoding incomplete: at least 2 bits missing");
			 	++errors;
				break;
	  
			case 2:
	    
				out += bits >> 10;
				break;
			
			case 3:
				
				out += bits >> 16;
				out += (bits >> 8) & 0xff;
				break;
		
		}
		
		++cursor;
    }

	return out;
}

	QCString
REncodeQuotedPrintable(const QCString & in)
{
	QCString s;
	return in;
}

	QCString
RDecodeQuotedPrintable(const QCString & in)
{
	QCString s;
	return in;
}


/*
int
main()
{
    static char inalphabet[256], decoder[256];
    int i, bits, c, char_count, errors = 0;

    for (i = (sizeof alphabet) - 1; i >= 0 ; i--) {
	inalphabet[alphabet[i]] = 1;
	decoder[alphabet[i]] = i;
    }

    char_count = 0;
    bits = 0;
    while ((c = getchar()) != EOF) {
	if (c == '=')
	  break;
	if (c > 255 || ! inalphabet[c])
	  continue;
	bits += decoder[c];
	char_count++;
	if (char_count == 4) {
	    putchar((bits >> 16));
	    putchar(((bits >> 8) & 0xff));
	    putchar((bits & 0xff));
	    bits = 0;
	    char_count = 0;
	} else {
	    bits <<= 6;
	}
    }
    if (c == EOF) {
	if (char_count) {
	    fprintf(stderr, "base64 encoding incomplete: at least %d bits truncated",
		    ((4 - char_count) * 6));
	    errors++;
	}
    } else { // c == "="
	switch (char_count) {
	  case 1:
	    fprintf(stderr, "base64 encoding incomplete: at least 2 bits missing");
	    errors++;
	    break;
	  case 2:
	    putchar((bits >> 10));
	    break;
	  case 3:
	    putchar((bits >> 16));
	    putchar(((bits >> 8) & 0xff));
	    break;
	}
    }
    exit(errors ? 1 : 0);
}
*/
