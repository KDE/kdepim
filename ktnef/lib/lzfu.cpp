/*
    lzfu.cpp

    Copyright (C) 2003 Michael Goffioul <goffioul@imec.be>

    This file is part of KTNEF, the KDE TNEF support library/program.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 */

#include "lzfu.h"

#include <stdint.h>
#include <sys/types.h>
#include <string.h>
#include <qiodevice.h>
#include <stdio.h>

//#define DO_DEBUG

#define LZFU_COMPRESSED		0x75465a4c
#define LZFU_UNCOMPRESSED	0x414c454d

#define LZFU_INITDICT	"{\\rtf1\\ansi\\mac\\deff0\\deftab720{\\fonttbl;}" \
						"{\\f0\\fnil \\froman \\fswiss \\fmodern \\fscrip" \
						"t \\fdecor MS Sans SerifSymbolArialTimes Ne" \
						"w RomanCourier{\\colortbl\\red0\\green0\\blue0" \
						"\r\n\\par \\pard\\plain\\f0\\fs20\\b\\i\\u\\tab" \
						"\\tx"
#define LZFU_INITLENGTH 207

typedef struct _lzfuheader {
	uint32_t cbSize;
	uint32_t cbRawSize;
	uint32_t dwMagic;
	uint32_t dwCRC;
} lzfuheader;

#define FLAG(f,n)	(f>>n)&0x1

/*typedef struct _blockheader {
	unsigned int offset:12;
	unsigned int length:4;
} blockheader;*/

#define OFFSET(b) (b>>4)&0xFFF
#define LENGTH(b) ((b&0xF)+2)

int lzfu_decompress(QIODevice *input, QIODevice *output)
{
	unsigned char window[4096];
	unsigned int wlength = 0, cursor = 0, ocursor = 0;
	lzfuheader lzfuhdr;
	//blockheader blkhdr;
	uint16_t blkhdr;
	char bFlags;
	int nFlags;

	memcpy(window, LZFU_INITDICT, LZFU_INITLENGTH);
	wlength = LZFU_INITLENGTH;
	if (input->readBlock((char*)&lzfuhdr, sizeof(lzfuhdr)) != sizeof(lzfuhdr))
	{
		fprintf(stderr, "unexpected eof, cannot read LZFU header\n");
		return -1;
	}
	cursor += sizeof( lzfuhdr );
#ifdef DO_DEBUG
	fprintf(stdout, "total size : %d\n", lzfuhdr.cbSize+4);
	fprintf(stdout, "raw size   : %d\n", lzfuhdr.cbRawSize);
	fprintf(stdout, "compressed : %s\n", (lzfuhdr.dwMagic == LZFU_COMPRESSED ? "yes" : "no"));
	fprintf(stdout, "CRC        : %x\n", lzfuhdr.dwCRC);
	fprintf(stdout, "\n");
#endif

	while (cursor < lzfuhdr.cbSize+4 && ocursor < lzfuhdr.cbRawSize && !input->atEnd())
	{
		if (input->readBlock(&bFlags, 1) != 1)
		{
			fprintf(stderr, "unexpected eof, cannot read chunk flag\n");
			return -1;
		}
		nFlags = 8;
		cursor++;
#ifdef DO_DEBUG
		fprintf(stdout, "Flags : ");
		for (int i=nFlags-1; i>=0; i--)
			fprintf(stdout, "%d", FLAG(bFlags, i));
		fprintf(stdout, "\n");
#endif
		for (int i=0; i<nFlags && ocursor<lzfuhdr.cbRawSize && cursor<lzfuhdr.cbSize+4; i++)
		{
			if (FLAG(bFlags, i))
			{
				// compressed chunck
				char c1, c2;
				if (input->readBlock(&c1, 1) != 1 || input->readBlock(&c2, 1) != 1)
				{
					fprintf(stderr, "unexpected eof, cannot read block header\n");
					return -1;
				}
				blkhdr = c1;
				blkhdr <<= 8;
				blkhdr |= (0xFF&c2);
				unsigned int offset = OFFSET(blkhdr), length = LENGTH(blkhdr);
				cursor += 2;
#ifdef DO_DEBUG
				fprintf( stdout, "block : offset=%.4d [%d], length=%.2d (0x%04X)\n", OFFSET( blkhdr ), wlength, LENGTH( blkhdr ), blkhdr );
#endif
				//if (offset >= wlength)
				//{
				//	break;
				//}
#ifdef DO_DEBUG
				fprintf( stdout, "block : " );
#endif
				for (unsigned int i=0; i<length; i++)
				{
					c1 = window[( offset+i ) % 4096];
					//if (wlength < 4096)
					//{
						window[wlength] = c1;
						wlength = ( wlength+1 ) % 4096;
					//}
#ifdef DO_DEBUG
					if ( c1 == '\n' )
						fprintf( stdout, "\nblock : " );
					else
						fprintf( stdout, "%c", c1 );
#endif
					output->putch(c1);
					ocursor++;
				}
#ifdef DO_DEBUG
				fprintf( stdout, "\n" );
#endif
			}
			else
			{
				// uncompressed chunk (char)
				char c = (char)input->getch();
				if (c == -1)
				{
					if (!input->atEnd())
					{
						fprintf(stderr, "unexpected eof, cannot read character\n");
						return -1;
					}
					break;
				}
#ifdef DO_DEBUG
				fprintf( stdout, "char  : %c\n", c );
#endif
				cursor++;
				//if (wlength < 4096)
				//{
					window[wlength] = c;
					wlength = ( wlength+1 ) % 4096;
				//}
				output->putch(c);
				ocursor++;
			}
		}

	}

	return 0;
}
