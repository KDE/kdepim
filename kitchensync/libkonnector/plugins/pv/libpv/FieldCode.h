/***************************************************************************
                          FieldCode.h  -  description
                             -------------------
    begin                : Mit Jul 10 2002
    copyright            : (C) 2002 by Selzer Michael, Thomas Bonk
    email                : selzer@student.uni-kl.de, thomas@ghecko.saar.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/** Defines for Private and Business Contacts */
#define NAME									0x00100
#define HOME_NUMBER				0x00300
#define BUSINESS_NUMBER		0x00310
#define FAX_NUMBER					0x00301
#define BUSINESS_FAX				0x00311
#define MOBILE								0x00302
#define ADDRESS							0x00200
#define EMAIL									0x00001
#define EMPLOYER						0x00110
#define BUSINESS_ADDRESS		0x00210
#define DEPARTMENT					0x00010
#define POSITION							0x00011
#define NOTE									0x00000

/** Defines for Contact Untitled */
#define FIELD_1									0x00100
#define FIELD_2									0x00000
#define FIELD_3									0x00001
#define FIELD_4									0x00002
#define FIELD_5									0x00003
#define FIELD_6									0x00004
#define FIELD_7									0x00005
#define FIELD_8									0x00006
#define FIELD_9									0x00007
#define FIELD_10								0x00008
#define FIELD_11								0x00009
#define FIELD_12								0x0000A
#define FIELD_13								0x0000B

/** Define for MEMO */
#define MEMO									0x00000

/** Define for Quick-Memo */
#define QM_CATEGORY				0x20000
#define QM_BITMAP_DATA		0x10200

/** Defines for Schedule */
#define DATE									0x90000
#define END_DATE						0x90001
#define START_TIME					0xA0000
#define END_TIME							0xA0001
#define ALARM_TIME					0xA0100
#define DESCRIPTION					0x00000
#define TYPE									0x20020

/** Defines for Todo */
#define CHECK								0x20010
#define DUE_DATE						0x90000
#define ALARM_DATE					0x90100
#define ALARM_TIME					0xA0100
#define CHECK_DATE					0x90010
#define PRIORITY							0x20011
#define CATEGORY						0x20012
#define DESCRIPTION					0x00000

/** Defines for Expense (PV) */
#define EX_DATE							0x90000
#define EX_PAYMENT_TYPE		0x00001
#define EX_AMOUNT					0x80100
#define EX_EXPENSE_TYPE		0x00002
#define EX_NOTE							0x00000

/** Defines for Mail and SMS*/
#define MAIL_DATA						0x10000

/** Defines for Spread Sheet */
#define SHEET_DATA					0x00001
#define CELL_DATA						0x00000

/** Defines for Pocket Sheet */
#define PS_SHEET_DATA			0x10010
#define PS_X_LINE_DATA			0x10020
#define PS_Y_LINE_DATA			0x10021
#define PS_CELL_DATA				0x10030

/** Defines for communication */
#define DATA_CHANGED			0x20800
#define NUMBER_OF_DATA		0x20801
