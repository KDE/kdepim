/*
    mapi.cpp

    Copyright (C) 2002 Michael Goffioul <goffioul@imec.be>

    This file is part of KTNEF, the KDE TNEF support library/program.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 */

#include "mapi.h"
#include <qmap.h>
#include <klocale.h>

static struct 
{
	int tag;
	const char *str;
} MAPI_TagStrings[] = 
{
	{ 0x0002, I18N_NOOP( "Alternate Recipient Allowed" ) },
	{ 0x001A, I18N_NOOP( "Message Class" ) },
	{ 0x0023, I18N_NOOP( "Originator Delivery Report Requested" ) },
	{ 0x0024, I18N_NOOP( "Originator Return Address" ) },
	{ 0x0026, I18N_NOOP( "Priority" ) },
	{ 0x0029, I18N_NOOP( "Read Receipt Requested" ) },
	{ 0x002B, I18N_NOOP( "Recipient Reassignment Prohibited" ) },
	{ 0x002E, I18N_NOOP( "Original Sensitivity" ) },
	{ 0x0031, I18N_NOOP( "Report Tag" ) },
	{ 0x0036, I18N_NOOP( "Sensitivity" ) },
	{ 0x0037, I18N_NOOP( "Subject" ) },
	{ 0x0039, I18N_NOOP( "Client Submit Time" ) },
	{ 0x003B, I18N_NOOP( "Sent Representing Search Key" ) },
	{ 0x003D, I18N_NOOP( "Subject Prefix" ) },
	{ 0x0041, I18N_NOOP( "Sent Representing Entry ID" ) },
	{ 0x0042, I18N_NOOP( "Sent Representing Name" ) },
	{ 0x0047, I18N_NOOP( "Message Submission ID" ) },
	{ 0x004D, I18N_NOOP( "Original Author Name" ) },
	{ 0x0062, I18N_NOOP( "Owner Appointment ID" ) },
	{ 0x0063, I18N_NOOP( "Response Requested" ) },
	{ 0x0064, I18N_NOOP( "Sent Representing Address Type" ) },
	{ 0x0065, I18N_NOOP( "Sent Representing E-mail Address" ) },
	{ 0x0070, I18N_NOOP( "Conversation Topic" ) },
	{ 0x0071, I18N_NOOP( "Conversation Index" ) },
	{ 0x007F, I18N_NOOP( "TNEF Correlation Key" ) },
	{ 0x0C17, I18N_NOOP( "Reply Requested" ) },
	{ 0x0C1A, I18N_NOOP( "Sender Name" ) },
	{ 0x0C1D, I18N_NOOP( "Sender Search Key" ) },
	{ 0x0C1E, I18N_NOOP( "Sender Address Type" ) },
	{ 0x0C1F, I18N_NOOP( "Sender E-mail Address" ) },
	{ 0x0E01, I18N_NOOP( "Delete After Submit" ) },
	{ 0x0E02, I18N_NOOP( "Display Bcc" ) },
	{ 0x0E03, I18N_NOOP( "Display Cc" ) },
	{ 0x0E04, I18N_NOOP( "Display To" ) },
	{ 0x0E06, I18N_NOOP( "Message Delivery Time" ) },
	{ 0x0E07, I18N_NOOP( "Message Flags" ) },
	{ 0x0E08, I18N_NOOP( "Message Size" ) },
	{ 0x0E09, I18N_NOOP( "Parent Entry ID" ) },
	{ 0x0E0A, I18N_NOOP( "Sent-Mail Entry ID" ) },
	{ 0x0E12, I18N_NOOP( "Message Recipients" ) },
	{ 0x0E14, I18N_NOOP( "Submit Flags" ) },
	{ 0x0E1B, I18N_NOOP( "Has Attachment" ) },
	{ 0x0E1D, I18N_NOOP( "Normalized Subject" ) },
	{ 0x0E1F, I18N_NOOP( "RTF In Sync" ) },
	{ 0x0E20, I18N_NOOP( "Attachment Size" ) },
	{ 0x0E21, I18N_NOOP( "Attachment Number" ) },
	{ 0x0FF4, I18N_NOOP( "Access" ) },
	{ 0x0FF7, I18N_NOOP( "Access Level" ) },
	{ 0x0FF8, I18N_NOOP( "Mapping Signature" ) },
	{ 0x0FF9, I18N_NOOP( "Record Key" ) },
	{ 0x0FFA, I18N_NOOP( "Store Record Key" ) },
	{ 0x0FFB, I18N_NOOP( "Store Entry ID" ) },
	{ 0x0FFE, I18N_NOOP( "Object Type" ) },
	{ 0x0FFF, I18N_NOOP( "Entry ID" ) },
	{ 0x1000, I18N_NOOP( "Message Body" ) },
	{ 0x1006, I18N_NOOP( "RTF Sync Body CRC" ) },
	{ 0x1007, I18N_NOOP( "RTF Sync Body Count" ) },
	{ 0x1008, I18N_NOOP( "RTF Sync Body Tag" ) },
	{ 0x1009, I18N_NOOP( "RTF Compressed" ) },
	{ 0x1010, I18N_NOOP( "RTF Sync Prefix Count" ) },
	{ 0x1011, I18N_NOOP( "RTF Sync Trailing Count" ) },
	{ 0x1013, I18N_NOOP( "HTML Message Body" ) },
	{ 0x1035, I18N_NOOP( "Message ID" ) },
	{ 0x1042, I18N_NOOP( "Parent's Message ID" ) },
	{ 0x1080, I18N_NOOP( "Action" ) },
	{ 0x1081, I18N_NOOP( "Action Flag" ) },
	{ 0x1082, I18N_NOOP( "Action Date" ) },
	{ 0x3001, I18N_NOOP( "Display Name" ) },
	{ 0x3007, I18N_NOOP( "Creation Time" ) },
	{ 0x3008, I18N_NOOP( "Last Modification Time" ) },
	{ 0x300B, I18N_NOOP( "Search Key" ) },
	{ 0x340D, I18N_NOOP( "Store Support Mask" ) },
	{ 0x3414, I18N_NOOP( "MDB Provider" ) },
	{ 0x3701, I18N_NOOP( "Attachment Data" ) },
	{ 0x3702, I18N_NOOP( "Attachment Encoding" ) },
	{ 0x3703, I18N_NOOP( "Attachment Extension" ) },
	{ 0x3705, I18N_NOOP( "Attachment Method" ) },
	{ 0x3707, I18N_NOOP( "Attachment Long File Name" ) },
	{ 0x370B, I18N_NOOP( "Attachment Rendering Position" ) },
	{ 0x370E, I18N_NOOP( "Attachment Mime Tag" ) },
	{ 0x3714, I18N_NOOP( "Attachment Flags" ) },
	{ 0x3A00, I18N_NOOP( "Account" ) },
	{ 0x3A05, I18N_NOOP( "Generation" ) },
	{ 0x3A06, I18N_NOOP( "Given Name" ) },
	{ 0x3A0A, I18N_NOOP( "Initials" ) },
	{ 0x3A0B, I18N_NOOP( "Keyword" ) },
	{ 0x3A0C, I18N_NOOP( "Language" ) },
	{ 0x3A0D, I18N_NOOP( "Location" ) },
	{ 0x3A11, I18N_NOOP( "Surname" ) },
	{ 0x3A16, I18N_NOOP( "Company Name" ) },
	{ 0x3A17, I18N_NOOP( "Title" ) },
	{ 0x3A18, I18N_NOOP( "Department Name" ) },
	{ 0x3A26, I18N_NOOP( "Country" ) },
	{ 0x3A27, I18N_NOOP( "Locality" ) },
	{ 0x3A28, I18N_NOOP( "State/Province" ) },
	{ 0x3A44, I18N_NOOP( "Middle Name" ) },
	{ 0x3A45, I18N_NOOP( "Display Name Prefix" ) },

	/* Some TNEF attributes */
	{ 0x0008, I18N_NOOP( "Owner Appointment ID" ) },
	{ 0x0009, I18N_NOOP( "Response Requested" ) },
	{ 0x8000, I18N_NOOP( "From" ) },
	{ 0x8004, I18N_NOOP( "Subject" ) },
	{ 0x8005, I18N_NOOP( "Date Sent" ) },
	{ 0x8006, I18N_NOOP( "Date Received" ) },
	{ 0x8007, I18N_NOOP( "Message Status" ) },
	{ 0x8008, I18N_NOOP( "Message Class" ) },
	{ 0x8009, I18N_NOOP( "Message ID" ) },
	{ 0x800A, I18N_NOOP( "Parent ID" ) },
	{ 0x800B, I18N_NOOP( "Conversation ID" ) },
	{ 0x800C, I18N_NOOP( "Body" ) },
	{ 0x800D, I18N_NOOP( "Priority" ) },
	{ 0x800F, I18N_NOOP( "Attachment Data" ) },
	{ 0x8010, I18N_NOOP( "Attachment Title" ) },
	{ 0x8011, I18N_NOOP( "Attachment Meta File" ) },
	{ 0x8012, I18N_NOOP( "Attachment Create Date" ) },
	{ 0x8013, I18N_NOOP( "Attachment Modify Date" ) },
	{ 0x8020, I18N_NOOP( "Date Modified" ) },
	{ 0x9001, I18N_NOOP( "Attachment Transport File Name" ) },
	{ 0x9002, I18N_NOOP( "Attachment Rendering Data" ) },
	{ 0x9003, I18N_NOOP( "MAPI Properties" ) },
	{ 0x9004, I18N_NOOP( "Recipients Table" ) },
	{ 0x9005, I18N_NOOP( "Attachment MAPI Properties" ) },
	{ 0x9006, I18N_NOOP( "TNEF Version" ) },
	{ 0x9007, I18N_NOOP( "OEM Code Page" ) },

	{ 0, 0 }
},
MAPI_NamedTagStrings[] = 
{
	{ 0x8005, I18N_NOOP( "Contact File Under" ) },
	{ 0x8017, I18N_NOOP( "Contact Last Name And First Name" ) },
	{ 0x8018, I18N_NOOP( "Contact Company And Full Name" ) },
  
	{ 0x8080, I18N_NOOP( "Contact EMail-1 Full" ) },
	{ 0x8082, I18N_NOOP( "Contact EMail-1 Address Type" ) },
	{ 0x8083, I18N_NOOP( "Contact EMail-1 Address" ) },
	{ 0x8084, I18N_NOOP( "Contact EMail-1 Display Name" ) },
	{ 0x8085, I18N_NOOP( "Contact EMail-1 Entry ID" ) },
  
	{ 0x8090, I18N_NOOP( "Contact EMail-2 Full" ) },
	{ 0x8092, I18N_NOOP( "Contact EMail-2 Address Type" ) },
	{ 0x8093, I18N_NOOP( "Contact EMail-2 Address" ) },
	{ 0x8094, I18N_NOOP( "Contact EMail-2 Display Name" ) },
	{ 0x8095, I18N_NOOP( "Contact EMail-2 Entry ID" ) },
  
	{ 0x8208, I18N_NOOP( "Appointment Location" ) },
	{ 0x8208, I18N_NOOP( "Appointment Location" ) },
	{ 0x820D, I18N_NOOP( "Appointment Start Date" ) },
	{ 0x820E, I18N_NOOP( "Appointment End Date" ) },
	{ 0x8213, I18N_NOOP( "Appointment Duration" ) },
	{ 0x8218, I18N_NOOP( "Appointment Response Status" ) },
	{ 0x8223, I18N_NOOP( "Appointment Is Recurring" ) },
	{ 0x8231, I18N_NOOP( "Appointment Recurrence Type" ) },
	{ 0x8232, I18N_NOOP( "Appointment Recurrence Pattern" ) },
	{ 0x8502, I18N_NOOP( "Reminder Time" ) },
	{ 0x8503, I18N_NOOP( "Reminder Set" ) },
	{ 0x8516, I18N_NOOP( "Start Date" ) },
	{ 0x8517, I18N_NOOP( "End Date" ) },
	{ 0x8560, I18N_NOOP( "Reminder Next Time" ) },
	{ 0, 0 }
};
static QMap<int,QString> MAPI_TagMap;
static QMap<int,QString> MAPI_NamedTagMap;

