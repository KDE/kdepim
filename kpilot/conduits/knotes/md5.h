/* md5.h			KPilot
**
** Copyright is unclear.
**
** This was ripped from somewhere in the KDE libs. I forget where.
*/

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

/*
** Bug reports and questions can be sent to adridg@cs.kun.nl
*/
#ifndef MD5_H
#define MD5_H

/* most modern machine should have 32 bits ints 
   * (some configure option should be added)
   */
typedef unsigned int uint32;

#ifdef __cplusplus
extern "C"
{
#endif
struct Bin_MD5Context {
  uint32 buf[4];
  uint32 bits[2];
  unsigned char in[64];
};

void Bin_MD5Init(struct Bin_MD5Context *context);
void Bin_MD5Update(struct Bin_MD5Context *context, 
		   unsigned char const *buf,
		   unsigned len);
void Bin_MD5Final(unsigned char digest[16], 
		  struct Bin_MD5Context *context);
void Bin_MD5Transform (uint32 buf[4], uint32 const in[16]);

#ifdef __cplusplus
}
#endif

#endif /* !MD5_H */


/* $Log$
 * Revision 1.2  2001/02/07 15:46:31  adridg
 * Updated copyright headers for source release. Added CVS log. No code change.
 */
