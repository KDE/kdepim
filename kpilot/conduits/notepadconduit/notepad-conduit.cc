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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

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
#include <qcstring.h>

extern "C"
{
unsigned long version_conduit_notepad = Pilot::PLUGIN_API;
}

NotepadConduit::NotepadConduit(KPilotLink *d, const char *n,
	const QStringList &args) : ConduitAction(d, n, args)
{
	FUNCTIONSETUP;
	fConduitName=i18n("Notepad");
	thread = 0L;

}

NotepadConduit::~NotepadConduit()
{
	FUNCTIONSETUP;
}

/* virtual */ bool NotepadConduit::exec()
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGKPILOT << fname << ": In exec() @" << (unsigned long) this << endl;
#endif

	QDir dir(NotepadConduitSettings::outputDirectory());
	if(!dir.exists() && !dir.mkdir(dir.path())) {
		emit logError(i18n("Unable to open %1").arg(dir.path()));
		delayDone();
		return false;
	}
	else {
		thread = new NotepadActionThread(this, deviceLink());
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
		DEBUGKPILOT << fname << ": Notepad thread done." << endl;
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

NotepadActionThread::NotepadActionThread(QObject *parent, KPilotLink *link) :
	fParent(parent), fLink(link), notSaved(0), saved(0)
{
	FUNCTIONSETUP;
}

void NotepadActionThread::run()
{
	FUNCTIONSETUP;

	PilotDatabase *db = fLink->database( CSL1("npadDB") );

	int n = db->recordCount();

	if ( n > 0 )
	{
		QValueList<recordid_t> vl = db->idList();
		QValueList<recordid_t>::iterator it;
		struct NotePad a;
		for ( it = vl.begin(); it != vl.end(); ++it )
		{
			PilotRecord *pr = db->readRecordById(*it);
			if(pr)
			{
				unpack_NotePad(&a, (unsigned char*)pr->data(), pr->size());
				saveImage(&a);
				free_NotePad(&a);
			}
		}
	}
	KPILOT_DELETE(db);
	QApplication::postEvent(fParent, new QEvent(QEvent::User));
}

static void saveImageFromBITS(QImage &image, struct NotePad *n, unsigned int width)
{
	FUNCTIONSETUP;
	image.setColor(0, qRgb(0xaa, 0xc1 ,0x91));
	image.setColor(1, qRgb(0x30, 0x36, 0x29));

	int x = 0;
	int y = 0;
	int pos = 0;
	for(unsigned int i=0; i<n->body.dataLen/2; ++i)
	{
		for(int j=0; j<n->data[i].repeat; ++j)
		{
			for(int k=0; k<8; ++k)
			{
				y = pos / width;
				x = pos % width ;

				image.setPixel( x, y,
					(n->data[i].data & 1<<(7-k)) ? 1 : 0 );
				++pos;
			}
		}
	}
}

static void saveImageFromUNCOMPRESSED(QImage &image, struct NotePad *n, unsigned int width)
{
	FUNCTIONSETUP;

	image.setColor(0, qRgb(0xaa, 0xc1 ,0x91));
	image.setColor(1, qRgb(0x30, 0x36, 0x29));

	unsigned int pos = 0;
	unsigned int x,y;

	for (unsigned int i=0; i<n->body.dataLen / 2; ++i)
	{
		for (unsigned int k=0; k<8; ++k)
		{
			y = pos / width;
			x = pos % width ;

			image.setPixel( x, y,
				(n->data[i].repeat & 1<<(7-k)) ? 1 : 0 );
			++pos;
		}

		for (unsigned int k=0; k<8; ++k)
		{
			y = pos / width;
			x = pos % width ;

			image.setPixel( x, y,
				(n->data[i].data & 1<<(7-k)) ? 1 : 0 );
			++pos;
		}
	}
}

void NotepadActionThread::saveImage(struct NotePad *n)
{
	FUNCTIONSETUP;

	// Width needs adjusting, based on whether it's low res (+8)
	// or a hi-res notepad image.
	int width = n->body.width + ( n->body.width > 160 ? 16 : 8 );
	int height = n->body.height;


	QImage image(width, height, 8, 2);

	switch (n->body.dataType)
	{
	case NOTEPAD_DATA_BITS :
		saveImageFromBITS( image,n,width );
		break;
	case NOTEPAD_DATA_UNCOMPRESSED :
		saveImageFromUNCOMPRESSED( image,n,width );
		break;
	case NOTEPAD_DATA_PNG :
		image.loadFromData((uchar*)(n->data), n->body.dataLen);
		break;
	default :
		// Unknown data type
		WARNINGKPILOT << "Unknown data type: " << n->body.dataType << endl;
		return;

		// TODO: Post a warning to the UI
	}

	QString filename(n->name);
	if(filename.isEmpty())
	{
		filename.sprintf("%4d-%02d-%02d_%02d-%02d-%02d",
			n->changeDate.year,
			n->changeDate.month,
			n->changeDate.day,
			n->changeDate.hour,
			n->changeDate.min,
			n->changeDate.sec);
	}
	QString imgname = QString("%1/%2.png").arg(NotepadConduitSettings::outputDirectory()).arg(filename);


#ifdef DEBUG
	DEBUGKPILOT << fname << ": Notepad " << imgname << endl;
#endif
	if(!image.save(imgname, "PNG", -1))
		++notSaved;
	else
		++saved;
}

