/*
	libvcard - vCard parsing library for vCard version 3.0

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

#include <qcstring.h>

#include <Enum.h>

using namespace VCARD;

// There are 31 possible types, not including extensions.
const QCString paramNames [] =
{
	"NAME",
	"PROFILE",
	"SOURCE",
	"FN",
	"N",
	"NICKNAME",
	"PHOTO",
	"BDAY",
	"ADR",
	"LABEL",
	"TEL",
	"EMAIL",
	"MAILER",
	"TZ",
	"GEO",
	"TITLE",
	"ROLE",
	"LOGO",
	"AGENT",
	"ORG",
	"CATEGORIES",
	"NOTE",
	"PRODID",
	"REV",
	"SORT-STRING",
	"SOUND",
	"UID",
	"URL",
	"VERSION",
	"CLASS",
	"KEY"
};

const ParamType paramTypesTable[] = {
	ParamNone,		// NAME
	ParamNone,		// PROFILE
	ParamSource,	// SOURCE
	ParamText,		// FN
	ParamText,		// N
	ParamText,		// NICKNAME
	ParamImage,		// PHOTO (inline/refer)
	ParamDate,		// BDAY ("VALUE = "date-time/date)
	ParamAddrText,	// ADR (adr-param/text-param)
	ParamAddrText,	// LABEL (adr-param/text-param)
	ParamTel,		// TEL
	ParamEmail,		// EMAIL
	ParamText,		// MAILER
	ParamNone,		// TZ
	ParamNone,		// GEO
	ParamText,		// TITLE
	ParamText,		// ROLE
	ParamImage,		// LOGO
	ParamAgent,		// AGENT
	ParamText,		// ORG
	ParamText,		// CATEGORIES
	ParamText,		// NOTE
	ParamNone,		// PRODID
	ParamDate,		// REV
	ParamText,		// SORT-STRING
	ParamSound,		// SOUND
	ParamNone,		// UID
	ParamNone,		// URL
	ParamNone,		// VERSION
	ParamNone,		// CLASS
	ParamTextBin,	// KEY
	ParamTextNS		// X
};

	ParamType
EntityTypeToParamType(EntityType e)
{
	ParamType t(ParamUnknown);

	switch (e) {
	
	//---------------------------------------------------------------//	
		case EntityAgent:		t = ParamAgent;		break;
	//---------------------------------------------------------------//	
		case EntitySound:		t = ParamSound;		break;
	//---------------------------------------------------------------//	
		case EntitySource:		t = ParamSource;	break;
	//---------------------------------------------------------------//	
		case EntityTelephone:	t = ParamTel;		break;
	//---------------------------------------------------------------//	
		case EntityEmail:		t = ParamEmail;		break;
	//---------------------------------------------------------------//	
		case EntityKey:			t = ParamTextBin;	break;
	//---------------------------------------------------------------//	
		case EntityExtension:	t = ParamTextNS;	break;
	//---------------------------------------------------------------//	
		case EntityAddress:
		case EntityLabel:		t = ParamAddrText;	break;
	//---------------------------------------------------------------//	
		case EntityBirthday:
		case EntityRevision:	t = ParamDate;		break;
	//---------------------------------------------------------------//	
		case EntityPhoto:
		case EntityLogo:		t = ParamImage;		break;
	//---------------------------------------------------------------//	
		case EntityOrganisation:
		case EntityTitle:
		case EntityRole:
		case EntityFullName:
		case EntityMailer:
		case EntityN:
		case EntitySortString:
		case EntityNickname:
		case EntityCategories:
		case EntityNote:		t = ParamText;		break;
	//---------------------------------------------------------------//		
		case EntityProductID:
		case EntityTimeZone:
		case EntityUID:
		case EntityURL:
		case EntityClass:
		case EntityGeo:
		case EntityName:
		case EntityVersion:
		case EntityProfile:
		default:				t = ParamNone;				break;
	//---------------------------------------------------------------//	

	}
	
	return t;
}

	ValueType
EntityTypeToValueType(EntityType e)
{
	ValueType t(ValueUnknown);

	switch (e) {
	
	//---------------------------------------------------------------//	
		case EntitySound:		t = ValueSound;		break;
	//---------------------------------------------------------------//	
		case EntityAgent:		t = ValueAgent;		break;
	//---------------------------------------------------------------//	
		case EntityAddress:		t = ValueAddress;	break;
	//---------------------------------------------------------------//	
		case EntityTelephone:	t = ValueTel;		break;
	//---------------------------------------------------------------//	
		case EntityKey:			t = ValueTextBin;	break;
	//---------------------------------------------------------------//	
		case EntityOrganisation:t = ValueOrg;		break;
	//---------------------------------------------------------------//	
		case EntityN:			t = ValueN;			break;
	//---------------------------------------------------------------//	
		case EntityTimeZone:	t = ValueUTC;		break;
	//---------------------------------------------------------------//		
		case EntityClass:		t = ValueClass;		break;
	//---------------------------------------------------------------//		
		case EntityGeo:			t = ValueFloat;		break;
	//---------------------------------------------------------------//		
		case EntitySource:
		case EntityURL:			t = ValueURI;		break;
	//---------------------------------------------------------------//		
		case EntityPhoto:
		case EntityLogo:		t = ValueImage;		break;
	//---------------------------------------------------------------//	
		case EntityBirthday:
		case EntityRevision:	t = ValueDate;		break;
	//---------------------------------------------------------------//	
		case EntityCategories:
		case EntityNickname:	t = ValueTextList;	break;
	//---------------------------------------------------------------//	
		case EntityLabel:
		case EntityExtension:
		case EntityEmail:
		case EntityTitle:
		case EntityRole:
		case EntityFullName:
		case EntityMailer:
		case EntityProductID:
		case EntityName:
		case EntitySortString:
		case EntityVersion:
		case EntityProfile:
		case EntityUID:
		case EntityNote:
		default:				t = ValueText;		break;
	//---------------------------------------------------------------//	

	}
	
	return t;
}

	EntityType
ParamNameToEntityType(const QCString & s)
{
	if (s.isEmpty()) return EntityUnknown;
	
	EntityType t(EntityUnknown);
	
	switch (s[0]) {

		case 'A':
			if (s == "ADR")
				t = EntityAddress;
			else if (s == "AGENT")
				t = EntityAgent;
			break;

		case 'B':
			if (s == "BDAY")
				t = EntityBirthday;
			break;

		case 'C':
			if (s == "CATEGORIES")
				t = EntityCategories;
			else if (s == "CLASS")
				t = EntityClass;
			break;

		case 'E':
			if (s == "EMAIL")
				t = EntityEmail;
			break;

		case 'F':
			if (s == "FN")
				t = EntityFullName;
			break;

		case 'G':
			if (s == "GEO")
				t = EntityGeo;
			break;

		case 'K':
			if (s == "KEY")
				t = EntityKey;
			break;

		case 'L':
			if (s == "LABEL")
				t = EntityLabel;
			else if (s == "LOGO")
				t = EntityLogo;
			break;

		case 'M':
			if (s == "MAILER")
				t = EntityMailer;
			break;
			
		case 'N':
			if (s == "N")
				t = EntityN;
			else if (s == "NAME")
				t = EntityName;
			else if (s == "NICKNAME")
				t = EntityNickname;
			break;

		case 'O':
			if (s == "ORG")
				t = EntityOrganisation;
			break;

		case 'P':
			if (s == "PHOTO")
				t = EntityPhoto;
			else if (s == "PRODID")
				t = EntityProductID;
			else if (s == "PROFILE")
				t = EntityProfile;
			break;
		
		case 'R':
			if (s == "REV")
				t = EntityRevision;
			else if (s == "ROLE")
				t = EntityRole;
			break;
			
		case 'S':
			if (s == "SORT-STRING")
				t = EntitySortString;
			else if (s == "SOUND")
				t = EntitySound;
			else if (s == "SOURCE")
				t = EntitySource;
			break;

		case 'T':
			if (s == "TEL")
				t = EntityTelephone;
			else if (s == "TITLE")
				t = EntityTitle;
			else if (s == "TZ")
				t = EntityTimeZone;
			break;

		case 'U':
			if (s == "UID")
				t = EntityUID;
			else if (s == "URL")
				t = EntityURL;
		case 'V':
			if (s == "VERSION")
				t = EntityVersion;
			break;

		case 'X':
			if (s.left(2) == "X-")
				t = EntityExtension;
			break;
			
		default:
			
			t = EntityUnknown;
	}
	
	return t;
}

