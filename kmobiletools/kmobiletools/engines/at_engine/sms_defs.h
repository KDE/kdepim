/***************************************************************************
   Copyright (C) 2007
   by Marco Gulino <marco@kmobiletools.org>,
   Alexander Rensmann <zerraxys@gmx.net>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/

/*
 * Flags for SMS decoding according to GSM03.40
 */

/* Description of the SM-TL header */
#define MTIMask       0x03
#define MTIDeliver    0x00
#define MTISubmit     0x01
#define MTIStatus     0x02
#define MTIReserverd  0x03

#define TPMoreMessagesToSend 0x04
#define TPReplyPath          0x08
#define TPStatusReport       0x20
#define TPUserDataHeader     0x40

/* Some macros */
#define HAS_MR(SMTL) ( (SMTL) & MTIMask ) == MTISubmit
#define HAS_VP(SMTL) ( (SMTL>>3) & MTIMask ) == MTISubmit
#define HAS_TIMESTAMP(SMTL) ( SMTL & MTIMask ) == MTIDeliver
#define HAS_USERDATAHEADER(SMTL) ( SMTL & TPUserDataHeader )

/* Flags in the TP-Data-Coding-Sheme */
#define DCSAlphabetMask     0x0C
#define DCSDefaultAlphabet  0x00        // 7 bit encoding
#define DCS8BitEncoding     0x04
#define DCSUCS2Encoding     0x08
#define DCSReserved         0x0C

/* Identifier for the user data header */
#define UDHIConcatenated8BitReference   0x00

/* Internal flags */
#define SMSGSMCharset 1
#define SMS8BitCharset 2
#define SMSUCS2Charset 3
