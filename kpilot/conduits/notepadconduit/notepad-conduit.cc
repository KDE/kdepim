/* KPilot
**
** Copyright (C) 2004 by Adriaan de Groot, Joern Ahrens
**
** The code for NotepadActionThread::unpackNotePad was taken from 
** Angus Ainslies read-notepad.c, which is part of pilot-link.
** NotepadActionThread::saveImage is also based on read-notepad.c.
**
** This file is part of the Notepad conduit, a conduit for KPilot that
** stores the notepad drawings to files.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#ifdef DEBUG
#undef DEBUG
#define DEBUG (1)
#else
#define DEBUG (1)
#endif

#define DEBUG (1)

#include "options.h"
#include "pilotUser.h"
#include "pilotSerialDatabase.h"

#include "notepad-conduit.h"  // The Conduit action
#include "notepadconduit.h"   // The settings class

#include <pi-notepad.h>

#include <qthread.h>
#include <qapplication.h>
#include <qvaluelist.h>
#include <qimage.h>
#include <qdir.h>

extern "C"
{
long version_conduit_notepad = KPILOT_PLUGIN_API;
const char *id_conduit_notepad =
	"$Id$";
}

NotepadConduit::NotepadConduit(KPilotDeviceLink *d,	const char *n,
	const QStringList &args) :	ConduitAction(d, n, args)
{
	FUNCTIONSETUP;
#ifdef DEBUG
	DEBUGCONDUIT << id_conduit_notepad << endl;
#endif
	fConduitName=i18n("Notepad");
	thread = 0L;

	(void) id_conduit_notepad;
}

NotepadConduit::~NotepadConduit()
{
	FUNCTIONSETUP;
}

/* virtual */ bool NotepadConduit::exec()
{
	FUNCTIONSETUP;
	
#ifdef DEBUG
	DEBUGCONDUIT << fname << ": In exec() @" << (unsigned long) this << endl;
#endif

	QDir dir(NotepadConduitSettings::outputDirectory());
	if(!dir.exists() && !dir.mkdir(dir.path())) {
		emit logError(i18n("Unable to open %1").arg(dir.path()));
		delayDone();
		return false;
	}
	else {
		thread = new NotepadActionThread(this, pilotSocket());
		thread->start();
		// tickle is disabled due to crashs during sync
		// -> PADP TX "unexpected package"
//		startTickle();
	}
	
	return true;
}

bool NotepadConduit::event(QEvent *e)
{
	FUNCTIONSETUP;

	if(e->type() == QEvent::User) {
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": Notepad thread done." << endl;
#endif
//		stopTickle();
		delayDone();
		if(thread->getFailed())
			logError(i18n("1 notepad could not be saved", "%n notepads could not be saved", thread->getFailed()));
		logMessage(i18n("1 notepad saved", "%n notepads saved", thread->getSaved()));
		delete thread;
		return true;
	}
	else 
		return ConduitAction::event(e);
}

//-----------------------------------------------------------------------------
// NotepadActionThread
//-----------------------------------------------------------------------------

NotepadActionThread::NotepadActionThread(QObject *parent, int pilotSocket) :
	fParent(parent), fPilotSocket(pilotSocket), notSaved(0), saved(0)
{
	FUNCTIONSETUP;
}

/*virtual*/ void NotepadActionThread::run()
{
	FUNCTIONSETUP;

	PilotSerialDatabase *db = new PilotSerialDatabase(fPilotSocket, "npadDB");

	int n = db->recordCount();
	QValueList<recordid_t> vl = db->idList();
	QValueList<recordid_t>::iterator it;
	struct NotePad a;
	for ( it = vl.begin(); it != vl.end(); ++it ) {
		PilotRecord *pr = db->readRecordById(*it);
		if(pr) {
			unpack_NotePad(&a, (unsigned char*)pr->getData(), pr->getLen());
			saveImage(&a);
		}
	}
	QApplication::postEvent(fParent, new QEvent(QEvent::User));
}

