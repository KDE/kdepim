#ifndef _doc_CONDUIT_H
#define _doc_CONDUIT_H
// doc-conduit.cc
//
// Copyright (C) 2002 by Reinhold Kainhofer
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



#include <plugin.h>

#include <kapplication.h>


class PilotLocalDatabase;
class PilotSerialDatabase;
class PilotDatabase;
class docDBInfo;
#include <qdir.h> 

#include "doc-factory.h"


class DOCConduit:public ConduitAction {
Q_OBJECT
protected:
	enum eSyncDirectionEnum {
		eSyncAll,
		eSyncPDAToPC,
		eSyncPCToPDA,
		eSyncNone
	} eSyncDirection;

public:
	DOCConduit(KPilotDeviceLink * o,
		const char *n = 0L, const QStringList & a = QStringList());
	 virtual ~ DOCConduit();

	bool encode(QStringList fileName, PilotDatabase * db);
	bool decode(PilotDatabase * db, QString fileName);
	virtual bool exec();
protected:
	virtual bool isCorrectDBTypeCreator(DBInfo dbinfo) {
		return dbinfo.type == dbtype() && dbinfo.creator == dbcreator();
	};
	virtual const unsigned long dbtype() {
		return get_long(DOCConduitFactory::dbDOCtype);
	}
	virtual const unsigned long dbcreator() {
		return get_long(DOCConduitFactory::dbDOCcreator);
	}

	bool needsSync(DBInfo dbinfo, enum eSyncDirectionEnum & dir);
public slots:
	void syncNextDB();
	void syncNextDOC();
	void cleanup();

 private:
	 /**
    * Read the global KPilot config file for settings
    * particular to the docConduit conduit.
    */
	void readConfig();

	 /**
    * If necessary, copy the database from the palm to a local dir. 
    * Also initialize the docDBInfo that will be passed to the docconverter
    */
	PilotDatabase *preSyncAction(DBInfo &dbinfo, eSyncDirectionEnum direction) const;

	bool doSync(DBInfo&dbinfo, eSyncDirectionEnum dir);
	 /**
    * Clean up after the sync. The bool parameter res tells 
    * the function if the  conversion was successful or not
    */
	bool postSyncAction(PilotDatabase * dbinfo, eSyncDirectionEnum direction, bool res = true);

	bool textChanged(QString docfn);


	QString fDOCDir, fPDBDir;
	bool fKeepPDBLocally;
	enum eConflictResolutionEnum {
		eResNone,
		ePDAOverride,
		ePCOverride,
		ePCOverrideOnBookmarkChange,
		eResAsk
	} eConflictResolution;
	int fBookmarks;
	bool fCompress;
	QStringList dbnames;
	QString docfilename, pdbfilename;
	long int dbnr;

	QStringList docnames;
	QStringList::Iterator dociterator;
};




// $Log$
//
#endif
