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
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 */

#ifndef KTNEFPARSER_H
#define	KTNEFPARSER_H

#include <qptrlist.h>
#include <qstring.h>
#include <qmap.h>
#include <kdepimmacros.h>

#include <kdemacros.h>

class KTNEFAttach;
class KTNEFMessage;
class KTNEFProperty;

class KDE_EXPORT KTNEFParser
{
public:
	KTNEFParser();
	~KTNEFParser();

	bool openFile(const QString& filename);
	bool openDevice( QIODevice *device );
	bool extractFile(const QString& filename);
	bool extractFileTo(const QString& filename, const QString& dirname);
	bool extractAll();
	void setDefaultExtractDir(const QString& dirname);
	KTNEFMessage* message() const;

private:
	bool decodeAttachment();
	bool decodeMessage();
	bool extractAttachmentTo(KTNEFAttach *att, const QString& dirname);
	bool parseDevice();
	void checkCurrent(int state);
	bool readMAPIProperties(QMap<int,KTNEFProperty*>& pros, KTNEFAttach *attach = 0);
	void deleteDevice();

private:
	class ParserPrivate;
	ParserPrivate *d;
};

#endif
