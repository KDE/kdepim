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

#ifdef __GNUG__
# pragma implementation "RMM_Utility.h"
#endif

#include <qregexp.h>

#include <RMM_Utility.h>
#include <RMM_Defines.h>

using namespace RMM;

const char * B64Chars =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	QCString
RMM::RToCrLfEol(const QCString & in)
{
	QCString s;
	s.replace(QRegExp("[^\\r]\\n"), "\r\n");
	return in;
}

	QCString
RMM::RToLfEol(const QCString & in)
{
	QCString s;
	s.replace(QRegExp("\\r\\n"), "\n");
	return in;
}

	QCString
RMM::RToCrEol(const QCString & in)
{
	QCString s;
	s.replace(QRegExp("[\\r]\\n"), "\r");
	return in;
}

	QCString
RMM::RToLocalEol(const QCString & in)
{
	QCString s;
	s.replace(QRegExp("[\\r]\\n"), "\n");
	return in;
}

	QCString
RMM::REncodeBase64(const QCString & input)
{
	register int c1, c2, c3, ch;
	
	register int i(0);
	
	register const char * c(input.data());
	
	register int length(input.length());
	
	char * outBuf = new char [length + length + (length / 3) + 1];
	
	register char * outCursor = outBuf;
	
	int linelen(0);
	
	// In each iteration of this loop it is only possible to increase the size
	// of the output string by 7. The input cursor is incremented 3 each time.
	// This means the output can only be twice the input plus 1/3 the input
	// size plus that '\n' at the end).
	// We can allocate a buffer that's twice the size of the
	// input at startup and simply walk a pointer over it.
	// While this isn't exactly conservative memory usage, it works and if
	// you're getting large base64-encoded mail, that's your problem.
	
	while (i < length) {
		
		c1 = c[i++];
		c2 = c[i++];
		c3 = c[i++];
		
		if (linelen >= 74) {
			*outCursor++ = '\n';
			linelen = 0;
		}
		
		ch = c1 >> 2;
		*outCursor++ = B64Chars[ch];
		
		if (c2 != '\0') {
			
			ch = ((c1 & 0x3) << 4) | (c2 >> 4);
			*outCursor++ = B64Chars[ch];
		
		} else {
		
			ch = (c1 & 0x3) << 4;
			*outCursor++ = B64Chars[ch];
			*outCursor++ = '=';
			*outCursor++ = '=';
			break;
		}
		
		if (c3 != '\0') {
		
			ch = ((c2 & 0xf) << 2) | (c3 >> 6);
			*outCursor++ = B64Chars[ch];
		
		} else {
		
			ch = (c2 & 0xf) << 2;
			*outCursor++ = B64Chars[ch];
			*outCursor++ = '=';
			break;
		}
		
		ch = c3 & 0x3f;
		*outCursor++ = ch;
		linelen += 4;
	}
	
	*outCursor = '\n';
	
	QCString out(outBuf);
	
	delete [] outBuf;
	outBuf = 0;
	
	return out;
}
	QCString
RMM::RDecodeBase64(const QCString & in)
{
	QCString s(in);
	return s;
}
#if 0
	// We can't decode to more than half the length,
	// so this is sufficient.

	char * outBuf = new char [in.length() / 2];
	register char * outCursor = outBuf;

	register const char * input = in.data();
	register int cursor = 0;

	static char alphabet[256], decoder[256];
	
	register int i(sizeof B64Chars);
	
	do {

		alphabet	[B64Chars[i]] = 1;
		decoder		[B64Chars[i]] = --i; // i should have been 1 less to start.

	} while (i != -1);

	int bits(0);
	int charCount(0);
	int errors(0);
	
	register int c(input[cursor]);
	
	while (c != '\0') {
	
		if (c == '=')
			break;
		
		if (c > 255 || ! alphabet[c])
	  		continue;
		
		bits += decoder[c];
		
		++charCount;
		
		if (charCount == 4) {
		
			out += bits >> 16;
			out += (bits >> 8) & 0xff;
		   	out += bits & 0xff;
			bits = charCount = 0;
		
		} else {
			
			bits <<= 6;
		}
	
	}
	
	if (c == '\0') {
	
		if (charCount != 0) {
//			rmmDebug("base64 encoding incomplete: at least " +
//				QCString().setNum((4 - charCount) * 6) + " bits truncated");
			++errors;
		}
		
	} else { // c == "="
	
		switch (charCount) {
	
		  	case 1:
	  
//			  	rmmDebug("base64 encoding incomplete: at least 2 bits missing");
			 	++errors;
				break;
	  
			case 2:
		
				*outCursor++ = bits >> 10;
				break;
			
			case 3:
				
				*outCursor++ = bits >> 16;
				*outCursor++ = (bits >> 8) & 0xff;
				break;
		
		}
		
		++cursor;
	}

	return out;
}
#endif

	QCString
RMM::REncodeQuotedPrintable(const QCString & in)
{
	QCString s;
	return in;
}

	QCString
RMM::RDecodeQuotedPrintable(const QCString & in)
{
	QCString s;
	return in;
}


/*
int
main()
{
	static char alphabet[256], decoder[256];
	int i, bits, c, charCount, errors = 0;

	for (i = (sizeof alphabet) - 1; i >= 0 ; i--) {
	alphabet[alphabet[i]] = 1;
	decoder[alphabet[i]] = i;
	}

	charCount = 0;
	bits = 0;
	while ((c = getchar()) != EOF) {
	if (c == '=')
	  break;
	if (c > 255 || ! alphabet[c])
	  continue;
	bits += decoder[c];
	charCount++;
	if (charCount == 4) {
		putchar((bits >> 16));
		putchar(((bits >> 8) & 0xff));
		putchar((bits & 0xff));
		bits = 0;
		charCount = 0;
	} else {
		bits <<= 6;
	}
	}
	if (c == EOF) {
	if (charCount) {
		fprintf(stderr, "base64 encoding incomplete: at least %d bits truncated",
			((4 - charCount) * 6));
		errors++;
	}
	} else { // c == "="
	switch (charCount) {
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

