/* conduitApp.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** This file defines what a conduit application is. It resembles
** a KApplication in the sense that you create one, give it a
** conduit object, and call exec() and be done with it.
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

#ifndef _KPILOT_CONDUITAPP_H
#define _KPILOT_CONDUITAPP_H

#ifndef _KAPP_H_
#include <kapp.h>
#endif

#ifndef _KPILOT_BASECONDUIT_H
#include "baseConduit.h"
#endif


class KAboutData;
class KCmdLineOptions;
class KCmdLineArgs;


class ConduitApp
{
public:
	/**
	* Make a conduit app from the command-line
	* arguments, an application name (not for
	* humans -- this is an application identifier)
	* and a banner (which isn't really relevant in
	* KDE2).
	*
	* Before calling exec() you will want to call
	* one or more of the following functions:
	* @ref setConduit
	* @ref addAuthor
	*/
	ConduitApp(int &argc,
		char **argv,
		const char *rAppName,
		const char *conduitName,
		const char *version=0L);

	/**
	* Attaches a particular conduit to the application.
	*/
	void setConduit(BaseConduit* conduit);


	/**
	* Add an additional author to the conduit's
	* KAboutData. The constructor sets a number of 
	* standard things in the AboutData, so only
	* special extra authors need to be added 
	* explicitly.
	*/
	void addAuthor(const char *name,
		const char *task,
		const char *email=0L);

	/**
	* Add KCmdLineOptions to the conduit's application.
	* All conduits have a set of standard options
	* (info, hotsync, setup, backup) but particular
	* conduits may want more.
	*
	* Once all the options needed have been added,
	* call @ref getOptions to find out what the
	* results of parsing all the options are.
	*/
	void addOptions(KCmdLineOptions *);

	/**
	* Returns the parsed options -- which are really
	* arguments, not options. Confusing.
	*/
	KCmdLineArgs *getOptions();

	/**
	* Returns the mode the conduit application is
	* in. The application is in mode Error until
	* a mode is explicitly set. After that it 
	* is in that mode, although it can return to
	* mode error if errors occur.
	*/
	BaseConduit::eConduitMode getMode();

	/**
	* Run the conduit app. This is probably the
	* last thing main() does before returning.
	*/
	int exec(bool withDCOP=false,bool withGUI=true);


protected:
	KAboutData *fAbout;
	KApplication *fApp;
	bool fCmd;

private:
	BaseConduit* fConduit;
	BaseConduit::eConduitMode fMode;

	int &fArgc;
	char **fArgv;


	bool setupDCOP();
} ;
#else
#warning "File doubly included"
#endif

// $Log$
// Revision 1.12  2001/02/06 08:05:19  adridg
// Fixed copyright notices, added CVS log, added surrounding #ifdefs. No code changes.
//
// Revision 1.11  2000/12/31 16:44:00  adridg
// Patched up the debugging stuff again
//
// Revision 1.10  2000/12/22 07:47:04  adridg
// Added DCOP support to conduitApp. Breaks binary compatibility.
//
// Revision 1.9  2000/10/29 22:11:06  adridg
// Added debug-merge feature to conduits
