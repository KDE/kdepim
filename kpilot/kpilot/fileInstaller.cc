/* fileInstaller.cc			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** This is a class that does "the work" of adding and deleting
** files in the pending_install directory of KPilot. It is used
** by the fileInstallWidget and by the daemon's drag-and-drop
** file accepter.
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

static const char *fileinstaller_id =
	"$Id$";


#ifndef _KPILOT_OPTIONS_H
#include "options.h"
#endif

#include <unistd.h>

#ifndef QSTRING_H
#include <qstring.h>
#endif
#ifndef QSTRLIST_H
#include <qstrlist.h>
#endif
#ifndef QDIR_H
#include <qdir.h>
#endif

#ifndef _KGLOBAL_H
#include <kglobal.h>
#endif
#ifndef _KSTDDIRS_H
#include <kstddirs.h>
#endif
#ifndef _KURL_H
#include <kurl.h>
#endif
#ifndef _KIO_NETACCESS_H
#include <kio/netaccess.h>
#endif

#include "fileInstaller.moc"

FileInstaller::FileInstaller()
{
	FUNCTIONSETUP;

	fDirName = KGlobal::dirs()->saveLocation("data",
		"kpilot/pending_install/");
	fPendingCopies = 0;

	(void) fileinstaller_id;
}

/* virtual */ FileInstaller::~FileInstaller()
{
}


void FileInstaller::clearPending()
{
	FUNCTIONSETUP;

	unsigned int i;

	QDir installDir(fDirName);

	// Start from 2 to skip . and ..
	//
	for (i=2; i < installDir.count(); i++)
	{
		unlink((fDirName + installDir[i]).latin1());
	}

	if (i>2) { emit filesChanged(); }
}

/* virtual */ bool FileInstaller::runCopy(const QString& s)
{
	FUNCTIONSETUP;

	DEBUGDAEMON << fname
		<< ": Copying "
		<< s
		<< endl;

	KURL srcName(s);
	KURL destDir(fDirName + "/" + srcName.filename());
	return KIO::NetAccess::copy(srcName, destDir);
}

	

void FileInstaller::addFiles(QStrList& fileList)
{
	FUNCTIONSETUP;

	unsigned int i = 0;
	unsigned int succ = 0;

	while (i < fileList.count())
	{
		if (runCopy(fileList.at(i))) succ++;
		i++;
	}

	if (succ) { emit filesChanged(); }
}

void FileInstaller::addFiles(QStringList& fileList)
{
	FUNCTIONSETUP;

	unsigned int i = 0;
	unsigned int succ = 0;

	while (i < fileList.count())
	{
		if (runCopy(fileList[i])) succ++;
		i++;
	}

	if (succ) { emit filesChanged(); }
}

void FileInstaller::addFile(const QString& file)
{
	FUNCTIONSETUP;

	if (runCopy(file))
	{
		emit(filesChanged());
	}
}

/* slot */ void FileInstaller::copyCompleted()
{
}

const QStringList FileInstaller::fileNames() const
{
	FUNCTIONSETUP;

	QDir installDir(fDirName);

	return installDir.entryList(QDir::Files | 
		QDir::NoSymLinks | 
		QDir::Readable);
}



// $Log$
// Revision 1.2  2001/03/04 13:11:58  adridg
// Actually use the fileInstaller object
//
// Revision 1.1  2001/03/01 20:41:11  adridg
// Added class to factor out code in daemon and fileinstallwidget
//

