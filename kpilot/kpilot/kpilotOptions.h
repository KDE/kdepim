// kpilotOptions.h
//
// Copyright (C) 1998,1999 Dan Pilone
// Copyright (C) 2000 Adriaan de Groot
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING.
//
// $Revision$


#ifndef __KPILOT_OPTIONS_H
#define __KPILOT_OPTIONS_H

#include "gsetupDialog.h"
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


#endif
