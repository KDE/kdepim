/* testactions			KPilot
**
** Copyright (C) 2005 by Adriaan de Groot <groot@kde.org)
**
** Test the functions related to sync actions.
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

#include "idmapper.h"
#include "options.h"

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>

#include <qdir.h>
#include <qfile.h>

#define TESTFILE "Testing/mapping.xml"
#define CONDUIT CSL1("knotes")

/**
 * If the file does not exist it should be created by the idmapper.
 */
bool test1()
{
	FUNCTIONSETUP;

	IDMapper *mapper = new IDMapper( TESTFILE );

	delete mapper;
	mapper = 0l;

	QFile f( TESTFILE );
	bool result = f.exists();
	
	if( result )
	{
		DEBUGKPILOT << fname << ": passed" << endl;
	}
	else
	{
		DEBUGKPILOT << fname << ": failed" << endl;
	}
	
	return result;
}

/**
 * Test if a uid gets added when it's registered.
 */
bool test2()
{
	FUNCTIONSETUP;

	IDMapper *mapper = new IDMapper( TESTFILE );
	mapper->registerPCObjectId( CONDUIT, CSL1("testuid-1") );

	delete mapper;
	mapper = 0l;
	
	// Creating a new mapper ensures that the file is readed. So we know if this
	// test passess that the data is saved and readed from the xml file correctly.
	mapper = new IDMapper( TESTFILE );

	bool result1 = ( mapper->getPCObjectIds( CONDUIT ).size() == 1 );
	bool result2 = false;
	
	if( result1 )
		result2 = ( mapper->getPCObjectIds( CONDUIT ).first() == "testuid-1" );
	
	
	if( result1 && result2 )
	{
		DEBUGKPILOT << fname << ": passed" << endl;
	}
	else
	{
		DEBUGKPILOT << fname << ": " << result1 << " " << result2 << endl;
		DEBUGKPILOT << fname << ": failed" << endl;
	}
	
	delete mapper;
	mapper = 0l;
	
	return (result1 && result2);
}

/**
 * Set the pid for uid "testuid-1". getHHObjectIds should return 1 item now and
 * that should be the same as the one which is set.
 */
bool test3()
{
	FUNCTIONSETUP;

	IDMapper *mapper = new IDMapper( TESTFILE );
	mapper->setHHObjectId( CONDUIT, "testuid-1", 100 );

	delete mapper;
	mapper = 0l;

	// Creating a new mapper ensures that the file is readed. So we know if this
	// test passess that the data is saved and readed from the xml file correctly.
	mapper = new IDMapper( TESTFILE );

	bool result1 = ( mapper->getHHObjectIds( CONDUIT ).size() == 1 );
	bool result2 = false;
	
	if( result1 )
		result2 = ( mapper->getHHObjectIds( CONDUIT ).first() == 100 );
	
	
	if( result1 && result2 )
	{
		DEBUGKPILOT << fname << ": passed" << endl;
	}
	else
	{
		DEBUGKPILOT << fname << ": " << result1 << " " << result2 << endl;
		DEBUGKPILOT << fname << ": failed" << endl;
	}
	
	delete mapper;
	mapper = 0l;
	
	return (result1 && result2);
}

/**
 * Test if a pid is stored correctly when it's registered.
 */
bool test4()
{
	FUNCTIONSETUP;

	IDMapper *mapper = new IDMapper( TESTFILE );
	mapper->registerHHObjectId( CONDUIT, 150 );

	delete mapper;
	mapper = 0l;
	
	// Creating a new mapper ensures that the file is readed. So we know if this
	// test passess that the data is saved and readed from the xml file correctly.
	mapper = new IDMapper( TESTFILE );

	// We have two pids registered at this moment
	bool result1 = ( mapper->getHHObjectIds( CONDUIT ).size() == 2 );
	bool result2 = false;
	
	// This prevents the test from chrashing when getHHObjectIds.size is 0.
	if( result1 )
		result2 = ( mapper->getHHObjectIds( CONDUIT ).contains( 150 ) );
	
	if( result1 && result2 )
	{
		DEBUGKPILOT << fname << ": passed" << endl;
	}
	else
	{
		DEBUGKPILOT << fname << ": " << result1 << " " << result2 << endl;
		DEBUGKPILOT << fname << ": failed" << endl;
	}
	
	delete mapper;
	mapper = 0l;
	
	return (result1 && result2);
}

/**
 * Set the uid for pid 150. getPcObjectIds should return 2 items now and
 * it should contain the one which is just set.
 */
bool test5()
{
	FUNCTIONSETUP;

	IDMapper *mapper = new IDMapper( TESTFILE );
	mapper->setPCObjectId( CONDUIT, 150, "testuid-2" );

	delete mapper;
	mapper = 0l;

	// Creating a new mapper ensures that the file is readed. So we know if this
	// test passess that the data is saved and readed from the xml file correctly.
	mapper = new IDMapper( TESTFILE );

	bool result1 = ( mapper->getPCObjectIds( CONDUIT ).size() == 2 );
	bool result2 = false;
	
	if( result1 )
		result2 = ( mapper->getPCObjectIds( CONDUIT ).contains( "testuid-2" ) );
	
	
	if( result1 && result2 )
	{
		DEBUGKPILOT << fname << ": passed" << endl;
	}
	else
	{
		DEBUGKPILOT << fname << ": " << result1 << " " << result2 << endl;
		DEBUGKPILOT << fname << ": failed" << endl;
	}
	
	delete mapper;
	mapper = 0l;
	
	return (result1 && result2);
}


/**
 * Test for the hasPCId function.
 */
bool test6()
{
	FUNCTIONSETUP;

	IDMapper *mapper = new IDMapper( TESTFILE );

	// Should be true (PCId is set to "testuid-2").
	bool result = mapper->hasPCId( CONDUIT, 150 );

	delete mapper;
	mapper = 0l;
	
	if( result )
	{
		DEBUGKPILOT << fname << ": passed" << endl;
	}
	else
	{
		DEBUGKPILOT << fname << ": failed" << endl;
	}
	
	return result;
}

/**
 * Test for the hasHHId function.
 */
bool test7()
{
	FUNCTIONSETUP;

	IDMapper *mapper = new IDMapper( TESTFILE );

	// Should be true (HHId is set to "150").
	bool result = mapper->hasHHId( CONDUIT, "testuid-2" );

	delete mapper;
	mapper = 0l;
	
	if( result )
	{
		DEBUGKPILOT << fname << ": passed" << endl;
	}
	else
	{
		DEBUGKPILOT << fname << ": failed" << endl;
	}
	
	return result;
}

int main(int argc, char **argv)
{
	KApplication::disableAutoDcopRegistration();
	KAboutData aboutData("testidmapper","Test IDMapper","0.1");
	KCmdLineArgs::init(argc,argv,&aboutData);

	KApplication app( false, false );

	// Remove file from previous test run
	QDir test( "Testing" );
	if( !test.exists() ) {
		QDir current;
		current.mkdir( "Testing" );
	}
	
	QFile f( TESTFILE );
	if( f.exists() )
		QFile::remove( TESTFILE );

	if( test1() && test2() && test3() && 
			test4() && test5() && test6() &&
			test7() )
		return 0;
	else
		return 1;
}


