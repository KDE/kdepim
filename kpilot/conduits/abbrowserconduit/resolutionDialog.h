#ifndef RESOLUTIONDIALOG_H
#define RESOLUTIONDIALOG_H
/* resolutionDialog.h			KPilot
**
** Copyright (C) 2002 by Reinhold Kainhofer
**
** See the .cc file for an explanation of what this file is for.
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
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include <kdialogbase.h>
class QButtonGroup;
class QCheckBox;
class ResolutionDlg : public KDialogBase
{ 
    Q_OBJECT

public:
	ResolutionDlg( QWidget* parent=0, QString caption="", QString Text="", QStringList lst=QStringList(), QString remember="");
	~ResolutionDlg();

	QButtonGroup* ResolutionButtonGroup;
	QCheckBox* rememberCheck;
};

#endif // MYDIALOG_H
