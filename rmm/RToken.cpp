/*
	Empath - Mailer for KDE
	
	Copyright (C) 1998 Rik Hemsley rikkus@postmaster.co.uk
	
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

#include <string.h>
#include <stddef.h>
#include <iostream>
#include <qstring.h>
#include <qstrlist.h>

#include <RMM_Defines.h>

	Q_UINT32
RTokenise(
	const char * str,
	const char * delim,
	QStrList & l,
	bool skipComments,
	bool quotedTokens = true)
{
	// FIXME no stderr !
	cerr << "RTokenise (\"" << str << "\", \"" << delim << "\") called" << endl;
	l.clear();
	
	if (!delim || !str || strlen(delim) == 0 || strlen(str) == 0) return 0;
	
	char * len = (char *)(str + strlen(str));	// End of string.
	
	int rl = strlen(str);
	char rstart[rl + 1];
	char * r = rstart;
	
	
	const char * i = str;		// Cursor.
	
	while (i <= len) {

		// I'm ignoring all non-printable chars.
		// Note that 127 (DEL) is ignored here. So what ? RFC822 is fucked up.
		// I'm not going to pander to its self-contradictory crap.
		if ((*i < 32 || *i == 127) && *i != '\t') {
			cerr << "Ignoring non-printable" << endl;
			++i;
			continue;
		}

		if (*i == '\\') { // Escaped chars go straight through.
			cerr << "Passing through an escaped char" << endl;
			*r++ = *i++;
			if (i <= len)
				*r++ = *i++;
			continue;
		}
		
		if (strchr(delim, *i) != 0) {
			// We hit a delimiter. If we have some text, make a new token.
			// This has the effect that multiple delimiters are collapsed.
			*r = '\0';
			if (r != rstart) {
				//cerr << "TOKEN " << rstart << endl;
				l.append(rstart);
			}
			r = rstart;
			++i;
			continue;
		}
		
		// If we find quote, make a token from everything until next quote.
		// Ignore '\"' (quoted quote :)
		// Bracing quote marks are included.
		// Adding the quote marks means we can test string[0] from the token
		// list to see if it's a quoted string.
		
		if (*i == '"' && quotedTokens) {
			
			cerr << "hit quote" << endl;

			// If there's anything in the buffer, make a token from that first.
			if (r != rstart) {
				cerr << "adding token from what was already held" << endl;
				*r = '\0';
				l.append(rstart);
				r = rstart;
			}
			
			cerr << "Just making token from what we have" << endl;
			
			do {
				cerr << len - i << endl;
				*r++ = *i++;
			} while (i < len - 1 && (*(i - 1) != '\\' && *i != ')'));
			
			*r++ = *i++; // End quote mark

			*r = '\0';
				//cerr << "TOKEN " << rstart << endl;
			l.append(rstart);
			r = rstart;
			++i;
			continue;
		}
					
		// If open brace and not a quoted brace, do nothing until we find
		// the ending brace or the end of the string.
		if (*i == '(') {

			cerr << "hit comment" << endl;
			// If there's anything in the buffer, make a token from that first.
			if (r != rstart) {
				*r = '\0';
			//	cerr << "TOKEN " << rstart << endl;
				l.append(rstart);
				r = rstart;
			}
	
			do {
				cerr << i - len << endl;
				*r++ = *i++;
			} while (i < len && (*(i - 1) != '\\' && *i != ')'));
			
			*r++ = *i++; // Closing brace

			*r = '\0';
			
			if (!skipComments) {
				//cerr << "TOKEN " << rstart << endl;
				l.append(rstart);
			}
			r = rstart;
			++i;
			continue;
		}

		*r++ = *i++;
	}

	// Catch last token
	if (r != rstart) {
		*r = '\0';
		l.append(rstart);
	}
	
	r = 0;

	return l.count();
}

#ifdef TEST
main()
{
	QString s = "Hello I'd \\\"like\" to be \"tokenised\r\n (I'm a \"with quote\" comment) without end";

	QString d = " ";

	QStrList l;
	l.setAutoDelete(true);

	int ntokens = RTokenise(s, d, l, false);
	
	cerr << "-----------------\n" << ntokens << " tokens found" << endl;

	QStrListIterator it(l);

	for (; it.current(); ++it)
		cerr << "TOKEN \"" << it.current() << "\"" << endl;
	
}
#endif

