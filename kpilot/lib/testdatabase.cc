/* testdatabase			KPilot
**
** Copyright (C) 2005 by Adriaan de Groot <groot@kde.org)
**
** Test the functions related to local databases.
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

#include "options.h"
#include "pilotLocalDatabase.h"
#include "pilotRecord.h"


typedef struct { int id,size; } recordInfo;

#define NO_EXIST   (-2)
#define END        (-1)

/* These tables of data are taken from various databases I have
   (but which I cannot commit to SVN due to license issues).
   The aesop listing is from an eBook of Aesop's fables.
   The way to create these tables is to use a third-party
   tool such as par to read the database:

   ./par l /tmp/Aesop.pdb | awk '{print "{",$3,",",$4,"},";}'

*/
recordInfo nonexistent[] = {
	{ NO_EXIST, 0 }
} ;

recordInfo aesop[] = {
{ 7307264 , 214 },
{ 7307265 , 1564 },
{ 7307266 , 1575 },
{ 7307267 , 2214 },
{ 7307268 , 2276 },
{ 7307269 , 2148 },
{ 7307270 , 2194 },
{ 7307271 , 2178 },
{ 7307272 , 2220 },
{ 7307273 , 2216 },
{ 7307274 , 2181 },
{ 7307275 , 2183 },
{ 7307276 , 2197 },
{ 7307277 , 2010 },
{ 7307278 , 2198 },
{ 7307279 , 2196 },
{ 7307280 , 2243 },
{ 7307281 , 2211 },
{ 7307282 , 2274 },
{ 7307283 , 364 },
{ 7307284 , 49124 },
	{ END, 0 }
} ;

int checkDatabase(const char *path, recordInfo *info)
{
	FUNCTIONSETUP;

	PilotLocalDatabase db(QString::fromLatin1(path));
	if (!db.isDBOpen())
	{
		kdDebug() << "No database " << path << endl;
		if ( info[0].id == NO_EXIST )
		{
			kdDebug() << "This was expected" << endl;
			return 0;
		}
		else
		{
			return 1;
		}
	}

	if ( info[0].id == NO_EXIST )
	{
		kdDebug() << "Database not expected" << endl;
		return 1;
	}

	int fail = 0;
	int index = 0;
	PilotRecord *r;
	while( (r = db.readRecordByIndex(index) ) )
	{
		kdDebug() << "[" << index << "] id=" << r->id() << " size=" << r->getLen() << endl;
		if ( info[index].id != r->id() )
		{
			kdDebug() << "* Bad ID (expected" << r->id() << ")" << endl;
			fail++;
		}
		else if ( info[index].size != r->getLen() )
		{
			kdDebug() << "* Bad size (expected " << info[index].size << ")" << endl;
			fail++;
		}
		index++;
	}
	if ( info[index].id != END )
	{
		kdDebug() << "* End wasn't expected yet." << endl;
		r++;
	}

	if (fail)
	{
		kdDebug() << "* " << fail << " failures." << endl;
		return 1;
	}
	return 0;
}


struct { const char *path; recordInfo *info; } tests[] = {
	{ "/tmp/nonexistant/nonexistent", nonexistent },
	{ "/tmp/Aesop", aesop },
	{ 0L, 0L }
} ;

int main(int argc, char **argv)
{
	int r = 0;
	int i = 0;
#ifdef DEBUG
	debug_level=4;
#endif

	while ( tests[i].path )
	{
		kdDebug() << "*** Test " << i << endl;
		if ( checkDatabase( tests[i].path, tests[i].info ) )
		{
			r++;
		}
		i++;
	}

	if (r)
	{
		kdDebug() << "***\n*** Failed " << r << " tests." << endl;
		return 1;
	}
	return 0;
}

