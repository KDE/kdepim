/* KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
**
** This is a C++ wrapper for the pilot's address database structures.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
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

#include <qtextcodec.h>

#include "pilotAddress.h"


#define MAXFIELDS 19

PilotAddress::PilotAddress(struct AddressAppInfo &appInfo,
	PilotRecord * rec) :
	PilotAppCategory(rec),
	fAppInfo(appInfo),
	fAddressInfo()
{
	FUNCTIONSETUPL(4);
	if (rec) unpack_Address(&fAddressInfo, (unsigned char *) rec->getData(), rec->getLen());
	(void) pilotadress_id;
}

PilotAddress::PilotAddress(struct AddressAppInfo &appInfo) :
	PilotAppCategory(),
	fAppInfo(appInfo)
{
	FUNCTIONSETUPL(4);
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
	FUNCTIONSETUPL(4);
	_copyAddressInfo(copyFrom.fAddressInfo);
}

PilotAddress & PilotAddress::operator = (const PilotAddress & copyFrom)
{
	FUNCTIONSETUPL(4);
	PilotAppCategory::operator = (copyFrom);
	_copyAddressInfo(copyFrom.fAddressInfo);
	return *this;
}

bool PilotAddress::operator==(const PilotAddress &compareTo)
{
	FUNCTIONSETUPL(4);
	// TODO: call == of PilotAppCategory. I don't think this is necessary, but I'm not so sure...
//	if (!(PilotAppCategory)(this)->operator==(compareTo) ) return false;

	// now compare all the fields stored in the fAddressInfo.entry array of char*[19]
	for (int i=0; i<MAXFIELDS; i++) {
		// if one is NULL, and the other non-empty, they are not equal for sure
		if ( !getFieldP(i) && compareTo.getFieldP(i)) return false;
		if ( getFieldP(i) && !compareTo.getFieldP(i)) return false;
		// test for getField(i)!=... to prevent strcmp or NULL strings!  None or both can be zero, but not a single one.
		if ( (getFieldP(i) != compareTo.getFieldP(i)) && ( strcmp(getFieldP(i), compareTo.getFieldP(i)) ) )  return false;
	}
	return true;
}


void PilotAddress::_copyAddressInfo(const struct Address &copyFrom)
{
	FUNCTIONSETUPL(4);
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
	FUNCTIONSETUPL(4);
	free_Address(&fAddressInfo);
}

QString PilotAddress::getTextRepresentation(bool richText)
{
	QString text, tmp;

	QString par = richText?CSL1("<p>"):CSL1("");
	QString ps = richText?CSL1("</p>"):CSL1("\n");
	QString br = richText?CSL1("<br/>"):CSL1("\n");

	// title + name
	text += par;
	if (!getField(entryTitle).isEmpty())
	{
		text += rtExpand(getField(entryTitle), richText);
		text += CSL1(" ");
	}

	tmp = richText?CSL1("<b><big>%1%2%3</big></b>"):CSL1("%1%2%3");
	if (!getField(entryFirstname).isEmpty())
		tmp=rtExpand(tmp.arg(getField(entryFirstname)), richText).arg(CSL1(" "));
	else
		tmp=tmp.arg(CSL1(" ")).arg(CSL1(" "));
	tmp=tmp.arg(rtExpand(getField(entryLastname), richText));
	text += tmp;
	text += ps;

	// company
	if (!getField(entryCompany).isEmpty())
	{
		text += par;
		text += rtExpand(getField(entryCompany), richText);
		text += ps;
	}

	// phone numbers (+ labels)
	text += par;
	for (int i = entryPhone1; i <= entryPhone5; i++)
		if (!getField(i).isEmpty())
		{
			if (richText)
			{
				if (getShownPhone() == i - entryPhone1)
					tmp=CSL1("<small>%1: </small><b>%2</b>");
				else
					tmp=CSL1("<small>%1: </small>%2");
			}
			else
				tmp=CSL1("%1: %2");
			tmp=tmp.arg(PilotAppCategory::codec()->toUnicode(
				fAppInfo.phoneLabels[getPhoneLabelIndex(i-entryPhone1)]));
			tmp=tmp.arg(rtExpand(getField(i), richText));
			text += tmp;
			text += br;
		}
	text += ps;

	// address, city, state, country
	text += par;
	if (!getField(entryAddress).isEmpty())
	{
		text += rtExpand(getField(entryAddress), richText);
		text += br;
	}
	if (!getField(entryCity).isEmpty())
	{
		text += rtExpand(getField(entryCity), richText);
		text += CSL1(" ");
	}
	if (!getField(entryState).isEmpty())
	{
		text += rtExpand(getField(entryState), richText);
		text += CSL1(" ");
	}
	if (!getField(entryZip).isEmpty())
	{
		text += rtExpand(getField(entryZip), richText);
	}
	text += br;
	if (!getField(entryCountry).isEmpty())
	{
		text += rtExpand(getField(entryCountry), richText);
		text += br;
	}
	text += ps;

	// custom fields
	text += par;
	for (int i = entryCustom1; i <= entryCustom4; i++)
		if (!getField(i).isEmpty())
		{
			text += rtExpand(getField(i), richText);
			text += br;
		}
	text += ps;

	// note
	if (!getField(entryNote).isEmpty())
	{
		text += richText?CSL1("<hr/>"):CSL1("-----------------------------\n");
		text += par;
		text += rtExpand(getField(entryNote), richText);
		text += ps;
	}
	return text;
}

QString PilotAddress::getCategoryLabel() const
{
	int cat(getCat());
	if (cat>0) return codec()->toUnicode(fAppInfo.category.name[cat]);
	else return QString::null;
}

QString PilotAddress::getField(int field) const
{
	return codec()->toUnicode(fAddressInfo.entry[field]);
}

int PilotAddress::_getNextEmptyPhoneSlot() const
{
	FUNCTIONSETUPL(4);
	for (int phoneSlot = entryPhone1; phoneSlot <= entryPhone5;
		phoneSlot++)
	{
		QString phoneField = getField(phoneSlot);

		if (phoneField.isEmpty())
			return phoneSlot;
	}
	return entryCustom4;
}

void PilotAddress::setPhoneField(EPhoneType type, const QString &field,
	bool overflowCustom)
{
	FUNCTIONSETUPL(4);
	// first look to see if the type is already assigned to a fieldSlot
	//QString typeStr(_typeToStr(type));
	//int appPhoneLabelNum = _getAppPhoneLabelNum(typeStr);
	int appPhoneLabelNum = (int) type;
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
			QString typeStr(
				codec()->toUnicode(fAppInfo.phoneLabels[appPhoneLabelNum]));

			custom4Field += typeStr + CSL1(" ") + fieldStr;
			setField(entryCustom4, custom4Field);
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
	FUNCTIONSETUPL(4);
	for (int index = 0; index < 5; index++)
	{
		if (fAddressInfo.phoneLabel[index] == appTypeNum)
			return index + entryPhone1;
	}

	return -1;
}

QString PilotAddress::getPhoneField(EPhoneType type, bool checkCustom4) const
{
	FUNCTIONSETUPL(4);
	// given the type, need to find which slot is associated with it
	//QString typeToStr(_typeToStr(type));
	//int appTypeNum = _getAppPhoneLabelNum(typeToStr);
	int appTypeNum = (int) type;

	int fieldSlot = _findPhoneFieldSlot(appTypeNum);

	if (fieldSlot != -1)
		return getField(fieldSlot);

	// look through custom 4 for the field
	if (!checkCustom4)
		return QString::null;

	// look for the phone type str
	QString typeToStr(codec()->toUnicode(fAppInfo.phoneLabels[appTypeNum]));
	QString customField(getField(entryCustom4));
	int foundField = customField.find(typeToStr);

	if (foundField == -1)
		return QString::null;

	// parse out the next token
	int startPos = foundField + typeToStr.length() + 1;
	int endPos = customField.find(' ', startPos);

	if (endPos == -1)
		endPos = customField.length();
	QString field = customField.mid(startPos, endPos);

	field = field.simplifyWhiteSpace();

	// return the token
	return field;
}


int PilotAddress::_getAppPhoneLabelNum(const QString & phoneType) const
{
	FUNCTIONSETUPL(4);
	for (int index = 0; index < 8; index++)
	{
		if (phoneType == codec()->toUnicode(fAppInfo.phoneLabels[index]))
			return index;
	}

	return -1;
}

void PilotAddress::setShownPhone(EPhoneType type)
{
	FUNCTIONSETUPL(4);
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

void PilotAddress::setField(int field, const QString &text)
{
	FUNCTIONSETUPL(4);
	// This will have either been created with unpack_Address, and/or will
	// be released with free_Address, so use malloc/free here:
	if (fAddressInfo.entry[field])
	{
		free(fAddressInfo.entry[field]);
		fAddressInfo.entry[field]=0L;
	}
	if (!text.isEmpty())
	{
		fAddressInfo.entry[field] = (char *) malloc(text.length() + 1);
		strlcpy(fAddressInfo.entry[field], codec()->fromUnicode(text), text.length() + 1);
	}
	else
	{
		fAddressInfo.entry[field] = 0L;
	}
}

void *PilotAddress::pack(void *buf, int *len)
{
	FUNCTIONSETUPL(4);
	int i;

	i = pack_Address(&fAddressInfo, (unsigned char *) buf, *len);
	*len = i;
	return buf;
}

