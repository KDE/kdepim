/* testcategories			KPilot
**
** Copyright (C) 2005 by Adriaan de Groot <groot@kde.org)
**
** Test the functions related to category handling.
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
#include "pilotAppInfo.h"

// Name of a bogus broken DB
#define BOGUS_NAME "bogus"

// Name of an actual DB
#define MEMO_NAME "MemoDB"

QStringList categories( const PilotAppInfoBase *appinfo )
{
	QStringList cats;

	for (unsigned int i=0; i<Pilot::CATEGORY_COUNT; i++)
	{
		QString cat = appinfo->categoryName(i);
		if (!cat.isEmpty())
		{
			QString s = CSL1("(%1:%2)").arg(i).arg(cat);
			cats.append(s);
		}
	}

	return cats;
}

QStringList listCategories( const QString &dir, const char *dbname )
{
	QStringList cats;
	PilotLocalDatabase *database = new PilotLocalDatabase( dir, dbname );
	if (!database->isOpen()) return cats;

	PilotAppInfoBase *appinfo = new PilotAppInfoBase( database );
	appinfo->dump();

	cats = categories( appinfo );

	delete appinfo;
	delete database;
	return cats;
}

void badAppInfoCreation()
{
	FUNCTIONSETUP;
	PilotAppInfoBase *appinfo = new PilotAppInfoBase( 0L );
	appinfo->dump();
	KPILOT_DELETE( appinfo ) ;

	PilotLocalDatabase *database = new PilotLocalDatabase( BOGUS_NAME );
	appinfo = new PilotAppInfoBase( database );
	appinfo->dump();
	KPILOT_DELETE( appinfo );
}

void categoryNames( const QString &dir )
{
	PilotLocalDatabase *database = new PilotLocalDatabase( dir, MEMO_NAME );
	if (!database->isOpen())
	{
		WARNINGKPILOT << "Can not open database '" << MEMO_NAME << "'" << endl;
		return;
	}

	PilotAppInfoBase *appinfo = new PilotAppInfoBase( database );
	appinfo->dump();

	DEBUGKPILOT << "# Done dumping" << endl;

	if (!appinfo->categoryInfo())
	{
		WARNINGKPILOT << "Could not read required database" << endl;
		return;
	}

	const char *funnyname = "OneTwoThreeFourFiveSixSevenEight";
	const int funnyname_length = strlen(funnyname);

	if (funnyname_length < 20)
	{
		WARNINGKPILOT << "String of example category names is too short." << endl;
		return;
	}

	DEBUGKPILOT << "# Updating category names with various lengths." << endl;
	DEBUGKPILOT << "# Expect three truncation errors and two bad category numbers." << endl;
	for (unsigned int i=0; i<Pilot::CATEGORY_COUNT+2; i++)
	{
		QString name = QString::fromLatin1(funnyname+funnyname_length-i-3);
		if (!appinfo->setCategoryName(i,name))
		{
			WARNINGKPILOT << "Failed to set category " << i << " name to <" << name << ">" << endl;
		}
		else
		{
			QString categoryname = appinfo->categoryName(i);
			if (categoryname != name)
			{
				WARNINGKPILOT << "Category name " << i
					<< " set to <" << name
					<< "> and returns <"
					<< categoryname << ">" << endl;
			}
		}
	}

	DEBUGKPILOT << "# Final categories\n#   " << categories( appinfo ).join("\n#   ") << endl;
}

static const KCmdLineOptions options[] =
{
	{"verbose", "Verbose output", 0},
	{"data-dir <path>","Set data directory", "."},
	KCmdLineLastOption
};


int main(int argc, char **argv)
{
	KApplication::disableAutoDcopRegistration();

	KAboutData aboutData("testcategories","Test Categories","0.1");
	KCmdLineArgs::init(argc,argv,&aboutData);
	KCmdLineArgs::addCmdLineOptions( options );

	KApplication app( false, false );

	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

#ifdef DEBUG
	debug_level= (args->isSet("verbose")) ? 4 : 0;
#endif

	Q_UNUSED(argc);
	Q_UNUSED(argv);

	static const char *files[] = {
		MEMO_NAME,
		"AddressDB",
		"MailDB",
		"ToDoDB",
		0L
	};

	QString datadir = args->getOption("data-dir");

	DEBUGKPILOT << "### testcategories\n#\n#" << endl;
	DEBUGKPILOT << "# Listing categories from database files.\n#" << endl;

	Pilot::setupPilotCodec( CSL1("Latin1") );

	// Include arbitrary break-off point, in case
	for (unsigned int i = 0; i<sizeof(files)/sizeof(const char *) ; i++)
	{
		if (!files[i])
		{
			break;
		}
		DEBUGKPILOT << "# Categories (" << files[i] << "): " << endl;
		DEBUGKPILOT << "#   " << listCategories( datadir, files[i] ).join("\n#   ") << "\n#\n";
	}
	// Should bail, not crash
	DEBUGKPILOT << "# Categories (nonexistent): " << endl;
	(void) listCategories( datadir, "nonexistent" );

	DEBUGKPILOT << "# Categories (bogus): " << endl;
	(void) listCategories( datadir, BOGUS_NAME );

	DEBUGKPILOT << "#\n# Trying to pass broken pointers to category functions.\n# Four errors are expected.\n#" << endl;
	badAppInfoCreation();

	DEBUGKPILOT << "#\n# Checking category names." << endl;
	categoryNames( datadir );

	DEBUGKPILOT << "# OK.\n" << endl;
	return 0;
}

