/* pilotAddress.cc			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** This is a C++ wrapper for the pilot's address database structures.
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
** Bug reports and questions can be sent to kde-pim@kde.org
*/


static const char *pilotadress_id =
	"$Id$";

#ifndef _KPILOT_OPTIONS_H
#include "options.h"
#endif

#include <stdlib.h>
#include <assert.h>

#ifndef _KPILOT_PILOTADDRESS_H
#include "pilotAddress.h"
#endif


const int PilotAddress::APP_BUFFER_SIZE = 0xffff;

PilotAddress::PilotAddress(struct AddressAppInfo &appInfo,
	PilotRecord * rec) :
	PilotAppCategory(rec), 
	fAppInfo(appInfo),
	fAddressInfo()
{
	FUNCTIONSETUP;
	unpack_Address(&fAddressInfo, (unsigned char *) rec->getData(),
		rec->getLen());
	(void) pilotadress_id;
}

PilotAddress::PilotAddress(struct AddressAppInfo &appInfo) :
	PilotAppCategory(),
	fAppInfo(appInfo)
{
	FUNCTIONSETUP;
	reset();

	// assign the phoneLabel so it doesn't appear in the pilot as
	// work for all fields, but at least shows other fields
	fAddressInfo.phoneLabel[0] = (int) eWork;
	fAddressInfo.phoneLabel[1] = (int) eHome;
	fAddressInfo.phoneLabel[2] = (int) eOther;
	fAddressInfo.phoneLabel[3] = (int) eMobile;
	fAddressInfo.phoneLabel[4] = (int) eEmail;
}

PilotAddress::PilotAddress(const PilotAddress & copyFrom) :
	PilotAppCategory(copyFrom),
	fAppInfo(copyFrom.fAppInfo), 
	fAddressInfo()
{
	FUNCTIONSETUP;
	_copyAddressInfo(copyFrom.fAddressInfo);
}

PilotAddress & PilotAddress::operator = (const PilotAddress & copyFrom)
{
	FUNCTIONSETUP;
	PilotAppCategory::operator = (copyFrom);
	_copyAddressInfo(copyFrom.fAddressInfo);
	return *this;
}

void PilotAddress::_copyAddressInfo(const struct Address &copyFrom)
{
	FUNCTIONSETUP;
	fAddressInfo.showPhone = copyFrom.showPhone;

	for (int labelLp = 0; labelLp < 5; labelLp++)
	{
		fAddressInfo.phoneLabel[labelLp] =
			copyFrom.phoneLabel[labelLp];
	}

	for (int entryLp = 0; entryLp < 19; entryLp++)
	{
		if (copyFrom.entry[entryLp])
			fAddressInfo.entry[entryLp] =
				qstrdup(copyFrom.entry[entryLp]);
		else
			fAddressInfo.entry[entryLp] = 0L;
	}
}


PilotAddress::~PilotAddress()
{
	FUNCTIONSETUP;
	free_Address(&fAddressInfo);
}

bool PilotAddress::setCategory(const char *label)
{
	FUNCTIONSETUP;
	for (int catId = 0; catId < 16; catId++)
	{
		QString aCat = fAppInfo.category.name[catId];

		if (label == aCat)
		{
			setCat(catId);
			return true;
		}
		else
			// if empty, then no more labels; add it 
		if (aCat.isEmpty())
		{
			qstrncpy(fAppInfo.category.name[catId], label, 16);
			setCat(catId);
			return true;
		}
	}
	// if got here, the category slots were full
	return false;
}

int PilotAddress::_getNextEmptyPhoneSlot() const
{
	FUNCTIONSETUP;
	for (int phoneSlot = entryPhone1; phoneSlot <= entryPhone5;
		phoneSlot++)
	{
		QString phoneField = getField(phoneSlot);

		if (phoneField.isEmpty())
			return phoneSlot;
	}
	return entryCustom4;
}

void PilotAddress::setPhoneField(EPhoneType type, const char *field,
	bool overflowCustom)
{
	FUNCTIONSETUP;
	// first look to see if the type is already assigned to a fieldSlot
	//QString typeStr(_typeToStr(type));
	//int appPhoneLabelNum = _getAppPhoneLabelNum(typeStr);
	int appPhoneLabelNum = (int) type;
	QString typeStr(fAppInfo.phoneLabels[appPhoneLabelNum]);
	QString fieldStr(field);
	int fieldSlot = _findPhoneFieldSlot(appPhoneLabelNum);

	if (fieldSlot == -1)
		fieldSlot = _getNextEmptyPhoneSlot();

	// store the overflow phone
	if (fieldSlot == entryCustom4)
	{
		if (!fieldStr.isEmpty() && overflowCustom)
		{
			QString custom4Field = getField(entryCustom4);

			custom4Field += typeStr + " " + fieldStr;
			setField(entryCustom4, custom4Field.latin1());
		}
	}
	else			// phone field 1 - 5; straight forward storage
	{
		setField(fieldSlot, field);
		int labelIndex = fieldSlot - entryPhone1;

		fAddressInfo.phoneLabel[labelIndex] = appPhoneLabelNum;
	}
}

