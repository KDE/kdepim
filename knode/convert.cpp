/***************************************************************************
                          convert.cpp  -  description
                             -------------------
   
    copyright            : (C) 1999 by Christian Thurner
    email                : cthurner@freepage.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/

#include <stdlib.h>
#include <iostream>

#include <qfile.h>
#include <qdir.h>
#include <klocale.h>

#include "utilities.h"

enum Importance {ignored=0, normal=1, watched=2};

struct dynDataOld {
	uint id, idRef;
	Importance imp;
	bool read;
	short thrLevel;
};

struct dynDataNew {
	uint id, idRef;
	bool read;
	short thrLevel, score;
};

void convert011();

void convert(const char *ver)
{
	if(
			(strcmp(ver, "0.1.1")==0) ||
			(strcmp(ver, "0.1")==0)				) convert011();
			
	else if((strcmp(ver, "0.1.5")==0) ||
					(strcmp(ver, "0.1.6")==0)	||
					(strcmp(ver, "0.1.7")==0) ||
					(strcmp(ver, "0.1.8")==0) ||
					(strcmp(ver, "0.1.9")==0) ||
					(strcmp(ver, "0.1.10")==0) ||
					(strcmp(ver, "0.1.11")==0) ||
					(strcmp(ver, "0.1.12")==0)
					)
					
					cout << I18N_NOOP("Your files are up to date. There is no need to convert them.\n");
					
	else {
		cout << I18N_NOOP("Unknown version : ") << ver << endl;
		exit(1);
	}
}



void convert011()
{
	/*QList<dynDataNew> newList;
	newList.setAutoDelete(true);
	
	QFile f;
	QCString fName;
	QStrList *entries;
	dynDataOld oldDat;
	dynDataNew *newDat;
			
	QDir d=QDir::home();
	d.cd(".knode");
	entries=(QStrList*)d.entryList("*.grp", QDir::Dirs);
	
	if(!d.cd("Filters")) {
		d.mkdir("Filters");
	  QCString cmd="cp " + kapp->kde_datadir() + "/knode/filters/* ";
		cmd+=knDir()+"Filters/";
		
		system(cmd);
	}		
	
	for(char *var=entries->first(); var; var=entries->next()) {
		
	  newList.clear();
		fName=knDir();
		fName+=var;
		fName+="/dynamic";
		
		f.setName(fName);
		
		if(f.open(IO_ReadOnly)) {
		  	
			while(!f.atEnd()) {
		  	
				f.readBlock((char*) &oldDat, sizeof(dynDataOld));
		  		
				newDat=new dynDataNew;
		 		newDat->id=oldDat.id;
		 		newDat->idRef=oldDat.idRef;
		 		newDat->thrLevel=oldDat.thrLevel;
		 		newDat->read=oldDat.read;
		 		newDat->score=50;
		  		
		 		newList.append(newDat);
		 	}
		  	
		 	f.close();
		
		 	if (f.open(IO_WriteOnly)) {	  		
		 		for(dynDataNew *var=newList.first(); var; var=newList.next())
		 			f.writeBlock((char*) var, sizeof(dynDataNew));
		 	  f.close();
		 	  qDebug("%s : converted successfully !", fName.data());
		 	}
		  			
		 	else {
		 		qDebug("Could not open new file in %s !!", var);
		 	  exit(1);
		 	}
		}
		
		else {
			qDebug("Could not read file in %s !!", var);
		  exit(1);
		}
		  	
	}*/		
	cout << I18N_NOOP("Sorry, conversion does not work in this version :-(\n");	  		
}






