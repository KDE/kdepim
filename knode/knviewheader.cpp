// -*- tab-width: 2 -*-

/***************************************************************************
                          knviewheader.cpp  -  description
                             -------------------

    copyright            : (C) 2000 by Christian Thurner
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

#include <ksimpleconfig.h>
#include <kstddirs.h>

#include "knviewheader.h"
#include "utilities.h"

QList<KNViewHeader> KNViewHeader::instances;

KNViewHeader::KNViewHeader()
{
	flags.fill(false, 8);	
}



KNViewHeader::~KNViewHeader()
{
}


void KNViewHeader::loadAll()
{
	QString fname(KGlobal::dirs()->findResource("appdata","headers.rc"));
	if (fname == QString::null)
		return;
  KSimpleConfig headerConf(fname,true);
	QStringList headers = headerConf.groupList();
	headers.remove("<default>");
	
	KNViewHeader *h;
	QValueList<int> flags;
	
  QStringList::Iterator it;
	for( it = headers.begin(); it != headers.end(); ++it ) {
		h=newItem();
		headerConf.setGroup((*it));
		h->n_ame = headerConf.readEntry("Name");
		h->h_eader = headerConf.readEntry("Header");
		flags = headerConf.readIntListEntry("Flags");
		if (h->n_ame.isNull()||h->h_eader.isNull()||(flags.count()!=8)) {
			qDebug("KNViewHeader::loadAll() : ignoring invalid/incomplete Header");	
			remove(h);
		} else {
			for (int i=0; i<8; i++)
				h->flags.setBit(i,(flags[i]>0));
			h->createTags();
		}
	}
}



void KNViewHeader::saveAll()
{
	QString dir(KGlobal::dirs()->saveLocation("appdata"));
	if (dir==QString::null) {
		displayInternalFileError();
		return;
	}
  KSimpleConfig headerConf(dir+"headers.rc");
 	QStringList oldHeaders = headerConf.groupList();
 	
 	QStringList::Iterator oldIt=oldHeaders.begin();
	for( ;oldIt != oldHeaders.end(); ++oldIt )      // remove all old groups
		headerConf.deleteGroup((*oldIt));             // find a better way to do it?
 	
	QValueList<int> flags;
	int idx=0;
	
	for( QListIterator<KNViewHeader> it(instances); it.current(); ++it ) {
		headerConf.setGroup(QString::number(idx++));
		headerConf.writeEntry("Name",(*it)->n_ame);
		headerConf.writeEntry("Header",(*it)->h_eader);
		flags.clear();
		for (int i=0; i<8; i++) {
			if ((*it)->flags[i])
				flags << 1;
			else
				flags << 0;
		}
		headerConf.writeEntry("Flags",flags);
	}	
}



KNViewHeader* KNViewHeader::newItem()
{
	KNViewHeader *h=new KNViewHeader();
	instances.append(h);
	// qDebug("KNViewHeader::newItem() : instance added"); too verbose
	return h;
}


void KNViewHeader::remove(KNViewHeader *h)
{
	instances.setAutoDelete(true);
	if (!instances.remove(h))
		qDebug("KNViewHeader::remove() : cannot find pointer in list !!");	
	// else	qDebug("KNViewHeader::remove() : instance removed");  too verbose
	
	instances.setAutoDelete(false);
}



void KNViewHeader::clear()
{
	instances.setAutoDelete(true);
	instances.clear();
	instances.setAutoDelete(false);
}



void KNViewHeader::up(KNViewHeader *h)
{
	int idx=instances.findRef(h);
	if(idx!=-1) {
		instances.take(idx);
		instances.insert(idx-1, h);
		qDebug("KNViewHeader::up() : item moved up");
	}
	else qDebug("KNViewHeader::up() : item not found in list");		
}



void KNViewHeader::down(KNViewHeader *h)
{
	int idx=instances.findRef(h);
	if(idx!=-1) {
		instances.take(idx);
		instances.insert(idx+1, h);
		qDebug("KNViewHeader::up() : item moved down");
	}
	else qDebug("KNViewHeader::up() : item not found in list");
}



void  KNViewHeader::createTags()
{
	const char *tokens[] = { 	"<font size=+1>","</font>","<b>","</b>",
														"<i>","</i>","<u>","</u>" };
	
	for(int i=0; i<4; i++) tags[i]=QString::null;
	
	if(flags.at(0)) {    // <font>
		tags[0]=tokens[0];
		tags[1]=tokens[1];
	}
	if(flags.at(4)) {
		tags[2]=tokens[0];
		tags[3]=tokens[1];
	}

	if(flags.at(1)) {     // <b>
		tags[0]+=tokens[2];
		tags[1]+=tokens[3];
	}
	if(flags.at(5)) {
		tags[2]+=tokens[2];
		tags[3]+=tokens[3];
	}	
	
	if(flags.at(2)) {     // <i>
		tags[0]+=tokens[4];
		tags[1]+=tokens[5];
	}
	if(flags.at(6)) {
		tags[2]+=tokens[4];
		tags[3]+=tokens[5];
	}
	
	if(flags.at(3)) {    // <u>
		tags[0]+=tokens[6];
		tags[1]+=tokens[7];
	}
	if(flags.at(7)) {
		tags[2]+=tokens[6];
		tags[3]+=tokens[7];
	}
}


