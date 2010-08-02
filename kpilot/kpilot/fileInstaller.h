/* fileInstaller.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#ifndef _KPILOT_FILEINSTALLER_H
#define _KPILOT_FILEINSTALLER_H

#include <tqobject.h>


class TQStringList;
class TQString;

class FileInstaller : public QObject
{
	Q_OBJECT
public:
	FileInstaller();
	virtual ~FileInstaller();

	void clearPending();

	void addFiles( const TQStringList&, TQWidget* w );
	void addFile( const TQString&, TQWidget* w );

    void deleteFile(const TQString &);
    void deleteFiles(const TQStringList &);

	/**
	* Returns information about this installer. Note particularly
	* that fileNames() returns only filenames, not paths. In particular,
	* you'll need to prepend dir()+"/" to get pathnames.
	*/
	const TQString &dir() const { return fDirName; } ;
	const TQStringList fileNames() const ;



protected:
	virtual bool runCopy( const TQString &src, TQWidget*w );

public slots:
	void copyCompleted();
	void setEnabled(bool);

signals:
	void filesChanged();
private:
	TQString fDirName;
	int fPendingCopies;
	bool enabled;
} ;

#endif
