/* kpalmdoc_dlg.cpp
**
** Copyright (C) 2003 by Reinhold Kainhofer
**
** This is the main dialog of the KDE PalmDOC converter.
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
#ifndef CONVERTERDLG_H
#define CONVERTERDLG_H

#include <kdialogbase.h>
class ConverterDlgBase;
class DOCConverter;

class ConverterDlg : public KDialogBase
{
    Q_OBJECT

public:
	ConverterDlg( QWidget *parent=0, const QString& caption=0);
	~ConverterDlg();

protected slots:
	virtual void slotClose();
	void slotToText();
	void slotToPDB();
	void slotDirectories(bool dir);
	void slotUser1();
protected:
	void writeSettings();
	void readSettings();

	// These two functions convert one single file to or from a pdb database
	bool convertTXTtoPDB(QString txtdir, QString txtfile,
		QString pdbdir, QString pdbfile, DOCConverter*conv);
	bool convertPDBtoTXT(QString pdbdir, QString pdbfile,
		QString txtdir, QString txtfile, DOCConverter*conv);


	// The actual dialog widget (designer created) holding all controls
	ConverterDlgBase*dlg;
	// Settings
	bool askOverwrite;
	bool verbose;
};

#endif // CONVERTERDLG_H
