/* pilot.cc			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
** Copyright (C) 2003-2006 Adriaan de Groot <groot@kde.org>
**
** These are the base class structures that reside on the
** handheld device -- databases and their parts.
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

#include <qtextcodec.h>
#include <qmutex.h>
#include <kcharsets.h>
#include <kglobal.h>

#include "pilot.h"
#include "pilotDatabase.h"
#include "pilotAppInfo.h"
#include "pilotRecord.h"


namespace Pilot
{
static QTextCodec *codec = 0L;
static QMutex* mutex = 0L;


QString fromPilot( const char *c, int len )
{
	mutex->lock();
	QString str = codec->toUnicode(c,len);
	mutex->unlock();
	return str;
}

QString fromPilot( const char *c )
{
	mutex->lock();
	QString str = codec->toUnicode(c);
	mutex->unlock();
	return str;
}

QCString toPilot( const QString &s )
{
	mutex->lock();
	QCString str = codec->fromUnicode(s);
	mutex->unlock();
	return str;
}

int toPilot( const QString &s, char *buf, int len)
{
	mutex->lock();
	// See toPilot() below.
	memset( buf, 0, len );
	int used = len;
	QCString cbuf = codec->fromUnicode(s,used);
	if (used > len)
	{
		used=len;
	}
	memcpy( buf, cbuf.data(), used );
	mutex->unlock();
	return used;
}

int toPilot( const QString &s, unsigned char *buf, int len)
{
	mutex->lock();
	// Clear the buffer
	memset( buf, 0, len );

	// Convert to 8-bit encoding
	int used = len;
	QCString cbuf = codec->fromUnicode(s,used);

	// Will it fit in the buffer?
	if (used > len)
	{
		// Ought to be impossible, anyway, since 8-bit encodings
		// are shorter than the UTF-8 encodings (1 byte per character
		// vs. 1-or-more byte per character).
		used=len;
	}

	// Fill the buffer with encoded data.
	memcpy( buf, cbuf.data(), used );
	mutex->unlock();
	return used;
}

bool setupPilotCodec(const QString &s)
{
	FUNCTIONSETUP;
	mutex = new QMutex();
	mutex->lock();
	QString encoding(KGlobal::charsets()->encodingForName(s));

	DEBUGKPILOT << fname << ": Using codec name " << s << endl;
	DEBUGKPILOT << fname << ": Creating codec " << encoding << endl;

	// if the desired codec can't be found, latin1 will be returned anyway, no need to do this manually
	codec = KGlobal::charsets()->codecForName(encoding);

	if (codec)
	{
		DEBUGKPILOT << fname << ": Got codec " << codec->name() << endl;
	}

	mutex->unlock();
	return codec;
}

QString codecName()
{
	return QString::fromLatin1(codec->name());
}

QString category(const struct CategoryAppInfo *info, unsigned int i)
{
	if (!info || (i>=CATEGORY_COUNT))
	{
		return QString::null;
	}

	mutex->lock();
	QString str = codec->toUnicode(info->name[i],
                                  MIN(strlen(info->name[i]), CATEGORY_SIZE-1));
	mutex->unlock();
	return str;
}


int findCategory(const struct CategoryAppInfo *info,
	const QString &selectedCategory,
	bool unknownIsUnfiled)
{
	FUNCTIONSETUP;

	if (!info)
	{
		WARNINGKPILOT << "Bad CategoryAppInfo pointer" << endl;
		return -1;
	}

	int currentCatID = -1;
	for (unsigned int i=0; i<CATEGORY_COUNT; i++)
	{
		if (!info->name[i][0]) continue;
		if (selectedCategory == category(info, i))
		{
			currentCatID = i;
			break;
		}
	}

	if (-1 == currentCatID)
	{
		DEBUGKPILOT << fname << ": Category name "
			<< selectedCategory << " not found." << endl;
	}
	else
	{
		DEBUGKPILOT << fname << ": Matched category " << currentCatID << endl;
	}

	if ((currentCatID == -1) && unknownIsUnfiled)
		currentCatID = 0;
	return currentCatID;
}

int insertCategory(struct CategoryAppInfo *info,
	const QString &label,
	bool unknownIsUnfiled)
{
	FUNCTIONSETUP;

	if (!info)
	{
		WARNINGKPILOT << "Bad CategoryAppInfo pointer" << endl;
		return -1;
	}


	int c = findCategory(info,label,unknownIsUnfiled);
	if (c<0)
	{
		// This is the case when the category is not known
		// and unknownIsUnfiled is false.
		for (unsigned int i=0; i<CATEGORY_COUNT; i++)
		{
			if (!info->name[i][0])
			{
				c = i;
				break;
			}
		}

		if ((c>0) && (c < (int)CATEGORY_COUNT))
		{
			// 0 is always unfiled, can't change that.
			toPilot(label,info->name[c],CATEGORY_SIZE);
		}
		else
		{
			WARNINGKPILOT << "Category name "
				<< label
				<< " could not be added." << endl;
			c = -1;
		}
	}

	return c;
}

void dumpCategories(const struct CategoryAppInfo *info)
{
	FUNCTIONSETUP;

	if (!info)
	{
		WARNINGKPILOT << "Dumping bad pointer." << endl;
		return;
	}

	DEBUGKPILOT << fname << " lastUniqueId: "
		<< (int) info->lastUniqueID << endl;
	for (unsigned int i = 0; i < CATEGORY_COUNT; i++)
	{
		if (!info->name[i][0]) continue;
		DEBUGKPILOT << fname << ": " << i << " = "
			<< (int)(info->ID[i]) << " <"
			<< info->name[i] << ">" << endl;
	}
}


}


