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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
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

FileInstaller::FileInstaller() :
	enabled(true)
{
	FUNCTIONSETUP;

	fDirName = KGlobal::dirs()->saveLocation("data",
		CSL1("kpilot/pending_install/"));
	fPendingCopies = 0;

	(void) fileinstaller_id;
}

/* virtual */ FileInstaller::~FileInstaller()
{
	FUNCTIONSETUP;
}


void FileInstaller::clearPending()
{
	FUNCTIONSETUP;

	unsigned int i;

	QDir installDir(fDirName);

	// Start from 2 to skip . and ..
	//
	for (i = 2; i < installDir.count(); i++)
	{
		QFile::remove(fDirName + installDir[i]);
	}

	if (i > 2)
	{
		emit filesChanged();
	}
}

/* virtual */ bool FileInstaller::runCopy(const QString & s)
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGDAEMON << fname << ": Copying " << s << endl;
#endif

	KURL srcName(s);
	KURL destDir(fDirName + CSL1("/") + srcName.filename());

	return KIO::NetAccess::copy(srcName, destDir);
}


void FileInstaller::addFiles(const QStringList & fileList)
{
	FUNCTIONSETUP;

	if (!enabled) return;
	
	unsigned int succ = 0;

	for(QStringList::ConstIterator it = fileList.begin();
	    it != fileList.end(); ++it)
	{
		if (runCopy(*it))
			succ++;
	}

	if (succ)
	{
		emit filesChanged();
	}
}

void FileInstaller::addFile(const QString & file)
{
	FUNCTIONSETUP;

	if (!enabled) return;
	
	if (runCopy(file))
	{
		emit(filesChanged());
	}
}

/* slot */ void FileInstaller::copyCompleted()
{
	FUNCTIONSETUP;
}

const QStringList FileInstaller::fileNames() const
{
	FUNCTIONSETUP;

	QDir installDir(fDirName);

	return installDir.entryList(QDir::Files |
		QDir::NoSymLinks | QDir::Readable);
}

/* slot */ void FileInstaller::setEnabled(bool b)
{
	FUNCTIONSETUP;
	enabled=b;
}


