/* fileInstaller.h			KPilot
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

#ifndef __FILEINSTALLER_H
#define __FILEINSTALLER_H

#include <qobject.h>
#include <qstring.h>

class QStrList;

class FileInstaller : public QObject
{
	Q_OBJECT
public:
	FileInstaller();
	virtual ~FileInstaller();

	void clearPending();

	void addFiles(QStrList&);
	void addFile(const QString&);

protected:
	virtual bool runCopy(const QString &src);

public slots:
	void copyCompleted();

signals:
	void filesChanged();
private:
	QString fDirName;
	int fPendingCopies;
} ;

#endif

// $Log:$

