/*
    ktnefparser.h

    Copyright (C) 2002 Michael Goffioul <kdeprint@swing.be>

    This file is part of KTNEF, the KDE TNEF support library/program.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifndef KTNEFPARSER_H
#define	KTNEFPARSER_H

#include <tqptrlist.h>
#include <tqstring.h>
#include <tqmap.h>
#include <kdepimmacros.h>

class KTNEFAttach;
class KTNEFMessage;
class KTNEFProperty;

class KDE_EXPORT KTNEFParser
{
public:
	KTNEFParser();
	~KTNEFParser();

	bool openFile(const TQString& filename);
	bool openDevice( TQIODevice *device );
	bool extractFile(const TQString& filename);
	bool extractFileTo(const TQString& filename, const TQString& dirname);
	bool extractAll();
	void setDefaultExtractDir(const TQString& dirname);
	KTNEFMessage* message() const;

private:
	bool decodeAttachment();
	bool decodeMessage();
	bool extractAttachmentTo(KTNEFAttach *att, const TQString& dirname);
	bool parseDevice();
	void checkCurrent(int state);
	bool readMAPIProperties(TQMap<int,KTNEFProperty*>& pros, KTNEFAttach *attach = 0);
	void deleteDevice();

private:
	class ParserPrivate;
	ParserPrivate *d;
};

#endif
