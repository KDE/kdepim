#ifndef FORM1_H
#define FORM1_H

//#include <qvariant.h>
//#include <qdialog.h>
#include <kdialogbase.h>
#include "doc-conduit.h"

class QLabel;
class QComboBox;
class QPushButton;
class QGridLayout;
class QGroupBox;
class QTimer;

typedef struct conflictEntry {
	QLabel*dbname;
	QComboBox*resolution;
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
	QGridLayout* Form1Layout;
	QGridLayout* resolutionGroupBoxLayout;
	syncInfoList*syncInfo;
	
	QValueList<conflictEntry> conflictEntries;
	QGroupBox* resolutionGroupBox;
	QLabel* textLabel1;
	QLabel* textLabel2;

protected slots:
	virtual void slotOk();
	void slotInfo(int index);

};

#endif // FORM1_H
