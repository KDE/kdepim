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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"

#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>

#include "pilotLocalDatabase.h"
#include "pilotRecord.h"
#include "pilotMemo.h"


/* Return values for the various check* functions. They
   return OK if all is OK; ERR is for generic errors.
   ERR_NO_EXIST is returned if something (usually a database
   or other file) doesn't exist that should. The latter
   error might be ignored.
*/
#define OK           (0)
#define ERR          (1)
#define ERR_NO_EXIST (2)


/* Data about the records in a database. The id field is
   interpreted specially for the first and last entries.
*/
typedef struct { int id,size; } recordInfo;

/* Use END in the last recordInfo struct describing a database
   to indicate you expect the database to end there. Use NO_EXIST
   as the ID in the first struct to indicate that the database
   is expected _not_ to exist.
*/
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
	if (!db.isOpen())
	{
		kdDebug() << "No database " << path << endl;
		if ( info[0].id == NO_EXIST )
		{
			kdDebug() << "This was expected" << endl;
			return OK;
		}
		else
		{
			return ERR_NO_EXIST;
		}
	}

	if ( info[0].id == NO_EXIST )
	{
		kdDebug() << "Database not expected" << endl;
		return ERR;
	}

	int fail = 0;
	int index = 0;
	PilotRecord *r;
	while( (r = db.readRecordByIndex(index) ) )
	{
		kdDebug() << "[" << index << "] id=" << r->id() << " size=" << r->size() << endl;
		if ( ((recordid_t)info[index].id) != r->id() )
		{
			kdDebug() << "* Bad ID (expected" << r->id() << ")" << endl;
			fail++;
		}
		else if ( info[index].size != r->size() )
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
		return ERR;
	}
	return OK;
}

const char *categoryNames[4] =
{
	"aardvarks",
	"toolongToBeaCategoryName",
	"personal",
	"impersonal"
} ;

QStringList listCategories()
{
	QStringList cats;
	PilotLocalDatabase *l = new PilotLocalDatabase("./data/MemoDB");
	PilotMemoInfo *m = new PilotMemoInfo(l);

	if (!l->isOpen()) return cats;

	cats.append(CSL1("Unfiled"));
	m->dump();

	for (int i=0; i<20; i++)
	{
		PilotRecord *r = l->readRecordByIndex(i);
		kdDebug() << "Read record " << (void *)r << " with id=" << r->id() << endl;
		if (!r) break;
	}

	for (int i=0; i<4; i++)
	{
		QString s = m->category(i);
		kdDebug() << "Category " << i << ": " << (s.isEmpty() ? CSL1("<empty>") : s) << endl;
		cats.append(s);
/*
		if (i<((sizeof(categoryNames) / sizeof(categoryNames[0]))))
			m->setCategoryName(i,QString::fromLatin1(categoryNames[i]));
*/
	}

	m->write(l);

	delete m;
	delete l;

	return cats;
}

int checkCategories()
{
	QStringList l = listCategories();
	QStringList m = listCategories();

	if (l.isEmpty() || m.isEmpty()) return ERR;
	if (l!=m) return ERR;
	return OK;
}

int checkMemo()
{
	PilotLocalDatabase *l = new PilotLocalDatabase("./data/MemoDB");
	if (!l->isOpen()) return ERR_NO_EXIST;

	PilotMemoInfo *m = new PilotMemoInfo(l);
	m->dump();

	QString c = m->category(1);
	if (c != CSL1("Business"))
	{
		kdDebug() << "* Category 1 is not 'Business' but " << c << endl;
		return ERR;
	}

	m->setCategoryName(2,CSL1("Aardvark"));
	m->dump();

	c = m->category(2);
	if (c != CSL1("Aardvark"))
	{
		kdDebug() << "* Category 2 is not 'Aardvark' but " << c << endl;
		return ERR;
	}


	delete m;
	delete l;
	return OK;
}

static const KCmdLineOptions options[] =
{
  {"verbose", "Verbose output", 0},
  KCmdLineLastOption
};


int main(int argc, char **argv)
{
	KApplication::disableAutoDcopRegistration();

	KAboutData aboutData("testdatabase","Test Databases","0.1");
	KCmdLineArgs::init(argc,argv,&aboutData);
	KCmdLineArgs::addCmdLineOptions( options );

	KApplication app( false, false );

	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

	Q_UNUSED(args)

	int r = 0;
	int i = 0;
#ifdef DEBUG
	debug_level=4;
#endif

	Q_UNUSED(argc);
	Q_UNUSED(argv);

#define ALLOW_NO_EXIST (1)
	static struct { const char *path; recordInfo *info; int flags; } 
		tests[] =
	{
		{ "/tmp/nonexistant/nonexistent", nonexistent,0 },
		{ "/tmp/Aesop", aesop, ALLOW_NO_EXIST },
		{ 0L, 0L, 0 }
	} ;

	while ( tests[i].path )
	{
		kdDebug() << "*** Test " << i << endl;
		int ret = checkDatabase( tests[i].path, tests[i].info );
		if ( ret )
		{
			if ( (ret==ERR_NO_EXIST) && 
				(tests[i].flags & ALLOW_NO_EXIST) )
			{
				kdDebug() << "* Test database doesn't exist, ignored." << endl;
			}
			else
			{
				r++;
			}
		}
		i++;
	}

	kdDebug() << "*** Test " << i << endl;
	if (checkMemo()) r++;
	i++;

	if (r)
	{
		kdDebug() << "***\n*** Failed " << r << " tests." << endl;
		return 1;
	}
	return 0;
}

