/* KPilot
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


#include "options.h"

#include <unistd.h>

#include <qstring.h>
#include <qstrlist.h>
#include <qdir.h>

#include <kglobal.h>
#include <kstandarddirs.h>
#include <kurl.h>
#include <kio/netaccess.h>
#include <kmessagebox.h>

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

/* virtual */ bool FileInstaller::runCopy(const QString & s, QWidget* w )
{
	FUNCTIONSETUP;

	if(!(s.endsWith("pdb", false) || s.endsWith("prc", false))) {
		KMessageBox::detailedSorry(w, i18n("Cannot install %1").arg(s),
			i18n("Only PalmOS database files (like *.pdb and *.prc) can be installed by the file installer."));
		return false;
	}

#ifdef DEBUG
	DEBUGDAEMON << fname << ": Copying " << s << endl;
#endif

	KURL srcName(s);
	KURL destDir(fDirName + CSL1("/") + srcName.filename());

#if KDE_IS_VERSION(3,1,9)
	return KIO::NetAccess::copy(srcName, destDir, w);
#else
	return KIO::NetAccess::copy(srcName,destDir);
#endif
}


void FileInstaller::addFiles(const QStringList & fileList, QWidget* w)
{
	FUNCTIONSETUP;

	if (!enabled) return;

	unsigned int succ = 0;

	for(QStringList::ConstIterator it = fileList.begin();
	    it != fileList.end(); ++it)
	{
		if (runCopy( *it, w ))
			succ++;
	}

	if (succ)
	{
		emit filesChanged();
	}
}

void FileInstaller::addFile( const QString & file, QWidget* w )
{
	FUNCTIONSETUP;

	if (!enabled) return;

	if (runCopy(file, w))
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


