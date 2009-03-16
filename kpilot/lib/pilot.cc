/* pilot.cc			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone <dan@kpilot.org>
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
#include "pilot.h"

#include <QtCore/QTextCodec>
#include <QtCore/QMutex>

#include <kcharsets.h>
#include <kglobal.h>

#include "options.h"
#include "pilotDatabase.h"
#include "pilotAppInfo.h"
#include "pilotRecord.h"


namespace Pilot
{
static QTextCodec *codec = 0L;
static QMutex *mutex = 0L;


QString fromPilot( const char *c, int len )
{
	QMutexLocker locker(mutex);
	// Obviously bogus length
	if (len<1)
	{
		return QString();
	}
	QString str;
	// See if the C string is short
	for (int i=0; i<len; ++i)
	{
		if (!c[i])
		{
			str = codec->toUnicode(c,i);
			break; // leave after first \0 encountered
		}
	}
	// Use the whole length
	if (str.isEmpty()) {
		str = codec->toUnicode(c,len);
	}
	return str;
}

QString fromPilot( const char *c )
{
	QMutexLocker locker(mutex);
	QString str = codec->toUnicode(c);
	return str;
}

QByteArray  toPilot( const QString &s )
{
	QMutexLocker locker(mutex);
	QByteArray str = codec->fromUnicode(s);
	return str;
}

int toPilot( const QString &s, char *buf, int len)
{
	FUNCTIONSETUPL(4);
	QMutexLocker locker(mutex);
	// Clear out the entire buffer
	memset( buf, 0, len );
	if (len<1) // short-circuit for bad input
	{
		return 0;
	}

	// Convert at most len characters of s
	QByteArray cbuf = codec->fromUnicode(
		(s.length() > len) ? s.left(len) : s );
	// How many characters was that, then?
	int used = cbuf.size();
	if (used > len)
	{
		// This ought to be impossible: converting len
		// unicode characters to 8-bit should never
		// generate more characters.
		used=len;
		WARNINGKPILOT << "Conversion to 8-bit of "
			<< len << " characters yielded " << used;
	}
	// Get what's left (note no NUL termination)
	memcpy( buf, cbuf.data(), used );
	return used;
}

int toPilot( const QString &s, unsigned char *buf, int len)
{
	return toPilot(s,reinterpret_cast<char *>(buf),len);
}

bool setupPilotCodec(const QString &s)
{
	FUNCTIONSETUP;
	mutex = new QMutex();
	QMutexLocker locker(mutex);
	QString encoding(KGlobal::charsets()->encodingForName(s));

	DEBUGKPILOT << "Using codec name" << s;
	DEBUGKPILOT << "Creating codec" << encoding;

	// if the desired codec can't be found, latin1 will be returned anyway, no need to do this manually
	codec = KGlobal::charsets()->codecForName(encoding);

	if (codec)
	{
		DEBUGKPILOT << "Got codec" << codec->name().constData();
	}

	return codec;
}

QString codecName()
{
	return QString::fromLatin1(codec->name());
}

QString category(const struct CategoryAppInfo *info, unsigned int i)
{
	QMutexLocker locker(mutex);
	if (!info || (i>=CATEGORY_COUNT))
	{
		return QString();
	}

	QString str = codec->toUnicode(info->name[i],
                                  MIN(strlen(info->name[i]), CATEGORY_SIZE-1));
	return str;
}


int findCategory(const struct CategoryAppInfo *info,
	const QString &selectedCategory,
	bool unknownIsUnfiled)
{
	FUNCTIONSETUP;

	if (!info)
	{
		WARNINGKPILOT << "Bad CategoryAppInfo pointer";
		return -1;
	}

	int currentCatID = -1;
	for (unsigned int i=0; i<CATEGORY_COUNT; ++i)
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
		DEBUGKPILOT << "Category name ["
			<< selectedCategory << "] not found.";
	}
	else
	{
		DEBUGKPILOT << "Matched category" << currentCatID;
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
		WARNINGKPILOT << "Bad CategoryAppInfo pointer";
		return -1;
	}


	int c = findCategory(info,label,unknownIsUnfiled);
	if (c<0)
	{
		// This is the case when the category is not known
		// and unknownIsUnfiled is false.
		for (unsigned int i=0; i<CATEGORY_COUNT; ++i)
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
			WARNINGKPILOT << "Category name ["
				<< label
				<< "] could not be added.";
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
		WARNINGKPILOT << "Dumping bad pointer.";
		return;
	}

	DEBUGKPILOT << "lastUniqueId:"
		<< (int) info->lastUniqueID;
	for (unsigned int i = 0; i < CATEGORY_COUNT; ++i)
	{
		if (!info->name[i][0]) continue;
		DEBUGKPILOT << i << '='
			<< (int)(info->ID[i]) << " ["
			<< info->name[i] << ']';
	}
}


}


