/* *******************************************************
   KPilot - Hot-Sync Software for Unix.
   Copyright 1998 by Dan Pilone
   This code is released under the GNU PUBLIC LICENSE.
   Please see the file 'COPYING' included in the KPilot
   distribution.
   *******************************************************
 */

#ifndef __KPILOT_OPTIONS_H
#define __KPILOT_OPTIONS_H

#include <qtabdlg.h>
#include <qlined.h>
#include <qchkbox.h>
#include <qcombo.h>
#include <qbttngrp.h>
#include <qgrpbox.h>
#include <qradiobt.h>
#include <kconfig.h>
#include <kfm.h>

// This class implements the abstract behavior of a settings page,
// allow for construction, destruction, and the updating of the config
// file. Pages in KPilotOptions should inherit from this class so
// that they can be called properly.
//
//
class KPilotOptionsPage : public QWidget
{
	Q_OBJECT

public:
	KPilotOptionsPage(QWidget *parent,KConfig *);
	virtual ~KPilotOptionsPage();

public:
	virtual void commitChanges(KConfig *)=0;
	void cancelChanges();

	virtual const char *groupName() const;
} ;

class KPilotOptionsPrivacy : public KPilotOptionsPage
{
	Q_OBJECT

public:
	KPilotOptionsPrivacy(QWidget *,KConfig *);
	virtual ~KPilotOptionsPrivacy();

	virtual void commitChanges(KConfig *);

private:
	QCheckBox *fuseSecret;
} ;


class KPilotOptionsGeneral : public KPilotOptionsPage
{
	Q_OBJECT

public:
	// Parent, config as other pages; name is the untranslated
	// name of the page, used as the tab name.
	//
	//
	KPilotOptionsGeneral(QTabDialog *parent,KConfig*,char *name);
	virtual ~KPilotOptionsGeneral();

	virtual void commitChanges(KConfig *);

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

class KPilotOptionsAddress : public KPilotOptionsPage
{
	Q_OBJECT

public:
	KPilotOptionsAddress(QWidget *parent,KConfig*);
	virtual ~KPilotOptionsAddress();

	virtual void commitChanges(KConfig *);
	virtual const char *groupName() const;

public:
	static const char *theGroupName();
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



// The options dialog has several tabs for configuring *general*
// KPilot options -- ie. options that persist across all conduits
// or that control the behavior of the UI itself.
//
// Current pages:
//	* Hardware setup
//	* Address setup (really doesn't belong here)
//	* Privacy Settings
//
//
class KPilotOptions : public QTabDialog
{
	Q_OBJECT

public:
	KPilotOptions(QWidget* parent);
	~KPilotOptions();

public slots:
	void commitChanges();
	void cancelChanges();


private:
	void setupWidget();


	KPilotOptionsPage *generalPage;	// Device Settings
	KPilotOptionsPage *privacyPage;	// Show private/secret?
	KPilotOptionsPage *addressPage;	// Address Display format


};


#endif
