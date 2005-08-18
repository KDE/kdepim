/*
    This file is part of KAddressbook.
    Copyright (c) 2000 - 2000 Hans Dijkema <kmailcvt@hum.org>
                  2003 - 2003 Helge Deller <deller@kde.org>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef PAB_XXPORT_H
#define PAB_XXPORT_H

#include <xxport.h>

class PABXXPort : public KAB::XXPort
{
  Q_OBJECT

  public:
    PABXXPort( KABC::AddressBook *ab, QWidget *parent, const char *name = 0 );

    QString identifier() const { return "pab"; }

  public slots:
    KABC::AddresseeList importContacts( const QString &data ) const;

  private:
    void doExport( QFile *fp, const KABC::AddresseeList &list );
};




/////////////////////////////////////////////////////////////////////////////	

/*
 *  MS Windows tags as reengineered from an MS Exchange .PAB and
 *  Outlook .PAB file.
 */

/////////////////////////////////////////////////////////////////////////////	

#define MS_GIVEN_NAME   			0x3a13
#define MS_GIVEN_NAME_1 			0x3a45
#define MS_GIVEN_NAME_2 			0x3a47
#define MS_GIVEN_NAME_3				0x3a4f
#define MS_GIVEN_NAME_4				0x3001
#define MS_GIVEN_NAME_5				0x3a20
#define SET_MS_GIVEN_NAME 						\
		MS_GIVEN_NAME,MS_GIVEN_NAME_1,MS_GIVEN_NAME_2,		\
		MS_GIVEN_NAME_3,MS_GIVEN_NAME_4,MS_GIVEN_NAME_5
		
/////////////////////////////////////////////////////////////////////////////	
		
#define MS_EMAIL					0x3a56
#define MS_EMAIL_1                  			0x3003
#define SET_MS_EMAIL							\
		MS_EMAIL,MS_EMAIL_1

/////////////////////////////////////////////////////////////////////////////	

#define MS_FIRSTNAME					0x3a06
#define SET_MS_FIRSTNAME						\
		MS_FIRSTNAME

/////////////////////////////////////////////////////////////////////////////	

#define MS_LASTNAME					0x3a11
#define SET_MS_LASTNAME							\
		MS_LASTNAME


/////////////////////////////////////////////////////////////////////////////	

#define MS_MIDDLENAME					0x3a44
#define SET_MS_MIDDLENAME	\
		MS_MIDDLENAME

/////////////////////////////////////////////////////////////////////////////	

#define MS_TITLE					0x3a17
#define SET_MS_TITLE		\
		MS_TITLE

/////////////////////////////////////////////////////////////////////////////

#define MS_ADDRESS					0x3a15
#define MS_ADDRESS_1					0x3a29
#define MS_ADDRESS_2					0x3a59
#define SET_MS_ADDRESS		\
		MS_ADDRESS, MS_ADDRESS_1, MS_ADDRESS_2

/////////////////////////////////////////////////////////////////////////////

#define MS_ZIP						0x3a5b
#define MS_ZIP_1					0x3a2a
#define SET_MS_ZIP		\
		MS_ZIP, MS_ZIP_1

/////////////////////////////////////////////////////////////////////////////

#define MS_STATE					0x3a28
#define MS_STATE_1					0x3a5c
#define SET_MS_STATE		\
		MS_STATE, MS_STATE_1

/////////////////////////////////////////////////////////////////////////////

#define MS_TOWN						0x3a27
#define MS_TOWN_1					0x3a59
#define SET_MS_TOWN		\
		MS_TOWN, MS_TOWN_1

/////////////////////////////////////////////////////////////////////////////

#define MS_COUNTRY					0x3a26
#define MS_COUNTRY_1					0x3a5a
#define SET_MS_COUNTRY		\
		MS_COUNTRY, MS_COUNTRY_1

/////////////////////////////////////////////////////////////////////////////

#define MS_TEL						0x3a08
#define MS_TEL_1					0x3a09
#define MS_TEL_2					0x3a1a
#define MS_TEL_3					0x3a1b
#define MS_TEL_4					0x3a1f
#define MS_TEL_5					0x3a1d
#define MS_TEL_6					0x3a2d
#define MS_TEL_7					0x3a2f
#define SET_MS_TEL		\
		MS_TEL,MS_TEL_1,MS_TEL_2,MS_TEL_3,MS_TEL_4,	\
		MS_TEL_5,MS_TEL_6,MS_TEL_7

/////////////////////////////////////////////////////////////////////////////

#define MS_MOBILE					0x3a1c
#define MS_MOBILE_1					0x3a1e
#define MS_MOBILE_2					0x3a21
#define SET_MS_MOBILE		\
		MS_MOBILE,MS_MOBILE_1,MS_MOBILE_2

/////////////////////////////////////////////////////////////////////////////

#define MS_FAX						0x3a23
#define MS_FAX_1					0x3a24
#define MS_FAX_2					0x3a25
#define MS_FAX_3					0x3a2c
#define SET_MS_FAX		\
		MS_FAX,MS_FAX_1,MS_FAX_2,MS_FAX_3

/////////////////////////////////////////////////////////////////////////////

#define MS_ORG						0x3a16
#define SET_MS_ORGANIZATION	\
		MS_ORG

/////////////////////////////////////////////////////////////////////////////	

#define MS_DEP						0x3a18
#define SET_MS_DEPARTMENT	\
		MS_DEP

/////////////////////////////////////////////////////////////////////////////	

#define MS_COMMENT					0x3004
#define SET_MS_COMMENT		\
		MS_COMMENT

/////////////////////////////////////////////////////////////////////////////	

#define SET_NOT_USED		\
		0x3002,	\
		0x300b,	\
		0x3a2e,	\
		0x3a30,	\
		0x3a19
		// 3002 probably address type
		// 300b some sort of key
		// 3a2e secretary tel number
		// 3a30 name of secretary
		// 3a19 office location



/////////////////////////////////////////////////////////////////////////////	

/*
 * HP Openmail as reengineered from the X.400 .PAB file.
 */

/////////////////////////////////////////////////////////////////////////////

#define HP_OPENMAIL_JOB                            0x672b
#define HP_OPENMAIL_ORGANIZATION                   0x6728
#define HP_OPENMAIL_DEPARTMENT                     0x6729
#define HP_OPENMAIL_SUBDEP                         0x672b
#define HP_OPENMAIL_LOCATION_OF_WORK               0x672a

/////////////////////////////////////////////////////////////////////////////

#endif