int PilotAddress::_findPhoneFieldSlot(int appTypeNum) const
{
	FUNCTIONSETUP;
	for (int index = 0; index < 5; index++)
	{
		if (fAddressInfo.phoneLabel[index] == appTypeNum)
			return index + entryPhone1;
	}

	return -1;
}

const char *PilotAddress::getPhoneField(EPhoneType type, bool checkCustom4) const
{
	FUNCTIONSETUP;
	// given the type, need to find which slot is associated with it
	//QString typeToStr(_typeToStr(type));
	//int appTypeNum = _getAppPhoneLabelNum(typeToStr);
	int appTypeNum = (int) type;
	QString typeToStr(fAppInfo.phoneLabels[appTypeNum]);

	int fieldSlot = _findPhoneFieldSlot(appTypeNum);

	if (fieldSlot != -1)
		return getField(fieldSlot);

	// look through custom 4 for the field
	if (!checkCustom4)
		return 0L;

	// look for the phone type str
	QString customField(getField(entryCustom4));
	int foundField = customField.find(typeToStr);

	if (foundField == -1)
		return 0L;

	// parse out the next token
	int startPos = foundField + typeToStr.length() + 1;
	int endPos = customField.find(' ', startPos);

	if (endPos == -1)
		endPos = customField.length();
	QString field = customField.mid(startPos, endPos);

	field = field.simplifyWhiteSpace();

	// return the token
	return field.latin1();
}


int PilotAddress::_getAppPhoneLabelNum(const QString & phoneType) const
{
	FUNCTIONSETUP;
	for (int index = 0; index < 8; index++)
	{
		if (phoneType == fAppInfo.phoneLabels[index])
			return index;
	}

	return -1;
}

void PilotAddress::setShownPhone(EPhoneType type)
{
	FUNCTIONSETUP;
	int appPhoneLabelNum = (int) type;
	int fieldSlot = _findPhoneFieldSlot(appPhoneLabelNum);

	if (fieldSlot == -1)
	{
		if (type != eHome)
		{
			setShownPhone(eHome);
			return;
		}
		fieldSlot = entryPhone1;
	}
	fAddressInfo.showPhone = fieldSlot - entryPhone1;
}

void PilotAddress::setField(int field, const char *text)
{
	FUNCTIONSETUP;
	// This will have either been created with unpack_Address, and/or will
	// be released with free_Address, so use malloc/free here:
	if (fAddressInfo.entry[field])
	{
		free(fAddressInfo.entry[field]);
	}
	if (text)
	{
		fAddressInfo.entry[field] = (char *) malloc(strlen(text) + 1);
		strcpy(fAddressInfo.entry[field], text);
	}
	else
	{
		fAddressInfo.entry[field] = 0L;
	}
}

void *PilotAddress::pack(void *buf, int *len)
{
	FUNCTIONSETUP;
	int i;

	i = pack_Address(&fAddressInfo, (unsigned char *) buf, *len);
	*len = i;
	return buf;
}

// $Log$
// Revision 1.21  2001/09/29 16:26:18  adridg
// The big layout change
//
// Revision 1.20  2001/05/07 22:14:47  stern
// Fixed phone localization bug
//
// Revision 1.19  2001/05/07 19:26:41  adridg
// Possible fix for abbrowser phone label corruption
//
// Revision 1.18  2001/04/16 13:54:17  adridg
// --enable-final file inclusion fixups
//
// Revision 1.17  2001/04/13 22:13:38  stern
// Added setShownPhoneField method
//
// Revision 1.16  2001/04/11 16:45:03  stern
// Fixed bug in copying an address
//
// Revision 1.15  2001/04/11 11:02:37  leitner
// A void function must not return anything. Also there was an uninitialize
// variable being used.
//
// Revision 1.14  2001/04/04 21:20:32  stern
// Added support for category information and copy constructors
//
// Revision 1.13  2001/04/02 21:56:22  stern
// Fixed bugs in getPhoneField and setPhoneField methods
//
// Revision 1.12  2001/03/29 21:40:55  stern
// Added APP_BUFFER_SIZE to pilotAddress
//
// Revision 1.11  2001/03/19 23:12:39  stern
// Made changes necessary for upcoming abbrowser conduit.
//
// Mainly, I added two public methods to PilotAddress that allow for easier
// setting and getting of phone fields.
//
// I also have added some documentation throughout as I have tried to figure
// out how everything works.
//
// Revision 1.10  2001/03/09 09:46:15  adridg
// Large-scale #include cleanup
//
// Revision 1.9  2001/02/08 08:13:44  habenich
// exchanged the common identifier "id" with source unique <sourcename>_id for --enable-final build
//
// Revision 1.8  2001/02/05 20:58:48  adridg
// Fixed copyright headers for source releases. No code changed
//