int NotepadActionThread::unpackNotePad(struct NotePad *a, unsigned char *buffer, int len)
{
	FUNCTIONSETUP;
	
	unsigned char *start = buffer;

	a->createDate.sec = (unsigned short int) get_short(buffer);
	buffer += 2;
	a->createDate.min = (unsigned short int) get_short(buffer);
	buffer += 2;
	a->createDate.hour = (unsigned short int) get_short(buffer);
	buffer += 2;
	a->createDate.day = (unsigned short int) get_short(buffer);
	buffer += 2;
	a->createDate.month = (unsigned short int) get_short(buffer);
	buffer += 2;
	a->createDate.year = (unsigned short int) get_short(buffer);
	buffer += 2;
	
	a->createDate.s = (unsigned short int) get_short(buffer);
	buffer += 2;
	
	a->changeDate.sec = (unsigned short int) get_short(buffer);
	buffer += 2;
	a->changeDate.min = (unsigned short int) get_short(buffer);
	buffer += 2;
	a->changeDate.hour = (unsigned short int) get_short(buffer);
	buffer += 2;
	a->changeDate.day = (unsigned short int) get_short(buffer);
	buffer += 2;
	a->changeDate.month = (unsigned short int) get_short(buffer);
	buffer += 2;
	a->changeDate.year = (unsigned short int) get_short(buffer);
	buffer += 2;
	
	a->changeDate.s = (unsigned short int) get_short(buffer);
	buffer += 2;
	
	a->flags = (unsigned short int) get_short(buffer);
	buffer += 2;
	
	if( a->flags & NOTEPAD_FLAG_ALARM ) {
			a->alarmDate.sec = (unsigned short int) get_short(buffer);
			buffer += 2;
			a->alarmDate.min = (unsigned short int) get_short(buffer);
			buffer += 2;
			a->alarmDate.hour = (unsigned short int) get_short(buffer);
			buffer += 2;
			a->alarmDate.day = (unsigned short int) get_short(buffer);
			buffer += 2;
			a->alarmDate.month = (unsigned short int) get_short(buffer);
			buffer += 2;
			a->alarmDate.year = (unsigned short int) get_short(buffer);
			buffer += 2;
		
			a->alarmDate.s = (unsigned short int) get_short(buffer);
			buffer += 2;
		}
		
		if( a->flags & NOTEPAD_FLAG_NAME ) {
			a->name = strdup((char *) buffer);
		
			buffer += strlen( a->name ) + 1;
				
			if( (strlen( a->name ) + 1)%2 == 1)
				++buffer;
				
		}
		else {
			a->name = NULL;
		}
		
	
	if( a->flags & NOTEPAD_FLAG_BODY ) {
			a->body.bodyLen = get_long( buffer );
			buffer += 4;
		
			a->body.width = get_long( buffer );
			buffer += 4;
		
			a->body.height = get_long( buffer );
			buffer += 4;
		
			a->body.l1 = get_long( buffer );
			buffer += 4;
		
			a->body.dataType = get_long( buffer );
			buffer += 4;
		
			a->body.dataLen = get_long( buffer );
			buffer += 4;
		
			a->data = (dataRec_t*)malloc( a->body.dataLen );
		
			if( a->data == NULL )
				return( 0 );
		
			memcpy( a->data, buffer, a->body.dataLen );
		}
	
	return ( buffer - start );	/* FIXME: return real length */
}

void NotepadActionThread::saveImage(struct NotePad *n)
{
	FUNCTIONSETUP;
	
	int i,j,k,datapoints = 0;
	QImage image(n->body.width+8, n->body.height, 8, 2);
	
	image.setColor(0, qRgb(0xaa, 0xc1 ,0x91) );
	image.setColor(1, qRgb(0x30, 0x36, 0x29) );

	int x = 0;	
	int y = 0;
	int pos = 0;
	for(i=0; i<n->body.dataLen/2; ++i)
	{
		datapoints += n->data[i].repeat;
		for(j=0; j<n->data[i].repeat; ++j)
		{
			for(k=0; k<8; ++k)
			{
				y = pos / 160;
				x = pos % 160;

				if(n->data[i].data & 1<<(7-k))
					image.setPixel(x,y,1);
				else
		    		image.setPixel(x,y,0);
				++pos;
			}
		}
	}
	QString imgname = QString("%1/%2.png").arg(NotepadConduitSettings::outputDirectory()).arg(n->name);
	DEBUGCONDUIT << fname << ": Notepad " << imgname << endl;
	if(!image.save(imgname, "PNG", -1))
		++notSaved;
	else
		++saved;
}

