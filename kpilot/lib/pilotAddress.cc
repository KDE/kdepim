/* KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
** Copyright (C) 2007 by Adriaan de Groot <groot@kde.org>
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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/


#include "options.h"


#include <stdlib.h>
#include <assert.h>

#include <qnamespace.h>
#include <qstringlist.h>

#include "pilotAddress.h"

static const char *default_address_category_names[] = {
	"Unfiled",
	"Business",
	"Personal",
	"Quicklist",
	0L
} ;

static const char *default_address_field_labels[] = {
	"Last name",
	"First name",
	"Company",
	"Work",
	"Home",
	"Fax",
	"Other",
	"E-mail",
	"Addr(W)",
	"City",
	"State",
	"Zip Code",
	"Country",
	"Title",
	"Custom 1",
	"Custom 2",
	"Custom 3",
	"Custom 4",
	"Note",
	0L
} ;

void PilotAddressInfo::resetToDefault()
{
	FUNCTIONSETUP;
	// Reset to all 0s
	memset(&fInfo,0,sizeof(fInfo));
	// Fill up default categories
	for (unsigned int i=0; (i<4) && default_address_category_names[i]; ++i)
	{
		strncpy(fInfo.category.name[i],default_address_category_names[i],sizeof(fInfo.category.name[0]));
	}
	// Weird hack, looks like there's an extra copy of Unfiled
	strncpy(fInfo.category.name[15],default_address_category_names[0],sizeof(fInfo.category.name[0]));

	// And fill up the default labels.
	for (unsigned int i=0; (i<19) && default_address_field_labels[i]; ++i)
	{
		strncpy(fInfo.labels[i],default_address_field_labels[i],sizeof(fInfo.labels[0]));
	}
}

QString PilotAddressInfo::phoneLabel(EPhoneType i) const
{
	if (i<=eMobile)
	{
		return Pilot::fromPilot(info()->phoneLabels[i]);
	}
	else
	{
		return QString();
	}
}

PhoneSlot::PhoneSlot( const int v )
{
	i = entryPhone1;
	operator=(v);
}

const PhoneSlot &PhoneSlot::operator=( const int &v )
{
	if ( (entryPhone1 <= v) && (v <= entryPhone5) )
	{
		i = v;
	}
	else
	{
		i = invalid;
	}
	return *this;
}

const PhoneSlot &PhoneSlot::operator++()
{
	if ( (i!=invalid) && (i<entryPhone5) )
	{
		++i;
	}
	else
	{
		i = invalid;
	}
	return *this;
}

/* static */ const PhoneSlot PhoneSlot::begin()
{
	return PhoneSlot( entryPhone1 );
}

/* static */ const PhoneSlot PhoneSlot::end()
{
	return PhoneSlot( invalid );
}

unsigned int PhoneSlot::toOffset() const
{
	if ( isValid() )
	{
		return i-entryPhone1;
	}
	else
	{
		return 0;
	}
}

unsigned int PhoneSlot::toField() const
{
	if ( isValid() )
	{
		return i;
	}
	else
	{
		return entryPhone1;
	}
}

PhoneSlot::operator QString() const
{
	return QString("%1,%2").arg(toOffset()).arg(toField());
}

#define MAXFIELDS 19

PilotAddress::PilotAddress(PilotRecord *rec) :
	PilotRecordBase(rec),
	fAddressInfo()
{
	FUNCTIONSETUPL(4);
	memset(&fAddressInfo,0,sizeof(fAddressInfo));

	if (rec)
	{
		pi_buffer_t b;
		b.data = (unsigned char *) rec->data();
		b.allocated = b.used = rec->size();
		unpack_Address(&fAddressInfo, &b, address_v1);
	}
	else
	{
		fAddressInfo.phoneLabel[0] = (int) PilotAddressInfo::eWork;
		fAddressInfo.phoneLabel[1] = (int) PilotAddressInfo::eHome;
		fAddressInfo.phoneLabel[2] = (int) PilotAddressInfo::eOther;
		fAddressInfo.phoneLabel[3] = (int) PilotAddressInfo::eMobile;
		fAddressInfo.phoneLabel[4] = (int) PilotAddressInfo::eEmail;
	}
}

PilotAddress::PilotAddress(const PilotAddress & copyFrom) :
	PilotRecordBase(copyFrom),
	fAddressInfo()
{
	FUNCTIONSETUPL(4);
	_copyAddressInfo(copyFrom.fAddressInfo);
}

PilotAddress & PilotAddress::operator = (const PilotAddress & copyFrom)
{
	FUNCTIONSETUPL(4);
	PilotRecordBase::operator = (copyFrom);
	_copyAddressInfo(copyFrom.fAddressInfo);
	return *this;
}

