/* kpilotOptions.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** This is the main setup dialog for KPilot, whch allows the user to
** setup the physical connection with the pilot.
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
#ifndef _KPILOT_KPILOTOPTIONS_H
#define _KPILOT_KPILOTOPTIONS_H

#ifndef _KPILOT_GSETUPDIALOG_H
#include "gsetupDialog.h"
#endif

class QLabel;
class QCheckBox;
class QLineEdit;
class QComboBox;
class QRadioButton;
class QGroupBox;
class QButtonGroup;


// This config page is now being used for
// "special db settings," not just privacy settings.
//
//
class KPilotOptionsPrivacy : public setupDialogPage
{
	Q_OBJECT

public:
	KPilotOptionsPrivacy(setupDialog *,KConfig&);

	virtual int commitChanges(KConfig&);

private:
	QCheckBox *fuseSecret;
	QLineEdit *fBackupOnly,*fSkipDB;
} ;

class KPilotOptionsSync : public setupDialogPage
{
	Q_OBJECT

public:
	KPilotOptionsSync(setupDialog *,KConfig&);

	virtual int commitChanges(KConfig&);

private:
	QCheckBox* fSyncFiles;
	QCheckBox* fOverwriteRemote;
	QCheckBox *fSyncLastPC;
	QCheckBox *fForceFirstTime;
	QCheckBox *fPreferFastSync;
} ;

class KPilotOptionsGeneral : public setupDialogPage
{
	Q_OBJECT

public:
	KPilotOptionsGeneral(setupDialog *parent,KConfig&);
	virtual ~KPilotOptionsGeneral();

	virtual int commitChanges(KConfig&);

private:
	QLineEdit* fPilotDevice;
	QComboBox* fPilotSpeed;
	QLineEdit* fUserName;


	QCheckBox* fStartDaemonAtLogin;
	QCheckBox *fKillDaemonOnExit;
	QCheckBox* fStartKPilotAtHotSync;
	QCheckBox* fDockDaemon;
} ;

/**
* The address page is a strange beast -- it behaves
* almost like an external conduit. It has its own
* setup page *AND* its own group in the config file.
*/
class KPilotOptionsAddress : public setupDialogPage
{
	Q_OBJECT

public:
	KPilotOptionsAddress(setupDialog *parent,KConfig&);

	virtual int commitChanges(KConfig&);

public:
	static int getDisplayMode(KConfig&);

protected:
	void setRadio(int);
	int getRadio() const;

private:
	QButtonGroup *displayGroup;
	QRadioButton *fNormalDisplay;
	QRadioButton *fCompanyDisplay;

	// Address Prefs:
	QGroupBox *formatGroup;
	QLineEdit* fIncomingFormat;
	QLineEdit* fOutgoingFormat;
	QCheckBox* fUseKeyField;

	static QString fGroupName;
} ;



class KPilotOptions : public setupDialog
{
	Q_OBJECT

public:
	KPilotOptions(QWidget* parent);

	static bool isNewer(KConfig&);

private:
	static int fConfigVersion;
};


#else
#ifdef DEBUG
#warning "File doubly included"
#endif
#endif


// $Log$
// Revision 1.15  2001/04/01 17:31:11  adridg
// --enable-final and #include fixes
//
// Revision 1.14  2001/03/09 09:46:15  adridg
// Large-scale #include cleanup
//
// Revision 1.13  2001/02/06 08:05:19  adridg
// Fixed copyright notices, added CVS log, added surrounding #ifdefs. No code changes.
//
