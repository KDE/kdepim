// options.cc
//
// Copyright (C) 2000 Adriaan de Groot
//
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING.
//
static const char *id="$Id$";

#include <stream.h>
#include <getopt.h>
#include <unistd.h>
#include "options.h"

// The daemon also has a debug level
//
//
int debug_level=0;
const char *tabs="\t\t\t\t\t\t";

void usage(const char *banner, struct option *longOptions)
{
	int i;

	cerr << banner ;

	cerr << "Accepted options are:\n";

	for (i=0; longOptions[i].name; i++)
	{
		cerr << "\t--" <<
			longOptions[i].name;

		if (longOptions[i].val>' ')
		{
			cerr << " (-" << (char)longOptions[i].val << ')' ;
		}
		cerr << '\n';
	}
}

