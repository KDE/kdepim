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
class KFM;
class QRadioButton;
class QGroupBox;

class KPilotOptionsPrivacy : public setupDialogPage
{
	Q_OBJECT

public:
	KPilotOptionsPrivacy(setupDialog *,KConfig *);

	virtual int commitChanges(KConfig *);
	virtual const char *tabName();

private:
	QCheckBox *fuseSecret;
} ;


class KPilotOptionsGeneral : public setupDialogPage
{
	Q_OBJECT

public:
	KPilotOptionsGeneral(setupDialog *parent,KConfig*);
	virtual ~KPilotOptionsGeneral();

	virtual int commitChanges(KConfig *);
	virtual const char *tabName();

public slots:
	void slotSetupDaemon();
	void slotKFMFinished();

private:
	QLineEdit* fPilotDevice;
	QComboBox* fPilotSpeed;
	QLineEdit* fUserName;

	QCheckBox* fSyncFiles;

	QCheckBox* fStartDaemonAtLogin;
	QCheckBox* fStartKPilotAtHotSync;
	QCheckBox* fDockDaemon;

	QCheckBox* fOverwriteRemote;

	KFM* fKFM;
} ;

class KPilotOptionsAddress : public setupDialogPage
{
	Q_OBJECT

public:
	KPilotOptionsAddress(setupDialog *parent,KConfig*);

	virtual int commitChanges(KConfig *);
	virtual const char *tabName();

public:
	static const char *groupName();
	static int getDisplayMode(KConfig *c=NULL);

protected:
	void setRadio(int);
	int getRadio() const;

private:
	QButtonGroup *displayGroup;
	QRadioButton *fNormalDisplay,*fCompanyDisplay;

	// Address Prefs:
	QGroupBox *formatGroup;
	QLineEdit* fIncomingFormat;
	QLineEdit* fOutgoingFormat;
	QCheckBox* fUseKeyField;
} ;



class KPilotOptions : public setupDialog
{
	Q_OBJECT

public:
	KPilotOptions(QWidget* parent);

};


#endif
