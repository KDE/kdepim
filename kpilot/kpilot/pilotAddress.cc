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
** Bug reports and questions can be sent to adridg@cs.kun.nl
*/


static const char *pilotadress_id="$Id$";

#include "options.h"

#include <stdlib.h>
#include <assert.h>

#ifndef _KPILOT_PILOTADDRESS_H
#include "pilotAddress.h"
#endif


const int PilotAddress::APP_BUFFER_SIZE = 0xffff;

PilotAddress::PilotAddress(const struct AddressAppInfo &appInfo,
			   PilotRecord* rec)
      : PilotAppCategory(rec), fAppInfo(appInfo)
    {
    unpack_Address(&fAddressInfo, (unsigned char*)rec->getData(), rec->getLen());
    (void)pilotadress_id;
    }

QString PilotAddress::_typeToStr(EPhoneType type) const
    {
    QString s;
    switch(type)
	{
	case eWork : s = "Work"; break;
	case eHome : s = "Home"; break;
	case eFax : s = "Fax"; break;
	case eOther : s = "Other"; break;
	case ePager : s = "Pager"; break;
	case eMobile : s = "Mobile"; break;
	case eEmail : s = "E-mail"; break;
	case eMain :
	default : s = "Main"; break;
	}
    return s;
    }

int PilotAddress::_getNextEmptyPhoneSlot() const
    {
    for (int phoneSlot = entryPhone1;phoneSlot <= entryPhone5;phoneSlot++)
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
    int fieldSlot = _getNextEmptyPhoneSlot();
    QString fieldStr(field);
    QString typeStr(_typeToStr(type));
    
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
    else // phone field 1 - 5; straight forward storage
	{
	setField(fieldSlot, field);
	fAddressInfo.phoneLabel[fieldSlot] = _getAppPhoneLabelNum(typeStr);
	}
    }

const char *PilotAddress::getPhoneField(EPhoneType type,
					bool checkCustom4) const
    {
    // given the type, need to find which slot is associated with it
    QString typeToStr(_typeToStr(type));
    int appTypeNum = _getAppPhoneLabelNum(typeToStr);
    for (int index=0;index < 5;index++)
	if (fAddressInfo.phoneLabel[index] == appTypeNum)
	    return getField(index+entryPhone1);

    // look through custom 4 for the field
    if (!checkCustom4)
	return 0L;

    // look for the phone type str
    QString customField(getField(entryCustom4));
    int foundField = customField.find(typeToStr);
    if (foundField == -1)
	return 0L;

    // parse out the next token
    int startPos = foundField+typeToStr.length()+1;
    int endPos = customField.find(' ', startPos);
    if (endPos == -1)
	endPos = customField.length();
    QString field = customField.mid(startPos, endPos);
    field = field.simplifyWhiteSpace();

    // return the token
    return field.latin1();
    }


int PilotAddress::_getAppPhoneLabelNum(const QString &phoneType) const
    {
    for (int index=0;index < 8;index++)
	if (phoneType == fAppInfo.phoneLabels[index])
	    return index;
    qDebug("PilotAddress::getAppPhoneLabelNum can't find index for phoneType = %s", phoneType.latin1());
    assert(0);
    return -1;
    }

void 
PilotAddress::setField(int field, const char* text)
    {
    // This will have either been created with unpack_Address, and/or will
    // be released with free_Address, so use malloc/free here:
    if(fAddressInfo.entry[field])
	{
	free(fAddressInfo.entry[field]);
	}
    if (text)
      {
	fAddressInfo.entry[field] = (char*)malloc(strlen(text) + 1);
	strcpy(fAddressInfo.entry[field], text);
      }
    else
      fAddressInfo.entry[field] = 0L;
    }

void*
PilotAddress::pack(void *buf, int *len)
    {
    int i;
    i = pack_Address(&fAddressInfo, (unsigned char*)buf, *len);
    *len = i;
    return buf;
    }

// $Log$
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
