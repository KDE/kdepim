// conduitApp.h
//
// Copyright (C) 1998-2000 Dan Pilone
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING. 
//
// This is conduitApp.h for KPilot 4 / KDE2. It turns out to be
// impossible to have one .h file for both KDE1 and KDE2 -- there
// are problems with moc.
//
// $Revision$


#ifndef __CONDUIT_APP_H
#define __CONDUIT_APP_H

#include <kapp.h>
#include "options.h"
#include "baseConduit.h"

#ifdef KDE2
class KAboutData;
class KCmdLineOptions;
class KCmdLineArgs;
#endif


#ifdef KDE2
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
		const char *version);

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
	BaseConduit::eConduitMode getMode() { return fMode; }

	/**
	* Run the conduit app. This is probably the
	* last thing main() does before returning.
	*/
	int exec();

protected:
	KAboutData *fAbout;
	KApplication *fApp;
	bool fCmd;

private:
	BaseConduit* fConduit;
	BaseConduit::eConduitMode fMode;

	int &fArgc;
	char **fArgv;
} ;
#else
// class ConduitApp : protected KApplication
// {
//   Q_OBJECT
// 
// public:
// 	ConduitApp(int& argc, 
// 		char** argv, 
// 		const QCString& rAppName,
// 		const char *banner);
// 
//   void setConduit(BaseConduit* conduit);
//   void quit() { KApplication::quit(); }
//   int exec();
// 
//   BaseConduit::eConduitMode getMode() { return fMode; }
// 
// protected:
// 	BaseConduit::eConduitMode handleOptions(const char *,int&,char**);
// 
// private:
//   BaseConduit* fConduit;
//   BaseConduit::eConduitMode fMode;
// };
#endif

#endif
