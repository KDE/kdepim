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
#include <kconfig.h>
#include <kdebug.h>
#include "options.h"

#ifdef DEBUG
// The daemon also has a debug level
//
//
int debug_level=0;
const char *tabs="\t\t\t\t\t\t";

void listConfig(kdbgstream& s,KConfig &c)
{
	FUNCTIONSETUP;

	QMap<QString,QString> m ;
	QStringList l = c.groupList();
	QStringList::Iterator i;

	s << fname << ": Listing groups in config file" << endl ;
	for (i=l.begin(); i!=l.end(); ++i)
	{
		s << fname 
			<< ": "
			<< *i
			<< endl;

		m = c.entryMap(*i);
		QMap<QString,QString>::ConstIterator j;

		for (j=m.begin(); j!=m.end(); ++j)
		{
			s << fname
				<< ": "
				<< j.key()
				<< "="
				<< j.data()
				<< endl;
		}
	}
	/* NOTREACHED */
	(void) id;
}

void listStrList(kdbgstream& s,const QStringList& l)
{
	FUNCTIONSETUP;

	QStringList::ConstIterator i;
	s << fname << ": Elements of string list:" << endl;

	for (i=l.begin(); i!=l.end(); ++i)
	{
		s << fname << ": " << *i << endl;
	}
}



QString qstringExpansion(const QString& s)
{
	QString t;

	for (unsigned i=0; i<s.length(); i++)
	{
		t+=s[i];
		t+=' ';
		t+=QString::number(s[i].unicode());
		t+=' ';
	}

	return t;
}

QString charExpansion(const char *s)
{
	QString t;

	while (*s)
	{
		t+=QChar(*s);
		t+=' ';
		t+=QString::number(*s);
		t+=' ';
		s++;
	}

	return t;
}


#else
#ifdef TEST_DEBUG
debugName const fname((int) id);
kndbgstream operator << (kndbgstream s, const debugName&)
{
	return s;
}
#else
const int fname = (int) id;
#endif
#endif
