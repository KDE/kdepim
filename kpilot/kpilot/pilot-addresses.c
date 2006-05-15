/*
 * pilot-addresses.c:  Palm address transfer utility
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <unistd.h>

#include "pi-socket.h"
#include "pi-dlp.h"
#include "pi-address.h"
#include "pi-header.h"

/* These are indexes in the tabledelims array */
enum terminators { term_newline=0,
	term_comma=1,
	term_semi=2,
	term_tab=3 } ;
terminators tabledelim = term_comma;
char 	tabledelims[4] = { '\n', ',', ';', '\t' };



int realentry[21] =
    { 0, 1, 13, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 14, 15, 16, 17, 18, 19, 20 };

const char *tableheads[21] = {
	"Last name",	/* 0 	*/
	"First name", 	/* 1	*/
	"Title", 	/* 2	(real entry 13)*/
	"Company", 	/* 3	*/
	"Phone1", 	/* 4	*/
	"Phone2",	/* 5	*/
	"Phone3", 	/* 6	*/
	"Phone4", 	/* 7	*/
	"Phone5", 	/* 8	*/
	"Address", 	/* 9	*/
	"City", 	/* 10	*/
	"State",	/* 11	*/
	"Zip Code",	/* 12	*/
	"Country", 	/* 13	*/
	"Custom 1", 	/* 14	*/
	"Custom 2", 	/* 15	*/
	"Custom 3", 	/* 16	*/
	"Custom 4", 	/* 17	*/
	"Note",		/* 18	*/
	"Private", 	/* 19	*/
	"Category"	/* 20	*/
};

int
	augment 	= 0,
	defaultcategory = 0;



/***********************************************************************
 *
 * Function:    inchar
 *
 * Summary:     Turn the protected name back into the "original"
 *		characters
 *
 * Parameters:
 *
 * Returns:     Modified character, 'c'
 *
 ***********************************************************************/
int inchar(FILE * in)
{
	int 	c;	/* switch */

	c = getc(in);
	if (c == '\\') {
		c = getc(in);
		switch (c) {
		case 'b':
			c = '\b';
			break;
		case 'f':
			c = '\f';
			break;
		case 'n':
			c = '\n';
			break;
		case 't':
			c = '\t';
			break;
		case 'r':
			c = '\r';
			break;
		case 'v':
			c = '\v';
			break;
		case '\\':
			c = '\\';
			break;
		default:
			ungetc(c, in);
			c = '\\';
			break;
		}
	}
	return c;
}


/***********************************************************************
 *
 * Function:    read_field
 *
 * Summary:     Reach each field of the CSV during read_file
 *
 * Parameters:  dest    <-> Buffer for storing field contents
 *              in      --> Inbound filehandle
 *              length  --> Size of buffer
 *
 * Returns:     0 for end of line
 *              1 for , termination
 *              2 for ; termination
 *              3 for \t termination
 *             -1 on end of file
 *
 *              Note that these correspond to indexes in the tabledelims
 *              array, and should be preserved.
 *
 ***********************************************************************/
int read_field(char *dest, FILE *in, size_t length)
{
	int 	c;

	if (length<=1) return -1;
	/* reserve space for trailing NUL */
	length--;

	do {	/* Absorb whitespace */
		c = getc(in);
		if(c == '\n') {
			*dest = 0;
			return term_newline;
		}

	} while ((c != EOF) && ((c == ' ') || (c == '\t') || (c == '\r')));

	if (c == '"') {
		c = inchar(in);

		while (c != EOF) {
			if (c == '"') {
				c = inchar(in);
				if (c != '"')
					break;
			}
			*dest++ = c;
			if (!(--length))
				break;
			c = inchar(in);
		}
	} else {
		while (c != EOF) {
			if ((c == '\n') || (c == tabledelims[tabledelim])) {
				break;
			}
			*dest++ = c;
			if (!(--length))
				break;
			c = inchar(in);
		}
	}
	*dest++ = '\0';

	/* Absorb whitespace */
	while ((c != EOF) && ((c == ' ') || (c == '\t')))
		c = getc(in);

	if (c == ',')
		return term_comma;

	else if (c == ';')
		return term_semi;

	else if (c == '\t')
		return term_tab;

	else if (c == EOF)
		return -1;	/* No more */
	else
		return term_newline;
}


/***********************************************************************
 *
 * Function:    outchar
 *
 * Summary:     Protect each of the 'illegal' characters in the output
 *
 * Parameters:  filehandle
 *
 * Returns:     Nothing
 *
 ***********************************************************************/
void outchar(char c, FILE * out)
{
		switch (c) {
		case '"':
			putc('"', out);
			putc('"', out);
			break;
		case '\b':
			putc('\\', out);
			putc('b', out);
			break;
		case '\f':
			putc('\\', out);
			putc('f', out);
			break;
		case '\n':
			putc('\\', out);
			putc('n', out);
			break;
		case '\t':
			putc('\\', out);
			putc('t', out);
			break;
		case '\r':
			putc('\\', out);
			putc('r', out);
			break;
		case '\v':
			putc('\\', out);
			putc('v', out);
			break;
		case '\\':
			putc('\\', out);
			putc('\\', out);
			break;
		default:
			putc(c, out);
			break;
		}
}


/***********************************************************************
 *
 * Function:    write_field
 *
 * Summary:     Write out each field in the CSV
 *
 * Parameters:  out    --> output file handle
 *              source --> NUL-terminated data to output
 *              more   --> delimiter number
 *
 * Returns:
 *
 ***********************************************************************/
int write_field(FILE * out, const char *source, enum terminators more)
{
	putc('"', out);

	while (*source) {
		outchar(*source, out);
		source++;
	}
	putc('"', out);

	putc(tabledelims[more], out);
	return 0;
}




/***********************************************************************
 *
 * Function:    match_phone
 *
 * Summary:     Find and match the 'phone' entries in 'buf'
 *
 * Parameters:
 *
 * Returns:
 *
 ***********************************************************************/
int match_phone(char *buf, struct AddressAppInfo *aai)
{
	int 	i;

	for (i = 0; i < 8; i++)
		if (strncasecmp(buf, aai->phoneLabels[i], sizeof(aai->phoneLabels[0])) == 0)
			return i;
	return atoi(buf);	/* 0 is default */
}



/***********************************************************************
 *
 * Function:    write_file
 *
 * Summary:     Writes Address records in CSV format to <file>
 *
 * Parameters:  filehandle
 *
 * Returns:     0
 *
 ***********************************************************************/

void write_record_CSV(FILE *out, const struct AddressAppInfo *aai, const struct Address *addr, const int attribute, const int category, const int augment)
{
	int j;
	char buffer[16];

	if (augment && (category || addr->showPhone)) {
		write_field(out,
				aai->category.name[category],
				term_semi);
		write_field(out,
				aai->phoneLabels[addr->phoneLabel[addr->showPhone]],
				term_semi);
	}

	for (j = 0; j < 19; j++) {
		if (addr->entry[realentry[j]]) {
			if (augment && (j >= 4) && (j <= 8)) {
				write_field(out,
						aai->phoneLabels[addr->phoneLabel
								[j - 4]], term_semi);
			}
			write_field(out, addr->entry[realentry[j]],
					tabledelim);
		} else {
			write_field(out, "", tabledelim);
		}
	}

	snprintf(buffer, sizeof(buffer), "%d", (attribute & dlpRecAttrSecret) ? 1 : 0);
	write_field(out, buffer, tabledelim);

	write_field(out,
		aai->category.name[category],
		term_newline);
}