bool PilotAddress::operator==(const PilotAddress &compareTo)
{
	FUNCTIONSETUPL(4);

	// now compare all the fields stored in the fAddressInfo.entry array of char*[19]
	for (int i=0; i<MAXFIELDS; i++) {
		// if one is NULL, and the other non-empty, they are not equal for sure
		if ( !getFieldP(i) && compareTo.getFieldP(i))
		{
			return false;
		}
		if ( getFieldP(i) && !compareTo.getFieldP(i))
		{
			return false;
		}

		// test for getField(i)!=... to prevent strcmp or NULL strings!  None or both can be zero, but not a single one.
		if ( (getFieldP(i) != compareTo.getFieldP(i)) && ( strcmp(getFieldP(i), compareTo.getFieldP(i)) ) )
		{
			return false;
		}
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

	for (unsigned int i = 0; i< MAXFIELDS; ++i)
	{
		if (copyFrom.entry[i])
		{
			fAddressInfo.entry[i] = qstrdup(copyFrom.entry[i]);
		}
		else
		{
			fAddressInfo.entry[i] = 0L;
		}
	}
}


PilotAddress::~PilotAddress()
{
	FUNCTIONSETUPL(4);
	free_Address(&fAddressInfo);
}

QString PilotAddress::getTextRepresentation(const PilotAddressInfo *info, Qt::TextFormat richText) const
{
	QString text, tmp;

	QString par = (richText==Qt::RichText) ?CSL1("<p>"): QString();
	QString ps = (richText==Qt::RichText) ?CSL1("</p>"):CSL1("\n");
	QString br = (richText==Qt::RichText) ?CSL1("<br/>"):CSL1("\n");

	// title + name
	text += par;
	if (!getField(entryTitle).isEmpty())
	{
		text += rtExpand(getField(entryTitle), richText);
		text += CSL1(" ");
	}

	tmp = richText ? CSL1("<b><big>%1 %2</big></b>") : CSL1("%1 %2");
	QString firstName = getField(entryFirstname);
	if (firstName.isEmpty())
	{
		// So replace placeholder for first name (%1) with empty
		tmp = tmp.arg(QString());
	}
	else
	{
		tmp = tmp.arg(rtExpand(firstName,richText));
	}
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
	for ( PhoneSlot i = PhoneSlot::begin(); i.isValid(); ++i )
	{
		if (!getField(i.toField()).isEmpty())
		{
			if (richText)
			{
				if (getShownPhone() == i)
				{
					tmp=CSL1("<small>%1: </small><b>%2</b>");
				}
				else
				{
					tmp=CSL1("<small>%1: </small>%2");
				}
			}
			else
			{
				tmp=CSL1("%1: %2");
			}
			if (info)
			{
				tmp=tmp.arg(info->phoneLabel( getPhoneType( i ) ));
			}
			else
			{
				tmp=tmp.arg(CSL1("Contact: "));
			}
			tmp=tmp.arg(rtExpand(getField(i.toField()), richText));
			text += tmp;
			text += br;
		}
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
	{
		if (!getField(i).isEmpty())
		{
			text += rtExpand(getField(i), richText);
			text += br;
		}
	}
	text += ps;

	// category
	if (info)
	{
		QString categoryName = info->categoryName( category() );
		if (!categoryName.isEmpty())
		{
			text += par;
			text += rtExpand(categoryName, richText);
			text += ps;
		}
	}

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

QStringList PilotAddress::getEmails() const
{
	QStringList list;

	for ( PhoneSlot i = PhoneSlot::begin(); i.isValid(); ++i)
	{
		PilotAddressInfo::EPhoneType t = getPhoneType( i );
		if ( t == PilotAddressInfo::eEmail )
		{
			QString s = getField(i.toField());
			if (!s.isEmpty())
			{
				list.append(s);
			}
		}
	}

	return list;
}

void PilotAddress::setEmails(const QStringList &list)
{
	FUNCTIONSETUPL(4);
	QString test;

	// clear all e-mails first
	for ( PhoneSlot i = PhoneSlot::begin(); i.isValid(); ++i )
	{
		PilotAddressInfo::EPhoneType t = getPhoneType( i );
		if (t == PilotAddressInfo::eEmail)
		{
			setField(i.toField(), QString() );
		}
	}

	for(QStringList::ConstIterator listIter = list.begin();
		   listIter != list.end(); ++listIter)
	{
		QString email = *listIter;
		if (!setPhoneField(PilotAddressInfo::eEmail, email, NoFlags).isValid())
		{
			WARNINGKPILOT << "Email accounts overflowed, silently dropped." << endl;
		}
	}
}

QString PilotAddress::getField(int field) const
{
	if ( (entryLastname <= field) && (field <= entryNote) )
	{
		return Pilot::fromPilot(fAddressInfo.entry[field]);
	}
	else
	{
		return QString();
	}
}

PhoneSlot PilotAddress::_getNextEmptyPhoneSlot() const
{
	FUNCTIONSETUPL(4);
	for (PhoneSlot i = PhoneSlot::begin(); i.isValid(); ++i)
	{
		const char *phoneField = getFieldP(i.toField());

		if (!phoneField || !phoneField[0])
		{
			return i;
		}
	}
	return PhoneSlot();
}

PhoneSlot PilotAddress::setPhoneField(PilotAddressInfo::EPhoneType type,
	const QString &field,
	PhoneHandlingFlags flags)
{
	FUNCTIONSETUPL(4);

	const bool overwriteExisting = (flags == Replace);
	PhoneSlot fieldSlot;
	if (overwriteExisting)
	{
		fieldSlot = _findPhoneFieldSlot(type);
	}

	if ( !fieldSlot.isValid() )
	{
		fieldSlot = _getNextEmptyPhoneSlot();
	}

	// store the overflow phone
	if ( !fieldSlot.isValid() )
	{
		DEBUGKPILOT << fname << ": Phone would overflow." << endl;
	}
	else			// phone field 1 - 5; straight forward storage
	{
		setField(fieldSlot.toField(), field);
		fAddressInfo.phoneLabel[fieldSlot.toOffset()] = (int) type;
	}
	return fieldSlot;
}

PhoneSlot PilotAddress::_findPhoneFieldSlot(PilotAddressInfo::EPhoneType t) const
{
	FUNCTIONSETUPL(4);
	for ( PhoneSlot i = PhoneSlot::begin(); i.isValid(); ++i )
	{
		if ( getPhoneType(i) == t )
		{
			return i;
		}
	}

	return PhoneSlot();
}

QString PilotAddress::getPhoneField(PilotAddressInfo::EPhoneType type) const
{
	FUNCTIONSETUPL(4);
	PhoneSlot fieldSlot = _findPhoneFieldSlot(type);

	if (fieldSlot.isValid())
	{
		return getField(fieldSlot.toField());
	}

	return QString();
}

PhoneSlot PilotAddress::getShownPhone() const
{
	// The slot is stored as an offset
	return PhoneSlot(entryPhone1 + fAddressInfo.showPhone);
}

const PhoneSlot &PilotAddress::setShownPhone( const PhoneSlot &v )
{
	FUNCTIONSETUPL(4);
	if (v.isValid())
	{
		fAddressInfo.showPhone = v.toOffset();
	}
	return v;
}

PhoneSlot PilotAddress::setShownPhone(PilotAddressInfo::EPhoneType type)
{
	FUNCTIONSETUPL(4);
	PhoneSlot fieldSlot = _findPhoneFieldSlot(type);

	// Did we find a slot with the requested type?
	if (!fieldSlot.isValid())
	{
		// No, so look for first non-empty phone slot
		for ( fieldSlot = PhoneSlot::begin(); fieldSlot.isValid(); ++fieldSlot )
		{
			const char *p = getFieldP(fieldSlot.toField());
			if (p && p[0])
			{
				break;
			}
		}
		// If all of them are empty, then use first slot instead
		if (!fieldSlot.isValid())
		{
			fieldSlot = PhoneSlot::begin();
		}
	}
	setShownPhone(fieldSlot);
	return fieldSlot;
}

PilotAddressInfo::EPhoneType PilotAddress::getPhoneType( const PhoneSlot &field ) const
{
	if ( field.isValid() )
	{
		return (PilotAddressInfo::EPhoneType) fAddressInfo.phoneLabel[field.toOffset()];
	}
	else
	{
		return PilotAddressInfo::eNone;
	}
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
		Pilot::toPilot(text, fAddressInfo.entry[field], text.length()+1);
	}
	else
	{
		fAddressInfo.entry[field] = 0L;
	}
}

PilotRecord *PilotAddress::pack() const
{
	FUNCTIONSETUPL(4);
	int i;

	pi_buffer_t *b = pi_buffer_new( sizeof(fAddressInfo) );
	i = pack_Address(const_cast<Address_t *>(&fAddressInfo), b, address_v1);
	if (i<0)
	{
		return 0L;
	}
	// pack_Address sets b->used
	return new PilotRecord( b, this );
}
