#ifndef CONFLICTDIALOG_H
#define CONFLICTDIALOG_H
// doc-conflictdialog.h
//
// Copyright (C) 2003 by Reinhold Kainhofer
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING. 
//
// $Revision$
//

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/


#include <kdialogbase.h>
#include "doc-conduit.h"


#define SCROLL_TABLE

#ifdef SCROLL_TABLE
class QComboTableItem;
class QTable;
#else
class QComboBox;
class QGridLayout;
class QGroupBox;
#endif

class QLabel;
class QPushButton;
class QTimer;
class KPilotDeviceLink;


typedef struct conflictEntry {
#ifdef SCROLL_TABLE
	QString dbname;
	QComboTableItem*resolution;
#else
	QLabel*dbname;
	QComboBox* resolution;
#endif
	QPushButton*info;
	int index;
	bool conflict;
};


class ResolutionDialog : public KDialogBase
{
	Q_OBJECT

public:
	ResolutionDialog( QWidget* parent=0, const QString& caption=i18n("Resolution Dialog"), syncInfoList*sinfo=0L, KPilotDeviceLink*lnk=0L);
	~ResolutionDialog();
	
	bool hasConflicts;
public slots:
	void _tickle();
protected:
	QTimer* tickleTimer;
	KPilotDeviceLink* fHandle;

protected:
	syncInfoList*syncInfo;
	
#ifdef SCROLL_TABLE
	QTable* resolutionTable;
#else
	QGroupBox* resolutionGroupBox;
	QGridLayout*resolutionGroupBoxLayout;
#endif

	QValueList<conflictEntry> conflictEntries;
	QLabel *textLabel1,*textLabel2;

protected slots:
	virtual void slotOk();
	void slotInfo(int index);

};

#endif // CONFLICTDIALOG_H