QString mapiTagString( int key )
{
	if ( MAPI_TagMap.count() == 0 )
	{
		for ( int i=0; MAPI_TagStrings[ i ].str; i++ )
			MAPI_TagMap[ MAPI_TagStrings[ i ].tag ] = MAPI_TagStrings[ i ].str;
	}
	QMap<int,QString>::ConstIterator it = MAPI_TagMap.find( key );
	if ( it == MAPI_TagMap.end() )
		return QString().sprintf( "0x%04X", key );
	else
		return QString().sprintf( "0x%04X ________: ", key ) + *it;
}

QString mapiNamedTagString( int key, int tag )
{
	if ( MAPI_NamedTagMap.count() == 0 )
	{
		for ( int i=0; MAPI_NamedTagStrings[ i ].str; i++ )
			MAPI_NamedTagMap[ MAPI_NamedTagStrings[ i ].tag ] = MAPI_NamedTagStrings[ i ].str;
	}
	QMap<int,QString>::ConstIterator it = MAPI_NamedTagMap.find( key );
	if ( it == MAPI_NamedTagMap.end() )
		if ( tag >= 0 )
			return QString().sprintf( "0x%04X [0x%04X]: ", tag, key ) + *it;
		else
			return QString().sprintf( "0x%04X ________:", key ) + *it;
	else
		return *it;
}
